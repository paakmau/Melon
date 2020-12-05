#include <MelonTask/TaskManager.h>

namespace MelonTask {

TaskManager* TaskManager::instance() {
    static TaskManager sInstance;
    return &sInstance;
}

std::shared_ptr<TaskHandle> TaskManager::schedule(std::function<void()> const& procedure) {
    std::shared_ptr<TaskHandle> taskHandle = std::make_shared<TaskHandle>(procedure);
    _waitingTaskQueue.emplace(taskHandle);
    return taskHandle;
}

std::shared_ptr<TaskHandle> TaskManager::schedule(std::function<void()> const& procedure, std::vector<std::shared_ptr<TaskHandle>> const& predecessors) {
    std::shared_ptr<TaskHandle> taskHandle = std::make_shared<TaskHandle>(procedure);
    _waitingTaskAndPredecessorsQueue.emplace(std::make_pair(taskHandle, predecessors));
    return taskHandle;
}

std::shared_ptr<TaskHandle> TaskManager::schedule(std::function<void()> const& procedure, std::vector<std::shared_ptr<TaskHandle>>&& predecessors) {
    std::shared_ptr<TaskHandle> taskHandle = std::make_shared<TaskHandle>(procedure);
    _waitingTaskAndPredecessorsQueue.emplace(std::make_pair(taskHandle, std::move(predecessors)));
    return taskHandle;
}

std::shared_ptr<TaskHandle> TaskManager::combine(std::vector<std::shared_ptr<TaskHandle>> const& taskHandles) {
    return schedule(nullptr, taskHandles);
}

void TaskManager::activateWaitingTasks() {
    {
        std::lock_guard lock(_taskQueueMutex);
        while (!_waitingTaskQueue.empty()) {
            _taskQueue.emplace(_waitingTaskQueue.front());
            _waitingTaskQueue.pop();
        }
    }
    while (!_waitingTaskAndPredecessorsQueue.empty()) {
        std::shared_ptr<TaskHandle> const& taskHandle = _waitingTaskAndPredecessorsQueue.front().first;
        std::vector<std::shared_ptr<TaskHandle>> const& predecessors = _waitingTaskAndPredecessorsQueue.front().second;
        taskHandle->initPredecessors(predecessors);
        _waitingTaskAndPredecessorsQueue.pop();
    }
    _taskQueueConditionVariable.notify_all();
}

TaskManager::TaskManager() {
    for (unsigned int i = 0; i < kWorkerCount; i++)
        _workers.emplace_back(std::make_unique<TaskWorker>());
}

TaskManager::~TaskManager() {
    _stopped = true;
    for (std::unique_ptr<TaskWorker> const& worker : _workers)
        worker->notify_stopped();
    _taskQueueConditionVariable.notify_all();
    for (std::unique_ptr<TaskWorker> const& worker : _workers)
        worker->join();
}

void TaskManager::queueTask(std::shared_ptr<TaskHandle> const& task) {
    std::lock_guard lock(_taskQueueMutex);
    _taskQueue.push(task);
    _taskQueueConditionVariable.notify_one();
}

std::shared_ptr<TaskHandle> TaskManager::getNextTask() {
    std::unique_lock lock(_taskQueueMutex);
    // To avoid spurious wakeup
    while (_taskQueue.empty() && !_stopped) _taskQueueConditionVariable.wait(lock);
    if (_stopped) return nullptr;
    std::shared_ptr<TaskHandle> task = _taskQueue.front();
    _taskQueue.pop();
    return task;
}

}  // namespace MelonTask

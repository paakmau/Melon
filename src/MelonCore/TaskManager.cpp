#include <MelonCore/TaskHandle.h>
#include <MelonCore/TaskManager.h>
#include <MelonCore/TaskWorker.h>

namespace Melon {

TaskManager* TaskManager::instance() {
    static TaskManager sInstance;
    return &sInstance;
}

std::shared_ptr<TaskHandle> TaskManager::schedule(const std::function<void()>& procedure) {
    std::shared_ptr<TaskHandle> taskHandle = std::make_shared<TaskHandle>(procedure);
    _waitingTaskQueue.emplace(taskHandle);
    return taskHandle;
}

std::shared_ptr<TaskHandle> TaskManager::schedule(const std::function<void()>& procedure, const std::vector<std::shared_ptr<TaskHandle>>& predecessors) {
    std::shared_ptr<TaskHandle> taskHandle = std::make_shared<TaskHandle>(procedure);
    _waitingTaskAndPredecessorsQueue.emplace(std::make_pair(taskHandle, predecessors));
    return taskHandle;
}

std::shared_ptr<TaskHandle> TaskManager::schedule(const std::function<void()>& procedure, std::vector<std::shared_ptr<TaskHandle>>&& predecessors) {
    std::shared_ptr<TaskHandle> taskHandle = std::make_shared<TaskHandle>(procedure);
    _waitingTaskAndPredecessorsQueue.emplace(std::make_pair(taskHandle, std::move(predecessors)));
    return taskHandle;
}

std::shared_ptr<TaskHandle> TaskManager::combine(const std::vector<std::shared_ptr<TaskHandle>>& taskHandles) {
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
        const std::shared_ptr<TaskHandle>& taskHandle = _waitingTaskAndPredecessorsQueue.front().first;
        const std::vector<std::shared_ptr<TaskHandle>>& predecessors = _waitingTaskAndPredecessorsQueue.front().second;
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
    _stop = true;
    for (const std::unique_ptr<TaskWorker>& worker : _workers)
        worker->stop();
    _taskQueueConditionVariable.notify_all();
    for (const std::unique_ptr<TaskWorker>& worker : _workers)
        worker->join();
}

void TaskManager::queueTask(const std::shared_ptr<TaskHandle>& task) {
    std::lock_guard lock(_taskQueueMutex);
    _taskQueue.push(task);
    _taskQueueConditionVariable.notify_one();
}

std::shared_ptr<TaskHandle> TaskManager::getNextTask() {
    std::unique_lock lock(_taskQueueMutex);
    // To avoid spurious wakeup
    while (_taskQueue.empty() && !_stop) _taskQueueConditionVariable.wait(lock);
    if (_stop) return nullptr;
    std::shared_ptr<TaskHandle> task = _taskQueue.front();
    _taskQueue.pop();
    return task;
}

}  // namespace Melon

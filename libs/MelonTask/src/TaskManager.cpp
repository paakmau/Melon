#include <MelonTask/TaskManager.h>

namespace MelonTask {

TaskManager* TaskManager::instance() {
    static TaskManager sInstance;
    return &sInstance;
}

std::shared_ptr<TaskHandle> TaskManager::schedule(std::function<void()> const& procedure) {
    std::shared_ptr<TaskHandle> taskHandle = std::make_shared<TaskHandle>(procedure);
    m_WaitingTaskQueue.emplace(taskHandle);
    return taskHandle;
}

std::shared_ptr<TaskHandle> TaskManager::schedule(std::function<void()> const& procedure, std::vector<std::shared_ptr<TaskHandle>> const& predecessors) {
    std::shared_ptr<TaskHandle> taskHandle = std::make_shared<TaskHandle>(procedure);
    m_WaitingTaskAndPredecessorsQueue.emplace(std::make_pair(taskHandle, predecessors));
    return taskHandle;
}

std::shared_ptr<TaskHandle> TaskManager::schedule(std::function<void()> const& procedure, std::vector<std::shared_ptr<TaskHandle>>&& predecessors) {
    std::shared_ptr<TaskHandle> taskHandle = std::make_shared<TaskHandle>(procedure);
    m_WaitingTaskAndPredecessorsQueue.emplace(std::make_pair(taskHandle, std::move(predecessors)));
    return taskHandle;
}

std::shared_ptr<TaskHandle> TaskManager::combine(std::vector<std::shared_ptr<TaskHandle>> const& taskHandles) {
    return schedule(nullptr, taskHandles);
}

void TaskManager::activateWaitingTasks() {
    {
        std::lock_guard lock(m_TaskQueueMutex);
        while (!m_WaitingTaskQueue.empty()) {
            m_TaskQueue.emplace(m_WaitingTaskQueue.front());
            m_WaitingTaskQueue.pop();
        }
    }
    while (!m_WaitingTaskAndPredecessorsQueue.empty()) {
        std::shared_ptr<TaskHandle> const& taskHandle = m_WaitingTaskAndPredecessorsQueue.front().first;
        std::vector<std::shared_ptr<TaskHandle>> const& predecessors = m_WaitingTaskAndPredecessorsQueue.front().second;
        taskHandle->initPredecessors(predecessors);
        m_WaitingTaskAndPredecessorsQueue.pop();
    }
    m_TaskQueueConditionVariable.notify_all();
}

TaskManager::TaskManager() {
    for (unsigned int i = 0; i < k_WorkerCount; i++)
        m_Workers.emplace_back(std::make_unique<TaskWorker>());
}

TaskManager::~TaskManager() {
    m_Stopped = true;
    for (std::unique_ptr<TaskWorker> const& worker : m_Workers)
        worker->notify_stopped();
    m_TaskQueueConditionVariable.notify_all();
    for (std::unique_ptr<TaskWorker> const& worker : m_Workers)
        worker->join();
}

void TaskManager::queueTask(std::shared_ptr<TaskHandle> const& task) {
    std::lock_guard lock(m_TaskQueueMutex);
    m_TaskQueue.push(task);
    m_TaskQueueConditionVariable.notify_one();
}

std::shared_ptr<TaskHandle> TaskManager::getNextTask() {
    std::unique_lock lock(m_TaskQueueMutex);
    // To avoid spurious wakeup
    while (m_TaskQueue.empty() && !m_Stopped) m_TaskQueueConditionVariable.wait(lock);
    if (m_Stopped) return nullptr;
    std::shared_ptr<TaskHandle> task = m_TaskQueue.front();
    m_TaskQueue.pop();
    return task;
}

}  // namespace MelonTask

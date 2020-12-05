#pragma once

#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskWorker.h>

#include <memory>
#include <mutex>
#include <queue>
#include <vector>

namespace MelonTask {

class TaskHandle;
class TaskWorker;

class TaskManager {
  public:
    static constexpr unsigned int k_WorkerCount = 8;

    std::shared_ptr<TaskHandle> schedule(std::function<void()> const& procedure);
    std::shared_ptr<TaskHandle> schedule(std::function<void()> const& procedure, std::vector<std::shared_ptr<TaskHandle>> const& predecessors);
    std::shared_ptr<TaskHandle> schedule(std::function<void()> const& procedure, std::vector<std::shared_ptr<TaskHandle>>&& predecessors);
    std::shared_ptr<TaskHandle> combine(std::vector<std::shared_ptr<TaskHandle>> const& taskHandles);
    // 规划的任务不会执行而是存放在等待队列
    // 调用该函数把等待队列中的任务加入执行队列
    void activateWaitingTasks();

    static TaskManager* instance();

  private:
    TaskManager();
    ~TaskManager();

    void queueTask(std::shared_ptr<TaskHandle> const& taskHandle);
    std::shared_ptr<TaskHandle> getNextTask();

    bool m_Stopped{};
    std::queue<std::shared_ptr<TaskHandle>> m_WaitingTaskQueue;
    std::queue<std::pair<std::shared_ptr<TaskHandle>, std::vector<std::shared_ptr<TaskHandle>>>> m_WaitingTaskAndPredecessorsQueue;
    std::queue<std::shared_ptr<TaskHandle>> m_TaskQueue;
    std::mutex m_TaskQueueMutex;
    std::condition_variable m_TaskQueueConditionVariable;
    std::vector<std::unique_ptr<TaskWorker>> m_Workers;

    friend class TaskHandle;
    friend class TaskWorker;
};

}  // namespace MelonTask

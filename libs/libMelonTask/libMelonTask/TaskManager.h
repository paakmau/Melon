#pragma once

#include <libMelonTask/TaskHandle.h>
#include <libMelonTask/TaskWorker.h>

#include <array>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

namespace MelonTask {

class TaskHandle;
class TaskWorker;

class TaskManager {
  public:
    TaskManager();
    ~TaskManager();

    static constexpr unsigned int k_WorkerCount = 8;

    std::shared_ptr<TaskHandle> schedule(std::function<void()> const& procedure);
    std::shared_ptr<TaskHandle> schedule(std::function<void()> const& procedure, std::vector<std::shared_ptr<TaskHandle>> const& predecessors);
    std::shared_ptr<TaskHandle> schedule(std::function<void()> const& procedure, std::vector<std::shared_ptr<TaskHandle>>&& predecessors);
    std::shared_ptr<TaskHandle> combine(std::vector<std::shared_ptr<TaskHandle>> const& taskHandles);
    // Scheduled tasks won't be able to executed at once, because they are put in a waiting queue
    // Calling this function will activate tasks in the waiting queue
    void activateWaitingTasks();

  private:
    void queueTask(std::shared_ptr<TaskHandle> const& taskHandle);
    std::shared_ptr<TaskHandle> getNextTask();

    bool m_Stopped{};
    std::queue<std::shared_ptr<TaskHandle>> m_WaitingTaskQueue;
    std::queue<std::pair<std::shared_ptr<TaskHandle>, std::vector<std::shared_ptr<TaskHandle>>>> m_WaitingTaskAndPredecessorsQueue;
    std::queue<std::shared_ptr<TaskHandle>> m_TaskQueue;
    std::mutex m_TaskQueueMutex;
    std::condition_variable m_TaskQueueConditionVariable;
    std::array<std::unique_ptr<TaskWorker>, k_WorkerCount> m_Workers;

    friend class TaskHandle;
    friend class TaskWorker;
};

}  // namespace MelonTask

#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <vector>

namespace Melon {

class TaskHandle;
class TaskWorker;

class TaskManager {
   public:
    static constexpr unsigned int kWorkerCount = 8;

    std::shared_ptr<TaskHandle> schedule(const std::function<void()>& procedure);
    std::shared_ptr<TaskHandle> schedule(const std::function<void()>& procedure, const std::vector<std::shared_ptr<TaskHandle>>& predecessors);
    std::shared_ptr<TaskHandle> schedule(const std::function<void()>& procedure, std::vector<std::shared_ptr<TaskHandle>>&& predecessors);
    std::shared_ptr<TaskHandle> combine(const std::vector<std::shared_ptr<TaskHandle>>& taskHandles);
    // 规划的任务不会执行而是存放在等待队列
    // 调用该函数把等待队列中的任务加入执行队列
    void activateWaitingTasks();

    static TaskManager* instance();

   private:
    TaskManager();
    ~TaskManager();

    void queueTask(const std::shared_ptr<TaskHandle>& taskHandle);
    std::shared_ptr<TaskHandle> getNextTask();

    bool _stop{};
    std::queue<std::shared_ptr<TaskHandle>> _waitingTaskQueue;
    std::queue<std::pair<std::shared_ptr<TaskHandle>, std::vector<std::shared_ptr<TaskHandle>>>> _waitingTaskAndPredecessorsQueue;
    std::queue<std::shared_ptr<TaskHandle>> _taskQueue;
    std::mutex _taskQueueMutex;
    std::condition_variable _taskQueueConditionVariable;
    std::vector<std::unique_ptr<TaskWorker>> _workers;

    friend class TaskHandle;
    friend class TaskWorker;
};

}  // namespace Melon

#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

namespace Melon {

class TaskManager;

class TaskHandle : public std::enable_shared_from_this<TaskHandle> {
  public:
    TaskHandle(TaskManager* taskManager, std::function<void()> const& procedure) : m_TaskManager(taskManager), m_Procedure(procedure), m_FinishSharedFuture(m_FinishPromise.get_future()) {}
    void complete();
    bool finished();

  private:
    void initPredecessors(std::vector<std::shared_ptr<TaskHandle>> const& predecessors);
    bool appendSuccessor(std::shared_ptr<TaskHandle> successor);
    void execute();
    void notifyFinished();
    void notifyPredecessorFinished();

    TaskManager* const m_TaskManager;
    std::function<void()> m_Procedure;
    std::atomic<unsigned int> m_PredecessorCount;
    std::vector<std::shared_ptr<TaskHandle>> m_Successors;
    std::mutex m_Mtx;
    std::condition_variable m_Cv;
    bool m_Finished{};
    std::mutex m_FinishedMutex;
    std::promise<void> m_FinishPromise;
    std::shared_future<void> m_FinishSharedFuture;

    friend class TaskManager;
    friend class TaskWorker;
};

}  // namespace Melon

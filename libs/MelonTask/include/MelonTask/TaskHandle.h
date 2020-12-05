#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

namespace MelonTask {

class TaskHandle : public std::enable_shared_from_this<TaskHandle> {
  public:
    TaskHandle(std::function<void()> const& procedure);
    void complete();
    bool finished();

  private:
    void initPredecessors(std::vector<std::shared_ptr<TaskHandle>> const& predecessors);
    bool appendSuccessor(std::shared_ptr<TaskHandle> successor);
    void execute();
    void notifyFinished();
    void notifyPredecessorFinished();

    std::function<void()> _procedure;
    std::atomic<unsigned int> _predecessorCount;
    std::vector<std::shared_ptr<TaskHandle>> _successors;
    std::mutex _mtx;
    std::condition_variable _cv;
    bool _finished{};
    std::mutex _finishedMutex;
    std::promise<void> _finishPromise;
    std::shared_future<void> _finishSharedFuture;

    friend class TaskManager;
    friend class TaskWorker;
};

}  // namespace MelonTask

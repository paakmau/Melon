#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

namespace MelonTask {

void TaskHandle::complete() {
    _finishSharedFuture.get();
}

bool TaskHandle::finished() {
    return _finished;
}

TaskHandle::TaskHandle(const std::function<void()>& procedure) : _procedure(procedure), _finishSharedFuture(_finishPromise.get_future()) {}

void TaskHandle::initPredecessors(const std::vector<std::shared_ptr<TaskHandle>>& predecessors) {
    // 用于避免前驱任务完成而导致入队
    _predecessorCount = predecessors.size() + 1;
    for (std::shared_ptr<TaskHandle> predecessor : predecessors)
        if (!predecessor || !predecessor->appendSuccessor(shared_from_this()))
            _predecessorCount--;
    notifyPredecessorFinished();
}

bool TaskHandle::appendSuccessor(std::shared_ptr<TaskHandle> successor) {
    std::lock_guard lock(_finishedMutex);
    if (!_finished) {
        _successors.emplace_back(successor);
        return true;
    }
    return false;
}

void TaskHandle::execute() {
    if (_procedure)
        _procedure();
}

void TaskHandle::notifyFinished() {
    std::lock_guard lock(_finishedMutex);
    _finished = true;
    _finishPromise.set_value();
    for (auto& successors : _successors)
        successors->notifyPredecessorFinished();
}

void TaskHandle::notifyPredecessorFinished() {
    if (--_predecessorCount == 0)
        TaskManager::instance()->queueTask(shared_from_this());
}

}  // namespace MelonTask

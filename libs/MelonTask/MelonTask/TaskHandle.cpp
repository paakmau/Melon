#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

namespace MelonTask {

void TaskHandle::complete() {
    m_FinishSharedFuture.get();
}

bool TaskHandle::finished() {
    return m_Finished;
}

void TaskHandle::initPredecessors(std::vector<std::shared_ptr<TaskHandle>> const& predecessors) {
    // To avoid unexpected enqueueing
    m_PredecessorCount = predecessors.size() + 1;
    for (std::shared_ptr<TaskHandle> predecessor : predecessors)
        if (!predecessor || !predecessor->appendSuccessor(shared_from_this()))
            m_PredecessorCount--;
    notifyPredecessorFinished();
}

bool TaskHandle::appendSuccessor(std::shared_ptr<TaskHandle> successor) {
    std::lock_guard lock(m_FinishedMutex);
    if (!m_Finished) {
        m_Successors.emplace_back(successor);
        return true;
    }
    return false;
}

void TaskHandle::execute() {
    if (m_Procedure)
        m_Procedure();
}

void TaskHandle::notifyFinished() {
    std::lock_guard lock(m_FinishedMutex);
    m_Finished = true;
    m_FinishPromise.set_value();
    for (auto& successors : m_Successors)
        successors->notifyPredecessorFinished();
}

void TaskHandle::notifyPredecessorFinished() {
    if (--m_PredecessorCount == 0)
        m_TaskManager->queueTask(shared_from_this());
}

}  // namespace MelonTask

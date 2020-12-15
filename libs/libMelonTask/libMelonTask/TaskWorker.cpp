#include <libMelonTask/TaskHandle.h>
#include <libMelonTask/TaskManager.h>
#include <libMelonTask/TaskWorker.h>

#include <cstdio>
#include <functional>
#include <memory>

namespace MelonTask {

TaskWorker::TaskWorker(TaskManager* taskManager) : m_TaskManager(taskManager) {
    m_Thread = std::thread(&TaskWorker::threadEntryPoint, this);
}

void TaskWorker::threadEntryPoint() {
    while (!m_Stopped) {
        std::shared_ptr<TaskHandle> task = m_TaskManager->getNextTask();
        if (task) {
            task->execute();
            task->notifyFinished();
        }
    }
}

void TaskWorker::notify_stopped() {
    m_Stopped = true;
}

void TaskWorker::join() {
    m_Thread.join();
}

}  // namespace MelonTask

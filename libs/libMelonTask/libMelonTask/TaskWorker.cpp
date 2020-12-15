#include <libMelonTask/TaskHandle.h>
#include <libMelonTask/TaskManager.h>
#include <libMelonTask/TaskWorker.h>

#include <cstdio>
#include <functional>
#include <memory>

namespace MelonTask {

TaskWorker::TaskWorker() {
    m_Thread = std::thread(&TaskWorker::threadEntryPoint, this);
}

void TaskWorker::threadEntryPoint() {
    while (!m_Stopped) {
        std::shared_ptr<TaskHandle> task = TaskManager::instance()->getNextTask();
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

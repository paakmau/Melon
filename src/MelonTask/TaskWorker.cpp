#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>
#include <MelonTask/TaskWorker.h>

#include <cstdio>
#include <functional>
#include <memory>

namespace MelonTask {

TaskWorker::TaskWorker() {
    _thread = std::thread(&TaskWorker::threadEntryPoint, this);
}

void TaskWorker::threadEntryPoint() {
    while (!_stopped) {
        std::shared_ptr<TaskHandle> task = TaskManager::instance()->getNextTask();
        if (task) {
            task->execute();
            task->notifyFinished();
        }
    }
}

void TaskWorker::notify_stopped() {
    _stopped = true;
}

void TaskWorker::join() {
    _thread.join();
}

}  // namespace MelonTask

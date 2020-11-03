#pragma once

#include <thread>

namespace MelonCore {

class TaskWorker {
   public:
    TaskWorker();
    void threadEntryPoint();
    void notify_stopped();
    void join();

   private:
    std::thread _thread;
    bool _stopped{};
};

}  // namespace MelonCore

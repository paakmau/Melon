#pragma once

#include <thread>

namespace Melon {

class TaskWorker {
   public:
    TaskWorker();
    void threadEntryPoint();
    void stop();
    void join();

   private:
    std::thread _thread;
    bool _stop{};
};

}  // namespace Melon

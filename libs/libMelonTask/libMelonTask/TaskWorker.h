#pragma once

#include <thread>

namespace MelonTask {

class TaskWorker {
  public:
    TaskWorker();
    void threadEntryPoint();
    void notify_stopped();
    void join();

  private:
    std::thread m_Thread;
    bool m_Stopped{};
};

}  // namespace MelonTask

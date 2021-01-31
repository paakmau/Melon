#pragma once

#include <thread>

namespace Melon {

class TaskManager;

class TaskWorker {
  public:
    TaskWorker(TaskManager* taskManager);
    void threadEntryPoint();
    void notify_stopped();
    void join();

  private:
    TaskManager* const m_TaskManager;
    std::thread m_Thread;
    bool m_Stopped{};
};

}  // namespace Melon

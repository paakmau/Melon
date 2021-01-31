#pragma once

#include <MelonCore/ResourceManager.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonTask/TaskManager.h>

#include <functional>
#include <memory>
#include <vector>

namespace Melon {

class TaskHandle;
class Instance;
class EntityManager;

class World {
  public:
    World(MelonTask::TaskManager* taskManager);

    template <typename Type, typename... Args>
    void registerSystem(Args&&... args);

    void enter(Instance* instance, Time* time, ResourceManager* resourceManager);
    void update();
    void exit();

  private:
    MelonTask::TaskManager* const m_TaskManager;
    EntityManager m_EntityManager;
    std::vector<std::unique_ptr<SystemBase>> m_Systems;
    std::function<void()> m_EntityCommandBufferExecutor;
};

template <typename Type, typename... Args>
void World::registerSystem(Args&&... args) {
    m_Systems.emplace_back(std::make_unique<Type>(std::forward<Args>(args)...));
}

}  // namespace Melon

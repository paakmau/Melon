#pragma once

#include <libMelonCore/SystemBase.h>

#include <functional>
#include <memory>
#include <vector>

namespace MelonCore {

class TaskHandle;
class EntityManager;

class World {
  public:
    World(EntityManager* entityManager);

    template <typename Type, typename... Args>
    void registerSystem(Args&&... args);

    void enter();
    void update();
    void exit();

  private:
    EntityManager* const m_EntityManager;
    std::vector<std::unique_ptr<SystemBase>> m_Systems;
    std::function<void()> m_EntityCommandBufferExecutor;
};

template <typename Type, typename... Args>
void World::registerSystem(Args&&... args) {
    m_Systems.emplace_back(std::make_unique<Type>(std::forward<Args>(args)...));
}

}  // namespace MelonCore

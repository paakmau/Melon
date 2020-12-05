#pragma once

#include <MelonCore/SystemBase.h>

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
    EntityManager* const _entityManager;
    std::vector<std::unique_ptr<SystemBase>> _systems;
    std::function<void()> _entityCommandBufferExecutor;
};

template <typename Type, typename... Args>
void World::registerSystem(Args&&... args) {
    _systems.emplace_back(std::make_unique<Type>(std::forward<Args>(args)...));
}

}  // namespace MelonCore

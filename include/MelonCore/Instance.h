#pragma once

#include <MelonCore/World.h>
#include <MelonCore/EntityManager.h>

#include <memory>

namespace MelonCore {

class EntityManager;
class World;

class Instance {
   public:
    Instance();

    // EntityManager* entityManager() const { return _entityManager.get(); }
    // World* systemManager() const { return _defaultWorld.get(); }

    template <typename T, typename... Args>
    void registerSystem(Args&&... args);

    void startGame();

   private:
    void mainLoop();

    std::unique_ptr<EntityManager> _entityManager;
    std::unique_ptr<World> _defaultWorld;
};

template <typename T, typename... Args>
void Instance::registerSystem(Args&&... args) {
    _defaultWorld->registerSystem<T>(std::forward<Args>(args)...);
}

}  // namespace MelonCore

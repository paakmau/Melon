#pragma once

#include <MelonCore/EntityManager.h>
#include <MelonCore/World.h>

#include <memory>

namespace MelonCore {

class EntityManager;
class World;

class Instance {
  public:
    char const* const& applicationName() const { return _applicationName; }
    char const*& applicationName() { return _applicationName; }

    template <typename Type, typename... Args>
    void registerSystem(Args&&... args);

    void start();
    void quit();

    static Instance* instance();

  private:
    Instance();

    void mainLoop();

    char const* _applicationName{};

    std::unique_ptr<EntityManager> _entityManager;
    std::unique_ptr<World> _defaultWorld;

    bool _shouldQuit{};
};

template <typename Type, typename... Args>
void Instance::registerSystem(Args&&... args) {
    _defaultWorld->registerSystem<Type>(std::forward<Args>(args)...);
}

}  // namespace MelonCore

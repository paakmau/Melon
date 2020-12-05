#pragma once

#include <MelonCore/EntityManager.h>
#include <MelonCore/World.h>

#include <memory>

namespace MelonCore {

class EntityManager;
class World;

class Instance {
  public:
    const char* const& applicationName() const { return _applicationName; }
    const char*& applicationName() { return _applicationName; }

    template <typename T, typename... Args>
    void registerSystem(Args&&... args);

    void start();
    void quit();

    static Instance* instance();

  private:
    Instance();

    void mainLoop();

    const char* _applicationName{};

    std::unique_ptr<EntityManager> _entityManager;
    std::unique_ptr<World> _defaultWorld;

    bool _shouldQuit{};
};

template <typename T, typename... Args>
void Instance::registerSystem(Args&&... args) {
    _defaultWorld->registerSystem<T>(std::forward<Args>(args)...);
}

}  // namespace MelonCore

#include <MelonCore/EntityManager.h>
#include <MelonCore/Instance.h>
#include <MelonCore/Time.h>
#include <MelonCore/World.h>

#include <memory>

namespace MelonCore {

void Instance::start() {
    _defaultWorld->enter();
    mainLoop();
    _defaultWorld->exit();
}

void Instance::quit() {
    _shouldQuit = true;
}

void Instance::mainLoop() {
    Time::instance()->initialize();
    while (!_shouldQuit) {
        Time::instance()->update();
        _defaultWorld->update();
    }
}

Instance* Instance::instance() {
    static Instance sInstance;
    return &sInstance;
}

Instance::Instance() {
    _entityManager = std::make_unique<EntityManager>();
    _defaultWorld = std::make_unique<World>(_entityManager.get());
}

}  // namespace MelonCore

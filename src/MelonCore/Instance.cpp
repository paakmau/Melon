#include <MelonCore/EntityManager.h>
#include <MelonCore/Instance.h>
#include <MelonCore/World.h>
#include <MelonCore/Time.h>

#include <memory>

namespace MelonCore {

Instance::Instance() {
    _entityManager = std::make_unique<EntityManager>();
    _defaultWorld = std::make_unique<World>(_entityManager.get());
}

void Instance::startGame() {
    _defaultWorld->enter();
    mainLoop();
    _defaultWorld->exit();
}

void Instance::mainLoop() {
    Time::instance()->init();
    while (true) {
        Time::instance()->update();
        _defaultWorld->update();
    }
}

}  // namespace MelonCore

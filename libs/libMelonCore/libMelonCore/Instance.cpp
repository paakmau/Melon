#include <libMelonCore/EntityManager.h>
#include <libMelonCore/Instance.h>
#include <libMelonCore/Time.h>
#include <libMelonCore/World.h>

#include <memory>

namespace MelonCore {

void Instance::start() {
    m_DefaultWorld->enter();
    mainLoop();
    m_DefaultWorld->exit();
}

void Instance::quit() {
    m_ShouldQuit = true;
}

void Instance::mainLoop() {
    Time::instance()->initialize();
    while (!m_ShouldQuit) {
        Time::instance()->update();
        m_DefaultWorld->update();
    }
}

Instance* Instance::instance() {
    static Instance sInstance;
    return &sInstance;
}

Instance::Instance() {
    m_EntityManager = std::make_unique<EntityManager>();
    m_DefaultWorld = std::make_unique<World>(m_EntityManager.get());
}

}  // namespace MelonCore

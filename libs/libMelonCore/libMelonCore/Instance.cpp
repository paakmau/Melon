#include <libMelonCore/Instance.h>

#include <memory>

namespace MelonCore {

Instance::Instance() {
    m_DefaultWorld = std::make_unique<World>(&m_TaskManager);
}

void Instance::start() {
    m_DefaultWorld->enter(this, &m_Time);
    mainLoop();
    m_DefaultWorld->exit();
}

void Instance::quit() {
    m_ShouldQuit = true;
}

void Instance::mainLoop() {
    m_Time.initialize();
    while (!m_ShouldQuit) {
        m_Time.update();
        m_DefaultWorld->update();
    }
}

}  // namespace MelonCore

#include <libMelonCore/EntityManager.h>
#include <libMelonCore/SystemBase.h>
#include <libMelonCore/World.h>
#include <libMelonTask/TaskManager.h>

namespace MelonCore {

World::World(EntityManager* entityManager) : m_EntityManager(entityManager) {
    m_EntityCommandBufferExecutor = [entityManager]() {
        entityManager->executeEntityCommandBuffers();
    };
}

void World::enter() {
    for (std::unique_ptr<SystemBase> const& system : m_Systems)
        system->enter(m_EntityManager);
}

void World::update() {
    // Schedule entity command buffer executor
    std::vector<std::shared_ptr<MelonTask::TaskHandle>> predecessors(m_Systems.size());
    for (unsigned int i = 0; i < m_Systems.size(); i++)
        predecessors[i] = m_Systems[i]->predecessor();
    std::shared_ptr<MelonTask::TaskHandle> taskHandle = MelonTask::TaskManager::instance()->schedule(m_EntityCommandBufferExecutor, predecessors);
    MelonTask::TaskManager::instance()->activateWaitingTasks();
    for (std::unique_ptr<SystemBase> const& system : m_Systems)
        system->predecessor() = taskHandle;
    // Update systems
    for (std::unique_ptr<SystemBase> const& system : m_Systems)
        system->update();
}

void World::exit() {
    for (std::unique_ptr<SystemBase> const& system : m_Systems)
        system->exit();
}

}  // namespace MelonCore

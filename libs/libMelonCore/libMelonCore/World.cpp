#include <libMelonCore/EntityManager.h>
#include <libMelonCore/SystemBase.h>
#include <libMelonCore/World.h>
#include <libMelonTask/TaskManager.h>

namespace MelonCore {

World::World(MelonTask::TaskManager* taskManager) : m_TaskManager(taskManager) {
    EntityManager* entityManager = &m_EntityManager;
    m_EntityCommandBufferExecutor = [entityManager]() {
        entityManager->executeEntityCommandBuffers();
    };
}

void World::enter(Instance* instance, Time* time) {
    for (std::unique_ptr<SystemBase> const& system : m_Systems)
        system->enter(instance, m_TaskManager, time, &m_EntityManager);
}

void World::update() {
    // Schedule entity command buffer executor
    std::vector<std::shared_ptr<MelonTask::TaskHandle>> predecessors(m_Systems.size());
    for (unsigned int i = 0; i < m_Systems.size(); i++)
        predecessors[i] = m_Systems[i]->predecessor();
    std::shared_ptr<MelonTask::TaskHandle> taskHandle = m_TaskManager->schedule(m_EntityCommandBufferExecutor, predecessors);
    m_TaskManager->activateWaitingTasks();
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

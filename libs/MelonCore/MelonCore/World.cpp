#include <MelonCore/EntityManager.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/World.h>
#include <MelonTask/TaskManager.h>

namespace Melon {

World::World(MelonTask::TaskManager* taskManager) : m_TaskManager(taskManager) {
    EntityManager* entityManager = &m_EntityManager;
    m_EntityCommandBufferExecutor = [entityManager]() {
        entityManager->executeEntityCommandBuffers();
    };
}

void World::enter(Instance* instance, Time* time, ResourceManager* resourceManager) {
    for (std::unique_ptr<SystemBase> const& system : m_Systems)
        system->enter(instance, m_TaskManager, time, resourceManager, &m_EntityManager);
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

}  // namespace Melon

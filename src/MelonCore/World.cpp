#include <MelonCore/EntityManager.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/World.h>
#include <MelonCore/TaskManager.h>

namespace Melon {

World::World(EntityManager* entityManager) : _entityManager(entityManager) {
    _entityCommandBufferExecutor = [entityManager]() {
        entityManager->executeEntityCommandBuffers();
    };
}

void World::enter() {
    for (const std::unique_ptr<SystemBase>& system : _systems)
        system->enter(_entityManager);
}

void World::update() {
    // Schedule entity command buffer executor
    std::vector<std::shared_ptr<TaskHandle>> predecessors(_systems.size());
    for (unsigned int i = 0; i < _systems.size(); i++)
        predecessors[i] = _systems[i]->predecessor();
    std::shared_ptr<TaskHandle> taskHandle = TaskManager::instance()->schedule(_entityCommandBufferExecutor, predecessors);
    TaskManager::instance()->activateWaitingTasks();
    for (const std::unique_ptr<SystemBase>& system : _systems)
        system->predecessor() = taskHandle;
    // Update systems
    for (const std::unique_ptr<SystemBase>& system : _systems)
        system->update();
}

void World::exit() {
    for (const std::unique_ptr<SystemBase>& system : _systems)
        system->exit();
}

}  // namespace Melon

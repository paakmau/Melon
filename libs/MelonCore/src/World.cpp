#include <MelonCore/EntityManager.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/World.h>
#include <MelonTask/TaskManager.h>

namespace MelonCore {

World::World(EntityManager* entityManager) : _entityManager(entityManager) {
    _entityCommandBufferExecutor = [entityManager]() {
        entityManager->executeEntityCommandBuffers();
    };
}

void World::enter() {
    for (std::unique_ptr<SystemBase> const& system : _systems)
        system->enter(_entityManager);
}

void World::update() {
    // Schedule entity command buffer executor
    std::vector<std::shared_ptr<MelonTask::TaskHandle>> predecessors(_systems.size());
    for (unsigned int i = 0; i < _systems.size(); i++)
        predecessors[i] = _systems[i]->predecessor();
    std::shared_ptr<MelonTask::TaskHandle> taskHandle = MelonTask::TaskManager::instance()->schedule(_entityCommandBufferExecutor, predecessors);
    MelonTask::TaskManager::instance()->activateWaitingTasks();
    for (std::unique_ptr<SystemBase> const& system : _systems)
        system->predecessor() = taskHandle;
    // Update systems
    for (std::unique_ptr<SystemBase> const& system : _systems)
        system->update();
}

void World::exit() {
    for (std::unique_ptr<SystemBase> const& system : _systems)
        system->exit();
}

}  // namespace MelonCore

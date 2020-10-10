#include <MelonCore/ChunkTask.h>
#include <MelonCore/EntityManager.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/TaskHandle.h>
#include <MelonCore/TaskManager.h>

namespace Melon {

SystemBase::SystemBase() {}

SystemBase::~SystemBase() {}

std::shared_ptr<TaskHandle> SystemBase::schedule(const std::shared_ptr<ChunkTask>& chunkTask, const EntityFilter& entityFilter, const std::shared_ptr<TaskHandle>& predecessor) {
    std::shared_ptr<std::vector<ChunkAccessor>> accessors = make_shared<std::vector<ChunkAccessor>>(_entityMananger->filterEntities(entityFilter));
    if (accessors->size() == 0) return predecessor;
    const unsigned int taskCount = std::min(TaskManager::kWorkerCount, (static_cast<unsigned int>(accessors->size()) - 1) / kMinChunkCountPerTask + 1);
    const unsigned int chunkCountPerTask = accessors->size() / taskCount;
    std::vector<std::shared_ptr<TaskHandle>> taskHandles(taskCount);
    for (unsigned int i = 0; i < taskCount; i++)
        taskHandles[i] = TaskManager::instance()->schedule(
            [chunkTask, accessors, chunkCountPerTask, i]() {
                for (unsigned int j = 0; j < chunkCountPerTask; j++)
                    chunkTask->execute((*accessors)[std::min(i * chunkCountPerTask + j, static_cast<unsigned int>(accessors->size()))]);
            },
            {predecessor});
    return TaskManager::instance()->combine(taskHandles);
}

void SystemBase::enter(EntityManager* entityManager) {
    _entityMananger = entityManager;
    onEnter();
    TaskManager::instance()->activateWaitingTasks();
}

void SystemBase::update() {
    if (_taskHandle)
        _taskHandle->complete();
    onUpdate();
    TaskManager::instance()->activateWaitingTasks();
}

void SystemBase::exit() {
    onExit();
}

}  // namespace Melon

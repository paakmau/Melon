#include <MelonCore/ChunkTask.h>
#include <MelonCore/EntityManager.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/TaskHandle.h>
#include <MelonCore/TaskManager.h>

namespace MelonCore {

SystemBase::SystemBase() {}

SystemBase::~SystemBase() {}

std::shared_ptr<TaskHandle> SystemBase::schedule(const std::shared_ptr<ChunkTask>& chunkTask, const EntityFilter& entityFilter, const std::shared_ptr<TaskHandle>& predecessor) {
    std::shared_ptr<std::vector<ChunkAccessor>> accessors = make_shared<std::vector<ChunkAccessor>>(_entityMananger->filterEntities(entityFilter));
    if (accessors->size() == 0) return predecessor;
    const unsigned int taskCount = std::min(TaskManager::kWorkerCount, (static_cast<unsigned int>(accessors->size()) - 1) / kMinChunkCountPerTask + 1);
    const unsigned int chunkCountPerTask = accessors->size() / taskCount;
    std::vector<std::shared_ptr<TaskHandle>> taskHandles(taskCount);
    unsigned int chunkCounter = 0;
    unsigned int entityCounter = 0;
    for (unsigned int i = 0; i < taskCount; i++) {
        // TODO:
        taskHandles[i] = TaskManager::instance()->schedule(
            [chunkTask, accessors, chunkCountPerTask, i, chunkCounter, entityCounter]() {
                for (unsigned int j = i * chunkCountPerTask; j < (i + 1) * chunkCountPerTask && j < accessors->size(); j++)
                    chunkTask->execute((*accessors)[j], chunkCounter, entityCounter);
            },
            {predecessor});
        chunkCounter += chunkCountPerTask;
        for (unsigned int j = i * chunkCountPerTask; j < (i + 1) * chunkCountPerTask && j < accessors->size(); j++)
            entityCounter += (*accessors)[j].entityCount();
    }
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

}  // namespace MelonCore

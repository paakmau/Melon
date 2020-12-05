#include <MelonCore/SystemBase.h>
#include <MelonTask/TaskManager.h>

namespace MelonCore {

SystemBase::SystemBase() {}

SystemBase::~SystemBase() {}

std::shared_ptr<MelonTask::TaskHandle> SystemBase::schedule(const std::shared_ptr<ChunkTask>& chunkTask, const EntityFilter& entityFilter, const std::shared_ptr<MelonTask::TaskHandle>& predecessor) {
    std::shared_ptr<std::vector<ChunkAccessor>> accessors = std::make_shared<std::vector<ChunkAccessor>>(_entityManager->filterEntities(entityFilter));
    if (accessors->size() == 0) return predecessor;
    const unsigned int taskCount = std::min(MelonTask::TaskManager::kWorkerCount, (static_cast<unsigned int>(accessors->size()) - 1) / kMinChunkCountPerTask + 1);
    const unsigned int chunkCountPerTask = accessors->size() / taskCount;
    std::vector<std::shared_ptr<MelonTask::TaskHandle>> taskHandles(taskCount);
    unsigned int chunkCounter = 0;
    unsigned int entityCounter = 0;
    for (unsigned int i = 0; i < taskCount; i++) {
        taskHandles[i] = MelonTask::TaskManager::instance()->schedule(
            [chunkTask, accessors, chunkCountPerTask, i, chunkCounter, entityCounter]() {
                for (unsigned int j = i * chunkCountPerTask; j < (i + 1) * chunkCountPerTask && j < accessors->size(); j++)
                    chunkTask->execute((*accessors)[j], chunkCounter, entityCounter);
            },
            {predecessor});
        chunkCounter += chunkCountPerTask;
        for (unsigned int j = i * chunkCountPerTask; j < (i + 1) * chunkCountPerTask && j < accessors->size(); j++)
            entityCounter += (*accessors)[j].entityCount();
    }
    return MelonTask::TaskManager::instance()->combine(taskHandles);
}

std::shared_ptr<MelonTask::TaskHandle> SystemBase::schedule(const std::shared_ptr<EntityCommandBufferChunkTask>& entityCommandBufferChunkTask, const EntityFilter& entityFilter, const std::shared_ptr<MelonTask::TaskHandle>& predecessor) {
    std::shared_ptr<std::vector<ChunkAccessor>> accessors = std::make_shared<std::vector<ChunkAccessor>>(_entityManager->filterEntities(entityFilter));
    if (accessors->size() == 0) return predecessor;
    const unsigned int taskCount = std::min(MelonTask::TaskManager::kWorkerCount, (static_cast<unsigned int>(accessors->size()) - 1) / kMinChunkCountPerTask + 1);
    const unsigned int chunkCountPerTask = accessors->size() / taskCount;
    std::vector<std::shared_ptr<MelonTask::TaskHandle>> taskHandles(taskCount);
    unsigned int chunkCounter = 0;
    unsigned int entityCounter = 0;
    for (unsigned int i = 0; i < taskCount; i++) {
        EntityCommandBuffer* entityCommandBuffer = _entityManager->createEntityCommandBuffer();
        taskHandles[i] = MelonTask::TaskManager::instance()->schedule(
            [entityCommandBufferChunkTask, accessors, chunkCountPerTask, i, chunkCounter, entityCounter, entityCommandBuffer]() {
                for (unsigned int j = i * chunkCountPerTask; j < (i + 1) * chunkCountPerTask && j < accessors->size(); j++)
                    entityCommandBufferChunkTask->execute((*accessors)[j], chunkCounter, entityCounter, entityCommandBuffer);
            },
            {predecessor});
        chunkCounter += chunkCountPerTask;
        for (unsigned int j = i * chunkCountPerTask; j < (i + 1) * chunkCountPerTask && j < accessors->size(); j++)
            entityCounter += (*accessors)[j].entityCount();
    }
    return MelonTask::TaskManager::instance()->combine(taskHandles);
}

void SystemBase::enter(EntityManager* entityManager) {
    _entityManager = entityManager;
    onEnter();
    MelonTask::TaskManager::instance()->activateWaitingTasks();
}

void SystemBase::update() {
    if (_taskHandle)
        _taskHandle->complete();
    onUpdate();
    MelonTask::TaskManager::instance()->activateWaitingTasks();
}

void SystemBase::exit() {
    onExit();
}

}  // namespace MelonCore

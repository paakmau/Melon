#include <libMelonCore/SystemBase.h>
#include <libMelonTask/TaskManager.h>

namespace Melon {

SystemBase::SystemBase() {}

SystemBase::~SystemBase() {}

std::shared_ptr<MelonTask::TaskHandle> SystemBase::schedule(std::shared_ptr<ChunkTask> const& chunkTask, const EntityFilter& entityFilter, std::shared_ptr<MelonTask::TaskHandle> const& predecessor) {
    std::shared_ptr<std::vector<ChunkAccessor>> accessors = std::make_shared<std::vector<ChunkAccessor>>(m_EntityManager->filterEntities(entityFilter));
    if (accessors->size() == 0) return predecessor;
    const unsigned int taskCount = std::min(MelonTask::TaskManager::k_WorkerCount, (static_cast<unsigned int>(accessors->size()) - 1) / k_MinChunkCountPerTask + 1);
    const unsigned int chunkCountPerTask = accessors->size() / taskCount;
    std::vector<std::shared_ptr<MelonTask::TaskHandle>> taskHandles(taskCount);
    unsigned int chunkCounter = 0;
    unsigned int entityCounter = 0;
    for (unsigned int i = 0; i < taskCount; i++) {
        taskHandles[i] = m_TaskManager->schedule(
            [chunkTask, accessors, chunkCountPerTask, i, chunkCounter, entityCounter]() {
                for (unsigned int j = i * chunkCountPerTask; j < (i + 1) * chunkCountPerTask && j < accessors->size(); j++)
                    chunkTask->execute((*accessors)[j], chunkCounter, entityCounter);
            },
            {predecessor});
        chunkCounter += chunkCountPerTask;
        for (unsigned int j = i * chunkCountPerTask; j < (i + 1) * chunkCountPerTask && j < accessors->size(); j++)
            entityCounter += (*accessors)[j].entityCount();
    }
    return m_TaskManager->combine(taskHandles);
}

std::shared_ptr<MelonTask::TaskHandle> SystemBase::schedule(std::shared_ptr<EntityCommandBufferChunkTask> const& entityCommandBufferChunkTask, const EntityFilter& entityFilter, std::shared_ptr<MelonTask::TaskHandle> const& predecessor) {
    std::shared_ptr<std::vector<ChunkAccessor>> accessors = std::make_shared<std::vector<ChunkAccessor>>(m_EntityManager->filterEntities(entityFilter));
    if (accessors->size() == 0) return predecessor;
    const unsigned int taskCount = std::min(MelonTask::TaskManager::k_WorkerCount, (static_cast<unsigned int>(accessors->size()) - 1) / k_MinChunkCountPerTask + 1);
    const unsigned int chunkCountPerTask = accessors->size() / taskCount;
    std::vector<std::shared_ptr<MelonTask::TaskHandle>> taskHandles(taskCount);
    unsigned int chunkCounter = 0;
    unsigned int entityCounter = 0;
    for (unsigned int i = 0; i < taskCount; i++) {
        EntityCommandBuffer* entityCommandBuffer = m_EntityManager->createEntityCommandBuffer();
        taskHandles[i] = m_TaskManager->schedule(
            [entityCommandBufferChunkTask, accessors, chunkCountPerTask, i, chunkCounter, entityCounter, entityCommandBuffer]() {
                for (unsigned int j = i * chunkCountPerTask; j < (i + 1) * chunkCountPerTask && j < accessors->size(); j++)
                    entityCommandBufferChunkTask->execute((*accessors)[j], chunkCounter, entityCounter, entityCommandBuffer);
            },
            {predecessor});
        chunkCounter += chunkCountPerTask;
        for (unsigned int j = i * chunkCountPerTask; j < (i + 1) * chunkCountPerTask && j < accessors->size(); j++)
            entityCounter += (*accessors)[j].entityCount();
    }
    return m_TaskManager->combine(taskHandles);
}

void SystemBase::enter(Instance* instance, MelonTask::TaskManager* taskManager, Time* time, EntityManager* entityManager) {
    m_Instance = instance;
    m_TaskManager = taskManager;
    m_Time = time;
    m_EntityManager = entityManager;
    onEnter();
    m_TaskManager->activateWaitingTasks();
}

void SystemBase::update() {
    if (m_TaskHandle)
        m_TaskHandle->complete();
    onUpdate();
    m_TaskManager->activateWaitingTasks();
}

void SystemBase::exit() {
    onExit();
}

}  // namespace Melon

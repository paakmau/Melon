#pragma once

#include <MelonCore/ChunkAccessor.h>
#include <MelonCore/EntityFilter.h>
#include <MelonCore/EntityManager.h>
#include <MelonTask/TaskHandle.h>

#include <memory>

namespace MelonCore {

class ChunkTask {
   public:
    virtual void execute(const ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) = 0;
};

class EntityCommandBufferChunkTask {
   public:
    virtual void execute(const ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, EntityCommandBuffer* entityCommandBuffer) = 0;
};

class SystemBase {
   public:
    static constexpr unsigned int kMinChunkCountPerTask = 16;

    SystemBase();
    virtual ~SystemBase();

   protected:
    virtual void onEnter() = 0;
    virtual void onUpdate() = 0;
    virtual void onExit() = 0;

    std::shared_ptr<MelonTask::TaskHandle> schedule(const std::shared_ptr<ChunkTask>& chunkTask, const EntityFilter& entityFilter, const std::shared_ptr<MelonTask::TaskHandle>& predecessor);
    std::shared_ptr<MelonTask::TaskHandle> schedule(const std::shared_ptr<EntityCommandBufferChunkTask>& entityCommandBufferChunkTask, const EntityFilter& entityFilter, const std::shared_ptr<MelonTask::TaskHandle>& predecessor);

    const std::shared_ptr<MelonTask::TaskHandle>& predecessor() const { return _taskHandle; }
    std::shared_ptr<MelonTask::TaskHandle>& predecessor() { return _taskHandle; }
    EntityManager* entityManager() const { return _entityManager; }

   private:
    void enter(EntityManager* entityManager);
    void update();
    void exit();

    std::shared_ptr<MelonTask::TaskHandle> _taskHandle;
    EntityManager* _entityManager;

    friend class World;
};

}  // namespace MelonCore

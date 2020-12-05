#pragma once

#include <MelonCore/ChunkAccessor.h>
#include <MelonCore/EntityFilter.h>
#include <MelonCore/EntityManager.h>
#include <MelonTask/TaskHandle.h>

#include <memory>

namespace MelonCore {

class ChunkTask {
  public:
    virtual void execute(ChunkAccessor const& chunkAccessor, unsigned int const& chunkIndex, unsigned int const& firstEntityIndex) = 0;
};

class EntityCommandBufferChunkTask {
  public:
    virtual void execute(ChunkAccessor const& chunkAccessor, unsigned int const& chunkIndex, unsigned int const& firstEntityIndex, EntityCommandBuffer* entityCommandBuffer) = 0;
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

    std::shared_ptr<MelonTask::TaskHandle> schedule(std::shared_ptr<ChunkTask> const& chunkTask, EntityFilter const& entityFilter, std::shared_ptr<MelonTask::TaskHandle> const& predecessor);
    std::shared_ptr<MelonTask::TaskHandle> schedule(std::shared_ptr<EntityCommandBufferChunkTask> const& entityCommandBufferChunkTask, EntityFilter const& entityFilter, std::shared_ptr<MelonTask::TaskHandle> const& predecessor);

    std::shared_ptr<MelonTask::TaskHandle> const& predecessor() const { return _taskHandle; }
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

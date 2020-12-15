#pragma once

#include <libMelonCore/ChunkAccessor.h>
#include <libMelonCore/EntityFilter.h>
#include <libMelonCore/EntityManager.h>
#include <libMelonTask/TaskHandle.h>

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
    static constexpr unsigned int k_MinChunkCountPerTask = 16;

    SystemBase();
    virtual ~SystemBase();

  protected:
    virtual void onEnter() = 0;
    virtual void onUpdate() = 0;
    virtual void onExit() = 0;

    std::shared_ptr<MelonTask::TaskHandle> schedule(std::shared_ptr<ChunkTask> const& chunkTask, EntityFilter const& entityFilter, std::shared_ptr<MelonTask::TaskHandle> const& predecessor);
    std::shared_ptr<MelonTask::TaskHandle> schedule(std::shared_ptr<EntityCommandBufferChunkTask> const& entityCommandBufferChunkTask, EntityFilter const& entityFilter, std::shared_ptr<MelonTask::TaskHandle> const& predecessor);

    std::shared_ptr<MelonTask::TaskHandle> const& predecessor() const { return m_TaskHandle; }
    std::shared_ptr<MelonTask::TaskHandle>& predecessor() { return m_TaskHandle; }
    EntityManager* entityManager() const { return m_EntityManager; }

  private:
    void enter(EntityManager* entityManager);
    void update();
    void exit();

    std::shared_ptr<MelonTask::TaskHandle> m_TaskHandle;
    EntityManager* m_EntityManager;

    friend class World;
};

}  // namespace MelonCore

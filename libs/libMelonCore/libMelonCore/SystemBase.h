#pragma once

#include <libMelonCore/ChunkAccessor.h>
#include <libMelonCore/EntityFilter.h>
#include <libMelonCore/EntityManager.h>
#include <libMelonCore/Time.h>
#include <libMelonTask/TaskHandle.h>
#include <libMelonTask/TaskManager.h>

#include <memory>

namespace Melon {

class Instance;

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

    Instance* const& instance() const { return m_Instance; }
    Instance*& instance() { return m_Instance; }
    MelonTask::TaskManager* const& taskManager() const { return m_TaskManager; }
    MelonTask::TaskManager*& taskManager() { return m_TaskManager; }
    Time* const& time() const { return m_Time; }
    Time*& time() { return m_Time; }
    EntityManager* const& entityManager() const { return m_EntityManager; }
    EntityManager*& entityManager() { return m_EntityManager; }

    std::shared_ptr<MelonTask::TaskHandle> const& predecessor() const { return m_TaskHandle; }
    std::shared_ptr<MelonTask::TaskHandle>& predecessor() { return m_TaskHandle; }

  private:
    void enter(Instance* instance, MelonTask::TaskManager* taskManager, Time* time, EntityManager* entityManager);
    void update();
    void exit();

    Instance* m_Instance;
    MelonTask::TaskManager* m_TaskManager;
    Time* m_Time;
    EntityManager* m_EntityManager;

    std::shared_ptr<MelonTask::TaskHandle> m_TaskHandle;

    friend class World;
};

}  // namespace Melon

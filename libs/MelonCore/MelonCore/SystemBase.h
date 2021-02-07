#pragma once

#include <MelonCore/ChunkAccessor.h>
#include <MelonCore/EntityFilter.h>
#include <MelonCore/EntityManager.h>
#include <MelonCore/EventManager.h>
#include <MelonCore/ResourceManager.h>
#include <MelonCore/Time.h>
#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

#include <memory>

namespace Melon {

class Instance;

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
    static constexpr unsigned int k_MinChunkCountPerTask = 16;

    SystemBase() {}
    virtual ~SystemBase() {}

  protected:
    virtual void onEnter() = 0;
    virtual void onUpdate() = 0;
    virtual void onExit() = 0;

    std::shared_ptr<TaskHandle> schedule(std::shared_ptr<ChunkTask> const& chunkTask, const EntityFilter& entityFilter, std::shared_ptr<TaskHandle> const& predecessor);
    std::shared_ptr<TaskHandle> schedule(std::shared_ptr<EntityCommandBufferChunkTask> const& entityCommandBufferChunkTask, const EntityFilter& entityFilter, std::shared_ptr<TaskHandle> const& predecessor);

    Instance* const& instance() const { return m_Instance; }
    TaskManager* const& taskManager() const { return m_TaskManager; }
    Time* const& time() const { return m_Time; }
    ResourceManager* const& resourceManager() const { return m_ResourceManager; }
    EntityManager* const& entityManager() const { return m_EntityManager; }
    EventManager* const& eventManager() const { return m_EventManager; }

    const std::shared_ptr<TaskHandle>& predecessor() const { return m_TaskHandle; }
    std::shared_ptr<TaskHandle>& predecessor() { return m_TaskHandle; }

  private:
    void enter(Instance* instance, TaskManager* taskManager, Time* time, ResourceManager* resourceManager, EntityManager* entityManager, EventManager* eventManager);
    void update();
    void exit();

    Instance* m_Instance;
    TaskManager* m_TaskManager;
    Time* m_Time;
    ResourceManager* m_ResourceManager;
    EntityManager* m_EntityManager;
    EventManager* m_EventManager;

    std::shared_ptr<TaskHandle> m_TaskHandle;

    friend class World;
};

}  // namespace Melon

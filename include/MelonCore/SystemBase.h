#pragma once

#include <memory>

namespace MelonCore {

class ChunkTask;
class EntityFilter;
class EntityManager;
class TaskHandle;

class SystemBase {
   public:
    static constexpr unsigned int kMinChunkCountPerTask = 16;

    SystemBase();
    virtual ~SystemBase();

   protected:
    virtual void onEnter() = 0;
    virtual void onUpdate() = 0;
    virtual void onExit() = 0;

    std::shared_ptr<TaskHandle> schedule(const std::shared_ptr<ChunkTask>& chunkTask, const EntityFilter& entityFilter, const std::shared_ptr<TaskHandle>& predecessor);

    const std::shared_ptr<TaskHandle>& predecessor() const { return _taskHandle; }
    std::shared_ptr<TaskHandle>& predecessor() { return _taskHandle; }
    EntityManager* entityManager() const { return _entityMananger; }

   private:
    void enter(EntityManager* entityManager);
    void update();
    void exit();

    std::shared_ptr<TaskHandle> _taskHandle;
    EntityManager* _entityMananger;

    friend class World;
};

}  // namespace MelonCore

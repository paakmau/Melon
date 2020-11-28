#include <MelonCore/Archetype.h>
#include <MelonCore/ChunkTask.h>
#include <MelonCore/Entity.h>
#include <MelonCore/EntityManager.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/TaskHandle.h>
#include <MelonCore/TaskManager.h>
#include <MelonCore/Time.h>
#include <MelonCore/Translation.h>

#include <array>
#include <cstdio>
#include <memory>

struct Spawner : public MelonCore::Component {
    unsigned int initialHealth;
    unsigned int spawnerCount;
};

struct Health : public MelonCore::Component {
    unsigned int value;
};

class SpawnerAndKillSystem : public MelonCore::SystemBase {
   protected:
    class SpawnerEntityCommandBufferChunkTask : public MelonCore::EntityCommandBufferChunkTask {
       public:
        SpawnerEntityCommandBufferChunkTask(const unsigned int& spawnerComponentId) : _spawnerComponentId(spawnerComponentId) {}
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, MelonCore::EntityCommandBuffer* entityCommandBuffer) override {
            Spawner* spawners = chunkAccessor.componentArray<Spawner>(_spawnerComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                Spawner& spawner = spawners[i];
                spawner.spawnerCount--;
                MelonCore::Entity entity = entityCommandBuffer->createEntity();
                entityCommandBuffer->addComponent<Health>(entity, Health{.value = spawner.initialHealth});
                if (spawner.spawnerCount == 0)
                    entityCommandBuffer->destroyEntity(chunkAccessor.entityArray()[i]);
            }
        }

        const unsigned int& _spawnerComponentId;
    };

    class KillEntityCommandBufferChunkTask : public MelonCore::EntityCommandBufferChunkTask {
       public:
        KillEntityCommandBufferChunkTask(const unsigned int& healthComponentId) : _healthComponentId(healthComponentId) {}
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, MelonCore::EntityCommandBuffer* entityCommandBuffer) override {
            Health* healths = chunkAccessor.componentArray<Health>(_healthComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                Health& health = healths[i];
                health.value--;
                if (health.value == 0)
                    entityCommandBuffer->destroyEntity(chunkAccessor.entityArray()[i]);
            }
        }

        const unsigned int& _healthComponentId;
    };

    void onEnter() override {
        MelonCore::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<Spawner>().createArchetype();

        constexpr unsigned int spawnerCount = 3;
        std::array<MelonCore::Entity, spawnerCount> entities;
        for (unsigned int i = 0; i < entities.size(); i++)
            entities[i] = entityManager()->createEntity(archetype);
        for (unsigned int i = 0; i < entities.size(); i += 3)
            entityManager()->setComponent(entities[i], Spawner{.initialHealth = 5, .spawnerCount = 1});
        for (unsigned int i = 1; i < entities.size(); i += 3)
            entityManager()->setComponent(entities[i], Spawner{.initialHealth = 3, .spawnerCount = 3});
        for (unsigned int i = 2; i < entities.size(); i += 3)
            entityManager()->setComponent(entities[i], Spawner{.initialHealth = 1, .spawnerCount = 4});

        _spawnerEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Spawner>().createEntityFilter();
        _monsterEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Health>().createEntityFilter();
        _spawnerComponentId = entityManager()->componentId<Spawner>();
        _healthComponentId = entityManager()->componentId<Health>();
    }

    void onUpdate() override {
        printf("Delta time : %f\n", MelonCore::Time::instance()->deltaTime());
        std::shared_ptr<MelonCore::TaskHandle> spawnerTaskHandle = schedule(std::make_shared<SpawnerEntityCommandBufferChunkTask>(_spawnerComponentId), _spawnerEntityFilter, predecessor());
        std::shared_ptr<MelonCore::TaskHandle> killTaskHandle = schedule(std::make_shared<KillEntityCommandBufferChunkTask>(_healthComponentId), _monsterEntityFilter, predecessor());
        predecessor() = MelonCore::TaskManager::instance()->combine({spawnerTaskHandle, killTaskHandle});
        if (entityManager()->entityCount(_spawnerEntityFilter) == 0 && entityManager()->entityCount(_monsterEntityFilter) == 0)
            MelonCore::Instance::instance()->quit();
    }

    void onExit() override {}

   private:
    MelonCore::EntityFilter _spawnerEntityFilter;
    MelonCore::EntityFilter _monsterEntityFilter;
    unsigned int _spawnerComponentId;
    unsigned int _healthComponentId;
    unsigned int _counter{};
};

int main() {
    MelonCore::Instance::instance()->registerSystem<SpawnerAndKillSystem>();
    MelonCore::Instance::instance()->start();
    return 0;
}
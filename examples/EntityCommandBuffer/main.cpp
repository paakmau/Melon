#include <MelonCore/Archetype.h>
#include <MelonCore/Entity.h>
#include <MelonCore/EntityManager.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonCore/Translation.h>
#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

#include <array>
#include <cstdio>
#include <memory>

struct Spawner : public Melon::DataComponent {
    unsigned int initialHealth;
    unsigned int spawnerCount;
};

struct Health : public Melon::DataComponent {
    unsigned int value;
};

class SpawnerAndKillSystem : public Melon::SystemBase {
  protected:
    class SpawnerEntityCommandBufferChunkTask : public Melon::EntityCommandBufferChunkTask {
      public:
        SpawnerEntityCommandBufferChunkTask(const unsigned int& spawnerComponentId) : m_SpawnerComponentId(spawnerComponentId) {}
        virtual void execute(const Melon::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, Melon::EntityCommandBuffer* entityCommandBuffer) override {
            Spawner* spawners = chunkAccessor.componentArray<Spawner>(m_SpawnerComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                Spawner& spawner = spawners[i];
                spawner.spawnerCount--;
                Melon::Entity entity = entityCommandBuffer->createEntity();
                entityCommandBuffer->addComponent<Health>(entity, Health{.value = spawner.initialHealth});
                if (spawner.spawnerCount == 0)
                    entityCommandBuffer->destroyEntity(chunkAccessor.entityArray()[i]);
            }
        }

        const unsigned int& m_SpawnerComponentId;
    };

    class KillEntityCommandBufferChunkTask : public Melon::EntityCommandBufferChunkTask {
      public:
        KillEntityCommandBufferChunkTask(const unsigned int& healthComponentId) : m_HealthComponentId(healthComponentId) {}
        virtual void execute(const Melon::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, Melon::EntityCommandBuffer* entityCommandBuffer) override {
            Health* healths = chunkAccessor.componentArray<Health>(m_HealthComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                Health& health = healths[i];
                health.value--;
                if (health.value == 0)
                    entityCommandBuffer->destroyEntity(chunkAccessor.entityArray()[i]);
            }
        }

        const unsigned int& m_HealthComponentId;
    };

    void onEnter() override {
        Melon::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<Spawner>().createArchetype();

        constexpr unsigned int spawnerCount = 3;
        std::array<Melon::Entity, spawnerCount> entities;
        for (unsigned int i = 0; i < entities.size(); i++)
            entities[i] = entityManager()->createEntity(archetype);
        for (unsigned int i = 0; i < entities.size(); i += 3)
            entityManager()->setComponent(entities[i], Spawner{.initialHealth = 5, .spawnerCount = 1});
        for (unsigned int i = 1; i < entities.size(); i += 3)
            entityManager()->setComponent(entities[i], Spawner{.initialHealth = 3, .spawnerCount = 3});
        for (unsigned int i = 2; i < entities.size(); i += 3)
            entityManager()->setComponent(entities[i], Spawner{.initialHealth = 1, .spawnerCount = 4});

        m_SpawnerEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Spawner>().createEntityFilter();
        m_MonsterEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Health>().createEntityFilter();
        m_SpawnerComponentId = entityManager()->componentId<Spawner>();
        m_HealthComponentId = entityManager()->componentId<Health>();
    }

    void onUpdate() override {
        printf("Delta time : %f\n", time()->deltaTime());
        std::shared_ptr<Melon::TaskHandle> spawnerTaskHandle = schedule(std::make_shared<SpawnerEntityCommandBufferChunkTask>(m_SpawnerComponentId), m_SpawnerEntityFilter, predecessor());
        std::shared_ptr<Melon::TaskHandle> killTaskHandle = schedule(std::make_shared<KillEntityCommandBufferChunkTask>(m_HealthComponentId), m_MonsterEntityFilter, predecessor());
        predecessor() = taskManager()->combine({spawnerTaskHandle, killTaskHandle});
        if (entityManager()->entityCount(m_SpawnerEntityFilter) == 0 && entityManager()->entityCount(m_MonsterEntityFilter) == 0)
            instance()->quit();
    }

    void onExit() override {}

  private:
    Melon::EntityFilter m_SpawnerEntityFilter;
    Melon::EntityFilter m_MonsterEntityFilter;
    unsigned int m_SpawnerComponentId;
    unsigned int m_HealthComponentId;
    unsigned int m_Counter{};
};

int main() {
    Melon::Instance()
        .registerSystem<SpawnerAndKillSystem>()
        .start();
    return 0;
}
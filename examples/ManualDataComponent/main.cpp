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

struct MonsterHealth : public Melon::DataComponent {
    unsigned int value;
};

struct PersistentDamage : public Melon::DataComponent {
    unsigned int value;
};

struct ManualDamageCounter : public Melon::ManualDataComponent {
    unsigned int index;
    // The count of damage taken
    unsigned int damageTakenCount;
};

class MonsterDamageCounterSystem : public Melon::SystemBase {
  protected:
    class DamageEntityCommandBufferChunkTask : public Melon::EntityCommandBufferChunkTask {
      public:
        DamageEntityCommandBufferChunkTask(const unsigned int& monsterHealthComponentId, const unsigned int& persistentDamageComponentId, const unsigned int& manualDamageCounterComponentId) : m_MonsterHealthComponentId(monsterHealthComponentId), m_PersistentDamageComponentId(persistentDamageComponentId), m_ManualDamageCounterComponentId(manualDamageCounterComponentId) {}
        virtual void execute(const Melon::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, Melon::EntityCommandBuffer* entityCommandBuffer) override {
            const Melon::Entity* entities = chunkAccessor.entityArray();
            MonsterHealth* monsterHealths = chunkAccessor.componentArray<MonsterHealth>(m_MonsterHealthComponentId);
            PersistentDamage* persistentDamages = chunkAccessor.componentArray<PersistentDamage>(m_PersistentDamageComponentId);
            ManualDamageCounter* manualDamageCounters = chunkAccessor.componentArray<ManualDamageCounter>(m_ManualDamageCounterComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                if (monsterHealths[i].value <= persistentDamages[i].value)
                    entityCommandBuffer->destroyEntity(entities[i]);
                else
                    monsterHealths[i].value -= persistentDamages[i].value;
                manualDamageCounters[i].damageTakenCount++;
            }
        }

        const unsigned int& m_MonsterHealthComponentId;
        const unsigned int& m_PersistentDamageComponentId;
        const unsigned int& m_ManualDamageCounterComponentId;
    };

    class CollectCounterCommandBufferChunkTask : public Melon::EntityCommandBufferChunkTask {
      public:
        CollectCounterCommandBufferChunkTask(const unsigned int& manualDamageCounterComponentId, std::vector<unsigned int>& damageTakenCounts) : m_ManualDamageCounterComponentId(manualDamageCounterComponentId), m_DamageTakenCounts(damageTakenCounts) {}
        virtual void execute(const Melon::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, Melon::EntityCommandBuffer* entityCommandBuffer) override {
            const Melon::Entity* entities = chunkAccessor.entityArray();
            ManualDamageCounter* manualDamageCounters = chunkAccessor.componentArray<ManualDamageCounter>(m_ManualDamageCounterComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                m_DamageTakenCounts[manualDamageCounters[i].index] = manualDamageCounters[i].damageTakenCount;
                entityCommandBuffer->removeComponent<ManualDamageCounter>(entities[i]);
            }
        }

        const unsigned int& m_ManualDamageCounterComponentId;

        std::vector<unsigned int>& m_DamageTakenCounts;
    };

    void onEnter() override {
        Melon::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<MonsterHealth, PersistentDamage, ManualDamageCounter>().createArchetype();

        std::array<Melon::Entity, 4> entities;
        for (unsigned int i = 0; i < entities.size(); i++)
            entities[i] = entityManager()->createEntity(archetype);

        entityManager()->setComponent(entities[0], MonsterHealth{.value = 3});
        entityManager()->setComponent(entities[0], PersistentDamage{.value = 2});
        entityManager()->setComponent(entities[0], ManualDamageCounter{.index = 0});

        entityManager()->setComponent(entities[1], MonsterHealth{.value = 8});
        entityManager()->setComponent(entities[1], PersistentDamage{.value = 3});
        entityManager()->setComponent(entities[1], ManualDamageCounter{.index = 1});

        entityManager()->setComponent(entities[2], MonsterHealth{.value = 4});
        entityManager()->setComponent(entities[2], PersistentDamage{.value = 1});
        entityManager()->setComponent(entities[2], ManualDamageCounter{.index = 2});

        entityManager()->setComponent(entities[3], MonsterHealth{.value = 6});
        entityManager()->setComponent(entities[3], PersistentDamage{.value = 2});
        entityManager()->setComponent(entities[3], ManualDamageCounter{.index = 3});

        m_MonsterEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<MonsterHealth, PersistentDamage, ManualDamageCounter>().createEntityFilter();
        m_CollectCounterEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<ManualDamageCounter>().rejectComponents<MonsterHealth>().createEntityFilter();
        m_MonsterHealthComponentId = entityManager()->componentId<MonsterHealth>();
        m_PersistentDamageComponentId = entityManager()->componentId<PersistentDamage>();
        m_ManualDamageCounterComponentId = entityManager()->componentId<ManualDamageCounter>();

        m_DamageTakenCounts.resize(4);
    }

    void onUpdate() override {
        printf("Delta time : %f\n", time()->deltaTime());
        std::shared_ptr<Melon::TaskHandle> damageTaskHandle = schedule(std::make_shared<DamageEntityCommandBufferChunkTask>(m_MonsterHealthComponentId, m_PersistentDamageComponentId, m_ManualDamageCounterComponentId), m_MonsterEntityFilter, predecessor());
        std::shared_ptr<Melon::TaskHandle> counterTaskHandle = schedule(std::make_shared<CollectCounterCommandBufferChunkTask>(m_ManualDamageCounterComponentId, m_DamageTakenCounts), m_CollectCounterEntityFilter, predecessor());
        predecessor() = taskManager()->combine({damageTaskHandle, counterTaskHandle});
        if (entityManager()->entityCount(m_MonsterEntityFilter) == 0 && entityManager()->entityCount(m_CollectCounterEntityFilter) == 0) {
            printf("Damage taken counts: ");
            for (const unsigned int& damageTakenCount : m_DamageTakenCounts)
                printf("%d ", damageTakenCount);
            puts("");
            instance()->quit();
        }
    }

    void onExit() override {}

  private:
    Melon::EntityFilter m_MonsterEntityFilter;
    Melon::EntityFilter m_CollectCounterEntityFilter;
    unsigned int m_MonsterHealthComponentId;
    unsigned int m_PersistentDamageComponentId;
    unsigned int m_ManualDamageCounterComponentId;

    std::vector<unsigned int> m_DamageTakenCounts;
};

int main() {
    Melon::Instance()
        .registerSystem<MonsterDamageCounterSystem>()
        .start();
    return 0;
}

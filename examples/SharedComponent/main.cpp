#include <MelonCore/Archetype.h>
#include <MelonCore/Entity.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonCore/Translation.h>

#include <array>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <utility>

struct Group : public Melon::SharedComponent {
    bool operator==(const Group& other) const {
        return id == other.id && salary == other.salary;
    }

    unsigned int id;
    unsigned int salary;
};

template <>
struct std::hash<Group> {
    std::size_t operator()(const Group& group) {
        return std::hash<unsigned int>()(group.id) ^ std::hash<unsigned int>()(group.salary);
    }
};

struct Money : public Melon::DataComponent {
    unsigned int value;
};

class GroupSystem : public Melon::SystemBase {
  protected:
    class GroupChunkTask : public Melon::ChunkTask {
      public:
        GroupChunkTask(const unsigned int& moneyComponentId, const unsigned int& groupSharedComponentId) : m_MoneyComponentId(moneyComponentId), m_GroupSharedComponentId(groupSharedComponentId) {}
        virtual void execute(const Melon::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
            Money* moneys = chunkAccessor.componentArray<Money>(m_MoneyComponentId);
            const Group* group = chunkAccessor.sharedComponent<Group>(m_GroupSharedComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                Money& money = moneys[i];
                money.value += group->salary;
            }
        }

        const unsigned int& m_MoneyComponentId;
        const unsigned int& m_GroupSharedComponentId;
    };

    void onEnter() override {
        Melon::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<Money>().markSharedComponents<Group>().createArchetype();

        std::array<Melon::Entity, 1024> entities;
        for (unsigned int i = 0; i < entities.size(); i++)
            entities[i] = entityManager()->createEntity(archetype);

        for (unsigned int i = 0; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{.id = 0, .salary = 10});

        for (unsigned int i = 1; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{.id = 1, .salary = 100});

        for (unsigned int i = 2; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{.id = 2, .salary = 1000});

        m_EntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Money>().requireSharedComponents<Group>().createEntityFilter();
        m_MoneyComponentId = entityManager()->componentId<Money>();
        m_GroupSharedComponentId = entityManager()->sharedComponentId<Group>();
    }

    void onUpdate() override {
        printf("Delta time : %f\n", time()->deltaTime());
        predecessor() = schedule(std::make_shared<GroupChunkTask>(m_MoneyComponentId, m_GroupSharedComponentId), m_EntityFilter, predecessor());
        if (m_Counter++ > 1000)
            instance()->quit();
    }

    void onExit() override {}

  private:
    Melon::EntityFilter m_EntityFilter;
    unsigned int m_MoneyComponentId;
    unsigned int m_GroupSharedComponentId;
    unsigned int m_Counter{};
};

int main() {
    Melon::Instance()
        .registerSystem<GroupSystem>()
        .start();
    return 0;
}

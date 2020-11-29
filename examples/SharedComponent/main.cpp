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

struct Group : public MelonCore::SharedComponent {
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

struct Money : public MelonCore::Component {
    unsigned int value;
};

class GroupSystem : public MelonCore::SystemBase {
   protected:
    class GroupChunkTask : public MelonCore::ChunkTask {
       public:
        GroupChunkTask(const unsigned int& moneyComponentId, const unsigned int& groupSharedComponentId) : _moneyComponentId(moneyComponentId), _groupSharedComponentId(groupSharedComponentId) {}
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
            Money* moneys = chunkAccessor.componentArray<Money>(_moneyComponentId);
            const Group* group = chunkAccessor.sharedComponent<Group>(_groupSharedComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                Money& money = moneys[i];
                money.value += group->salary;
            }
        }

        const unsigned int& _moneyComponentId;
        const unsigned int& _groupSharedComponentId;
    };

    void onEnter() override {
        MelonCore::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<Money>().markSharedComponents<Group>().createArchetype();

        std::array<MelonCore::Entity, 1024> entities;
        for (unsigned int i = 0; i < entities.size(); i++)
            entities[i] = entityManager()->createEntity(archetype);

        for (unsigned int i = 0; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{.id = 0, .salary = 10});

        for (unsigned int i = 1; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{.id = 1, .salary = 100});

        for (unsigned int i = 2; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{.id = 2, .salary = 1000});

        _entityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Money>().requireSharedComponents<Group>().createEntityFilter();
        _moneyComponentId = entityManager()->componentId<Money>();
        _groupSharedComponentId = entityManager()->sharedComponentId<Group>();
    }

    void onUpdate() override {
        printf("Delta time : %f\n", MelonCore::Time::instance()->deltaTime());
        predecessor() = schedule(std::make_shared<GroupChunkTask>(_moneyComponentId, _groupSharedComponentId), _entityFilter, predecessor());
        if (_counter++ > 1000)
            MelonCore::Instance::instance()->quit();
    }

    void onExit() override {}

   private:
    MelonCore::EntityFilter _entityFilter;
    unsigned int _moneyComponentId;
    unsigned int _groupSharedComponentId;
    unsigned int _counter{};
};

int main() {
    MelonCore::Instance::instance()->registerSystem<GroupSystem>();
    MelonCore::Instance::instance()->start();
    return 0;
}
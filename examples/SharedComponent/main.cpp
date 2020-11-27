#include <MelonCore/Archetype.h>
#include <MelonCore/ChunkTask.h>
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
    unsigned int id;
    unsigned int value;

    bool operator==(const Group& other) const {
        return id == other.id && value == other.value;
    }
};

template <>
struct std::hash<Group> {
    std::size_t operator()(const Group& group) {
        return std::hash<unsigned int>()(group.id) ^ std::hash<unsigned int>()(group.value);
    }
};

struct Person : public MelonCore::Component {
    unsigned int value;
};

class GroupSystem : public MelonCore::SystemBase {
   protected:
    class GroupChunkTask : public MelonCore::ChunkTask {
       public:
        GroupChunkTask(const unsigned int& personComponentId, const unsigned int& groupSharedComponentId) : _personComponentId(personComponentId), _groupSharedComponentId(groupSharedComponentId) {}
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
            Person* people = chunkAccessor.componentArray<Person>(_personComponentId);
            const Group* group = chunkAccessor.sharedComponent<Group>(_groupSharedComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                Person& person = people[i];
                person.value += group->value;
            }
        }

        const unsigned int& _personComponentId;
        const unsigned int& _groupSharedComponentId;
    };

    void onEnter() override {
        MelonCore::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<Person>().markSharedComponents<Group>().createArchetype();

        std::array<MelonCore::Entity, 1024> entities;
        for (unsigned int i = 0; i < entities.size(); i++)
            entities[i] = entityManager()->createEntity(archetype);

        for (unsigned int i = 0; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{.id = 0, .value = 10});

        for (unsigned int i = 1; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{.id = 1, .value = 100});

        for (unsigned int i = 2; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{.id = 2, .value = 1000});

        _entityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Person>().requireSharedComponents<Group>().createEntityFilter();
        _personComponentId = entityManager()->componentId<Person>();
        _groupSharedComponentId = entityManager()->sharedComponentId<Group>();
    }

    void onUpdate() override {
        printf("Delta time : %f\n", MelonCore::Time::instance()->deltaTime());
        predecessor() = schedule(std::make_shared<GroupChunkTask>(_personComponentId, _groupSharedComponentId), _entityFilter, predecessor());
        if (_counter++ > 1000)
            MelonCore::Instance::instance()->quit();
    }

    void onExit() override {}

   private:
    MelonCore::EntityFilter _entityFilter;
    unsigned int _personComponentId;
    unsigned int _groupSharedComponentId;
    unsigned int _counter{};
};

int main() {
    MelonCore::Instance::instance()->registerSystem<GroupSystem>();
    MelonCore::Instance::instance()->start();
    return 0;
}
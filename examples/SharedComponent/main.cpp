#include <MelonCore/ChunkTask.h>
#include <MelonCore/Entity.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonCore/Translation.h>
#include <MelonCore/TypeMark.h>

#include <array>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <utility>

struct Group {
    unsigned int id;

    bool operator==(const Group& other) const {
        return id == other.id;
    }
};

template <>
struct std::hash<Group> {
    std::size_t operator()(const Group& group) {
        return std::hash<unsigned int>()(group.id);
    }
};

struct Person {
    unsigned int value;
};

class GroupSystem : public MelonCore::SystemBase {
   protected:
    class GroupChunkTask : public MelonCore::ChunkTask {
       public:
        GroupChunkTask(const unsigned int& personComponentId, const unsigned int& groupSharedComponentId) : _personComponentId(personComponentId), _groupSharedComponentId(groupSharedComponentId) {}
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
            MelonCore::Entity* entities = chunkAccessor.entityArray();
            Person* people = chunkAccessor.componentArray<Person>(_personComponentId);
            const Group* group = chunkAccessor.sharedComponent<Group>(_groupSharedComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                Person& person = people[i];
                person.value += i + entities[i].id;
            }
        }

        const unsigned int& _personComponentId;
        const unsigned int& _groupSharedComponentId;
    };

    void onEnter() override {
        std::array<MelonCore::Entity, 1024> entities;
        for (unsigned int i = 0; i < entities.size(); i++)
            entities[i] = entityManager()->createEntity(MelonCore::TypeMark<Person>(), MelonCore::TypeMark<Group>());

        for (unsigned int i = 0; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{0});

        for (unsigned int i = 1; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{1});

        for (unsigned int i = 2; i < entities.size(); i += 3)
            entityManager()->setSharedComponent(entities[i], Group{2});

        _entityFilter = entityManager()->createEntityFilterBuilder().withComponents<Person>().withSharedComponents<Group>().createEntityFilter();
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
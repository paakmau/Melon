#include <MelonCore/Archetype.h>
#include <MelonCore/ChunkTask.h>
#include <MelonCore/Entity.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonCore/Translation.h>

#include <array>
#include <cstdio>
#include <memory>

struct Speed : public MelonCore::Component {
    unsigned int value;
};

class SpeedSystem : public MelonCore::SystemBase {
   protected:
    class SpeedChunkTask : public MelonCore::ChunkTask {
       public:
        SpeedChunkTask(const unsigned int& speedComponentId, const unsigned int& translationComponentId) : _speedComponentId(speedComponentId), _translationComponentId(translationComponentId) {}
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
            const MelonCore::Entity* entities = chunkAccessor.entityArray();
            Speed* feet = chunkAccessor.componentArray<Speed>(_speedComponentId);
            MelonCore::Translation* translations = chunkAccessor.componentArray<MelonCore::Translation>(_translationComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                const MelonCore::Entity& entity = entities[i];
                Speed& speed = feet[i];
                if (speed.value == entity.id)
                    translations[i].value += glm::vec3(0, 0, speed.value);
            }
        }

        const unsigned int& _speedComponentId;
        const unsigned int& _translationComponentId;
    };

    void onEnter() override {
        MelonCore::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<Speed>().createArchetype();

        std::array<MelonCore::Entity, 1024> entities;
        for (unsigned int i = 0; i < entities.size(); i++)
            entities[i] = entityManager()->createEntity(archetype);
        for (unsigned int i = 0; i * 3 < entities.size(); i++) {
            entityManager()->setComponent(entities[i], Speed{.value = entities[i].id});
            entityManager()->addComponent(entities[i], MelonCore::Translation{.value = glm::vec3(i % 10 + 10, i % 10 + 20, i % 10 + 30)});
        }
        for (unsigned int i = 0; i * 3 < entities.size(); i++) {
            entityManager()->setComponent(entities[i], Speed{.value = entities[i].id + 10U});
            entityManager()->addComponent(entities[i], MelonCore::Translation{.value = glm::vec3(i % 10 + 10, i % 10 + 20, i % 10 + 30)});
        }

        _entityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Speed, MelonCore::Translation>().createEntityFilter();
        _speedComponentId = entityManager()->componentId<Speed>();
        _translationComponentId = entityManager()->componentId<MelonCore::Translation>();
    }

    void onUpdate() override {
        printf("Delta time : %f\n", MelonCore::Time::instance()->deltaTime());
        predecessor() = schedule(std::make_shared<SpeedChunkTask>(_speedComponentId, _translationComponentId), _entityFilter, predecessor());
        if (_counter++ > 1000)
            MelonCore::Instance::instance()->quit();
    }

    void onExit() override {}

   private:
    MelonCore::EntityFilter _entityFilter;
    unsigned int _speedComponentId;
    unsigned int _translationComponentId;
    unsigned int _counter{};
};

int main() {
    MelonCore::Instance::instance()->registerSystem<SpeedSystem>();
    MelonCore::Instance::instance()->start();
    return 0;
}
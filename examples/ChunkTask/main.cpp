#include <libMelonCore/Archetype.h>
#include <libMelonCore/Entity.h>
#include <libMelonCore/Instance.h>
#include <libMelonCore/SystemBase.h>
#include <libMelonCore/Time.h>
#include <libMelonCore/Translation.h>

#include <array>
#include <cstdio>
#include <memory>

struct Speed : public Melon::Component {
    unsigned int value;
};

class SpeedSystem : public Melon::SystemBase {
  protected:
    class SpeedChunkTask : public Melon::ChunkTask {
      public:
        SpeedChunkTask(const unsigned int& speedComponentId, const unsigned int& translationComponentId) : _speedComponentId(speedComponentId), _translationComponentId(translationComponentId) {}
        virtual void execute(const Melon::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
            Speed* feet = chunkAccessor.componentArray<Speed>(_speedComponentId);
            Melon::Translation* translations = chunkAccessor.componentArray<Melon::Translation>(_translationComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                Speed& speed = feet[i];
                translations[i].value += glm::vec3(0, 0, speed.value);
            }
        }

        const unsigned int& _speedComponentId;
        const unsigned int& _translationComponentId;
    };

    void onEnter() override {
        Melon::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<Speed>().createArchetype();

        std::array<Melon::Entity, 1024> entities;
        for (unsigned int i = 0; i < entities.size(); i++)
            entities[i] = entityManager()->createEntity(archetype);
        for (unsigned int i = 0; i * 3 < entities.size(); i++) {
            entityManager()->setComponent(entities[i], Speed{.value = i % 10});
            entityManager()->addComponent(entities[i], Melon::Translation{.value = glm::vec3(i % 10 + 10, i % 10 + 20, i % 10 + 30)});
        }
        for (unsigned int i = 0; i * 3 < entities.size(); i++) {
            entityManager()->setComponent(entities[i], Speed{.value = i % 10 + 10U});
            entityManager()->addComponent(entities[i], Melon::Translation{.value = glm::vec3(i % 10 + 10, i % 10 + 20, i % 10 + 30)});
        }

        _entityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Speed, Melon::Translation>().createEntityFilter();
        _speedComponentId = entityManager()->componentId<Speed>();
        _translationComponentId = entityManager()->componentId<Melon::Translation>();
    }

    void onUpdate() override {
        printf("Delta time : %f\n", time()->deltaTime());
        predecessor() = schedule(std::make_shared<SpeedChunkTask>(_speedComponentId, _translationComponentId), _entityFilter, predecessor());
        if (_counter++ > 1000)
            instance()->quit();
    }

    void onExit() override {}

  private:
    Melon::EntityFilter _entityFilter;
    unsigned int _speedComponentId;
    unsigned int _translationComponentId;
    unsigned int _counter{};
};

int main() {
    Melon::Instance instance;
    instance.registerSystem<SpeedSystem>();
    instance.start();
    return 0;
}
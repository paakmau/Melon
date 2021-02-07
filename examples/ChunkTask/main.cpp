#include <MelonCore/Archetype.h>
#include <MelonCore/Entity.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonCore/Translation.h>

#include <array>
#include <cstdio>
#include <memory>

struct Speed : public Melon::DataComponent {
    unsigned int value;
};

class SpeedSystem : public Melon::SystemBase {
  protected:
    class SpeedChunkTask : public Melon::ChunkTask {
      public:
        SpeedChunkTask(const unsigned int& speedComponentId, const unsigned int& translationComponentId) : m_SpeedComponentId(speedComponentId), m_TranslationComponentId(translationComponentId) {}
        virtual void execute(const Melon::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
            Speed* feet = chunkAccessor.componentArray<Speed>(m_SpeedComponentId);
            Melon::Translation* translations = chunkAccessor.componentArray<Melon::Translation>(m_TranslationComponentId);
            for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
                Speed& speed = feet[i];
                translations[i].value += glm::vec3(0, 0, speed.value);
            }
        }

        const unsigned int& m_SpeedComponentId;
        const unsigned int& m_TranslationComponentId;
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

        m_EntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Speed, Melon::Translation>().createEntityFilter();
        m_SpeedComponentId = entityManager()->componentId<Speed>();
        m_TranslationComponentId = entityManager()->componentId<Melon::Translation>();
    }

    void onUpdate() override {
        printf("Delta time : %f\n", time()->deltaTime());
        predecessor() = schedule(std::make_shared<SpeedChunkTask>(m_SpeedComponentId, m_TranslationComponentId), m_EntityFilter, predecessor());
        if (m_Counter++ > 1000)
            instance()->quit();
    }

    void onExit() override {}

  private:
    Melon::EntityFilter m_EntityFilter;
    unsigned int m_SpeedComponentId;
    unsigned int m_TranslationComponentId;
    unsigned int m_Counter{};
};

int main() {
    Melon::Instance()
        .registerSystem<SpeedSystem>()
        .start();
    return 0;
}

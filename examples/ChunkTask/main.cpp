#include <MelonCore/ChunkTask.h>
#include <MelonCore/Entity.h>
#include <MelonCore/EntityFilter.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonCore/Translation.h>

#include <cstdio>
#include <vector>

struct Foot {
    int speed;
};

class ChunkTaskSystem : public MelonCore::SystemBase {
   protected:
    class FootChunkTask : public MelonCore::ChunkTask {
       public:
        FootChunkTask(unsigned int footComponentId, unsigned int translationComponentId) : _footComponentId(footComponentId), _translationComponentId(translationComponentId) {}
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
            MelonCore::Entity* entities = chunkAccessor.entityArray();
            Foot* feet = chunkAccessor.componentArray<Foot>(_footComponentId);
            MelonCore::Translation* translations = chunkAccessor.componentArray<MelonCore::Translation>(_translationComponentId);
            for (int i = 0; i < chunkAccessor.entityCount(); i++) {
                MelonCore::Entity entity = entities[i];
                Foot& foot = feet[i];
                translations[i].value += glm::vec3(0, 0, foot.speed);
            }
        }

        unsigned int _footComponentId;
        unsigned int _translationComponentId;
    };

    void onEnter() override {
        std::vector<MelonCore::Entity> entities;
        for (int i = 0; i < 1024; i++)
            entities.emplace_back(entityManager()->createEntity<Foot>());
        for (int i = 0; i * 3 < entities.size(); i++)
            entityManager()->addComponent(entities[i], MelonCore::Translation{});

        for (int i = 0; i * 3 < entities.size(); i++) {
            entityManager()->setComponent(entities[i], Foot{i % 10});
            entityManager()->setComponent(entities[i], MelonCore::Translation{glm::vec3(i % 10 + 10, i % 10 + 20, i % 10 + 30)});
        }

        _entityFilter = entityManager()->createEntityFilter<Foot, MelonCore::Translation>();
        _footComponentId = entityManager()->componentId<Foot>();
        _translationComponentId = entityManager()->componentId<MelonCore::Translation>();
    }

    void onUpdate() override {
        printf("Delta time : %f\n", MelonCore::Time::instance()->deltaTime());
        predecessor() = schedule(std::make_shared<FootChunkTask>(_footComponentId, _translationComponentId), _entityFilter, predecessor());
        if (_counter++ > 1000)
            MelonCore::Instance::instance()->quit();
    }

    void onExit() override {}

   private:
    MelonCore::EntityFilter _entityFilter;
    unsigned int _footComponentId;
    unsigned int _translationComponentId;
    unsigned int _counter{};
};

int main() {
    MelonCore::Instance::instance()->registerSystem<ChunkTaskSystem>();
    MelonCore::Instance::instance()->start();
    return 0;
}
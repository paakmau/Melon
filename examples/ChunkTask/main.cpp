#include <MelonCore/ChunkTask.h>
#include <MelonCore/Entity.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>

#include <cstdio>
#include <vector>

struct Foot {
    int speed;
};

struct Translation {
    int x;
    int y;
    int z;
};

class ChunkTaskSystem : public MelonCore::SystemBase {
   public:
    class FootChunkTask : public MelonCore::ChunkTask {
       public:
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor) override {
            MelonCore::Entity* entities = chunkAccessor.entityArray();
            Foot* feet = chunkAccessor.componentArray<Foot>(_footComponentId);
            for (int i = 0; i < chunkAccessor.entityCount(); i++) {
                MelonCore::Entity entity = entities[i];
                Foot& foot = feet[i];
                int speed = foot.speed;
                foot.speed += 1;
            }
        }
        FootChunkTask(unsigned int footComponentId) : _footComponentId(footComponentId) {}
        unsigned int _footComponentId;
    };

    void onEnter() override {
        std::vector<MelonCore::Entity> entities;
        for (int i = 0; i < 1024; i++)
            entities.emplace_back(entityManager()->createEntity<Foot>());
        for (int i = 0; i * 3 < entities.size(); i++)
            entityManager()->addComponent(entities[i], Translation{});

        for (int i = 0; i * 3 < entities.size(); i++) {
            entityManager()->setComponent(entities[i], Foot{i % 10});
            entityManager()->setComponent(entities[i], Translation{i % 10 + 10, i % 10 + 20, i % 10 + 30});
        }
        _footComponentId = entityManager()->componentId<Foot>();
    }

    void onUpdate() override {
        printf("Delta time : %f\n", MelonCore::Time::instance()->deltaTime());
        predecessor() = schedule(std::make_shared<FootChunkTask>(_footComponentId), entityManager()->createEntityFilter<Foot>(), predecessor());
    }

    void onExit() override {}

    unsigned int _footComponentId;
};

int main() {
    MelonCore::Instance instance;
    instance.registerSystem<ChunkTaskSystem>();
    instance.startGame();
    return 0;
}
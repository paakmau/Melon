#include <MelonCore/ArchetypeMask.h>
#include <MelonCore/EntityCommandBufferChunkTask.h>
#include <MelonCore/Instance.h>
#include <MelonCore/Rotation.h>
#include <MelonCore/Scale.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonCore/Translation.h>
#include <MelonFrontend/RenderMesh.h>
#include <MelonFrontend/RenderSystem.h>
#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

#include <cstdio>
#include <glm/vec3.hpp>
#include <memory>

struct DestructionTime : public MelonCore::Component {
    float value;
};

struct RotationSpeed : public MelonCore::Component {
    float value;
};

class RotationSystem : public MelonCore::SystemBase {
   protected:
    class RotationTask : public MelonCore::EntityCommandBufferChunkTask {
       public:
        RotationTask(const unsigned int& rotationComponentId, const unsigned int& rotationSpeedComponentId, const unsigned int& destructionTimeComponentId, const float& deltaTime) : _rotationComponentId(rotationComponentId), _rotationSpeedComponentId(rotationSpeedComponentId), _destructionTimeComponentId(destructionTimeComponentId), _deltaTime(deltaTime) {}
        virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, MelonCore::EntityCommandBuffer* entityCommandBuffer) override {
            const MelonCore::Entity* entities = chunkAccessor.entityArray();
            MelonCore::Rotation* rotations = chunkAccessor.componentArray<MelonCore::Rotation>(_rotationComponentId);
            RotationSpeed* rotationSpeeds = chunkAccessor.componentArray<RotationSpeed>(_rotationSpeedComponentId);
            DestructionTime* destructionTimes = chunkAccessor.componentArray<DestructionTime>(_destructionTimeComponentId);
            for (int i = 0; i < chunkAccessor.entityCount(); i++) {
                rotations[i].value = glm::rotate(rotations[i].value, glm::radians(_deltaTime * rotationSpeeds[i].value), glm::vec3(0.0f, 0.0f, 1.0f));
                destructionTimes[i].value -= _deltaTime;
                if (destructionTimes[i].value <= 0.0f)
                    entityCommandBuffer->destroyEntity(entities[i]);
            }
        }

        const unsigned int& _rotationComponentId;
        const unsigned int& _rotationSpeedComponentId;
        const unsigned int& _destructionTimeComponentId;

        float _deltaTime;
    };

    virtual void onEnter() override {
        MelonCore::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<MelonCore::Translation, MelonCore::Rotation, MelonCore::Scale, RotationSpeed, DestructionTime>().markSharedComponents<MelonFrontend::RenderMesh>().createArchetype();
        MelonFrontend::RenderMesh mesh = MelonFrontend::RenderMesh{
            .vertices{
                {.position = {-0.5f, -0.5f, 0.0f}},
                {.position = {0.5f, -0.5f, 0.0f}},
                {.position = {0.5f, 0.5f, 0.0f}},
                {.position = {-0.5f, 0.5f, 0.0f}}},
            .indices{0, 1, 2, 2, 3, 0}};
        for (int i = 0; i < 2; i++) {
            MelonCore::Entity entity = entityManager()->createEntity(archetype);
            entityManager()->setComponent<MelonCore::Translation>(entity, MelonCore::Translation{.value = glm::vec3(i * 2.0f - 1.0f, 0.0f, 0.0f)});
            entityManager()->setComponent<MelonCore::Rotation>(entity, MelonCore::Rotation{.value = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)))});
            entityManager()->setComponent<MelonCore::Scale>(entity, MelonCore::Scale{.value = glm::vec3(1.0f, 1.0f, 1.0f)});
            entityManager()->setComponent<RotationSpeed>(entity, RotationSpeed{.value = 10.0f});
            entityManager()->setComponent<DestructionTime>(entity, DestructionTime{.value = 5.0f});
            entityManager()->setSharedComponent<MelonFrontend::RenderMesh>(entity, mesh);
        }

        _rotationEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<MelonCore::Rotation, RotationSpeed, DestructionTime>().createEntityFilter();
        _rotationComponentId = entityManager()->componentId<MelonCore::Rotation>();
        _rotationSpeedComponentId = entityManager()->componentId<RotationSpeed>();
        _destructionTimeComponentId = entityManager()->componentId<DestructionTime>();
    }

    virtual void onUpdate() override {
        std::shared_ptr<RotationTask> rotationChunkTask = std::make_shared<RotationTask>(_rotationComponentId, _rotationSpeedComponentId, _destructionTimeComponentId, MelonCore::Time::instance()->deltaTime());
        predecessor() = schedule(rotationChunkTask, _rotationEntityFilter, predecessor());
    }

    virtual void onExit() override {}

    MelonCore::EntityFilter _rotationEntityFilter;
    unsigned int _rotationComponentId;
    unsigned int _rotationSpeedComponentId;
    unsigned int _destructionTimeComponentId;
};

int main() {
    MelonCore::Instance::instance()->applicationName() = "Rotation";
    MelonCore::Instance::instance()->registerSystem<MelonFrontend::RenderSystem>(800, 600);
    MelonCore::Instance::instance()->registerSystem<RotationSystem>();
    MelonCore::Instance::instance()->start();
    return 0;
}
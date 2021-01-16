#include <libMelonCore/ArchetypeMask.h>
#include <libMelonCore/EntityCommandBufferChunkTask.h>
#include <libMelonCore/Instance.h>
#include <libMelonCore/Rotation.h>
#include <libMelonCore/Scale.h>
#include <libMelonCore/SystemBase.h>
#include <libMelonCore/Time.h>
#include <libMelonCore/Translation.h>
#include <libMelonFrontend/Camera.h>
#include <libMelonFrontend/RenderMesh.h>
#include <libMelonFrontend/RenderSystem.h>
#include <libMelonTask/TaskHandle.h>
#include <libMelonTask/TaskManager.h>

#include <cstdio>
#include <glm/vec3.hpp>
#include <memory>

struct DestructionTime : public Melon::Component {
    float value;
};

struct RotationSpeed : public Melon::Component {
    float value;
};

class RotationSystem : public Melon::SystemBase {
  protected:
    class RotationTask : public Melon::EntityCommandBufferChunkTask {
      public:
        RotationTask(const unsigned int& rotationComponentId, const unsigned int& rotationSpeedComponentId, const unsigned int& destructionTimeComponentId, const float& deltaTime) : _rotationComponentId(rotationComponentId), _rotationSpeedComponentId(rotationSpeedComponentId), _destructionTimeComponentId(destructionTimeComponentId), _deltaTime(deltaTime) {}
        virtual void execute(const Melon::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, Melon::EntityCommandBuffer* entityCommandBuffer) override {
            const Melon::Entity* entities = chunkAccessor.entityArray();
            Melon::Rotation* rotations = chunkAccessor.componentArray<Melon::Rotation>(_rotationComponentId);
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
        // Create RenderMeshes
        Melon::Archetype* archetype = entityManager()->createArchetypeBuilder().markComponents<Melon::Translation, Melon::Rotation, Melon::Scale, RotationSpeed, DestructionTime>().markSharedComponents<Melon::RenderMesh>().createArchetype();
        Melon::RenderMesh mesh = Melon::RenderMesh{
            .vertices{
                {.position = {-0.5f, -0.5f, 0.0f}},
                {.position = {0.5f, -0.5f, 0.0f}},
                {.position = {0.5f, 0.5f, 0.0f}},
                {.position = {-0.5f, 0.5f, 0.0f}}},
            .indices{0, 1, 2, 2, 3, 0}};
        for (int i = 0; i < 2; i++) {
            Melon::Entity entity = entityManager()->createEntity(archetype);
            entityManager()->setComponent<Melon::Translation>(entity, Melon::Translation{.value = glm::vec3(i * 2.0f - 1.0f, 0.0f, 0.0f)});
            entityManager()->setComponent<Melon::Rotation>(entity, Melon::Rotation{.value = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)))});
            entityManager()->setComponent<Melon::Scale>(entity, Melon::Scale{.value = glm::vec3(1.0f, 1.0f, 1.0f)});
            entityManager()->setComponent<RotationSpeed>(entity, RotationSpeed{.value = 10.0f});
            entityManager()->setComponent<DestructionTime>(entity, DestructionTime{.value = 5.0f});
            entityManager()->setSharedComponent<Melon::RenderMesh>(entity, mesh);
        }

        // Create a Camera
        Melon::Entity cameraEntity = entityManager()->createEntity();
        entityManager()->addComponent<Melon::Translation>(cameraEntity, Melon::Translation{.value = glm::vec3(2.0f, 2.0f, 2.0f)});
        entityManager()->addComponent<Melon::Rotation>(cameraEntity, Melon::Rotation{.value = glm::quatLookAt(glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f)), glm::vec3(0.0f, 0.0f, 1.0f))});
        entityManager()->addComponent<Melon::Camera>(cameraEntity, Melon::Camera());
        entityManager()->addComponent<Melon::PerspectiveProjection>(cameraEntity, Melon::PerspectiveProjection{.fov = 45.0f, .near = 0.1f, .far = 10.0f});

        _rotationEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Melon::Rotation, RotationSpeed, DestructionTime>().createEntityFilter();
        _rotationComponentId = entityManager()->componentId<Melon::Rotation>();
        _rotationSpeedComponentId = entityManager()->componentId<RotationSpeed>();
        _destructionTimeComponentId = entityManager()->componentId<DestructionTime>();
    }

    virtual void onUpdate() override {
        std::shared_ptr<RotationTask> rotationChunkTask = std::make_shared<RotationTask>(_rotationComponentId, _rotationSpeedComponentId, _destructionTimeComponentId, time()->deltaTime());
        predecessor() = schedule(rotationChunkTask, _rotationEntityFilter, predecessor());
    }

    virtual void onExit() override {}

    Melon::EntityFilter _rotationEntityFilter;
    unsigned int _rotationComponentId;
    unsigned int _rotationSpeedComponentId;
    unsigned int _destructionTimeComponentId;
};

int main() {
    Melon::Instance instance;
    instance.applicationName() = "RenderMesh";
    instance.registerSystem<Melon::RenderSystem>(800, 600);
    instance.registerSystem<RotationSystem>();
    instance.start();
    return 0;
}
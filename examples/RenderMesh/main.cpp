#include <MelonCore/ArchetypeMask.h>
#include <MelonCore/EntityCommandBufferChunkTask.h>
#include <MelonCore/Instance.h>
#include <MelonCore/Rotation.h>
#include <MelonCore/Scale.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonCore/Translation.h>
#include <MelonFrontend/Camera.h>
#include <MelonFrontend/MeshResource.h>
#include <MelonFrontend/RenderMesh.h>
#include <MelonFrontend/RenderSystem.h>
#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

#include <cstdio>
#include <glm/vec3.hpp>
#include <memory>

struct DestructionTime : public Melon::DataComponent {
    float value;
};

struct RotationSpeed : public Melon::DataComponent {
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
        resourceManager()->addResource(Melon::MeshResource::create("mesh.obj"));
        Melon::RenderMesh mesh = Melon::RenderMesh{.meshResource = static_cast<Melon::MeshResource*>(resourceManager()->resource("mesh.obj"))};
        for (int i = 0; i < 2; i++) {
            Melon::Entity entity = entityManager()->createEntity(archetype);
            entityManager()->setComponent<Melon::Translation>(entity, Melon::Translation{.value = glm::vec3(i * 4.0f - 2.0f, 0.0f, 0.0f)});
            entityManager()->setComponent<Melon::Rotation>(entity, Melon::Rotation{.value = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)))});
            entityManager()->setComponent<Melon::Scale>(entity, Melon::Scale{.value = glm::vec3(1.0f, 1.0f, 1.0f)});
            entityManager()->setComponent<RotationSpeed>(entity, RotationSpeed{.value = 40.0f});
            entityManager()->setComponent<DestructionTime>(entity, DestructionTime{.value = 10.0f});
            entityManager()->setSharedComponent<Melon::RenderMesh>(entity, mesh);
        }

        // Create a Camera
        Melon::Entity cameraEntity = entityManager()->createEntity();
        entityManager()->addComponent<Melon::Translation>(cameraEntity, Melon::Translation{.value = glm::vec3(5.0f, 5.0f, 5.0f)});
        entityManager()->addComponent<Melon::Rotation>(cameraEntity, Melon::Rotation{.value = glm::quatLookAt(glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f)), glm::vec3(0.0f, 0.0f, 1.0f))});
        entityManager()->addComponent<Melon::Camera>(cameraEntity, Melon::Camera());
        entityManager()->addComponent<Melon::PerspectiveProjection>(cameraEntity, Melon::PerspectiveProjection{.fovy = 45.0f, .zNear = 0.1f, .zFar = 100.0f});

        // Create a Light
        Melon::Entity lightEntity = entityManager()->createEntity();
        entityManager()->addComponent<Melon::Light>(lightEntity, Melon::Light{.direction = glm::normalize(glm::vec3(-3.0f, -4.0f, -5.0f))});

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
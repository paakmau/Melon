#include <MelonCore/Combination.h>
#include <MelonCore/EntityManager.h>
#include <MelonCore/Instance.h>
#include <MelonCore/Rotation.h>
#include <MelonCore/Scale.h>
#include <MelonCore/Translation.h>
#include <MelonFrontend/Camera.h>
#include <MelonFrontend/Engine.h>
#include <MelonFrontend/RenderSystem.h>
#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

namespace Melon {

class CreatedRenderMeshTask : public ChunkTask {
  public:
    CreatedRenderMeshTask(const unsigned int& renderMeshComponentId, std::vector<Entity>& entities, std::vector<unsigned int>& renderMeshIndices) : m_RenderMeshComponentId(renderMeshComponentId), m_Entities(entities), m_RenderMeshIndices(renderMeshIndices) {}
    virtual void execute(const ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
        for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
            m_Entities[firstEntityIndex + i] = chunkAccessor.entityArray()[i];
            m_RenderMeshIndices[firstEntityIndex + i] = chunkAccessor.sharedComponentIndex(m_RenderMeshComponentId);
        }
    }

    const unsigned int& m_RenderMeshComponentId;

    std::vector<Entity>& m_Entities;
    std::vector<unsigned int>& m_RenderMeshIndices;
};

class RenderTask : public ChunkTask {
  public:
    RenderTask(std::vector<glm::mat4>& models, std::vector<const ManualRenderMesh*>& manualRenderMeshes, const unsigned int& translationComponentId, const unsigned int& rotationComponentId, const unsigned int& scaleComponentId, const unsigned int& manualRenderMeshComponentId) : m_Models(models), m_ManualRenderMeshes(manualRenderMeshes), m_TranslationComponentId(translationComponentId), m_RotationComponentId(rotationComponentId), m_ScaleComponentId(scaleComponentId), m_ManualRenderMeshComponentId(manualRenderMeshComponentId){};

    virtual void execute(const ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
        Translation* translations = chunkAccessor.componentArray<Translation>(m_TranslationComponentId);
        Rotation* rotations = chunkAccessor.componentArray<Rotation>(m_RotationComponentId);
        Scale* scales = chunkAccessor.componentArray<Scale>(m_ScaleComponentId);
        const ManualRenderMesh* manualRenderMesh = chunkAccessor.sharedComponent<ManualRenderMesh>(m_ManualRenderMeshComponentId);
        for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
            glm::mat4 model = glm::scale(glm::mat4(1.0f), scales[i].value);
            model = glm::mat4_cast(rotations[i].value) * model;
            model = glm::translate(model, translations[i].value);

            m_Models[firstEntityIndex + i] = model;
            m_ManualRenderMeshes[firstEntityIndex + i] = manualRenderMesh;
        }
    }

    std::vector<glm::mat4>& m_Models;
    std::vector<const ManualRenderMesh*>& m_ManualRenderMeshes;

    const unsigned int& m_TranslationComponentId;
    const unsigned int& m_RotationComponentId;
    const unsigned int& m_ScaleComponentId;
    const unsigned int& m_ManualRenderMeshComponentId;
};

class DestroyedRenderMeshTask : public ChunkTask {
  public:
    DestroyedRenderMeshTask(const unsigned int& manualRenderMeshComponentId, std::vector<Entity>& entities, std::vector<unsigned int>& manualRenderMeshIndices) : m_ManualRenderMeshComponentId(manualRenderMeshComponentId), m_Entities(entities), m_ManualRenderMeshIndices(manualRenderMeshIndices) {}
    virtual void execute(const ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
        for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
            m_Entities[firstEntityIndex + i] = chunkAccessor.entityArray()[i];
            m_ManualRenderMeshIndices[firstEntityIndex + i] = chunkAccessor.sharedComponentIndex(m_ManualRenderMeshComponentId);
        }
    }

    const unsigned int& m_ManualRenderMeshComponentId;

    std::vector<Entity>& m_Entities;
    std::vector<unsigned int>& m_ManualRenderMeshIndices;
};

RenderSystem::RenderSystem(const unsigned int& width, const unsigned int& height) : SystemBase(), m_CurrentWidth(width), m_CurrentHeight(height) {}

RenderSystem::~RenderSystem() {}

void RenderSystem::onEnter() {
    m_Engine.initialize(taskManager(), instance()->applicationName(), m_CurrentWidth, m_CurrentHeight);

    m_CreatedRenderMeshEntityFilter = entityManager()->createEntityFilterBuilder().requireSharedComponents<RenderMesh>().rejectSharedComponents<ManualRenderMesh>().createEntityFilter();
    m_RenderMeshEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Translation, Rotation, Scale>().requireSharedComponents<RenderMesh, ManualRenderMesh>().createEntityFilter();
    m_DestroyedRenderMeshEntityFilter = entityManager()->createEntityFilterBuilder().requireSharedComponents<ManualRenderMesh>().rejectSharedComponents<RenderMesh>().createEntityFilter();
    m_CameraEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<Translation, Rotation, Camera, PerspectiveProjection>().createEntityFilter();

    m_TranslationComponentId = entityManager()->componentId<Translation>();
    m_RotationComponentId = entityManager()->componentId<Rotation>();
    m_ScaleComponentId = entityManager()->componentId<Scale>();
    m_PerspectiveProjectionComponentId = entityManager()->componentId<PerspectiveProjection>();
    m_RenderMeshComponentId = entityManager()->sharedComponentId<RenderMesh>();
    m_ManualRenderMeshComponentId = entityManager()->sharedComponentId<ManualRenderMesh>();
}

void RenderSystem::onUpdate() {
    // CreatedRenderMeshTask
    const unsigned int createdRenderMeshCount = entityManager()->entityCount(m_CreatedRenderMeshEntityFilter);
    std::vector<Entity> createdRenderMeshEntities(createdRenderMeshCount);
    std::vector<unsigned int> createdRenderMeshIndices(createdRenderMeshCount);
    std::shared_ptr<TaskHandle> createdRenderMeshTaskHandle = schedule(std::make_shared<CreatedRenderMeshTask>(m_RenderMeshComponentId, createdRenderMeshEntities, createdRenderMeshIndices), m_CreatedRenderMeshEntityFilter, predecessor());

    // DestroyedRenderMeshTask
    const unsigned int destroyedRenderMeshCount = entityManager()->entityCount(m_DestroyedRenderMeshEntityFilter);
    std::vector<Entity> manualRenderMeshEntities(destroyedRenderMeshCount);
    std::vector<unsigned int> manualRenderMeshIndices(destroyedRenderMeshCount);
    std::shared_ptr<TaskHandle> destroyedRenderMeshTaskHandle = schedule(std::make_shared<DestroyedRenderMeshTask>(m_ManualRenderMeshComponentId, manualRenderMeshEntities, manualRenderMeshIndices), m_DestroyedRenderMeshEntityFilter, predecessor());

    // RenderTask
    const unsigned int renderMeshCount = entityManager()->entityCount(m_RenderMeshEntityFilter);
    std::vector<glm::mat4> models(renderMeshCount);
    std::vector<const ManualRenderMesh*> manualRenderMeshes(renderMeshCount);
    std::shared_ptr<TaskHandle> renderMeshTaskHandle = schedule(std::make_shared<RenderTask>(models, manualRenderMeshes, m_TranslationComponentId, m_RotationComponentId, m_ScaleComponentId, m_ManualRenderMeshComponentId), m_RenderMeshEntityFilter, predecessor());

    taskManager()->activateWaitingTasks();

    // Fetch components of a Camera. If not found, use a default Camera
    std::vector<ChunkAccessor> accessors = entityManager()->filterEntities(m_CameraEntityFilter);
    glm::vec3 cameraTranslation(0.0f, 0.0f, 0.0f);
    glm::quat cameraRotation = glm::quatLookAt(glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), m_Engine.windowAspectRatio(), 0.1f, 10.0f);
    projection[1][1] *= -1;
    for (auto accessor : accessors)
        for (unsigned int i = 0; i < accessor.entityCount(); i++) {
            cameraTranslation = accessor.componentArray<Translation>(m_TranslationComponentId)[i].value;
            cameraRotation = accessor.componentArray<Rotation>(m_RotationComponentId)[i].value;
            PerspectiveProjection perspectiveProjection = accessor.componentArray<PerspectiveProjection>(m_PerspectiveProjectionComponentId)[i];
            projection = glm::perspective(glm::radians(perspectiveProjection.fovy), m_Engine.windowAspectRatio(), perspectiveProjection.zNear, perspectiveProjection.zFar);
            projection[1][1] *= -1;
        }

    m_Engine.beginFrame();

    // Create ManualRenderMeshes
    createdRenderMeshTaskHandle->complete();
    for (unsigned int i = 0; i < createdRenderMeshIndices.size(); i++) {
        const unsigned int& createdRenderMeshIndex = createdRenderMeshIndices[i];
        if (m_RenderMeshReferenceCountMap.contains(createdRenderMeshIndex))
            m_RenderMeshReferenceCountMap[createdRenderMeshIndex]++;
        else {
            m_RenderMeshReferenceCountMap[createdRenderMeshIndex] = 1;
            const RenderMesh* createdRenderMesh = entityManager()->sharedComponent<RenderMesh>(createdRenderMeshIndex);
            m_MeshBufferMap.emplace(createdRenderMeshIndex, m_Engine.createMeshBuffer(createdRenderMesh->vertices, createdRenderMesh->indices));
        }
        const MeshBuffer& meshBuffer = m_MeshBufferMap[createdRenderMeshIndex];
        entityManager()->addSharedComponent<ManualRenderMesh>(createdRenderMeshEntities[i], ManualRenderMesh{.renderMeshIndex = createdRenderMeshIndex, .meshBuffer = meshBuffer});
    }

    // Destroy ManualRenderMeshes
    destroyedRenderMeshTaskHandle->complete();
    for (unsigned int i = 0; i < manualRenderMeshIndices.size(); i++) {
        const unsigned int& manualRenderMeshIndex = manualRenderMeshIndices[i];
        const ManualRenderMesh* manualRenderMesh = entityManager()->sharedComponent<ManualRenderMesh>(manualRenderMeshIndex);
        m_RenderMeshReferenceCountMap[manualRenderMesh->renderMeshIndex]--;
        if (m_RenderMeshReferenceCountMap[manualRenderMesh->renderMeshIndex] == 0) {
            m_RenderMeshReferenceCountMap.erase(manualRenderMesh->renderMeshIndex);
            m_MeshBufferMap.erase(manualRenderMesh->renderMeshIndex);
            m_Engine.destroyMeshBuffer(manualRenderMesh->meshBuffer);
        }
        entityManager()->removeSharedComponent<ManualRenderMesh>(manualRenderMeshEntities[i]);
    }

    // Add batches
    renderMeshTaskHandle->complete();
    m_Engine.beginBatches();
    std::vector<glm::mat4> batchModels;
    for (unsigned int i = 0; i < renderMeshCount; i++) {
        batchModels.push_back(models[i]);
        while (i + 1 < renderMeshCount && manualRenderMeshes[i] == manualRenderMeshes[i + 1]) batchModels.push_back(models[++i]);
        m_Engine.addBatch(batchModels, manualRenderMeshes[i]->meshBuffer);
        batchModels.clear();
    }
    m_Engine.endBatches();

    // Draw frame
    m_Engine.renderFrame(projection, cameraTranslation, cameraRotation);

    m_Engine.endFrame();
    if (m_Engine.windowClosed())
        instance()->quit();
}

void RenderSystem::onExit() {
    for (const auto& [index, meshBuffer] : m_MeshBufferMap)
        m_Engine.destroyMeshBuffer(meshBuffer);
    m_Engine.terminate();
}

}  // namespace Melon

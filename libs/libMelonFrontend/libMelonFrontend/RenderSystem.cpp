#include <libMelonCore/Combination.h>
#include <libMelonCore/EntityManager.h>
#include <libMelonCore/Instance.h>
#include <libMelonCore/Rotation.h>
#include <libMelonCore/Scale.h>
#include <libMelonCore/Translation.h>
#include <libMelonFrontend/Engine.h>
#include <libMelonFrontend/RenderSystem.h>
#include <libMelonTask/TaskHandle.h>
#include <libMelonTask/TaskManager.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

namespace MelonFrontend {

class CreatedRenderMeshTask : public MelonCore::ChunkTask {
  public:
    CreatedRenderMeshTask(unsigned int const& renderMeshComponentId, std::vector<MelonCore::Entity>& entities, std::vector<unsigned int>& renderMeshIndices) : m_RenderMeshComponentId(renderMeshComponentId), m_Entities(entities), m_RenderMeshIndices(renderMeshIndices) {}
    virtual void execute(MelonCore::ChunkAccessor const& chunkAccessor, unsigned int const& chunkIndex, unsigned int const& firstEntityIndex) override {
        for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
            m_Entities[firstEntityIndex + i] = chunkAccessor.entityArray()[i];
            m_RenderMeshIndices[firstEntityIndex + i] = chunkAccessor.sharedComponentIndex(m_RenderMeshComponentId);
        }
    }

    unsigned int const& m_RenderMeshComponentId;

    std::vector<MelonCore::Entity>& m_Entities;
    std::vector<unsigned int>& m_RenderMeshIndices;
};

class RenderTask : public MelonCore::ChunkTask {
  public:
    RenderTask(std::vector<glm::mat4>& models, std::vector<ManualRenderMesh const*>& manualRenderMeshes, unsigned int const& translationComponentId, unsigned int const& rotationComponentId, unsigned int const& scaleComponentId, unsigned int const& manualRenderMeshComponentId) : m_Models(models), m_ManualRenderMeshes(manualRenderMeshes), m_TranslationComponentId(translationComponentId), m_RotationComponentId(rotationComponentId), m_ScaleComponentId(scaleComponentId), m_ManualRenderMeshComponentId(manualRenderMeshComponentId){};

    virtual void execute(MelonCore::ChunkAccessor const& chunkAccessor, unsigned int const& chunkIndex, unsigned int const& firstEntityIndex) override {
        MelonCore::Translation* translations = chunkAccessor.componentArray<MelonCore::Translation>(m_TranslationComponentId);
        MelonCore::Rotation* rotations = chunkAccessor.componentArray<MelonCore::Rotation>(m_RotationComponentId);
        MelonCore::Scale* scales = chunkAccessor.componentArray<MelonCore::Scale>(m_ScaleComponentId);
        ManualRenderMesh const* manualRenderMesh = chunkAccessor.sharedComponent<ManualRenderMesh>(m_ManualRenderMeshComponentId);
        for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
            glm::mat4 model = glm::scale(glm::mat4(1.0f), scales[i].value);
            model = glm::mat4_cast(rotations[i].value) * model;
            model = glm::translate(model, translations[i].value);

            m_Models[firstEntityIndex + i] = model;
            m_ManualRenderMeshes[firstEntityIndex + i] = manualRenderMesh;
        }
    }

    std::vector<glm::mat4>& m_Models;
    std::vector<ManualRenderMesh const*>& m_ManualRenderMeshes;

    unsigned int const& m_TranslationComponentId;
    unsigned int const& m_RotationComponentId;
    unsigned int const& m_ScaleComponentId;
    unsigned int const& m_ManualRenderMeshComponentId;
};

class DestroyedRenderMeshTask : public MelonCore::ChunkTask {
  public:
    DestroyedRenderMeshTask(unsigned int const& manualRenderMeshComponentId, std::vector<MelonCore::Entity>& entities, std::vector<unsigned int>& manualRenderMeshIndices) : m_ManualRenderMeshComponentId(manualRenderMeshComponentId), m_Entities(entities), m_ManualRenderMeshIndices(manualRenderMeshIndices) {}
    virtual void execute(MelonCore::ChunkAccessor const& chunkAccessor, unsigned int const& chunkIndex, unsigned int const& firstEntityIndex) override {
        for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
            m_Entities[firstEntityIndex + i] = chunkAccessor.entityArray()[i];
            m_ManualRenderMeshIndices[firstEntityIndex + i] = chunkAccessor.sharedComponentIndex(m_ManualRenderMeshComponentId);
        }
    }

    unsigned int const& m_ManualRenderMeshComponentId;

    std::vector<MelonCore::Entity>& m_Entities;
    std::vector<unsigned int>& m_ManualRenderMeshIndices;
};

RenderSystem::RenderSystem(unsigned int const& width, unsigned int const& height) : SystemBase(), m_CurrentWidth(width), m_CurrentHeight(height) {}

RenderSystem::~RenderSystem() {}

void RenderSystem::onEnter() {
    Engine::instance()->initialize(MelonCore::Instance::instance()->applicationName(), m_CurrentWidth, m_CurrentHeight);

    m_CreatedRenderMeshEntityFilter = entityManager()->createEntityFilterBuilder().requireSharedComponents<RenderMesh>().rejectSharedComponents<ManualRenderMesh>().createEntityFilter();
    m_RenderMeshEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<MelonCore::Translation, MelonCore::Rotation, MelonCore::Scale>().requireSharedComponents<RenderMesh, ManualRenderMesh>().createEntityFilter();
    m_DestroyedRenderMeshEntityFilter = entityManager()->createEntityFilterBuilder().requireSharedComponents<ManualRenderMesh>().rejectSharedComponents<RenderMesh>().createEntityFilter();

    m_TranslationComponentId = entityManager()->componentId<MelonCore::Translation>();
    m_RotationComponentId = entityManager()->componentId<MelonCore::Rotation>();
    m_ScaleComponentId = entityManager()->componentId<MelonCore::Scale>();
    m_RenderMeshComponentId = entityManager()->sharedComponentId<RenderMesh>();
    m_ManualRenderMeshComponentId = entityManager()->sharedComponentId<ManualRenderMesh>();
}

void RenderSystem::onUpdate() {
    // CreatedRenderMeshTask
    unsigned int const createdRenderMeshCount = entityManager()->entityCount(m_CreatedRenderMeshEntityFilter);
    std::vector<MelonCore::Entity> createdRenderMeshEntities(createdRenderMeshCount);
    std::vector<unsigned int> createdRenderMeshIndices(createdRenderMeshCount);
    std::shared_ptr<MelonTask::TaskHandle> createdRenderMeshTaskHandle = schedule(std::make_shared<CreatedRenderMeshTask>(m_RenderMeshComponentId, createdRenderMeshEntities, createdRenderMeshIndices), m_CreatedRenderMeshEntityFilter, predecessor());

    // DestroyedRenderMeshTask
    unsigned int const destroyedRenderMeshCount = entityManager()->entityCount(m_DestroyedRenderMeshEntityFilter);
    std::vector<MelonCore::Entity> manualRenderMeshEntities(destroyedRenderMeshCount);
    std::vector<unsigned int> manualRenderMeshIndices(destroyedRenderMeshCount);
    std::shared_ptr<MelonTask::TaskHandle> destroyedRenderMeshTaskHandle = schedule(std::make_shared<DestroyedRenderMeshTask>(m_ManualRenderMeshComponentId, manualRenderMeshEntities, manualRenderMeshIndices), m_DestroyedRenderMeshEntityFilter, predecessor());

    // RenderTask
    unsigned int const renderMeshCount = entityManager()->entityCount(m_RenderMeshEntityFilter);
    glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), Engine::instance()->windowAspectRatio(), 0.1f, 10.0f);
    projection[1][1] *= -1;
    std::vector<glm::mat4> models(renderMeshCount);
    std::vector<ManualRenderMesh const*> manualRenderMeshes(renderMeshCount);
    std::shared_ptr<MelonTask::TaskHandle> renderMeshTaskHandle = schedule(std::make_shared<RenderTask>(models, manualRenderMeshes, m_TranslationComponentId, m_RotationComponentId, m_ScaleComponentId, m_ManualRenderMeshComponentId), m_RenderMeshEntityFilter, predecessor());

    MelonTask::TaskManager::instance()->activateWaitingTasks();

    Engine::instance()->beginFrame();

    // Create ManualRenderMeshes
    createdRenderMeshTaskHandle->complete();
    for (unsigned int i = 0; i < createdRenderMeshIndices.size(); i++) {
        unsigned int const& createdRenderMeshIndex = createdRenderMeshIndices[i];
        if (m_RenderMeshReferenceCountMap.contains(createdRenderMeshIndex))
            m_RenderMeshReferenceCountMap[createdRenderMeshIndex]++;
        else {
            m_RenderMeshReferenceCountMap[createdRenderMeshIndex] = 1;
            RenderMesh const* createdRenderMesh = entityManager()->sharedComponent<RenderMesh>(createdRenderMeshIndex);
            m_MeshBufferMap.emplace(createdRenderMeshIndex, Engine::instance()->createMeshBuffer(createdRenderMesh->vertices, createdRenderMesh->indices));
        }
        MeshBuffer const& meshBuffer = m_MeshBufferMap[createdRenderMeshIndex];
        entityManager()->addSharedComponent<ManualRenderMesh>(createdRenderMeshEntities[i], ManualRenderMesh{.renderMeshIndex = createdRenderMeshIndex, .meshBuffer = meshBuffer});
    }

    // Destroy ManualRenderMeshes
    destroyedRenderMeshTaskHandle->complete();
    for (unsigned int i = 0; i < manualRenderMeshIndices.size(); i++) {
        unsigned int const& manualRenderMeshIndex = manualRenderMeshIndices[i];
        ManualRenderMesh const* manualRenderMesh = entityManager()->sharedComponent<ManualRenderMesh>(manualRenderMeshIndex);
        m_RenderMeshReferenceCountMap[manualRenderMesh->renderMeshIndex]--;
        if (m_RenderMeshReferenceCountMap[manualRenderMesh->renderMeshIndex] == 0) {
            m_RenderMeshReferenceCountMap.erase(manualRenderMesh->renderMeshIndex);
            m_MeshBufferMap.erase(manualRenderMesh->renderMeshIndex);
            Engine::instance()->destroyMeshBuffer(manualRenderMesh->meshBuffer);
        }
        entityManager()->removeSharedComponent<ManualRenderMesh>(manualRenderMeshEntities[i]);
    }

    // Add batches
    renderMeshTaskHandle->complete();
    Engine::instance()->beginBatches();
    std::vector<glm::mat4> batchModels;
    for (unsigned int i = 0; i < renderMeshCount; i++) {
        batchModels.push_back(models[i]);
        while (i + 1 < renderMeshCount && manualRenderMeshes[i] == manualRenderMeshes[i + 1]) batchModels.push_back(models[++i]);
        Engine::instance()->addBatch(batchModels, manualRenderMeshes[i]->meshBuffer);
        batchModels.clear();
    }
    Engine::instance()->endBatches();

    // Draw frame
    Engine::instance()->renderFrame(projection * view);

    Engine::instance()->endFrame();
    if (Engine::instance()->windowClosed())
        MelonCore::Instance::instance()->quit();
}

void RenderSystem::onExit() {
    for (std::pair<unsigned int, MeshBuffer> const& entry : m_MeshBufferMap)
        Engine::instance()->destroyMeshBuffer(entry.second);
    Engine::instance()->terminate();
}

}  // namespace MelonFrontend

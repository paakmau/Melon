#include <MelonCore/Combination.h>
#include <MelonCore/EntityManager.h>
#include <MelonCore/Instance.h>
#include <MelonCore/Rotation.h>
#include <MelonCore/Scale.h>
#include <MelonCore/Translation.h>
#include <MelonFrontend/Engine.h>
#include <MelonFrontend/RenderSystem.h>
#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

namespace MelonFrontend {

class CreatedRenderMeshTask : public MelonCore::ChunkTask {
   public:
    CreatedRenderMeshTask(const unsigned int& renderMeshComponentId, std::vector<MelonCore::Entity>& entities, std::vector<unsigned int>& renderMeshIndices) : _renderMeshComponentId(renderMeshComponentId), _entities(entities), _renderMeshIndices(renderMeshIndices) {}
    virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
        for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
            _entities[firstEntityIndex + i] = chunkAccessor.entityArray()[i];
            _renderMeshIndices[firstEntityIndex + i] = chunkAccessor.sharedComponentIndex(_renderMeshComponentId);
        }
    }

    const unsigned int& _renderMeshComponentId;

    std::vector<MelonCore::Entity>& _entities;
    std::vector<unsigned int>& _renderMeshIndices;
};

class RenderTask : public MelonCore::ChunkTask {
   public:
    RenderTask(std::vector<glm::mat4>& models, std::vector<const ManualRenderMesh*>& manualRenderMeshes, const unsigned int& translationComponentId, const unsigned int& rotationComponentId, const unsigned int& scaleComponentId, const unsigned int& manualRenderMeshComponentId) : _models(models), _manualRenderMeshes(manualRenderMeshes), _translationComponentId(translationComponentId), _rotationComponentId(rotationComponentId), _scaleComponentId(scaleComponentId), _manualRenderMeshComponentId(manualRenderMeshComponentId){};

    virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
        MelonCore::Translation* translations = chunkAccessor.componentArray<MelonCore::Translation>(_translationComponentId);
        MelonCore::Rotation* rotations = chunkAccessor.componentArray<MelonCore::Rotation>(_rotationComponentId);
        MelonCore::Scale* scales = chunkAccessor.componentArray<MelonCore::Scale>(_scaleComponentId);
        const ManualRenderMesh* manualRenderMesh = chunkAccessor.sharedComponent<ManualRenderMesh>(_manualRenderMeshComponentId);
        for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
            glm::mat4 model = glm::scale(glm::mat4(1.0f), scales[i].value);
            model = glm::mat4_cast(rotations[i].value) * model;
            model = glm::translate(model, translations[i].value);

            _models[firstEntityIndex + i] = model;
            _manualRenderMeshes[firstEntityIndex + i] = manualRenderMesh;
        }
    }

    std::vector<glm::mat4>& _models;
    std::vector<const ManualRenderMesh*>& _manualRenderMeshes;

    const unsigned int& _translationComponentId;
    const unsigned int& _rotationComponentId;
    const unsigned int& _scaleComponentId;
    const unsigned int& _manualRenderMeshComponentId;
};

class DestroyedRenderMeshTask : public MelonCore::ChunkTask {
   public:
    DestroyedRenderMeshTask(const unsigned int& manualRenderMeshComponentId, std::vector<MelonCore::Entity>& entities, std::vector<unsigned int>& manualRenderMeshIndices) : _manualRenderMeshComponentId(manualRenderMeshComponentId), _entities(entities), _manualRenderMeshIndices(manualRenderMeshIndices) {}
    virtual void execute(const MelonCore::ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) override {
        for (unsigned int i = 0; i < chunkAccessor.entityCount(); i++) {
            _entities[firstEntityIndex + i] = chunkAccessor.entityArray()[i];
            _manualRenderMeshIndices[firstEntityIndex + i] = chunkAccessor.sharedComponentIndex(_manualRenderMeshComponentId);
        }
    }

    const unsigned int& _manualRenderMeshComponentId;

    std::vector<MelonCore::Entity>& _entities;
    std::vector<unsigned int>& _manualRenderMeshIndices;
};

RenderSystem::RenderSystem(const unsigned int& width, const unsigned int& height) : SystemBase(), _currentWidth(width), _currentHeight(height) {}

RenderSystem::~RenderSystem() {}

void RenderSystem::onEnter() {
    Engine::instance()->initialize(MelonCore::Instance::instance()->applicationName(), _currentWidth, _currentHeight);

    _createdRenderMeshEntityFilter = entityManager()->createEntityFilterBuilder().requireSharedComponents<RenderMesh>().rejectSharedComponents<ManualRenderMesh>().createEntityFilter();
    _renderMeshEntityFilter = entityManager()->createEntityFilterBuilder().requireComponents<MelonCore::Translation, MelonCore::Rotation, MelonCore::Scale>().requireSharedComponents<RenderMesh, ManualRenderMesh>().createEntityFilter();
    _destroyedRenderMeshEntityFilter = entityManager()->createEntityFilterBuilder().requireSharedComponents<ManualRenderMesh>().rejectSharedComponents<RenderMesh>().createEntityFilter();

    _translationComponentId = entityManager()->componentId<MelonCore::Translation>();
    _rotationComponentId = entityManager()->componentId<MelonCore::Rotation>();
    _scaleComponentId = entityManager()->componentId<MelonCore::Scale>();
    _renderMeshComponentId = entityManager()->sharedComponentId<RenderMesh>();
    _manualRenderMeshComponentId = entityManager()->sharedComponentId<ManualRenderMesh>();
}

void RenderSystem::onUpdate() {
    // CreatedRenderMeshTask
    const unsigned int createdRenderMeshCount = entityManager()->entityCount(_createdRenderMeshEntityFilter);
    std::vector<MelonCore::Entity> createdRenderMeshEntities(createdRenderMeshCount);
    std::vector<unsigned int> createdRenderMeshIndices(createdRenderMeshCount);
    std::shared_ptr<MelonTask::TaskHandle> createdRenderMeshTaskHandle = schedule(std::make_shared<CreatedRenderMeshTask>(_renderMeshComponentId, createdRenderMeshEntities, createdRenderMeshIndices), _createdRenderMeshEntityFilter, predecessor());

    // DestroyedRenderMeshTask
    const unsigned destroyedRenderMeshCount = entityManager()->entityCount(_destroyedRenderMeshEntityFilter);
    std::vector<MelonCore::Entity> manualRenderMeshEntities(destroyedRenderMeshCount);
    std::vector<unsigned int> manualRenderMeshIndices(destroyedRenderMeshCount);
    std::shared_ptr<MelonTask::TaskHandle> destroyedRenderMeshTaskHandle = schedule(std::make_shared<DestroyedRenderMeshTask>(_manualRenderMeshComponentId, manualRenderMeshEntities, manualRenderMeshIndices), _destroyedRenderMeshEntityFilter, predecessor());

    // RenderTask
    const unsigned int renderMeshCount = entityManager()->entityCount(_renderMeshEntityFilter);
    glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), Engine::instance()->windowAspectRatio(), 0.1f, 10.0f);
    projection[1][1] *= -1;
    std::vector<glm::mat4> models(renderMeshCount);
    std::vector<const ManualRenderMesh*> manualRenderMeshes(renderMeshCount);
    std::shared_ptr<MelonTask::TaskHandle> renderMeshTaskHandle = schedule(std::make_shared<RenderTask>(models, manualRenderMeshes, _translationComponentId, _rotationComponentId, _scaleComponentId, _manualRenderMeshComponentId), _renderMeshEntityFilter, predecessor());

    MelonTask::TaskManager::instance()->activateWaitingTasks();

    Engine::instance()->beginFrame();

    // Create ManualRenderMeshes
    createdRenderMeshTaskHandle->complete();
    for (unsigned int i = 0; i < createdRenderMeshIndices.size(); i++) {
        const unsigned int& createdRenderMeshIndex = createdRenderMeshIndices[i];
        if (_renderMeshReferenceCountMap.contains(createdRenderMeshIndex))
            _renderMeshReferenceCountMap[createdRenderMeshIndex]++;
        else {
            _renderMeshReferenceCountMap[createdRenderMeshIndex] = 1;
            const RenderMesh* createdRenderMesh = entityManager()->sharedComponent<RenderMesh>(createdRenderMeshIndex);
            _meshBufferMap.emplace(createdRenderMeshIndex, Engine::instance()->createMeshBuffer(createdRenderMesh->vertices, createdRenderMesh->indices));
        }
        const MeshBuffer& meshBuffer = _meshBufferMap[createdRenderMeshIndex];
        entityManager()->addSharedComponent<ManualRenderMesh>(createdRenderMeshEntities[i], ManualRenderMesh{.renderMeshIndex = createdRenderMeshIndex, .meshBuffer = meshBuffer});
    }

    // Destroy ManualRenderMeshes
    destroyedRenderMeshTaskHandle->complete();
    for (unsigned int i = 0; i < manualRenderMeshIndices.size(); i++) {
        const unsigned int& manualRenderMeshIndex = manualRenderMeshIndices[i];
        const ManualRenderMesh* manualRenderMesh = entityManager()->sharedComponent<ManualRenderMesh>(manualRenderMeshIndex);
        _renderMeshReferenceCountMap[manualRenderMesh->renderMeshIndex]--;
        if (_renderMeshReferenceCountMap[manualRenderMesh->renderMeshIndex] == 0) {
            _renderMeshReferenceCountMap.erase(manualRenderMesh->renderMeshIndex);
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
    for (const std::pair<unsigned int, MeshBuffer>& entry : _meshBufferMap)
        Engine::instance()->destroyMeshBuffer(entry.second);
    Engine::instance()->terminate();
}

}  // namespace MelonFrontend

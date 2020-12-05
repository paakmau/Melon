#include <MelonCore/EntityManager.h>

#include <typeindex>

namespace MelonCore {

EntityCommandBuffer::EntityCommandBuffer(EntityManager* entityManager) noexcept : _entityManager(entityManager) {}

Entity EntityCommandBuffer::createEntity() {
    Entity const entity = _entityManager->assignEntity();
    _procedures.emplace_back([this, entity]() {
        _entityManager->createEntityImmediately(entity);
    });
    return entity;
}

Entity EntityCommandBuffer::createEntity(Archetype* archetype) {
    Entity const entity = _entityManager->assignEntity();
    _procedures.emplace_back([this, entity, archetype]() {
        _entityManager->createEntityImmediately(entity, archetype);
    });
    return entity;
}

void EntityCommandBuffer::destroyEntity(Entity const& entity) {
    _procedures.emplace_back([this, entity]() {
        _entityManager->destroyEntityImmediately(entity);
    });
}

void EntityCommandBuffer::execute() {
    for (std::function<void()> const& procedure : _procedures)
        procedure();
    _procedures.clear();
}

EntityManager::EntityManager() : _mainEntityCommandBuffer(this) {}

Entity EntityManager::createEntity() {
    return _mainEntityCommandBuffer.createEntity();
}

Entity EntityManager::createEntity(Archetype* archetype) {
    return _mainEntityCommandBuffer.createEntity(archetype);
}

void EntityManager::destroyEntity(Entity const& entity) {
    _mainEntityCommandBuffer.destroyEntity(entity);
}

std::vector<ChunkAccessor> EntityManager::filterEntities(EntityFilter const& entityFilter) {
    std::vector<ChunkAccessor> accessors;
    for (std::pair<ArchetypeMask, Archetype*> const& entry : _archetypeMap)
        if (entityFilter.satisfied(entry.first))
            if (entry.second->entityCount() != 0)
                entry.second->filterEntities(entityFilter, _sharedComponentStore, accessors);
    return accessors;
}

unsigned int EntityManager::chunkCount(EntityFilter const& entityFilter) const {
    unsigned int count = 0;
    for (std::pair<ArchetypeMask, Archetype*> const& entry : _archetypeMap)
        if (entityFilter.satisfied(entry.first))
            if (entry.second->entityCount() != 0)
                count += entry.second->chunkCount();
    return count;
};

unsigned int EntityManager::entityCount(EntityFilter const& entityFilter) const {
    unsigned int count = 0;
    for (std::pair<ArchetypeMask, Archetype*> const& entry : _archetypeMap)
        if (entityFilter.satisfied(entry.first))
            if (entry.second->entityCount() != 0)
                count += entry.second->entityCount();
    return count;
};

unsigned int EntityManager::registerComponent(std::type_index const& typeIndex) {
    return _componentIdMap.try_emplace(typeIndex, _componentIdMap.size()).first->second;
}

unsigned int EntityManager::registerSharedComponent(std::type_index const& typeIndex) {
    return _sharedComponentIdMap.try_emplace(typeIndex, _sharedComponentIdMap.size()).first->second;
}

unsigned int EntityManager::registerSingletonComponent(std::type_index const& typeIndex) {
    return _singletonComponentIdMap.try_emplace(typeIndex, _singletonComponentIdMap.size()).first->second;
}

Archetype* EntityManager::createArchetype(ArchetypeMask&& mask, std::vector<unsigned int>&& componentIds, std::vector<std::size_t>&& componentSizes, std::vector<std::size_t>&& componentAligns, std::vector<unsigned int>&& sharedComponentIds) {
    if (_archetypeMap.contains(mask)) return _archetypeMap[mask];
    unsigned int const archetypeId = _archetypeIdCounter++;
    Archetype* archetype = _archetypes.emplace_back(std::make_unique<Archetype>(archetypeId, mask, componentIds, componentSizes, componentAligns, sharedComponentIds, &_chunkPool)).get();
    _archetypeMap.emplace(mask, archetype);
    return archetype;
}

Entity EntityManager::assignEntity() {
    std::lock_guard lock(_entityIdMutex);
    if (!_freeEntityIds.empty()) {
        unsigned int entityId = _freeEntityIds.back();
        _freeEntityIds.pop();
        return Entity{entityId};
    }
    _entityLocations.emplace_back(Archetype::EntityLocation::invalidEntityLocation());
    return Entity{_entityIdCounter++};
}

void EntityManager::createEntityImmediately(Entity const& entity) {
    Archetype* const archetype = createArchetypeBuilder().createArchetype();
    createEntityImmediately(entity, archetype);
}

void EntityManager::createEntityImmediately(Entity const& entity, Archetype* archetype) {
    Archetype::EntityLocation location;
    archetype->addEntity(entity, location);
    _entityLocations[entity.id] = location;
}

void EntityManager::destroyEntityImmediately(Entity const& entity) {
    Archetype::EntityLocation const location = _entityLocations[entity.id];
    Archetype* archetype = _archetypes[location.archetypeId].get();
    // If the archetype is partially manual, we should remove all the components not manual instead
    if (archetype->partiallyManual()) {
        std::vector<unsigned int> const notManualComponentIds = archetype->notManualComponentIds();
        for (unsigned int const& componentId : notManualComponentIds)
            removeComponentWithoutCheck(entity, componentId, false);
        std::vector<unsigned int> const notManualSharedComponentIds = archetype->notManualSharedComponentIds();
        for (unsigned int const& sharedComponentId : notManualSharedComponentIds)
            removeSharedComponentWithoutCheck(entity, sharedComponentId, false);
        return;
    }
    // If the archetype is fully manual, we should not destroy it
    if (archetype->fullyManual())
        return;
    destroyEntityWithoutCheck(entity, archetype, location);
}

void EntityManager::destroyEntityWithoutCheck(Entity const& entity, Archetype* archetype, Archetype::EntityLocation const& location) {
    std::vector<unsigned int> const& sharedComponentIds = archetype->sharedComponentIds();
    std::vector<unsigned int> sharedComponentIndices;
    Entity swappedEntity;
    archetype->removeEntity(location, sharedComponentIndices, swappedEntity);
    for (unsigned int i = 0; i < sharedComponentIds.size(); i++)
        _sharedComponentStore.pop(sharedComponentIds[i], sharedComponentIndices[i]);
    if (swappedEntity.valid())
        _entityLocations[swappedEntity.id] = location;
    _entityLocations[entity.id] = Archetype::EntityLocation::invalidEntityLocation();
}

void EntityManager::removeComponentWithoutCheck(Entity const& entity, unsigned int const& componentId, bool const& manual) {
    Archetype::EntityLocation const srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    ArchetypeMask mask = srcArchetype->mask();
    mask.markComponent(componentId, manual, false);

    Archetype* dstArchetype;
    if (_archetypeMap.contains(mask))
        dstArchetype = _archetypeMap[mask];
    else {
        std::vector<unsigned int> componentIds = srcArchetype->componentIds();
        std::vector<std::size_t> componentSizes = srcArchetype->componentSizes();
        std::vector<std::size_t> componentAligns = srcArchetype->componentAligns();
        for (unsigned int i = 0; i < componentIds.size(); i++)
            if (componentIds[i] == componentId) {
                componentIds.erase(componentIds.begin() + i);
                componentSizes.erase(componentSizes.begin() + i);
                componentAligns.erase(componentAligns.begin() + i);
                break;
            }
        std::vector<unsigned int> sharedComponentIds = srcArchetype->sharedComponentIds();
        dstArchetype = createArchetype(std::move(mask), std::move(componentIds), std::move(componentSizes), std::move(componentAligns), std::move(sharedComponentIds));
    }

    Entity srcSwappedEntity;
    Archetype::EntityLocation dstLocation;
    dstArchetype->moveEntityRemovingComponent(srcLocation, srcArchetype, dstLocation, srcSwappedEntity);
    _entityLocations[entity.id] = dstLocation;
    if (srcSwappedEntity.valid())
        _entityLocations[srcSwappedEntity.id] = srcLocation;
}

void EntityManager::removeSharedComponentWithoutCheck(Entity const& entity, unsigned int const& sharedComponentId, bool const& manual) {
    Archetype::EntityLocation const srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    ArchetypeMask mask = srcArchetype->mask();
    mask.markSharedComponent(sharedComponentId, manual, false);

    Archetype* dstArchetype;
    if (_archetypeMap.contains(mask))
        dstArchetype = _archetypeMap[mask];
    else {
        std::vector<unsigned int> componentIds = srcArchetype->componentIds();
        std::vector<std::size_t> componentSizes = srcArchetype->componentSizes();
        std::vector<std::size_t> componentAligns = srcArchetype->componentAligns();
        std::vector<unsigned int> sharedComponentIds = srcArchetype->sharedComponentIds();
        for (unsigned int i = 0; i < sharedComponentIds.size(); i++)
            if (sharedComponentIds[i] == sharedComponentId) {
                sharedComponentIds.erase(sharedComponentIds.begin() + i);
                break;
            }
        dstArchetype = createArchetype(std::move(mask), std::move(componentIds), std::move(componentSizes), std::move(componentAligns), std::move(sharedComponentIds));
    }

    unsigned int sharedComponentIndex;
    Entity srcSwappedEntity;
    Archetype::EntityLocation dstLocation;
    dstArchetype->moveEntityRemovingSharedComponent(srcLocation, srcArchetype, sharedComponentIndex, dstLocation, srcSwappedEntity);
    _entityLocations[entity.id] = dstLocation;
    if (srcSwappedEntity.valid())
        _entityLocations[srcSwappedEntity.id] = srcLocation;

    _sharedComponentStore.pop(sharedComponentId, sharedComponentIndex);
}

void EntityManager::executeEntityCommandBuffers() {
    _mainEntityCommandBuffer.execute();
    for (std::unique_ptr<EntityCommandBuffer> const& buffer : _taskEntityCommandBuffers)
        buffer->execute();
    _taskEntityCommandBuffers.clear();
}

}  // namespace MelonCore

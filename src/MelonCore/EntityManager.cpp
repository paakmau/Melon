#include <MelonCore/Entity.h>
#include <MelonCore/EntityManager.h>

#include <typeindex>

namespace MelonCore {

EntityCommandBuffer::EntityCommandBuffer(EntityManager* entityManager) : _entityManager(entityManager) {}

const Entity EntityCommandBuffer::createEntity(Archetype* archetype) {
    const Entity entity = _entityManager->assignEntity();
    _procedures.emplace_back([this, entity, archetype]() {
        _entityManager->createEntityImmediately(entity, archetype);
    });
    return entity;
}

void EntityCommandBuffer::destroyEntity(const Entity& entity) {
    _procedures.emplace_back([this, entity]() {
        _entityManager->destroyEntityImmediately(entity);
    });
}

void EntityCommandBuffer::execute() {
    for (const std::function<void()>& procedure : _procedures)
        procedure();
    _procedures.clear();
}

EntityManager::EntityManager() : _mainEntityCommandBuffer(this) {}

Entity EntityManager::createEntity(Archetype* archetype) {
    return _mainEntityCommandBuffer.createEntity(archetype);
}

void EntityManager::destroyEntity(const Entity& entity) {
    _mainEntityCommandBuffer.destroyEntity(entity);
}

std::vector<ChunkAccessor> EntityManager::filterEntities(const EntityFilter& entityFilter) {
    std::vector<ChunkAccessor> accessors;
    for (const std::pair<ArchetypeMask, Archetype*>& entry : _archetypeMap)
        if (entityFilter.satisfied(entry.first))
            if (entry.second->entityCount() != 0)
                entry.second->filterEntities(entityFilter, _sharedComponentStore, accessors);
    return accessors;
}

unsigned int EntityManager::chunkCount(const EntityFilter& entityFilter) const {
    unsigned int count = 0;
    for (const std::pair<ArchetypeMask, Archetype*>& entry : _archetypeMap)
        if (entityFilter.satisfied(entry.first))
            if (entry.second->entityCount() != 0)
                count += entry.second->chunkCount();
    return count;
};

unsigned int EntityManager::entityCount(const EntityFilter& entityFilter) const {
    unsigned int count = 0;
    for (const std::pair<ArchetypeMask, Archetype*>& entry : _archetypeMap)
        if (entityFilter.satisfied(entry.first))
            if (entry.second->entityCount() != 0)
                count += entry.second->entityCount();
    return count;
};

unsigned int EntityManager::registerComponent(const std::type_index& typeIndex) {
    if (_componentIdMap.contains(typeIndex)) return _componentIdMap[typeIndex];
    return _componentIdMap.emplace(typeIndex, _componentIdMap.size()).first->second;
}

unsigned int EntityManager::registerSharedComponent(const std::type_index& typeIndex) {
    if (_sharedComponentIdMap.contains(typeIndex)) return _sharedComponentIdMap[typeIndex];
    return _sharedComponentIdMap.emplace(typeIndex, _sharedComponentIdMap.size()).first->second;
}

Archetype* EntityManager::createArchetype(ArchetypeMask&& mask, std::vector<unsigned int>&& componentIds, std::vector<std::size_t>&& componentSizes, std::vector<std::size_t>&& componentAligns, std::vector<unsigned int>&& sharedComponentIds) {
    const unsigned int archetypeId = _archetypeIdCounter++;
    const std::unique_ptr<Archetype>& archetype = _archetypes.emplace_back(std::make_unique<Archetype>(archetypeId, mask, componentIds, componentSizes, componentAligns, sharedComponentIds, &_chunkPool));
    _archetypeMap.emplace(mask, archetype.get());
    return archetype.get();
}

const Entity EntityManager::assignEntity() {
    std::lock_guard lock(_entityIdMutex);
    if (!_freeEntityIds.empty()) {
        unsigned int entityId = _freeEntityIds.back();
        _freeEntityIds.pop();
        return Entity{entityId};
    }
    _entityLocations.emplace_back(EntityLocation{});
    return Entity{_entityIdCounter++};
}

void EntityManager::createEntityImmediately(const Entity& entity, Archetype* archetype) {
    EntityLocation location;
    archetype->addEntity(entity, location);
    _entityLocations[entity.id] = location;
}

void EntityManager::destroyEntityImmediately(const Entity& entity) {
    const EntityLocation& location = _entityLocations[entity.id];
    const std::vector<unsigned int>& sharedComponentIds = _archetypes[location.archetypeId]->sharedComponentIds();
    std::vector<unsigned int> sharedComponentIndices;
    Entity swappedEntity;
    _archetypes[location.archetypeId]->removeEntity(location, sharedComponentIndices, swappedEntity);
    for (unsigned int i = 0; i < sharedComponentIds.size(); i++)
        _sharedComponentStore.pop(sharedComponentIds[i], sharedComponentIndices[i]);
    _entityLocations[swappedEntity.id] = location;
}

void EntityManager::executeEntityCommandBuffers() {
    _mainEntityCommandBuffer.execute();
    for (EntityCommandBuffer& buffer : _taskEntityCommandBuffers)
        buffer.execute();
}

}  // namespace MelonCore

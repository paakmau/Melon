#include <libMelonCore/EntityManager.h>

#include <typeindex>

namespace MelonCore {

EntityCommandBuffer::EntityCommandBuffer(EntityManager* entityManager) noexcept : m_EntityManager(entityManager) {}

Entity EntityCommandBuffer::createEntity() {
    Entity const entity = m_EntityManager->assignEntity();
    m_Procedures.emplace_back([this, entity]() {
        m_EntityManager->createEntityImmediately(entity);
    });
    return entity;
}

Entity EntityCommandBuffer::createEntity(Archetype* archetype) {
    Entity const entity = m_EntityManager->assignEntity();
    m_Procedures.emplace_back([this, entity, archetype]() {
        m_EntityManager->createEntityImmediately(entity, archetype);
    });
    return entity;
}

void EntityCommandBuffer::destroyEntity(Entity const& entity) {
    m_Procedures.emplace_back([this, entity]() {
        m_EntityManager->destroyEntityImmediately(entity);
    });
}

void EntityCommandBuffer::execute() {
    for (std::function<void()> const& procedure : m_Procedures)
        procedure();
    m_Procedures.clear();
}

EntityManager::EntityManager() : m_MainEntityCommandBuffer(this) {}

Entity EntityManager::createEntity() {
    return m_MainEntityCommandBuffer.createEntity();
}

Entity EntityManager::createEntity(Archetype* archetype) {
    return m_MainEntityCommandBuffer.createEntity(archetype);
}

void EntityManager::destroyEntity(Entity const& entity) {
    m_MainEntityCommandBuffer.destroyEntity(entity);
}

std::vector<ChunkAccessor> EntityManager::filterEntities(EntityFilter const& entityFilter) {
    std::vector<ChunkAccessor> accessors;
    for (std::pair<ArchetypeMask, Archetype*> const& entry : m_ArchetypeMap)
        if (entityFilter.satisfied(entry.first))
            if (entry.second->entityCount() != 0)
                entry.second->filterEntities(entityFilter, m_SharedComponentStore, accessors);
    return accessors;
}

unsigned int EntityManager::chunkCount(EntityFilter const& entityFilter) const {
    unsigned int count = 0;
    for (std::pair<ArchetypeMask, Archetype*> const& entry : m_ArchetypeMap)
        if (entityFilter.satisfied(entry.first))
            if (entry.second->entityCount() != 0)
                count += entry.second->chunkCount();
    return count;
};

unsigned int EntityManager::entityCount(EntityFilter const& entityFilter) const {
    unsigned int count = 0;
    for (std::pair<ArchetypeMask, Archetype*> const& entry : m_ArchetypeMap)
        if (entityFilter.satisfied(entry.first))
            if (entry.second->entityCount() != 0)
                count += entry.second->entityCount();
    return count;
};

unsigned int EntityManager::registerComponent(std::type_index const& typeIndex) {
    return m_ComponentIdMap.try_emplace(typeIndex, m_ComponentIdMap.size()).first->second;
}

unsigned int EntityManager::registerSharedComponent(std::type_index const& typeIndex) {
    return m_SharedComponentIdMap.try_emplace(typeIndex, m_SharedComponentIdMap.size()).first->second;
}

unsigned int EntityManager::registerSingletonComponent(std::type_index const& typeIndex) {
    return m_SingletonComponentIdMap.try_emplace(typeIndex, m_SingletonComponentIdMap.size()).first->second;
}

Archetype* EntityManager::createArchetype(ArchetypeMask&& mask, std::vector<unsigned int>&& componentIds, std::vector<std::size_t>&& componentSizes, std::vector<std::size_t>&& componentAligns, std::vector<unsigned int>&& sharedComponentIds) {
    if (m_ArchetypeMap.contains(mask)) return m_ArchetypeMap[mask];
    unsigned int const archetypeId = m_ArchetypeIdCounter++;
    Archetype* archetype = m_Archetypes.emplace_back(std::make_unique<Archetype>(archetypeId, mask, componentIds, componentSizes, componentAligns, sharedComponentIds, &m_ChunkPool)).get();
    m_ArchetypeMap.emplace(mask, archetype);
    return archetype;
}

Entity EntityManager::assignEntity() {
    std::lock_guard lock(m_EntityIdMutex);
    if (!m_FreeEntityIds.empty()) {
        unsigned int entityId = m_FreeEntityIds.back();
        m_FreeEntityIds.pop();
        return Entity{entityId};
    }
    m_EntityLocations.emplace_back(Archetype::EntityLocation::invalidEntityLocation());
    return Entity{m_EntityIdCounter++};
}

void EntityManager::createEntityImmediately(Entity const& entity) {
    Archetype* const archetype = createArchetypeBuilder().createArchetype();
    createEntityImmediately(entity, archetype);
}

void EntityManager::createEntityImmediately(Entity const& entity, Archetype* archetype) {
    Archetype::EntityLocation location;
    archetype->addEntity(entity, location);
    m_EntityLocations[entity.id] = location;
}

void EntityManager::destroyEntityImmediately(Entity const& entity) {
    Archetype::EntityLocation const location = m_EntityLocations[entity.id];
    Archetype* archetype = m_Archetypes[location.archetypeId].get();
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
        m_SharedComponentStore.pop(sharedComponentIds[i], sharedComponentIndices[i]);
    if (swappedEntity.valid())
        m_EntityLocations[swappedEntity.id] = location;
    m_EntityLocations[entity.id] = Archetype::EntityLocation::invalidEntityLocation();
}

void EntityManager::removeComponentWithoutCheck(Entity const& entity, unsigned int const& componentId, bool const& manual) {
    Archetype::EntityLocation const srcLocation = m_EntityLocations[entity.id];
    Archetype* const srcArchetype = m_Archetypes[srcLocation.archetypeId].get();
    ArchetypeMask mask = srcArchetype->mask();
    mask.markComponent(componentId, manual, false);

    Archetype* dstArchetype;
    if (m_ArchetypeMap.contains(mask))
        dstArchetype = m_ArchetypeMap[mask];
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
    m_EntityLocations[entity.id] = dstLocation;
    if (srcSwappedEntity.valid())
        m_EntityLocations[srcSwappedEntity.id] = srcLocation;
}

void EntityManager::removeSharedComponentWithoutCheck(Entity const& entity, unsigned int const& sharedComponentId, bool const& manual) {
    Archetype::EntityLocation const srcLocation = m_EntityLocations[entity.id];
    Archetype* const srcArchetype = m_Archetypes[srcLocation.archetypeId].get();
    ArchetypeMask mask = srcArchetype->mask();
    mask.markSharedComponent(sharedComponentId, manual, false);

    Archetype* dstArchetype;
    if (m_ArchetypeMap.contains(mask))
        dstArchetype = m_ArchetypeMap[mask];
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
    m_EntityLocations[entity.id] = dstLocation;
    if (srcSwappedEntity.valid())
        m_EntityLocations[srcSwappedEntity.id] = srcLocation;

    m_SharedComponentStore.pop(sharedComponentId, sharedComponentIndex);
}

void EntityManager::executeEntityCommandBuffers() {
    m_MainEntityCommandBuffer.execute();
    for (std::unique_ptr<EntityCommandBuffer> const& buffer : m_TaskEntityCommandBuffers)
        buffer->execute();
    m_TaskEntityCommandBuffers.clear();
}

}  // namespace MelonCore

#pragma once

#include <MelonCore/Archetype.h>
#include <MelonCore/ChunkAccessor.h>
#include <MelonCore/Combination.h>
#include <MelonCore/Entity.h>
#include <MelonCore/ObjectPool.h>
#include <MelonCore/ObjectStore.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <queue>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace MelonCore {

class EntityManager;

class ArchetypeBuilder {
   public:
    template <typename... Ts>
    ArchetypeBuilder& markComponents();
    template <typename... Ts>
    ArchetypeBuilder& markSharedComponents();

    Archetype* createArchetype();

   private:
    ArchetypeBuilder(EntityManager* entityManager) : _entityManager(entityManager) {}
    std::vector<unsigned int> _componentIds;
    std::vector<std::size_t> _componentSizes;
    std::vector<std::size_t> _componentAligns;
    std::vector<unsigned int> _sharedComponentIds;
    ArchetypeMask _mask;
    EntityManager* _entityManager;

    friend class EntityManager;
};

class EntityFilterBuilder {
   public:
    template <typename... Ts>
    EntityFilterBuilder& requireComponents();
    template <typename... Ts>
    EntityFilterBuilder& rejectComponents();

    template <typename... Ts>
    EntityFilterBuilder& requireSharedComponents();
    template <typename... Ts>
    EntityFilterBuilder& rejectSharedComponents();

    template <typename T>
    EntityFilterBuilder& requireSharedComponent(const T& sharedComponent);
    template <typename T>
    EntityFilterBuilder& rejectSharedComponent(const T& sharedComponent);

    EntityFilter createEntityFilter();

   private:
    EntityFilterBuilder(EntityManager* entityManager) : _entityManager(entityManager) {}
    EntityFilter _entityFilter;
    EntityManager* _entityManager;

    friend class EntityManager;
};

class EntityCommandBuffer {
   public:
    EntityCommandBuffer(EntityManager* entityManager);

    const Entity createEntity(Archetype* archetype);
    void destroyEntity(const Entity& entity);
    template <typename T>
    void addComponent(const Entity& entity, const T& component);
    template <typename T>
    void removeComponent(const Entity& entity);
    template <typename T>
    void setComponent(const Entity& entity, const T& component);
    template <typename T>
    void addSharedComponent(const Entity& entity, const T& sharedComponent);
    template <typename T>
    void removeSharedComponent(const Entity& entity);
    template <typename T>
    void setSharedComponent(const Entity& entity, const T& sharedComponent);

   private:
    void execute();

    EntityManager* _entityManager;
    std::vector<std::function<void()>> _procedures;  // TODO: Improve performance

    friend class EntityManager;
};

class EntityManager {
   public:
    EntityManager();
    EntityManager(const EntityManager&) = delete;

    ArchetypeBuilder createArchetypeBuilder() { return ArchetypeBuilder(this); }

    Entity createEntity(Archetype* archetype);
    void destroyEntity(const Entity& entity);
    template <typename T>
    void addComponent(const Entity& entity, const T& component);
    template <typename T>
    void removeComponent(const Entity& entity);
    template <typename T>
    void setComponent(const Entity& entity, const T& component);
    template <typename T>
    void addSharedComponent(const Entity& entity, const T& sharedComponent);
    template <typename T>
    void removeSharedComponent(const Entity& entity);
    template <typename T>
    void setSharedComponent(const Entity& entity, const T& sharedComponent);

    template <typename T>
    unsigned int sharedComponentIndex(const T& sharedComponent);

    template <typename T>
    unsigned int componentId();
    template <typename T>
    unsigned int sharedComponentId();
    EntityFilterBuilder createEntityFilterBuilder() { return EntityFilterBuilder(this); }
    std::vector<ChunkAccessor> filterEntities(const EntityFilter& entityFilter);

    unsigned int chunkCount(const EntityFilter& entityFilter) const;
    unsigned int entityCount(const EntityFilter& entityFilter) const;

   private:
    template <typename T>
    unsigned int registerComponent();
    unsigned int registerComponent(const std::type_index& typeIndex);
    template <typename T>
    unsigned int registerSharedComponent();
    unsigned int registerSharedComponent(const std::type_index& typeIndex);
    Archetype* createArchetype(ArchetypeMask&& mask, std::vector<unsigned int>&& componentIds, std::vector<std::size_t>&& componentSizes, std::vector<std::size_t>&& componentAligns, std::vector<unsigned int>&& sharedComponentIds);
    const Entity assignEntity();
    void createEntityImmediately(const Entity& entity, Archetype* archetype);
    void destroyEntityImmediately(const Entity& entityId);
    template <typename T>
    void addComponentImmediately(const Entity& entity, const T& component);
    template <typename T>
    void removeComponentImmediately(const Entity& entity);
    template <typename T>
    void setComponentImmediately(const Entity& entity, const T& component);
    template <typename T>
    void addSharedComponentImmediately(const Entity& entity, const T& sharedComponent);
    template <typename T>
    void removeSharedComponentImmediately(const Entity& entity);
    template <typename T>
    void setSharedComponentImmediately(const Entity& entity, const T& sharedComponent);

    void executeEntityCommandBuffers();

    std::unordered_map<std::type_index, unsigned int> _componentIdMap;
    std::unordered_map<std::type_index, unsigned int> _sharedComponentIdMap;

    ObjectPool<Chunk> _chunkPool;
    ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount> _sharedComponentStore;

    unsigned int _archetypeIdCounter{};
    std::unordered_map<ArchetypeMask, Archetype*, ArchetypeMask::Hash> _archetypeMap;
    std::vector<std::unique_ptr<Archetype>> _archetypes;

    // TODO: To avoid contention, a few reserved Entity id could be passed to EntityCommandBuffer in main thread.
    std::mutex _entityIdMutex;
    unsigned int _entityIdCounter{};
    std::queue<unsigned int> _freeEntityIds;

    std::vector<EntityLocation> _entityLocations;

    EntityCommandBuffer _mainEntityCommandBuffer;
    std::vector<EntityCommandBuffer> _taskEntityCommandBuffers;

    friend class ArchetypeBuilder;
    friend class EntityCommandBuffer;
    friend class World;
    friend class SystemBase;
};

template <typename... Ts>
ArchetypeBuilder& ArchetypeBuilder::markComponents() {
    const std::vector<unsigned int> componentIds{_entityManager->componentId<Ts>()...};
    const std::vector<std::size_t> componentSizes{sizeof(Ts)...};
    const std::vector<std::size_t> componentAligns{alignof(Ts)...};
    _componentIds.insert(_componentIds.end(), componentIds.begin(), componentIds.end());
    _componentSizes.insert(_componentSizes.end(), componentSizes.begin(), componentSizes.end());
    _componentAligns.insert(_componentAligns.end(), componentAligns.begin(), componentAligns.end());
    _mask.markComponents(componentIds);
    return *this;
}

template <typename... Ts>
ArchetypeBuilder& ArchetypeBuilder::markSharedComponents() {
    const std::vector<unsigned int> sharedComponentIds{_entityManager->sharedComponentId<Ts>()...};
    _sharedComponentIds.insert(_sharedComponentIds.end(), sharedComponentIds.begin(), sharedComponentIds.end());
    _mask.markSharedComponents(sharedComponentIds);
    return *this;
}

inline Archetype* ArchetypeBuilder::createArchetype() {
    return _entityManager->createArchetype(std::move(_mask), std::move(_componentIds), std::move(_componentSizes), std::move(_componentAligns), std::move(_sharedComponentIds));
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::requireComponents() {
    const std::vector<unsigned int>& componentIds{_entityManager->componentId<Ts>()...};
    for (const unsigned int& cmptId : componentIds)
        _entityFilter.requiredComponentMask.set(cmptId);
    return *this;
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::rejectComponents() {
    const std::vector<unsigned int>& componentIds{_entityManager->componentId<Ts>()...};
    for (const unsigned int& cmptId : componentIds)
        _entityFilter.rejectedComponentMask.set(cmptId);
    return *this;
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::requireSharedComponents() {
    const std::vector<unsigned int>& sharedComponentIds{_entityManager->sharedComponentId<Ts>()...};
    for (const unsigned int& sharedCmptId : sharedComponentIds)
        _entityFilter.requiredSharedComponentMask.set(sharedCmptId);
    return *this;
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::rejectSharedComponents() {
    const std::vector<unsigned int>& sharedComponentIds{_entityManager->sharedComponentId<Ts>()...};
    for (const unsigned int& sharedCmptId : sharedComponentIds)
        _entityFilter.rejectedSharedComponentMask.set(sharedCmptId);
    return *this;
}

template <typename T>
EntityFilterBuilder& EntityFilterBuilder::requireSharedComponent(const T& sharedComponent) {
    const unsigned int sharedComponentId = _entityManager->sharedComponentId<T>();
    const unsigned int sharedComponentIndex = _entityManager->sharedComponentIndex(sharedComponent);
    _entityFilter.requiredSharedComponentIdAndIndices.emplace_back(std::make_pair(sharedComponentId, sharedComponentIndex));
}

template <typename T>
EntityFilterBuilder& EntityFilterBuilder::rejectSharedComponent(const T& sharedComponent) {
    const unsigned int sharedComponentId = _entityManager->sharedComponentId<T>();
    const unsigned int sharedComponentIndex = _entityManager->sharedComponentIndex(sharedComponent);
    _entityFilter.rejectedSharedComponentIdAndIndices.emplace_back(std::make_pair(sharedComponentId, sharedComponentIndex));
}

inline EntityFilter EntityFilterBuilder::createEntityFilter() {
    std::sort(_entityFilter.requiredSharedComponentIdAndIndices.begin(), _entityFilter.requiredSharedComponentIdAndIndices.end());
    return std::move(_entityFilter);
}

template <typename T>
void EntityCommandBuffer::addComponent(const Entity& entity, const T& component) {
    _procedures.emplace_back([this, entity, component]() {
        _entityManager->addComponentImmediately(entity, component);
    });
}

template <typename T>
void EntityCommandBuffer::removeComponent(const Entity& entity) {
    _procedures.emplace_back([this, entity]() {
        _entityManager->removeComponentImmediately<T>(entity);
    });
}

template <typename T>
void EntityCommandBuffer::setComponent(const Entity& entity, const T& component) {
    _procedures.emplace_back([this, entity, component]() {
        _entityManager->setComponentImmediately(entity, component);
    });
}

template <typename T>
void EntityCommandBuffer::addSharedComponent(const Entity& entity, const T& sharedComponent) {
    _procedures.emplace_back([this, entity, sharedComponent]() {
        _entityManager->addSharedComponentImmediately(entity, sharedComponent);
    });
}

template <typename T>
void EntityCommandBuffer::removeSharedComponent(const Entity& entity) {
    _procedures.emplace_back([this, entity]() {
        _entityManager->removeSharedComponentImmediately<T>(entity);
    });
}

template <typename T>
void EntityCommandBuffer::setSharedComponent(const Entity& entity, const T& sharedComponent) {
    _procedures.emplace_back([this, entity, sharedComponent]() {
        _entityManager->setSharedComponentImmediately(entity, sharedComponent);
    });
}

template <typename T>
void EntityManager::addComponent(const Entity& entity, const T& component) {
    _mainEntityCommandBuffer.addComponent(entity, component);
}

template <typename T>
void EntityManager::removeComponent(const Entity& entity) {
    _mainEntityCommandBuffer.removeComponent<T>(entity);
}

template <typename T>
void EntityManager::setComponent(const Entity& entity, const T& component) {
    _mainEntityCommandBuffer.setComponent(entity, component);
}

template <typename T>
void EntityManager::addSharedComponent(const Entity& entity, const T& sharedComponent) {
    _mainEntityCommandBuffer.addSharedComponent(entity, sharedComponent);
}

template <typename T>
void EntityManager::removeSharedComponent(const Entity& entity) {
    _mainEntityCommandBuffer.removeSharedComponent<T>(entity);
}

template <typename T>
void EntityManager::setSharedComponent(const Entity& entity, const T& sharedComponent) {
    _mainEntityCommandBuffer.setSharedComponent(entity, sharedComponent);
}

template <typename T>
unsigned int EntityManager::sharedComponentIndex(const T& sharedComponent) {
    const unsigned int sharedComponentId = registerSharedComponent<T>();
    return _sharedComponentStore.objectIndex(sharedComponentId, sharedComponent);
}

template <typename T>
unsigned int EntityManager::componentId() {
    return registerComponent<T>();
}

template <typename T>
unsigned int EntityManager::sharedComponentId() {
    return registerSharedComponent<T>();
}

template <typename T>
unsigned int EntityManager::registerComponent() {
    return registerComponent(typeid(T));
}

template <typename T>
unsigned int EntityManager::registerSharedComponent() {
    return registerSharedComponent(typeid(T));
}

template <typename T>
void EntityManager::addComponentImmediately(const Entity& entity, const T& component) {
    const EntityLocation& srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    const unsigned int componentId = registerComponent<T>();
    ArchetypeMask mask = srcArchetype->mask();
    mask.componentMask.set(componentId);

    Archetype* dstArchetype;
    if (_archetypeMap.contains(mask))
        dstArchetype = _archetypeMap[mask];
    else {
        std::vector<unsigned int> componentIds = srcArchetype->componentIds();
        std::vector<std::size_t> componentSizes = srcArchetype->componentSizes();
        std::vector<std::size_t> componentAligns = srcArchetype->componentAligns();
        componentIds.push_back(componentId);
        componentSizes.emplace_back(sizeof(T));
        componentAligns.emplace_back(alignof(T));
        std::vector<unsigned int> sharedComponentIds = srcArchetype->sharedComponentIds();
        dstArchetype = createArchetype(std::move(mask), std::move(componentIds), std::move(componentSizes), std::move(componentAligns), std::move(sharedComponentIds));
    }

    Entity srcSwappedEntity;
    EntityLocation dstLocation;
    dstArchetype->moveEntityAddingComponent(srcLocation, srcArchetype, componentId, static_cast<const void*>(&component), dstLocation, srcSwappedEntity);
    _entityLocations[entity.id] = dstLocation;
    _entityLocations[srcSwappedEntity.id] = srcLocation;
}

template <typename T>
void EntityManager::removeComponentImmediately(const Entity& entity) {
    const EntityLocation& srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    const unsigned int componentId = registerComponent<T>();
    ArchetypeMask mask = srcArchetype->mask();
    mask.componentMask.set(componentId, false);

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
                componentAligns.erase(componentAligns.begin() + 1);
                break;
            }
        std::vector<unsigned int> sharedComponentIds = srcArchetype->sharedComponentIds();
        dstArchetype = createArchetype(std::move(mask), std::move(componentIds), std::move(componentSizes), std::move(componentAligns), std::move(sharedComponentIds));
    }

    Entity srcSwappedEntity;
    EntityLocation dstLocation;
    dstArchetype->moveEntityRemovingComponent(srcLocation, srcArchetype, dstLocation, srcSwappedEntity);
    _entityLocations[entity.id] = dstLocation;
    _entityLocations[srcSwappedEntity.id] = srcLocation;
}

template <typename T>
void EntityManager::setComponentImmediately(const Entity& entity, const T& component) {
    const EntityLocation& location = _entityLocations[entity.id];
    Archetype* const archetype = _archetypes[location.archetypeId].get();
    const unsigned int componentId = _componentIdMap.at(typeid(T));
    archetype->setComponent(location, componentId, static_cast<const void*>(&component));
}

template <typename T>
void EntityManager::addSharedComponentImmediately(const Entity& entity, const T& sharedComponent) {
    const EntityLocation& srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    const unsigned int sharedComponentId = registerSharedComponent<T>();
    ArchetypeMask mask = srcArchetype->mask();
    mask.sharedComponentMask.set(sharedComponentId);

    Archetype* dstArchetype;
    if (_archetypeMap.contains(mask))
        dstArchetype = _archetypeMap[mask];
    else {
        std::vector<unsigned int> componentIds = srcArchetype->componentIds();
        std::vector<std::size_t> componentSizes = srcArchetype->componentSizes();
        std::vector<std::size_t> componentAligns = srcArchetype->componentAligns();
        std::vector<unsigned int> sharedComponentIds = srcArchetype->sharedComponentIds();
        sharedComponentIds.push_back(sharedComponentId);
        dstArchetype = createArchetype(std::move(mask), std::move(componentIds), std::move(componentSizes), std::move(componentAligns), std::move(sharedComponentIds));
    }

    unsigned int sharedComponentIndex = _sharedComponentStore.push(sharedComponentId, sharedComponent);

    Entity srcSwappedEntity;
    EntityLocation dstLocation;
    dstArchetype->moveEntityAddingSharedComponent(srcLocation, srcArchetype, sharedComponentId, sharedComponentIndex, dstLocation, srcSwappedEntity);
    _entityLocations[entity.id] = dstLocation;
    _entityLocations[srcSwappedEntity.id] = srcLocation;
}

template <typename T>
void EntityManager::removeSharedComponentImmediately(const Entity& entity) {
    const EntityLocation& srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    const unsigned int sharedComponentId = registerSharedComponent<T>();
    ArchetypeMask mask = srcArchetype->mask();
    mask.sharedComponentMask.set(sharedComponentId, false);

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
    EntityLocation dstLocation;
    dstArchetype->moveEntityRemovingSharedComponent(srcLocation, srcArchetype, sharedComponentIndex, dstLocation, srcSwappedEntity);
    _entityLocations[entity.id] = dstLocation;
    _entityLocations[srcSwappedEntity.id] = srcLocation;

    _sharedComponentStore.pop(sharedComponentId, sharedComponentIndex);
}

template <typename T>
void EntityManager::setSharedComponentImmediately(const Entity& entity, const T& sharedComponent) {
    const EntityLocation location = _entityLocations[entity.id];
    Archetype* const archetype = _archetypes[location.archetypeId].get();
    const unsigned int sharedComponentId = registerSharedComponent<T>();

    unsigned int sharedComponentIndex = _sharedComponentStore.push(sharedComponentId, sharedComponent);

    unsigned int originalSharedComponentIndex;
    EntityLocation dstLocation;
    Entity swappedEntity;
    archetype->setSharedComponent(location, sharedComponentId, sharedComponentIndex, originalSharedComponentIndex, dstLocation, swappedEntity);
    _entityLocations[entity.id] = dstLocation;
    _entityLocations[swappedEntity.id] = location;

    _sharedComponentStore.pop(sharedComponentId, originalSharedComponentIndex);
}

}  // namespace MelonCore

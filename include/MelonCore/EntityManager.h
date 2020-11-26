#pragma once

#include <MelonCore/Archetype.h>
#include <MelonCore/ChunkAccessor.h>
#include <MelonCore/Combination.h>
#include <MelonCore/Entity.h>
#include <MelonCore/ObjectPool.h>
#include <MelonCore/ObjectStore.h>
#include <MelonCore/TypeMark.h>

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
#include <vector>

namespace MelonCore {

class EntityManager;

class EntityCommandBuffer {
   public:
    EntityCommandBuffer(EntityManager* entityManager);

    template <typename... Ts, typename... Us>
    const Entity createEntity(TypeMark<Ts...> componentTypeMark, TypeMark<Us...> sharedComponentTypeMark);
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

    template <typename... Ts, typename... Us>
    Archetype* createArchetype(TypeMark<Ts...> componentTypeMark, TypeMark<Us...> sharedComponentMark);

    template <typename... Ts, typename... Us>
    Entity createEntity(TypeMark<Ts...> componentTypeMark, TypeMark<Us...> sharedComponentTypeMark);
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
    unsigned int componentId();
    template <typename T>
    unsigned int sharedComponentId();
    template <typename... Ts, typename... Us>
    const EntityFilter createEntityFilter(TypeMark<Ts...> componentTypeMark, TypeMark<Us...> sharedComponentTypeMark);
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
    template <typename... Ts, typename... Us>
    void createEntityImmediately(const Entity& entity, TypeMark<Ts...> componentTypeMark, TypeMark<Us...> sharedComponentTypeMark);
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

    friend class EntityCommandBuffer;
    friend class World;
    friend class SystemBase;
};

template <typename... Ts, typename... Us>
const Entity EntityCommandBuffer::createEntity(TypeMark<Ts...> componentTypeMark, TypeMark<Us...> sharedComponentTypeMark) {
    const Entity entity = _entityManager->assignEntity();
    _procedures.emplace_back([this, entity, componentTypeMark, sharedComponentTypeMark]() {
        _entityManager->createEntityImmediately(entity, componentTypeMark, sharedComponentTypeMark);
    });
    return entity;
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

template <typename... Ts, typename... Us>
Archetype* EntityManager::createArchetype(TypeMark<Ts...> componentTypeMark, TypeMark<Us...> sharedComponentMark) {
    std::vector<unsigned int> componentIds{registerComponent<Ts>()...};
    std::bitset<ArchetypeMask::kMaxComponentIdCount> componentMask;
    for (const unsigned int& cmptId : componentIds)
        componentMask.set(cmptId);
    std::vector<unsigned int> sharedComponentIds{registerSharedComponent<Us>()...};
    std::bitset<ArchetypeMask::kMaxSharedComponentIdCount> sharedComponentMask;
    for (const unsigned int& sharedCmptId : sharedComponentIds)
        sharedComponentMask.set(sharedCmptId);
    ArchetypeMask mask{componentMask, sharedComponentMask};
    if (_archetypeMap.contains(mask))
        return _archetypeMap.at(mask);
    return createArchetype(std::move(mask), std::move(componentIds), {sizeof(Ts)...}, {alignof(Ts)...}, std::move(sharedComponentIds));
}

template <typename... Ts, typename... Us>
Entity EntityManager::createEntity(TypeMark<Ts...> componentTypeMark, TypeMark<Us...> sharedComponentMark) {
    Archetype* const archetype = createArchetype(componentTypeMark, sharedComponentMark);
    return createEntity(archetype);
}

template <typename T>
void EntityManager::addComponent(const Entity& entity, const T& component) {
    unsigned int componentId = registerComponent(typeid(T));
    _mainEntityCommandBuffer.addComponent(entity, component);
}

template <typename T>
void EntityManager::removeComponent(const Entity& entity) {
    unsigned int componentId = _componentIdMap.at(typeid(T));
    _mainEntityCommandBuffer.removeComponent<T>(entity);
}

template <typename T>
void EntityManager::setComponent(const Entity& entity, const T& component) {
    unsigned int componentId = _componentIdMap.at(typeid(T));
    _mainEntityCommandBuffer.setComponent(entity, component);
}

template <typename T>
void EntityManager::addSharedComponent(const Entity& entity, const T& component) {
    unsigned int componentId = registerComponent(typeid(T));
    _mainEntityCommandBuffer.addSharedComponent<T>(entity, component);
}

template <typename T>
void EntityManager::removeSharedComponent(const Entity& entity) {
    unsigned int componentId = registerComponent(typeid(T));
    _mainEntityCommandBuffer.removeSharedComponent<T>(entity);
}

template <typename T>
void EntityManager::setSharedComponent(const Entity& entity, const T& sharedComponent) {
    unsigned int componentId = registerComponent(typeid(T));
    _mainEntityCommandBuffer.setSharedComponent<T>(entity, sharedComponent);
}

template <typename T>
unsigned int EntityManager::componentId() {
    return registerComponent<T>();
}

template <typename T>
unsigned int EntityManager::sharedComponentId() {
    return registerSharedComponent<T>();
}

template <typename... Ts, typename... Us>
const EntityFilter EntityManager::createEntityFilter(TypeMark<Ts...> componentTypeMark, TypeMark<Us...> sharedComponentTypeMark) {
    std::array<unsigned int, sizeof...(Ts)> componentIds{registerComponent<Ts>()...};
    std::bitset<ArchetypeMask::kMaxComponentIdCount> componentMask;
    for (const unsigned int& cmptId : componentIds)
        componentMask.set(cmptId);
    std::array<unsigned int, sizeof...(Us)> sharedComponentIds{registerSharedComponent<Us>()...};
    std::bitset<ArchetypeMask::kMaxSharedComponentIdCount> sharedComponentMask;
    for (const unsigned int& sharedCmptId : sharedComponentIds)
        sharedComponentMask.set(sharedCmptId);
    return EntityFilter{std::move(componentMask), std::move(sharedComponentMask)};
}

template <typename T>
unsigned int EntityManager::registerComponent() {
    return registerComponent(typeid(T));
}

template <typename T>
unsigned int EntityManager::registerSharedComponent() {
    return registerSharedComponent(typeid(T));
}

template <typename... Ts, typename... Us>
void EntityManager::createEntityImmediately(const Entity& entity, TypeMark<Ts...> componentTypeMark, TypeMark<Us...> sharedComponentTypeMark) {
    Archetype* const archetype = createArchetype(componentTypeMark, sharedComponentTypeMark);
    createEntityImmediately(entity, archetype);
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

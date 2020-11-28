#pragma once

#include <MelonCore/Archetype.h>
#include <MelonCore/ChunkAccessor.h>
#include <MelonCore/Combination.h>
#include <MelonCore/Component.h>
#include <MelonCore/Entity.h>
#include <MelonCore/EntityFilter.h>
#include <MelonCore/ObjectPool.h>
#include <MelonCore/ObjectStore.h>
#include <MelonCore/SharedComponent.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <tuple>
#include <type_traits>
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
    EntityCommandBuffer(EntityManager* entityManager) noexcept;

    Entity createEntity();
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

    EntityCommandBuffer* createEntityCommandBuffer() { return _taskEntityCommandBuffers.emplace_back(std::make_unique<EntityCommandBuffer>(this)).get(); }

    Entity createEntity();
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

    EntityFilterBuilder createEntityFilterBuilder() { return EntityFilterBuilder(this); }
    std::vector<ChunkAccessor> filterEntities(const EntityFilter& entityFilter);

    template <typename T>
    unsigned int componentId();
    template <typename T>
    unsigned int sharedComponentId();

    template <typename T>
    const T* sharedComponent(const unsigned int& sharedComponentIndex) const;
    template <typename T>
    unsigned int sharedComponentIndex(const T& sharedComponent) const;

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
    void createEntityImmediately(const Entity& entity);
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

    void destroyEntityWithoutCheck(const Entity& entity, Archetype* archetype, const Archetype::EntityLocation& location);
    void removeComponentWithoutCheck(const Entity& entity, const unsigned int& componentId, const bool& manual);
    void removeSharedComponentWithoutCheck(const Entity& entity, const unsigned int& sharedComponentId, const bool& manual);

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

    std::vector<Archetype::EntityLocation> _entityLocations;

    EntityCommandBuffer _mainEntityCommandBuffer;
    std::vector<std::unique_ptr<EntityCommandBuffer>> _taskEntityCommandBuffers;

    friend class ArchetypeBuilder;
    friend class EntityCommandBuffer;
    friend class World;
    friend class SystemBase;
};

template <typename... Ts>
ArchetypeBuilder& ArchetypeBuilder::markComponents() {
    // TODO: Check if derived from Component, which should be encapsulated in a method
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<Component, Ts>>...>, std::tuple<std::bool_constant<std::is_base_of_v<Component, Ts>>..., std::true_type>>);
    const std::vector<unsigned int> componentIds{_entityManager->componentId<Ts>()...};
    const std::vector<std::size_t> componentSizes{sizeof(Ts)...};
    const std::vector<std::size_t> componentAligns{alignof(Ts)...};
    _componentIds.insert(_componentIds.end(), componentIds.begin(), componentIds.end());
    _componentSizes.insert(_componentSizes.end(), componentSizes.begin(), componentSizes.end());
    _componentAligns.insert(_componentAligns.end(), componentAligns.begin(), componentAligns.end());
    _mask.markComponents(componentIds, {std::is_base_of_v<ManualComponent, Ts>...});
    return *this;
}

template <typename... Ts>
ArchetypeBuilder& ArchetypeBuilder::markSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>..., std::true_type>>);
    const std::vector<unsigned int> sharedComponentIds{_entityManager->sharedComponentId<Ts>()...};
    _sharedComponentIds.insert(_sharedComponentIds.end(), sharedComponentIds.begin(), sharedComponentIds.end());
    _mask.markSharedComponents(sharedComponentIds, {std::is_base_of_v<ManualSharedComponent, Ts>...});
    return *this;
}

inline Archetype* ArchetypeBuilder::createArchetype() {
    return _entityManager->createArchetype(std::move(_mask), std::move(_componentIds), std::move(_componentSizes), std::move(_componentAligns), std::move(_sharedComponentIds));
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::requireComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<Component, Ts>>...>, std::tuple<std::bool_constant<std::is_base_of_v<Component, Ts>>..., std::true_type>>);
    const std::vector<unsigned int>& componentIds{_entityManager->componentId<Ts>()...};
    for (const unsigned int& cmptId : componentIds)
        _entityFilter.requiredComponentMask.set(cmptId);
    return *this;
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::rejectComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<Component, Ts>>...>, std::tuple<std::bool_constant<std::is_base_of_v<Component, Ts>>..., std::true_type>>);
    const std::vector<unsigned int>& componentIds{_entityManager->componentId<Ts>()...};
    for (const unsigned int& cmptId : componentIds)
        _entityFilter.rejectedComponentMask.set(cmptId);
    return *this;
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::requireSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>..., std::true_type>>);
    const std::vector<unsigned int>& sharedComponentIds{_entityManager->sharedComponentId<Ts>()...};
    for (const unsigned int& sharedCmptId : sharedComponentIds)
        _entityFilter.requiredSharedComponentMask.set(sharedCmptId);
    return *this;
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::rejectSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>..., std::true_type>>);
    const std::vector<unsigned int>& sharedComponentIds{_entityManager->sharedComponentId<Ts>()...};
    for (const unsigned int& sharedCmptId : sharedComponentIds)
        _entityFilter.rejectedSharedComponentMask.set(sharedCmptId);
    return *this;
}

template <typename T>
EntityFilterBuilder& EntityFilterBuilder::requireSharedComponent(const T& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    const unsigned int sharedComponentId = _entityManager->sharedComponentId<T>();
    const unsigned int sharedComponentIndex = _entityManager->sharedComponentIndex(sharedComponent);
    _entityFilter.requiredSharedComponentIdAndIndices.emplace_back(std::make_pair(sharedComponentId, sharedComponentIndex));
}

template <typename T>
EntityFilterBuilder& EntityFilterBuilder::rejectSharedComponent(const T& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
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
    static_assert(std::is_base_of_v<Component, T>);
    _procedures.emplace_back([this, entity, component]() {
        _entityManager->addComponentImmediately(entity, component);
    });
}

template <typename T>
void EntityCommandBuffer::removeComponent(const Entity& entity) {
    static_assert(std::is_base_of_v<Component, T>);
    _procedures.emplace_back([this, entity]() {
        _entityManager->removeComponentImmediately<T>(entity);
    });
}

template <typename T>
void EntityCommandBuffer::setComponent(const Entity& entity, const T& component) {
    static_assert(std::is_base_of_v<Component, T>);
    _procedures.emplace_back([this, entity, component]() {
        _entityManager->setComponentImmediately(entity, component);
    });
}

template <typename T>
void EntityCommandBuffer::addSharedComponent(const Entity& entity, const T& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _procedures.emplace_back([this, entity, sharedComponent]() {
        _entityManager->addSharedComponentImmediately(entity, sharedComponent);
    });
}

template <typename T>
void EntityCommandBuffer::removeSharedComponent(const Entity& entity) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _procedures.emplace_back([this, entity]() {
        _entityManager->removeSharedComponentImmediately<T>(entity);
    });
}

template <typename T>
void EntityCommandBuffer::setSharedComponent(const Entity& entity, const T& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _procedures.emplace_back([this, entity, sharedComponent]() {
        _entityManager->setSharedComponentImmediately(entity, sharedComponent);
    });
}

template <typename T>
void EntityManager::addComponent(const Entity& entity, const T& component) {
    static_assert(std::is_base_of_v<Component, T>);
    _mainEntityCommandBuffer.addComponent(entity, component);
}

template <typename T>
void EntityManager::removeComponent(const Entity& entity) {
    static_assert(std::is_base_of_v<Component, T>);
    _mainEntityCommandBuffer.removeComponent<T>(entity);
}

template <typename T>
void EntityManager::setComponent(const Entity& entity, const T& component) {
    static_assert(std::is_base_of_v<Component, T>);
    _mainEntityCommandBuffer.setComponent(entity, component);
}

template <typename T>
void EntityManager::addSharedComponent(const Entity& entity, const T& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _mainEntityCommandBuffer.addSharedComponent(entity, sharedComponent);
}

template <typename T>
void EntityManager::removeSharedComponent(const Entity& entity) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _mainEntityCommandBuffer.removeSharedComponent<T>(entity);
}

template <typename T>
void EntityManager::setSharedComponent(const Entity& entity, const T& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _mainEntityCommandBuffer.setSharedComponent(entity, sharedComponent);
}

template <typename T>
unsigned int EntityManager::componentId() {
    static_assert(std::is_base_of_v<Component, T>);
    return registerComponent<T>();
}

template <typename T>
unsigned int EntityManager::sharedComponentId() {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    return registerSharedComponent<T>();
}

template <typename T>
const T* EntityManager::sharedComponent(const unsigned int& sharedComponentIndex) const {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    return _sharedComponentStore.object<T>(sharedComponentIndex);
}

template <typename T>
unsigned int EntityManager::sharedComponentIndex(const T& sharedComponent) const {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    const unsigned int sharedComponentId = registerSharedComponent<T>();
    return _sharedComponentStore.objectIndex(sharedComponentId, sharedComponent);
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
    const Archetype::EntityLocation srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    const unsigned int componentId = registerComponent<T>();
    ArchetypeMask mask = srcArchetype->mask();
    mask.markComponent(componentId, std::is_base_of_v<ManualComponent, T>);

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
    Archetype::EntityLocation dstLocation;
    dstArchetype->moveEntityAddingComponent(srcLocation, srcArchetype, componentId, static_cast<const void*>(&component), dstLocation, srcSwappedEntity);
    _entityLocations[entity.id] = dstLocation;
    if (srcSwappedEntity.valid())
        _entityLocations[srcSwappedEntity.id] = srcLocation;
}

template <typename T>
void EntityManager::removeComponentImmediately(const Entity& entity) {
    const Archetype::EntityLocation srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    // If the archetype is single and manual, it should be destroyed;
    if (srcArchetype->single() && srcArchetype->fullyManual()) {
        destroyEntityWithoutCheck(entity, srcArchetype, srcLocation);
        return;
    }
    removeComponentWithoutCheck(entity, registerComponent<T>(), std::is_base_of_v<ManualComponent, T>);
}

template <typename T>
void EntityManager::setComponentImmediately(const Entity& entity, const T& component) {
    const Archetype::EntityLocation location = _entityLocations[entity.id];
    Archetype* const archetype = _archetypes[location.archetypeId].get();
    const unsigned int componentId = _componentIdMap.at(typeid(T));
    archetype->setComponent(location, componentId, static_cast<const void*>(&component));
}

template <typename T>
void EntityManager::addSharedComponentImmediately(const Entity& entity, const T& sharedComponent) {
    const Archetype::EntityLocation srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    const unsigned int sharedComponentId = registerSharedComponent<T>();
    ArchetypeMask mask = srcArchetype->mask();
    mask.markSharedComponent(sharedComponentId, std::is_base_of_v<ManualSharedComponent, T>);

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
    Archetype::EntityLocation dstLocation;
    dstArchetype->moveEntityAddingSharedComponent(srcLocation, srcArchetype, sharedComponentId, sharedComponentIndex, dstLocation, srcSwappedEntity);
    _entityLocations[entity.id] = dstLocation;
    if (srcSwappedEntity.valid())
        _entityLocations[srcSwappedEntity.id] = srcLocation;
}

template <typename T>
void EntityManager::removeSharedComponentImmediately(const Entity& entity) {
    const Archetype::EntityLocation srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    // If the archetype is single and manual, it should be destroyed;
    if (srcArchetype->single() && srcArchetype->fullyManual()) {
        destroyEntityWithoutCheck(entity, srcArchetype, srcLocation);
        return;
    }
    removeSharedComponentWithoutCheck(entity, registerSharedComponent<T>(), std::is_base_of_v<ManualSharedComponent, T>);
}

template <typename T>
void EntityManager::setSharedComponentImmediately(const Entity& entity, const T& sharedComponent) {
    const Archetype::EntityLocation location = _entityLocations[entity.id];
    Archetype* const archetype = _archetypes[location.archetypeId].get();
    const unsigned int sharedComponentId = registerSharedComponent<T>();

    unsigned int sharedComponentIndex = _sharedComponentStore.push(sharedComponentId, sharedComponent);

    unsigned int originalSharedComponentIndex;
    Archetype::EntityLocation dstLocation;
    Entity swappedEntity;
    archetype->setSharedComponent(location, sharedComponentId, sharedComponentIndex, originalSharedComponentIndex, dstLocation, swappedEntity);
    _entityLocations[entity.id] = dstLocation;
    if (swappedEntity.valid())
        _entityLocations[swappedEntity.id] = location;

    _sharedComponentStore.pop(sharedComponentId, originalSharedComponentIndex);
}

}  // namespace MelonCore

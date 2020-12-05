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
#include <MelonCore/SingletonComponent.h>
#include <MelonCore/SingletonObjectStore.h>

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
    EntityFilterBuilder& requireSharedComponent(T const& sharedComponent);
    template <typename T>
    EntityFilterBuilder& rejectSharedComponent(T const& sharedComponent);

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
    void destroyEntity(Entity const& entity);
    template <typename T>
    void addComponent(Entity const& entity, T const& component);
    template <typename T>
    void removeComponent(Entity const& entity);
    template <typename T>
    void setComponent(Entity const& entity, T const& component);
    template <typename T>
    void addSharedComponent(Entity const& entity, T const& sharedComponent);
    template <typename T>
    void removeSharedComponent(Entity const& entity);
    template <typename T>
    void setSharedComponent(Entity const& entity, T const& sharedComponent);
    template <typename T>
    void addSingletonComponent(T const& singletonComponent);
    template <typename T>
    void removeSingletonComponent();
    template <typename T>
    void setSingletonComponent(T const& singletonComponent);

  private:
    void execute();

    EntityManager* _entityManager;
    std::vector<std::function<void()>> _procedures;  // TODO: Improve performance

    friend class EntityManager;
};

class EntityManager {
  public:
    static constexpr unsigned int kMaxSingletonComponentIdCount = 256U;

    EntityManager();
    EntityManager(EntityManager const&) = delete;

    ArchetypeBuilder createArchetypeBuilder() { return ArchetypeBuilder(this); }

    EntityCommandBuffer* createEntityCommandBuffer() { return _taskEntityCommandBuffers.emplace_back(std::make_unique<EntityCommandBuffer>(this)).get(); }

    Entity createEntity();
    Entity createEntity(Archetype* archetype);
    void destroyEntity(Entity const& entity);
    template <typename T>
    void addComponent(Entity const& entity, T const& component);
    template <typename T>
    void removeComponent(Entity const& entity);
    template <typename T>
    void setComponent(Entity const& entity, T const& component);
    template <typename T>
    void addSharedComponent(Entity const& entity, T const& sharedComponent);
    template <typename T>
    void removeSharedComponent(Entity const& entity);
    template <typename T>
    void setSharedComponent(Entity const& entity, T const& sharedComponent);
    template <typename T>
    void addSingletonComponent(T const& singletonComponent);
    template <typename T>
    void removeSingletonComponent();
    template <typename T>
    void setSingletonComponent(T const& singletonComponent);

    EntityFilterBuilder createEntityFilterBuilder() { return EntityFilterBuilder(this); }
    std::vector<ChunkAccessor> filterEntities(EntityFilter const& entityFilter);

    template <typename T>
    unsigned int componentId();
    template <typename T>
    unsigned int sharedComponentId();
    template <typename T>
    unsigned int singletonComponentId();

    template <typename T>
    T const* sharedComponent(unsigned int const& sharedComponentIndex) const;
    template <typename T>
    unsigned int sharedComponentIndex(T const& sharedComponent) const;

    template <typename T>
    T* singletonComponent(unsigned int const& singletonComponentId) const;

    unsigned int chunkCount(EntityFilter const& entityFilter) const;
    unsigned int entityCount(EntityFilter const& entityFilter) const;

  private:
    template <typename T>
    unsigned int registerComponent();
    unsigned int registerComponent(std::type_index const& typeIndex);
    template <typename T>
    unsigned int registerSharedComponent();
    unsigned int registerSharedComponent(std::type_index const& typeIndex);
    template <typename T>
    unsigned int registerSingletonComponent();
    unsigned int registerSingletonComponent(std::type_index const& typeIndex);
    Archetype* createArchetype(ArchetypeMask&& mask, std::vector<unsigned int>&& componentIds, std::vector<std::size_t>&& componentSizes, std::vector<std::size_t>&& componentAligns, std::vector<unsigned int>&& sharedComponentIds);
    Entity assignEntity();
    void createEntityImmediately(Entity const& entity);
    void createEntityImmediately(Entity const& entity, Archetype* archetype);
    void destroyEntityImmediately(Entity const& entityId);
    template <typename T>
    void addComponentImmediately(Entity const& entity, T const& component);
    template <typename T>
    void removeComponentImmediately(Entity const& entity);
    template <typename T>
    void setComponentImmediately(Entity const& entity, T const& component);
    template <typename T>
    void addSharedComponentImmediately(Entity const& entity, T const& sharedComponent);
    template <typename T>
    void removeSharedComponentImmediately(Entity const& entity);
    template <typename T>
    void setSharedComponentImmediately(Entity const& entity, T const& sharedComponent);
    template <typename T>
    void addSingletonComponentImmediately(T const& singletonComponent);
    template <typename T>
    void removeSingletonComponentImmediately();
    template <typename T>
    void setSingletonComponentImmediately(T const& singletonComponent);

    void destroyEntityWithoutCheck(Entity const& entity, Archetype* archetype, Archetype::EntityLocation const& location);
    void removeComponentWithoutCheck(Entity const& entity, unsigned int const& componentId, bool const& manual);
    void removeSharedComponentWithoutCheck(Entity const& entity, unsigned int const& sharedComponentId, bool const& manual);

    void executeEntityCommandBuffers();

    std::unordered_map<std::type_index, unsigned int> _componentIdMap;
    std::unordered_map<std::type_index, unsigned int> _sharedComponentIdMap;
    std::unordered_map<std::type_index, unsigned int> _singletonComponentIdMap;

    ObjectPool<Chunk> _chunkPool;

    ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount> _sharedComponentStore;
    SingletonObjectStore<kMaxSingletonComponentIdCount> _singletonComponentStore;

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
    std::vector<unsigned int> const componentIds{_entityManager->componentId<Ts>()...};
    std::vector<std::size_t> const componentSizes{sizeof(Ts)...};
    std::vector<std::size_t> const componentAligns{alignof(Ts)...};
    _componentIds.insert(_componentIds.end(), componentIds.begin(), componentIds.end());
    _componentSizes.insert(_componentSizes.end(), componentSizes.begin(), componentSizes.end());
    _componentAligns.insert(_componentAligns.end(), componentAligns.begin(), componentAligns.end());
    _mask.markComponents(componentIds, {std::is_base_of_v<ManualComponent, Ts>...});
    return *this;
}

template <typename... Ts>
ArchetypeBuilder& ArchetypeBuilder::markSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>..., std::true_type>>);
    std::vector<unsigned int> const sharedComponentIds{_entityManager->sharedComponentId<Ts>()...};
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
    std::vector<unsigned int> const& componentIds{_entityManager->componentId<Ts>()...};
    for (unsigned int const& cmptId : componentIds)
        _entityFilter.requiredComponentMask.set(cmptId);
    return *this;
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::rejectComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<Component, Ts>>...>, std::tuple<std::bool_constant<std::is_base_of_v<Component, Ts>>..., std::true_type>>);
    std::vector<unsigned int> const& componentIds{_entityManager->componentId<Ts>()...};
    for (unsigned int const& cmptId : componentIds)
        _entityFilter.rejectedComponentMask.set(cmptId);
    return *this;
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::requireSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>..., std::true_type>>);
    std::vector<unsigned int> const& sharedComponentIds{_entityManager->sharedComponentId<Ts>()...};
    for (unsigned int const& sharedCmptId : sharedComponentIds)
        _entityFilter.requiredSharedComponentMask.set(sharedCmptId);
    return *this;
}

template <typename... Ts>
EntityFilterBuilder& EntityFilterBuilder::rejectSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Ts>>..., std::true_type>>);
    std::vector<unsigned int> const& sharedComponentIds{_entityManager->sharedComponentId<Ts>()...};
    for (unsigned int const& sharedCmptId : sharedComponentIds)
        _entityFilter.rejectedSharedComponentMask.set(sharedCmptId);
    return *this;
}

template <typename T>
EntityFilterBuilder& EntityFilterBuilder::requireSharedComponent(T const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    unsigned int const sharedComponentId = _entityManager->sharedComponentId<T>();
    unsigned int const sharedComponentIndex = _entityManager->sharedComponentIndex(sharedComponent);
    _entityFilter.requiredSharedComponentIdAndIndices.emplace_back(std::make_pair(sharedComponentId, sharedComponentIndex));
}

template <typename T>
EntityFilterBuilder& EntityFilterBuilder::rejectSharedComponent(T const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    unsigned int const sharedComponentId = _entityManager->sharedComponentId<T>();
    unsigned int const sharedComponentIndex = _entityManager->sharedComponentIndex(sharedComponent);
    _entityFilter.rejectedSharedComponentIdAndIndices.emplace_back(std::make_pair(sharedComponentId, sharedComponentIndex));
}

inline EntityFilter EntityFilterBuilder::createEntityFilter() {
    std::sort(_entityFilter.requiredSharedComponentIdAndIndices.begin(), _entityFilter.requiredSharedComponentIdAndIndices.end());
    return std::move(_entityFilter);
}

template <typename T>
void EntityCommandBuffer::addComponent(Entity const& entity, T const& component) {
    static_assert(std::is_base_of_v<Component, T>);
    _procedures.emplace_back([this, entity, component]() {
        _entityManager->addComponentImmediately(entity, component);
    });
}

template <typename T>
void EntityCommandBuffer::removeComponent(Entity const& entity) {
    static_assert(std::is_base_of_v<Component, T>);
    _procedures.emplace_back([this, entity]() {
        _entityManager->removeComponentImmediately<T>(entity);
    });
}

template <typename T>
void EntityCommandBuffer::setComponent(Entity const& entity, T const& component) {
    static_assert(std::is_base_of_v<Component, T>);
    _procedures.emplace_back([this, entity, component]() {
        _entityManager->setComponentImmediately(entity, component);
    });
}

template <typename T>
void EntityCommandBuffer::addSharedComponent(Entity const& entity, T const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _procedures.emplace_back([this, entity, sharedComponent]() {
        _entityManager->addSharedComponentImmediately(entity, sharedComponent);
    });
}

template <typename T>
void EntityCommandBuffer::removeSharedComponent(Entity const& entity) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _procedures.emplace_back([this, entity]() {
        _entityManager->removeSharedComponentImmediately<T>(entity);
    });
}

template <typename T>
void EntityCommandBuffer::setSharedComponent(Entity const& entity, T const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _procedures.emplace_back([this, entity, sharedComponent]() {
        _entityManager->setSharedComponentImmediately(entity, sharedComponent);
    });
}

template <typename T>
void EntityCommandBuffer::addSingletonComponent(T const& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, T>);
    _procedures.emplace_back([this, singletonComponent]() {
        _entityManager->addSingletonComponentImmediately(singletonComponent);
    });
}

template <typename T>
void EntityCommandBuffer::removeSingletonComponent() {
    static_assert(std::is_base_of_v<SingletonComponent, T>);
    _procedures.emplace_back([this]() {
        _entityManager->removeSingletonComponentImmediately<T>();
    });
}

template <typename T>
void EntityCommandBuffer::setSingletonComponent(T const& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, T>);
    _procedures.emplace_back([this, singletonComponent]() {
        _entityManager->setSingletonComponentImmediately(singletonComponent);
    });
}

template <typename T>
void EntityManager::addComponent(Entity const& entity, T const& component) {
    static_assert(std::is_base_of_v<Component, T>);
    _mainEntityCommandBuffer.addComponent(entity, component);
}

template <typename T>
void EntityManager::removeComponent(Entity const& entity) {
    static_assert(std::is_base_of_v<Component, T>);
    _mainEntityCommandBuffer.removeComponent<T>(entity);
}

template <typename T>
void EntityManager::setComponent(Entity const& entity, T const& component) {
    static_assert(std::is_base_of_v<Component, T>);
    _mainEntityCommandBuffer.setComponent(entity, component);
}

template <typename T>
void EntityManager::addSharedComponent(Entity const& entity, T const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _mainEntityCommandBuffer.addSharedComponent(entity, sharedComponent);
}

template <typename T>
void EntityManager::removeSharedComponent(Entity const& entity) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _mainEntityCommandBuffer.removeSharedComponent<T>(entity);
}

template <typename T>
void EntityManager::setSharedComponent(Entity const& entity, T const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    _mainEntityCommandBuffer.setSharedComponent(entity, sharedComponent);
}

template <typename T>
void EntityManager::addSingletonComponent(T const& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, T>);
    _mainEntityCommandBuffer.addSingletonComponent(singletonComponent);
}

template <typename T>
void EntityManager::removeSingletonComponent() {
    static_assert(std::is_base_of_v<SingletonComponent, T>);
    _mainEntityCommandBuffer.removeSingletonComponent<T>();
}

template <typename T>
void EntityManager::setSingletonComponent(T const& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, T>);
    _mainEntityCommandBuffer.setSingletonComponent(singletonComponent);
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
unsigned int EntityManager::singletonComponentId() {
    static_assert(std::is_base_of_v<SingletonComponent, T>);
    return registerSingletonComponent<T>();
}

template <typename T>
T const* EntityManager::sharedComponent(unsigned int const& sharedComponentIndex) const {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    return _sharedComponentStore.object<T>(sharedComponentIndex);
}

template <typename T>
unsigned int EntityManager::sharedComponentIndex(T const& sharedComponent) const {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    unsigned int const sharedComponentId = registerSharedComponent<T>();
    return _sharedComponentStore.objectIndex(sharedComponentId, sharedComponent);
}

template <typename T>
T* EntityManager::singletonComponent(unsigned int const& singletonComponentId) const {
    static_assert(std::is_base_of_v<SingletonComponent, T>);
    return _singletonComponentStore.object<T>(singletonComponentId);
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
unsigned int EntityManager::registerSingletonComponent() {
    return registerSingletonComponent(typeid(T));
}

template <typename T>
void EntityManager::addComponentImmediately(Entity const& entity, T const& component) {
    Archetype::EntityLocation const srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    unsigned int const componentId = registerComponent<T>();
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
    dstArchetype->moveEntityAddingComponent(srcLocation, srcArchetype, componentId, static_cast<void const*>(&component), dstLocation, srcSwappedEntity);
    _entityLocations[entity.id] = dstLocation;
    if (srcSwappedEntity.valid())
        _entityLocations[srcSwappedEntity.id] = srcLocation;
}

template <typename T>
void EntityManager::removeComponentImmediately(Entity const& entity) {
    Archetype::EntityLocation const srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    // If the archetype is single and manual, it should be destroyed;
    if (srcArchetype->single() && srcArchetype->fullyManual()) {
        destroyEntityWithoutCheck(entity, srcArchetype, srcLocation);
        return;
    }
    removeComponentWithoutCheck(entity, registerComponent<T>(), std::is_base_of_v<ManualComponent, T>);
}

template <typename T>
void EntityManager::setComponentImmediately(Entity const& entity, T const& component) {
    Archetype::EntityLocation const location = _entityLocations[entity.id];
    Archetype* const archetype = _archetypes[location.archetypeId].get();
    unsigned int const componentId = _componentIdMap.at(typeid(T));
    archetype->setComponent(location, componentId, static_cast<void const*>(&component));
}

template <typename T>
void EntityManager::addSharedComponentImmediately(Entity const& entity, T const& sharedComponent) {
    Archetype::EntityLocation const srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    unsigned int const sharedComponentId = registerSharedComponent<T>();
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
void EntityManager::removeSharedComponentImmediately(Entity const& entity) {
    Archetype::EntityLocation const srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    // If the archetype is single and manual, it should be destroyed;
    if (srcArchetype->single() && srcArchetype->fullyManual()) {
        destroyEntityWithoutCheck(entity, srcArchetype, srcLocation);
        return;
    }
    removeSharedComponentWithoutCheck(entity, registerSharedComponent<T>(), std::is_base_of_v<ManualSharedComponent, T>);
}

template <typename T>
void EntityManager::setSharedComponentImmediately(Entity const& entity, T const& sharedComponent) {
    Archetype::EntityLocation const location = _entityLocations[entity.id];
    Archetype* const archetype = _archetypes[location.archetypeId].get();
    unsigned int const sharedComponentId = registerSharedComponent<T>();

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

template <typename T>
void EntityManager::addSingletonComponentImmediately(T const& singletonComponent) {
    _singletonComponentStore.push(registerSingletonComponent<T>(), singletonComponent);
}

template <typename T>
void EntityManager::removeSingletonComponentImmediately() {
    _singletonComponentStore.pop(registerSingletonComponent<T>());
}

template <typename T>
void EntityManager::setSingletonComponentImmediately(T const& singletonComponent) {
    *_singletonComponentStore.object(registerSingletonComponent<T>()) = singletonComponent;
}

}  // namespace MelonCore

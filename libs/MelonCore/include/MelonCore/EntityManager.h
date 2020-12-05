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
    template <typename... Types>
    ArchetypeBuilder& markComponents();
    template <typename... Types>
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
    template <typename... Types>
    EntityFilterBuilder& requireComponents();
    template <typename... Types>
    EntityFilterBuilder& rejectComponents();

    template <typename... Types>
    EntityFilterBuilder& requireSharedComponents();
    template <typename... Types>
    EntityFilterBuilder& rejectSharedComponents();

    template <typename Type>
    EntityFilterBuilder& requireSharedComponent(Type const& sharedComponent);
    template <typename Type>
    EntityFilterBuilder& rejectSharedComponent(Type const& sharedComponent);

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
    template <typename Type>
    void addComponent(Entity const& entity, Type const& component);
    template <typename Type>
    void removeComponent(Entity const& entity);
    template <typename Type>
    void setComponent(Entity const& entity, Type const& component);
    template <typename Type>
    void addSharedComponent(Entity const& entity, Type const& sharedComponent);
    template <typename Type>
    void removeSharedComponent(Entity const& entity);
    template <typename Type>
    void setSharedComponent(Entity const& entity, Type const& sharedComponent);
    template <typename Type>
    void addSingletonComponent(Type const& singletonComponent);
    template <typename Type>
    void removeSingletonComponent();
    template <typename Type>
    void setSingletonComponent(Type const& singletonComponent);

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
    template <typename Type>
    void addComponent(Entity const& entity, Type const& component);
    template <typename Type>
    void removeComponent(Entity const& entity);
    template <typename Type>
    void setComponent(Entity const& entity, Type const& component);
    template <typename Type>
    void addSharedComponent(Entity const& entity, Type const& sharedComponent);
    template <typename Type>
    void removeSharedComponent(Entity const& entity);
    template <typename Type>
    void setSharedComponent(Entity const& entity, Type const& sharedComponent);
    template <typename Type>
    void addSingletonComponent(Type const& singletonComponent);
    template <typename Type>
    void removeSingletonComponent();
    template <typename Type>
    void setSingletonComponent(Type const& singletonComponent);

    EntityFilterBuilder createEntityFilterBuilder() { return EntityFilterBuilder(this); }
    std::vector<ChunkAccessor> filterEntities(EntityFilter const& entityFilter);

    template <typename Type>
    unsigned int componentId();
    template <typename Type>
    unsigned int sharedComponentId();
    template <typename Type>
    unsigned int singletonComponentId();

    template <typename Type>
    Type const* sharedComponent(unsigned int const& sharedComponentIndex) const;
    template <typename Type>
    unsigned int sharedComponentIndex(Type const& sharedComponent) const;

    template <typename Type>
    Type* singletonComponent(unsigned int const& singletonComponentId) const;

    unsigned int chunkCount(EntityFilter const& entityFilter) const;
    unsigned int entityCount(EntityFilter const& entityFilter) const;

  private:
    template <typename Type>
    unsigned int registerComponent();
    unsigned int registerComponent(std::type_index const& typeIndex);
    template <typename Type>
    unsigned int registerSharedComponent();
    unsigned int registerSharedComponent(std::type_index const& typeIndex);
    template <typename Type>
    unsigned int registerSingletonComponent();
    unsigned int registerSingletonComponent(std::type_index const& typeIndex);
    Archetype* createArchetype(ArchetypeMask&& mask, std::vector<unsigned int>&& componentIds, std::vector<std::size_t>&& componentSizes, std::vector<std::size_t>&& componentAligns, std::vector<unsigned int>&& sharedComponentIds);
    Entity assignEntity();
    void createEntityImmediately(Entity const& entity);
    void createEntityImmediately(Entity const& entity, Archetype* archetype);
    void destroyEntityImmediately(Entity const& entityId);
    template <typename Type>
    void addComponentImmediately(Entity const& entity, Type const& component);
    template <typename Type>
    void removeComponentImmediately(Entity const& entity);
    template <typename Type>
    void setComponentImmediately(Entity const& entity, Type const& component);
    template <typename Type>
    void addSharedComponentImmediately(Entity const& entity, Type const& sharedComponent);
    template <typename Type>
    void removeSharedComponentImmediately(Entity const& entity);
    template <typename Type>
    void setSharedComponentImmediately(Entity const& entity, Type const& sharedComponent);
    template <typename Type>
    void addSingletonComponentImmediately(Type const& singletonComponent);
    template <typename Type>
    void removeSingletonComponentImmediately();
    template <typename Type>
    void setSingletonComponentImmediately(Type const& singletonComponent);

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

template <typename... Types>
ArchetypeBuilder& ArchetypeBuilder::markComponents() {
    // TODO: Check if derived from Component, which should be encapsulated in a method
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<Component, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<Component, Types>>..., std::true_type>>);
    std::vector<unsigned int> const componentIds{_entityManager->componentId<Types>()...};
    std::vector<std::size_t> const componentSizes{sizeof(Types)...};
    std::vector<std::size_t> const componentAligns{alignof(Types)...};
    _componentIds.insert(_componentIds.end(), componentIds.begin(), componentIds.end());
    _componentSizes.insert(_componentSizes.end(), componentSizes.begin(), componentSizes.end());
    _componentAligns.insert(_componentAligns.end(), componentAligns.begin(), componentAligns.end());
    _mask.markComponents(componentIds, {std::is_base_of_v<ManualComponent, Types>...});
    return *this;
}

template <typename... Types>
ArchetypeBuilder& ArchetypeBuilder::markSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Types>>..., std::true_type>>);
    std::vector<unsigned int> const sharedComponentIds{_entityManager->sharedComponentId<Types>()...};
    _sharedComponentIds.insert(_sharedComponentIds.end(), sharedComponentIds.begin(), sharedComponentIds.end());
    _mask.markSharedComponents(sharedComponentIds, {std::is_base_of_v<ManualSharedComponent, Types>...});
    return *this;
}

inline Archetype* ArchetypeBuilder::createArchetype() {
    return _entityManager->createArchetype(std::move(_mask), std::move(_componentIds), std::move(_componentSizes), std::move(_componentAligns), std::move(_sharedComponentIds));
}

template <typename... Types>
EntityFilterBuilder& EntityFilterBuilder::requireComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<Component, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<Component, Types>>..., std::true_type>>);
    std::vector<unsigned int> const& componentIds{_entityManager->componentId<Types>()...};
    for (unsigned int const& cmptId : componentIds)
        _entityFilter.requiredComponentMask.set(cmptId);
    return *this;
}

template <typename... Types>
EntityFilterBuilder& EntityFilterBuilder::rejectComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<Component, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<Component, Types>>..., std::true_type>>);
    std::vector<unsigned int> const& componentIds{_entityManager->componentId<Types>()...};
    for (unsigned int const& cmptId : componentIds)
        _entityFilter.rejectedComponentMask.set(cmptId);
    return *this;
}

template <typename... Types>
EntityFilterBuilder& EntityFilterBuilder::requireSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Types>>..., std::true_type>>);
    std::vector<unsigned int> const& sharedComponentIds{_entityManager->sharedComponentId<Types>()...};
    for (unsigned int const& sharedCmptId : sharedComponentIds)
        _entityFilter.requiredSharedComponentMask.set(sharedCmptId);
    return *this;
}

template <typename... Types>
EntityFilterBuilder& EntityFilterBuilder::rejectSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Types>>..., std::true_type>>);
    std::vector<unsigned int> const& sharedComponentIds{_entityManager->sharedComponentId<Types>()...};
    for (unsigned int const& sharedCmptId : sharedComponentIds)
        _entityFilter.rejectedSharedComponentMask.set(sharedCmptId);
    return *this;
}

template <typename Type>
EntityFilterBuilder& EntityFilterBuilder::requireSharedComponent(Type const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    unsigned int const sharedComponentId = _entityManager->sharedComponentId<Type>();
    unsigned int const sharedComponentIndex = _entityManager->sharedComponentIndex(sharedComponent);
    _entityFilter.requiredSharedComponentIdAndIndices.emplace_back(std::make_pair(sharedComponentId, sharedComponentIndex));
}

template <typename Type>
EntityFilterBuilder& EntityFilterBuilder::rejectSharedComponent(Type const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    unsigned int const sharedComponentId = _entityManager->sharedComponentId<Type>();
    unsigned int const sharedComponentIndex = _entityManager->sharedComponentIndex(sharedComponent);
    _entityFilter.rejectedSharedComponentIdAndIndices.emplace_back(std::make_pair(sharedComponentId, sharedComponentIndex));
}

inline EntityFilter EntityFilterBuilder::createEntityFilter() {
    std::sort(_entityFilter.requiredSharedComponentIdAndIndices.begin(), _entityFilter.requiredSharedComponentIdAndIndices.end());
    return std::move(_entityFilter);
}

template <typename Type>
void EntityCommandBuffer::addComponent(Entity const& entity, Type const& component) {
    static_assert(std::is_base_of_v<Component, Type>);
    _procedures.emplace_back([this, entity, component]() {
        _entityManager->addComponentImmediately(entity, component);
    });
}

template <typename Type>
void EntityCommandBuffer::removeComponent(Entity const& entity) {
    static_assert(std::is_base_of_v<Component, Type>);
    _procedures.emplace_back([this, entity]() {
        _entityManager->removeComponentImmediately<Type>(entity);
    });
}

template <typename Type>
void EntityCommandBuffer::setComponent(Entity const& entity, Type const& component) {
    static_assert(std::is_base_of_v<Component, Type>);
    _procedures.emplace_back([this, entity, component]() {
        _entityManager->setComponentImmediately(entity, component);
    });
}

template <typename Type>
void EntityCommandBuffer::addSharedComponent(Entity const& entity, Type const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    _procedures.emplace_back([this, entity, sharedComponent]() {
        _entityManager->addSharedComponentImmediately(entity, sharedComponent);
    });
}

template <typename Type>
void EntityCommandBuffer::removeSharedComponent(Entity const& entity) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    _procedures.emplace_back([this, entity]() {
        _entityManager->removeSharedComponentImmediately<Type>(entity);
    });
}

template <typename Type>
void EntityCommandBuffer::setSharedComponent(Entity const& entity, Type const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    _procedures.emplace_back([this, entity, sharedComponent]() {
        _entityManager->setSharedComponentImmediately(entity, sharedComponent);
    });
}

template <typename Type>
void EntityCommandBuffer::addSingletonComponent(Type const& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    _procedures.emplace_back([this, singletonComponent]() {
        _entityManager->addSingletonComponentImmediately(singletonComponent);
    });
}

template <typename Type>
void EntityCommandBuffer::removeSingletonComponent() {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    _procedures.emplace_back([this]() {
        _entityManager->removeSingletonComponentImmediately<Type>();
    });
}

template <typename Type>
void EntityCommandBuffer::setSingletonComponent(Type const& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    _procedures.emplace_back([this, singletonComponent]() {
        _entityManager->setSingletonComponentImmediately(singletonComponent);
    });
}

template <typename Type>
void EntityManager::addComponent(Entity const& entity, Type const& component) {
    static_assert(std::is_base_of_v<Component, Type>);
    _mainEntityCommandBuffer.addComponent(entity, component);
}

template <typename Type>
void EntityManager::removeComponent(Entity const& entity) {
    static_assert(std::is_base_of_v<Component, Type>);
    _mainEntityCommandBuffer.removeComponent<Type>(entity);
}

template <typename Type>
void EntityManager::setComponent(Entity const& entity, Type const& component) {
    static_assert(std::is_base_of_v<Component, Type>);
    _mainEntityCommandBuffer.setComponent(entity, component);
}

template <typename Type>
void EntityManager::addSharedComponent(Entity const& entity, Type const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    _mainEntityCommandBuffer.addSharedComponent(entity, sharedComponent);
}

template <typename Type>
void EntityManager::removeSharedComponent(Entity const& entity) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    _mainEntityCommandBuffer.removeSharedComponent<Type>(entity);
}

template <typename Type>
void EntityManager::setSharedComponent(Entity const& entity, Type const& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    _mainEntityCommandBuffer.setSharedComponent(entity, sharedComponent);
}

template <typename Type>
void EntityManager::addSingletonComponent(Type const& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    _mainEntityCommandBuffer.addSingletonComponent(singletonComponent);
}

template <typename Type>
void EntityManager::removeSingletonComponent() {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    _mainEntityCommandBuffer.removeSingletonComponent<Type>();
}

template <typename Type>
void EntityManager::setSingletonComponent(Type const& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    _mainEntityCommandBuffer.setSingletonComponent(singletonComponent);
}

template <typename Type>
unsigned int EntityManager::componentId() {
    static_assert(std::is_base_of_v<Component, Type>);
    return registerComponent<Type>();
}

template <typename Type>
unsigned int EntityManager::sharedComponentId() {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    return registerSharedComponent<Type>();
}

template <typename Type>
unsigned int EntityManager::singletonComponentId() {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    return registerSingletonComponent<Type>();
}

template <typename Type>
Type const* EntityManager::sharedComponent(unsigned int const& sharedComponentIndex) const {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    return _sharedComponentStore.object<Type>(sharedComponentIndex);
}

template <typename Type>
unsigned int EntityManager::sharedComponentIndex(Type const& sharedComponent) const {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    unsigned int const sharedComponentId = registerSharedComponent<Type>();
    return _sharedComponentStore.objectIndex(sharedComponentId, sharedComponent);
}

template <typename Type>
Type* EntityManager::singletonComponent(unsigned int const& singletonComponentId) const {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    return _singletonComponentStore.object<Type>(singletonComponentId);
}

template <typename Type>
unsigned int EntityManager::registerComponent() {
    return registerComponent(typeid(Type));
}

template <typename Type>
unsigned int EntityManager::registerSharedComponent() {
    return registerSharedComponent(typeid(Type));
}

template <typename Type>
unsigned int EntityManager::registerSingletonComponent() {
    return registerSingletonComponent(typeid(Type));
}

template <typename Type>
void EntityManager::addComponentImmediately(Entity const& entity, Type const& component) {
    Archetype::EntityLocation const srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    unsigned int const componentId = registerComponent<Type>();
    ArchetypeMask mask = srcArchetype->mask();
    mask.markComponent(componentId, std::is_base_of_v<ManualComponent, Type>);

    Archetype* dstArchetype;
    if (_archetypeMap.contains(mask))
        dstArchetype = _archetypeMap[mask];
    else {
        std::vector<unsigned int> componentIds = srcArchetype->componentIds();
        std::vector<std::size_t> componentSizes = srcArchetype->componentSizes();
        std::vector<std::size_t> componentAligns = srcArchetype->componentAligns();
        componentIds.push_back(componentId);
        componentSizes.emplace_back(sizeof(Type));
        componentAligns.emplace_back(alignof(Type));
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

template <typename Type>
void EntityManager::removeComponentImmediately(Entity const& entity) {
    Archetype::EntityLocation const srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    // If the archetype is single and manual, it should be destroyed;
    if (srcArchetype->single() && srcArchetype->fullyManual()) {
        destroyEntityWithoutCheck(entity, srcArchetype, srcLocation);
        return;
    }
    removeComponentWithoutCheck(entity, registerComponent<Type>(), std::is_base_of_v<ManualComponent, Type>);
}

template <typename Type>
void EntityManager::setComponentImmediately(Entity const& entity, Type const& component) {
    Archetype::EntityLocation const location = _entityLocations[entity.id];
    Archetype* const archetype = _archetypes[location.archetypeId].get();
    unsigned int const componentId = _componentIdMap.at(typeid(Type));
    archetype->setComponent(location, componentId, static_cast<void const*>(&component));
}

template <typename Type>
void EntityManager::addSharedComponentImmediately(Entity const& entity, Type const& sharedComponent) {
    Archetype::EntityLocation const srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    unsigned int const sharedComponentId = registerSharedComponent<Type>();
    ArchetypeMask mask = srcArchetype->mask();
    mask.markSharedComponent(sharedComponentId, std::is_base_of_v<ManualSharedComponent, Type>);

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

template <typename Type>
void EntityManager::removeSharedComponentImmediately(Entity const& entity) {
    Archetype::EntityLocation const srcLocation = _entityLocations[entity.id];
    Archetype* const srcArchetype = _archetypes[srcLocation.archetypeId].get();
    // If the archetype is single and manual, it should be destroyed;
    if (srcArchetype->single() && srcArchetype->fullyManual()) {
        destroyEntityWithoutCheck(entity, srcArchetype, srcLocation);
        return;
    }
    removeSharedComponentWithoutCheck(entity, registerSharedComponent<Type>(), std::is_base_of_v<ManualSharedComponent, Type>);
}

template <typename Type>
void EntityManager::setSharedComponentImmediately(Entity const& entity, Type const& sharedComponent) {
    Archetype::EntityLocation const location = _entityLocations[entity.id];
    Archetype* const archetype = _archetypes[location.archetypeId].get();
    unsigned int const sharedComponentId = registerSharedComponent<Type>();

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

template <typename Type>
void EntityManager::addSingletonComponentImmediately(Type const& singletonComponent) {
    _singletonComponentStore.push(registerSingletonComponent<Type>(), singletonComponent);
}

template <typename Type>
void EntityManager::removeSingletonComponentImmediately() {
    _singletonComponentStore.pop(registerSingletonComponent<Type>());
}

template <typename Type>
void EntityManager::setSingletonComponentImmediately(Type const& singletonComponent) {
    *_singletonComponentStore.object(registerSingletonComponent<Type>()) = singletonComponent;
}

}  // namespace MelonCore

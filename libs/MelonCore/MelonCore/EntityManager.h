#pragma once

#include <MelonCore/Archetype.h>
#include <MelonCore/ChunkAccessor.h>
#include <MelonCore/Combination.h>
#include <MelonCore/DataComponent.h>
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

namespace Melon {

class EntityManager;

class ArchetypeBuilder {
  public:
    template <typename... Types>
    ArchetypeBuilder& markComponents();
    template <typename... Types>
    ArchetypeBuilder& markSharedComponents();

    Archetype* createArchetype();

  private:
    ArchetypeBuilder(EntityManager* entityManager) : m_EntityManager(entityManager) {}
    std::vector<unsigned int> m_ComponentIds;
    std::vector<std::size_t> m_ComponentSizes;
    std::vector<std::size_t> m_ComponentAligns;
    std::vector<unsigned int> m_SharedComponentIds;
    ArchetypeMask m_Mask;
    EntityManager* m_EntityManager;

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
    EntityFilterBuilder& requireSharedComponent(const Type& sharedComponent);
    template <typename Type>
    EntityFilterBuilder& rejectSharedComponent(const Type& sharedComponent);

    EntityFilter createEntityFilter();

  private:
    EntityFilterBuilder(EntityManager* entityManager) : m_EntityManager(entityManager) {}
    EntityFilter m_EntityFilter;
    EntityManager* m_EntityManager;

    friend class EntityManager;
};

class EntityCommandBuffer {
  public:
    EntityCommandBuffer(EntityManager* entityManager) noexcept;

    Entity createEntity();
    Entity createEntity(Archetype* archetype);
    void destroyEntity(const Entity& entity);
    template <typename Type>
    void addComponent(const Entity& entity, const Type& component);
    template <typename Type>
    void removeComponent(const Entity& entity);
    template <typename Type>
    void setComponent(const Entity& entity, const Type& component);
    template <typename Type>
    void addSharedComponent(const Entity& entity, const Type& sharedComponent);
    template <typename Type>
    void removeSharedComponent(const Entity& entity);
    template <typename Type>
    void setSharedComponent(const Entity& entity, const Type& sharedComponent);
    template <typename Type>
    void addSingletonComponent(const Type& singletonComponent);
    template <typename Type>
    void removeSingletonComponent();
    template <typename Type>
    void setSingletonComponent(const Type& singletonComponent);

  private:
    void execute();

    EntityManager* m_EntityManager;
    std::vector<std::function<void()>> m_Procedures;  // TODO: Improve performance

    friend class EntityManager;
};

class EntityManager {
  public:
    static constexpr unsigned int k_MaxSingletonComponentIdCount = 256U;

    EntityManager();
    EntityManager(const EntityManager&) = delete;

    ArchetypeBuilder createArchetypeBuilder() { return ArchetypeBuilder(this); }

    EntityCommandBuffer* createEntityCommandBuffer() { return m_TaskEntityCommandBuffers.emplace_back(std::make_unique<EntityCommandBuffer>(this)).get(); }

    Entity createEntity();
    Entity createEntity(Archetype* archetype);
    void destroyEntity(const Entity& entity);
    template <typename Type>
    void addComponent(const Entity& entity, const Type& component);
    template <typename Type>
    void removeComponent(const Entity& entity);
    template <typename Type>
    void setComponent(const Entity& entity, const Type& component);
    template <typename Type>
    void addSharedComponent(const Entity& entity, const Type& sharedComponent);
    template <typename Type>
    void removeSharedComponent(const Entity& entity);
    template <typename Type>
    void setSharedComponent(const Entity& entity, const Type& sharedComponent);
    template <typename Type>
    void addSingletonComponent(const Type& singletonComponent);
    template <typename Type>
    void removeSingletonComponent();
    template <typename Type>
    void setSingletonComponent(const Type& singletonComponent);

    EntityFilterBuilder createEntityFilterBuilder() { return EntityFilterBuilder(this); }
    std::vector<ChunkAccessor> filterEntities(const EntityFilter& entityFilter);

    template <typename Type>
    unsigned int componentId();
    template <typename Type>
    unsigned int sharedComponentId();
    template <typename Type>
    unsigned int singletonComponentId();

    template <typename Type>
    const Type* sharedComponent(const unsigned int& sharedComponentIndex) const;
    template <typename Type>
    unsigned int sharedComponentIndex(const Type& sharedComponent) const;

    template <typename Type>
    Type* singletonComponent(const unsigned int& singletonComponentId) const;

    unsigned int chunkCount(const EntityFilter& entityFilter) const;
    unsigned int entityCount(const EntityFilter& entityFilter) const;

  private:
    template <typename Type>
    unsigned int registerComponent();
    unsigned int registerComponent(const std::type_index& typeIndex);
    template <typename Type>
    unsigned int registerSharedComponent();
    unsigned int registerSharedComponent(const std::type_index& typeIndex);
    template <typename Type>
    unsigned int registerSingletonComponent();
    unsigned int registerSingletonComponent(const std::type_index& typeIndex);
    Archetype* createArchetype(ArchetypeMask&& mask, std::vector<unsigned int>&& componentIds, std::vector<std::size_t>&& componentSizes, std::vector<std::size_t>&& componentAligns, std::vector<unsigned int>&& sharedComponentIds);
    Entity assignEntity();
    void createEntityImmediately(const Entity& entity);
    void createEntityImmediately(const Entity& entity, Archetype* archetype);
    void destroyEntityImmediately(const Entity& entityId);
    template <typename Type>
    void addComponentImmediately(const Entity& entity, const Type& component);
    template <typename Type>
    void removeComponentImmediately(const Entity& entity);
    template <typename Type>
    void setComponentImmediately(const Entity& entity, const Type& component);
    template <typename Type>
    void addSharedComponentImmediately(const Entity& entity, const Type& sharedComponent);
    template <typename Type>
    void removeSharedComponentImmediately(const Entity& entity);
    template <typename Type>
    void setSharedComponentImmediately(const Entity& entity, const Type& sharedComponent);
    template <typename Type>
    void addSingletonComponentImmediately(const Type& singletonComponent);
    template <typename Type>
    void removeSingletonComponentImmediately();
    template <typename Type>
    void setSingletonComponentImmediately(const Type& singletonComponent);

    void destroyEntityWithoutCheck(const Entity& entity, Archetype* archetype, const Archetype::EntityLocation& location);
    void removeComponentWithoutCheck(const Entity& entity, const unsigned int& componentId, const bool& manual);
    void removeSharedComponentWithoutCheck(const Entity& entity, const unsigned int& sharedComponentId, const bool& manual);

    void executeEntityCommandBuffers();

    std::unordered_map<std::type_index, unsigned int> m_ComponentIdMap;
    std::unordered_map<std::type_index, unsigned int> m_SharedComponentIdMap;
    std::unordered_map<std::type_index, unsigned int> m_SingletonComponentIdMap;

    ObjectPool<Chunk> m_ChunkPool;

    ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> m_SharedComponentStore;
    SingletonObjectStore<k_MaxSingletonComponentIdCount> m_SingletonComponentStore;

    unsigned int m_ArchetypeIdCounter{};
    std::unordered_map<ArchetypeMask, Archetype*, ArchetypeMask::Hash> m_ArchetypeMap;
    std::vector<std::unique_ptr<Archetype>> m_Archetypes;

    // TODO: To avoid contention, a few reserved Entity id could be passed to EntityCommandBuffer in main thread.
    std::mutex m_EntityIdMutex;
    unsigned int m_EntityIdCounter{};
    std::queue<unsigned int> m_FreeEntityIds;

    std::vector<Archetype::EntityLocation> m_EntityLocations;

    EntityCommandBuffer m_MainEntityCommandBuffer;
    std::vector<std::unique_ptr<EntityCommandBuffer>> m_TaskEntityCommandBuffers;

    friend class ArchetypeBuilder;
    friend class EntityCommandBuffer;
    friend class World;
    friend class SystemBase;
};

template <typename... Types>
ArchetypeBuilder& ArchetypeBuilder::markComponents() {
    // TODO: Check if derived from DataComponent, which should be encapsulated in a method
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<DataComponent, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<DataComponent, Types>>..., std::true_type>>);
    std::vector<unsigned int> const componentIds{m_EntityManager->componentId<Types>()...};
    std::vector<std::size_t> const componentSizes{sizeof(Types)...};
    std::vector<std::size_t> const componentAligns{alignof(Types)...};
    m_ComponentIds.insert(m_ComponentIds.end(), componentIds.begin(), componentIds.end());
    m_ComponentSizes.insert(m_ComponentSizes.end(), componentSizes.begin(), componentSizes.end());
    m_ComponentAligns.insert(m_ComponentAligns.end(), componentAligns.begin(), componentAligns.end());
    m_Mask.markComponents(componentIds, {std::is_base_of_v<ManualDataComponent, Types>...});
    return *this;
}

template <typename... Types>
ArchetypeBuilder& ArchetypeBuilder::markSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Types>>..., std::true_type>>);
    std::vector<unsigned int> const sharedComponentIds{m_EntityManager->sharedComponentId<Types>()...};
    m_SharedComponentIds.insert(m_SharedComponentIds.end(), sharedComponentIds.begin(), sharedComponentIds.end());
    m_Mask.markSharedComponents(sharedComponentIds, {std::is_base_of_v<ManualSharedComponent, Types>...});
    return *this;
}

inline Archetype* ArchetypeBuilder::createArchetype() {
    return m_EntityManager->createArchetype(std::move(m_Mask), std::move(m_ComponentIds), std::move(m_ComponentSizes), std::move(m_ComponentAligns), std::move(m_SharedComponentIds));
}

template <typename... Types>
EntityFilterBuilder& EntityFilterBuilder::requireComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<DataComponent, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<DataComponent, Types>>..., std::true_type>>);
    std::vector<unsigned int> const& componentIds{m_EntityManager->componentId<Types>()...};
    for (const unsigned int& cmptId : componentIds)
        m_EntityFilter.requiredComponentMask.set(cmptId);
    return *this;
}

template <typename... Types>
EntityFilterBuilder& EntityFilterBuilder::rejectComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<DataComponent, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<DataComponent, Types>>..., std::true_type>>);
    std::vector<unsigned int> const& componentIds{m_EntityManager->componentId<Types>()...};
    for (const unsigned int& cmptId : componentIds)
        m_EntityFilter.rejectedComponentMask.set(cmptId);
    return *this;
}

template <typename... Types>
EntityFilterBuilder& EntityFilterBuilder::requireSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Types>>..., std::true_type>>);
    std::vector<unsigned int> const& sharedComponentIds{m_EntityManager->sharedComponentId<Types>()...};
    for (const unsigned int& sharedCmptId : sharedComponentIds)
        m_EntityFilter.requiredSharedComponentMask.set(sharedCmptId);
    return *this;
}

template <typename... Types>
EntityFilterBuilder& EntityFilterBuilder::rejectSharedComponents() {
    static_assert(std::is_same_v<std::tuple<std::true_type, std::bool_constant<std::is_base_of_v<SharedComponent, Types>>...>, std::tuple<std::bool_constant<std::is_base_of_v<SharedComponent, Types>>..., std::true_type>>);
    std::vector<unsigned int> const& sharedComponentIds{m_EntityManager->sharedComponentId<Types>()...};
    for (const unsigned int& sharedCmptId : sharedComponentIds)
        m_EntityFilter.rejectedSharedComponentMask.set(sharedCmptId);
    return *this;
}

template <typename Type>
EntityFilterBuilder& EntityFilterBuilder::requireSharedComponent(const Type& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    const unsigned int sharedComponentId = m_EntityManager->sharedComponentId<Type>();
    const unsigned int sharedComponentIndex = m_EntityManager->sharedComponentIndex(sharedComponent);
    m_EntityFilter.requiredSharedComponentIdAndIndices.emplace_back(sharedComponentId, sharedComponentIndex);
    return *this;
}

template <typename Type>
EntityFilterBuilder& EntityFilterBuilder::rejectSharedComponent(const Type& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    const unsigned int sharedComponentId = m_EntityManager->sharedComponentId<Type>();
    const unsigned int sharedComponentIndex = m_EntityManager->sharedComponentIndex(sharedComponent);
    m_EntityFilter.rejectedSharedComponentIdAndIndices.emplace_back(sharedComponentId, sharedComponentIndex);
    return *this;
}

inline EntityFilter EntityFilterBuilder::createEntityFilter() {
    std::sort(m_EntityFilter.requiredSharedComponentIdAndIndices.begin(), m_EntityFilter.requiredSharedComponentIdAndIndices.end());
    return std::move(m_EntityFilter);
}

template <typename Type>
void EntityCommandBuffer::addComponent(const Entity& entity, const Type& component) {
    static_assert(std::is_base_of_v<DataComponent, Type>);
    m_Procedures.emplace_back([this, entity, component]() {
        m_EntityManager->addComponentImmediately(entity, component);
    });
}

template <typename Type>
void EntityCommandBuffer::removeComponent(const Entity& entity) {
    static_assert(std::is_base_of_v<DataComponent, Type>);
    m_Procedures.emplace_back([this, entity]() {
        m_EntityManager->removeComponentImmediately<Type>(entity);
    });
}

template <typename Type>
void EntityCommandBuffer::setComponent(const Entity& entity, const Type& component) {
    static_assert(std::is_base_of_v<DataComponent, Type>);
    m_Procedures.emplace_back([this, entity, component]() {
        m_EntityManager->setComponentImmediately(entity, component);
    });
}

template <typename Type>
void EntityCommandBuffer::addSharedComponent(const Entity& entity, const Type& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    m_Procedures.emplace_back([this, entity, sharedComponent]() {
        m_EntityManager->addSharedComponentImmediately(entity, sharedComponent);
    });
}

template <typename Type>
void EntityCommandBuffer::removeSharedComponent(const Entity& entity) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    m_Procedures.emplace_back([this, entity]() {
        m_EntityManager->removeSharedComponentImmediately<Type>(entity);
    });
}

template <typename Type>
void EntityCommandBuffer::setSharedComponent(const Entity& entity, const Type& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    m_Procedures.emplace_back([this, entity, sharedComponent]() {
        m_EntityManager->setSharedComponentImmediately(entity, sharedComponent);
    });
}

template <typename Type>
void EntityCommandBuffer::addSingletonComponent(const Type& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    m_Procedures.emplace_back([this, singletonComponent]() {
        m_EntityManager->addSingletonComponentImmediately(singletonComponent);
    });
}

template <typename Type>
void EntityCommandBuffer::removeSingletonComponent() {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    m_Procedures.emplace_back([this]() {
        m_EntityManager->removeSingletonComponentImmediately<Type>();
    });
}

template <typename Type>
void EntityCommandBuffer::setSingletonComponent(const Type& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    m_Procedures.emplace_back([this, singletonComponent]() {
        m_EntityManager->setSingletonComponentImmediately(singletonComponent);
    });
}

template <typename Type>
void EntityManager::addComponent(const Entity& entity, const Type& component) {
    static_assert(std::is_base_of_v<DataComponent, Type>);
    m_MainEntityCommandBuffer.addComponent(entity, component);
}

template <typename Type>
void EntityManager::removeComponent(const Entity& entity) {
    static_assert(std::is_base_of_v<DataComponent, Type>);
    m_MainEntityCommandBuffer.removeComponent<Type>(entity);
}

template <typename Type>
void EntityManager::setComponent(const Entity& entity, const Type& component) {
    static_assert(std::is_base_of_v<DataComponent, Type>);
    m_MainEntityCommandBuffer.setComponent(entity, component);
}

template <typename Type>
void EntityManager::addSharedComponent(const Entity& entity, const Type& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    m_MainEntityCommandBuffer.addSharedComponent(entity, sharedComponent);
}

template <typename Type>
void EntityManager::removeSharedComponent(const Entity& entity) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    m_MainEntityCommandBuffer.removeSharedComponent<Type>(entity);
}

template <typename Type>
void EntityManager::setSharedComponent(const Entity& entity, const Type& sharedComponent) {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    m_MainEntityCommandBuffer.setSharedComponent(entity, sharedComponent);
}

template <typename Type>
void EntityManager::addSingletonComponent(const Type& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    m_MainEntityCommandBuffer.addSingletonComponent(singletonComponent);
}

template <typename Type>
void EntityManager::removeSingletonComponent() {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    m_MainEntityCommandBuffer.removeSingletonComponent<Type>();
}

template <typename Type>
void EntityManager::setSingletonComponent(const Type& singletonComponent) {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    m_MainEntityCommandBuffer.setSingletonComponent(singletonComponent);
}

template <typename Type>
unsigned int EntityManager::componentId() {
    static_assert(std::is_base_of_v<DataComponent, Type>);
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
const Type* EntityManager::sharedComponent(const unsigned int& sharedComponentIndex) const {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    return m_SharedComponentStore.object<Type>(sharedComponentIndex);
}

template <typename Type>
unsigned int EntityManager::sharedComponentIndex(const Type& sharedComponent) const {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    const unsigned int sharedComponentId = registerSharedComponent<Type>();
    return m_SharedComponentStore.objectIndex(sharedComponentId, sharedComponent);
}

template <typename Type>
Type* EntityManager::singletonComponent(const unsigned int& singletonComponentId) const {
    static_assert(std::is_base_of_v<SingletonComponent, Type>);
    return m_SingletonComponentStore.object<Type>(singletonComponentId);
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
void EntityManager::addComponentImmediately(const Entity& entity, const Type& component) {
    const Archetype::EntityLocation srcLocation = m_EntityLocations[entity.id];
    Archetype* const srcArchetype = m_Archetypes[srcLocation.archetypeId].get();
    const unsigned int componentId = registerComponent<Type>();
    ArchetypeMask mask = srcArchetype->mask();
    mask.markComponent(componentId, std::is_base_of_v<ManualDataComponent, Type>);

    Archetype* dstArchetype;
    if (m_ArchetypeMap.contains(mask))
        dstArchetype = m_ArchetypeMap[mask];
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
    dstArchetype->moveEntityAddingComponent(srcLocation, srcArchetype, componentId, static_cast<const void*>(&component), dstLocation, srcSwappedEntity);
    m_EntityLocations[entity.id] = dstLocation;
    if (srcSwappedEntity.valid())
        m_EntityLocations[srcSwappedEntity.id] = srcLocation;
}

template <typename Type>
void EntityManager::removeComponentImmediately(const Entity& entity) {
    const Archetype::EntityLocation srcLocation = m_EntityLocations[entity.id];
    Archetype* const srcArchetype = m_Archetypes[srcLocation.archetypeId].get();
    // If the archetype is single and manual, it should be destroyed;
    if (srcArchetype->single() && srcArchetype->fullyManual()) {
        destroyEntityWithoutCheck(entity, srcArchetype, srcLocation);
        return;
    }
    removeComponentWithoutCheck(entity, registerComponent<Type>(), std::is_base_of_v<ManualDataComponent, Type>);
}

template <typename Type>
void EntityManager::setComponentImmediately(const Entity& entity, const Type& component) {
    const Archetype::EntityLocation location = m_EntityLocations[entity.id];
    Archetype* const archetype = m_Archetypes[location.archetypeId].get();
    const unsigned int componentId = m_ComponentIdMap.at(typeid(Type));
    archetype->setComponent(location, componentId, static_cast<const void*>(&component));
}

template <typename Type>
void EntityManager::addSharedComponentImmediately(const Entity& entity, const Type& sharedComponent) {
    const Archetype::EntityLocation srcLocation = m_EntityLocations[entity.id];
    Archetype* const srcArchetype = m_Archetypes[srcLocation.archetypeId].get();
    const unsigned int sharedComponentId = registerSharedComponent<Type>();
    ArchetypeMask mask = srcArchetype->mask();
    mask.markSharedComponent(sharedComponentId, std::is_base_of_v<ManualSharedComponent, Type>);

    Archetype* dstArchetype;
    if (m_ArchetypeMap.contains(mask))
        dstArchetype = m_ArchetypeMap[mask];
    else {
        std::vector<unsigned int> componentIds = srcArchetype->componentIds();
        std::vector<std::size_t> componentSizes = srcArchetype->componentSizes();
        std::vector<std::size_t> componentAligns = srcArchetype->componentAligns();
        std::vector<unsigned int> sharedComponentIds = srcArchetype->sharedComponentIds();
        sharedComponentIds.push_back(sharedComponentId);
        dstArchetype = createArchetype(std::move(mask), std::move(componentIds), std::move(componentSizes), std::move(componentAligns), std::move(sharedComponentIds));
    }

    unsigned int sharedComponentIndex = m_SharedComponentStore.push(sharedComponentId, sharedComponent);

    Entity srcSwappedEntity;
    Archetype::EntityLocation dstLocation;
    dstArchetype->moveEntityAddingSharedComponent(srcLocation, srcArchetype, sharedComponentId, sharedComponentIndex, dstLocation, srcSwappedEntity);
    m_EntityLocations[entity.id] = dstLocation;
    if (srcSwappedEntity.valid())
        m_EntityLocations[srcSwappedEntity.id] = srcLocation;
}

template <typename Type>
void EntityManager::removeSharedComponentImmediately(const Entity& entity) {
    const Archetype::EntityLocation srcLocation = m_EntityLocations[entity.id];
    Archetype* const srcArchetype = m_Archetypes[srcLocation.archetypeId].get();
    // If the archetype is single and manual, it should be destroyed;
    if (srcArchetype->single() && srcArchetype->fullyManual()) {
        destroyEntityWithoutCheck(entity, srcArchetype, srcLocation);
        return;
    }
    removeSharedComponentWithoutCheck(entity, registerSharedComponent<Type>(), std::is_base_of_v<ManualSharedComponent, Type>);
}

template <typename Type>
void EntityManager::setSharedComponentImmediately(const Entity& entity, const Type& sharedComponent) {
    const Archetype::EntityLocation location = m_EntityLocations[entity.id];
    Archetype* const archetype = m_Archetypes[location.archetypeId].get();
    const unsigned int sharedComponentId = registerSharedComponent<Type>();

    unsigned int sharedComponentIndex = m_SharedComponentStore.push(sharedComponentId, sharedComponent);

    unsigned int originalSharedComponentIndex;
    Archetype::EntityLocation dstLocation;
    Entity swappedEntity;
    archetype->setSharedComponent(location, sharedComponentId, sharedComponentIndex, originalSharedComponentIndex, dstLocation, swappedEntity);
    m_EntityLocations[entity.id] = dstLocation;
    if (swappedEntity.valid())
        m_EntityLocations[swappedEntity.id] = location;

    m_SharedComponentStore.pop(sharedComponentId, originalSharedComponentIndex);
}

template <typename Type>
void EntityManager::addSingletonComponentImmediately(const Type& singletonComponent) {
    m_SingletonComponentStore.push(registerSingletonComponent<Type>(), singletonComponent);
}

template <typename Type>
void EntityManager::removeSingletonComponentImmediately() {
    m_SingletonComponentStore.pop(registerSingletonComponent<Type>());
}

template <typename Type>
void EntityManager::setSingletonComponentImmediately(const Type& singletonComponent) {
    *m_SingletonComponentStore.object(registerSingletonComponent<Type>()) = singletonComponent;
}

}  // namespace Melon

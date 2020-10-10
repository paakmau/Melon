#pragma once

#include <MelonCore/Archetype.h>
#include <MelonCore/Combination.h>
#include <MelonCore/Entity.h>
#include <MelonCore/EntityFilter.h>
#include <MelonCore/ObjectPool.h>

#include <array>
#include <bitset>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <queue>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace Melon {

class EntityManager;

class EntityCommandBuffer {
   public:
    EntityCommandBuffer(EntityManager* entityManager);

    template <typename... T>
    const Entity createEntity();
    const Entity createEntity(const Archetype& archetype);
    void destroyEntity(const Entity& entity);
    template <typename T>
    void addComponent(const Entity& entity, const T& component);
    template <typename T>
    void removeComponent(const Entity& entity);
    template <typename T>
    void setComponent(const Entity& entity, const T& component);

   private:
    void execute();

    EntityManager* _entityManager;
    std::vector<std::function<void()>> _procedures;  // TODO: Improve performance

    friend class EntityManager;
};

class EntityManager {
   public:
    EntityManager();

    template <typename... T>
    const Archetype createArchetype();
    template <typename... T>
    const Entity createEntity();
    const Entity createEntity(const Archetype& archetype);
    void destroyEntity(const Entity& entity);
    template <typename T>
    void addComponent(const Entity& entity, const T& component);
    template <typename T>
    void removeComponent(const Entity& entity);
    template <typename T>
    void setComponent(const Entity& entity, const T& component);

    template <typename T>
    unsigned int componentId();
    template <typename... T>
    const EntityFilter createEntityFilter();
    std::vector<ChunkAccessor> filterEntities(const EntityFilter& entityFilter);

   private:
    template <typename T>
    unsigned int registerComponent();
    unsigned int registerComponent(const std::type_index& typeIndex);
    const Archetype createArchetype(std::vector<unsigned int>&& componentIds, std::bitset<1024>&& componentMask, std::vector<size_t>&& componentSizes, std::vector<size_t>&& componentAligns);
    const Entity assignEntity();
    template <typename... T>
    void createEntityImmediately(const Entity& entity);
    void createEntityImmediately(const Entity& entity, const Archetype& archetype);
    void destroyEntityImmediately(const Entity& entityId);
    template <typename T>
    void addComponentImmediately(const Entity& entity, const T& component);
    template <typename T>
    void removeComponentImmediately(const Entity& entity);
    template <typename T>
    void setComponentImmediately(const Entity& entity, const T& component);

    // template <typename... T>
    // void forEach(std::function<void(T...)> procedure);
    // template <typename... T>
    // void forEach(std::function<void(const Entity&, T...)> procedure);
    // template <typename... T>
    // void forEach(std::function<void(EntityCommandBuffer& buffer, const Entity&, T...)> procedure);
    // template <typename... T>
    // void forEach(std::function<void(EntityCommandBuffer& buffer, T...)> procedure);

    void executeEntityCommandBuffers();

    std::unordered_map<std::type_index, unsigned int> _componentIdMap;

    unsigned int _archetypeIdCounter{};
    std::unordered_map<std::bitset<1024>, Archetype> _archetypeMap;
    std::vector<std::vector<unsigned int>> _archetypeComponentIdArrays;
    std::vector<std::bitset<1024>> _archetypeComponentMasks;
    std::vector<std::vector<size_t>> _archetypeComponentSizeArrays;
    std::vector<std::vector<size_t>> _archetypeComponentAlignArrays;

    ObjectPool<Combination::Chunk> _chunkPool;
    std::vector<std::unique_ptr<Combination>> _archetypeCombinations;

    // TODO: To avoid contention, a few reserved Entity id could be passed to EntityCommandBuffer in main thread.
    std::mutex _entityIdMutex;
    unsigned int _entityIdCounter{};
    std::queue<unsigned int> _availableEntityIds;

    std::vector<unsigned int> _entityIndicesInCombination;
    std::vector<unsigned int> _entityArchetypeIds;

    EntityCommandBuffer _mainEntityCommandBuffer;
    std::vector<EntityCommandBuffer> _taskEntityCommandBuffers;

    friend class EntityCommandBuffer;
    friend class World;
    friend class SystemBase;
};

template <typename... T>
const Entity EntityCommandBuffer::createEntity() {
    const Entity entity = _entityManager->assignEntity();
    _procedures.emplace_back([this, entity]() {
        _entityManager->createEntityImmediately<T...>(entity);
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

template <typename... T>
const Archetype EntityManager::createArchetype() {
    std::array<std::type_index, sizeof...(T)> componentTypeIndices = {typeid(T)...};
    std::vector<unsigned int> componentIds(sizeof...(T));
    for (unsigned int i = 0; i < componentTypeIndices.size(); i++)
        componentIds[i] = registerComponent(componentTypeIndices[i]);
    std::bitset<1024> componentMask;
    for (const unsigned int& cmptId : componentIds)
        componentMask.set(cmptId);
    if (_archetypeMap.contains(componentMask))
        return _archetypeMap.at(componentMask);
    return createArchetype(std::move(componentIds), std::move(componentMask), {sizeof(T)...}, {alignof(T)...});
}

template <typename... T>
const Entity EntityManager::createEntity() {
    const Archetype archetype = createArchetype<T...>();
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
unsigned int EntityManager::componentId() {
    return registerComponent<T>();
}

template <typename... T>
const EntityFilter EntityManager::createEntityFilter() {
    std::array<std::type_index, sizeof...(T)> componentTypeIndices = {typeid(T)...};
    std::vector<unsigned int> componentIds(sizeof...(T));
    for (unsigned int i = 0; i < componentTypeIndices.size(); i++) {
        componentIds[i] = registerComponent(componentTypeIndices[i]);
    }
    std::sort(componentIds.begin(), componentIds.end());
    std::bitset<1024> componentMask;
    for (const unsigned int& cmptId : componentIds)
        componentMask.set(cmptId);
    return EntityFilter{std::move(componentMask)};
}

template <typename T>
unsigned int EntityManager::registerComponent() {
    return registerComponent(typeid(T));
}

template <typename... T>
void EntityManager::createEntityImmediately(const Entity& entity) {
    const Archetype archetype = createArchetype<T...>();
    _entityArchetypeIds[entity.id] = archetype.id;
    createEntityImmediately(entity, archetype);
}

template <typename T>
void EntityManager::addComponentImmediately(const Entity& entity, const T& component) {
    const unsigned int srcArchetypeId = _entityArchetypeIds[entity.id];
    const unsigned int componentId = registerComponent<T>();
    std::bitset<1024> componentMask = _archetypeComponentMasks[srcArchetypeId];
    componentMask.set(componentId);
    unsigned int dstArchetypeId;
    if (_archetypeMap.contains(componentMask))
        dstArchetypeId = _archetypeMap.at(componentMask).id;
    else {
        std::vector<unsigned int> componentIds = _archetypeComponentIdArrays[srcArchetypeId];
        std::vector<size_t> componentSizes = _archetypeComponentSizeArrays[srcArchetypeId];
        std::vector<size_t> componentAligns = _archetypeComponentAlignArrays[srcArchetypeId];
        componentIds.push_back(componentId);
        componentSizes.push_back(sizeof(T));
        componentAligns.push_back(alignof(T));
        dstArchetypeId = createArchetype(std::move(componentIds), std::move(componentMask), std::move(componentSizes), std::move(componentAligns)).id;
    }
    Combination* const srcCombination = _archetypeCombinations[srcArchetypeId].get();
    Combination* const dstCombination = _archetypeCombinations[dstArchetypeId].get();
    const unsigned int entityIndexInSrcCombination = _entityIndicesInCombination[entity.id];

    unsigned int entityIndexInDstCombination = dstCombination->copyEntity(entityIndexInSrcCombination, srcCombination);
    dstCombination->setComponent(_entityIndicesInCombination[entity.id], componentId, &component);
    _entityArchetypeIds[entity.id] = dstArchetypeId;
    _entityIndicesInCombination[entity.id] = entityIndexInDstCombination;

    Entity movedEntity = srcCombination->removeEntity(entityIndexInSrcCombination);
    _entityIndicesInCombination[movedEntity.id] = entityIndexInSrcCombination;
}

template <typename T>
void EntityManager::removeComponentImmediately(const Entity& entity) {
    const unsigned int srcArchetypeId = _entityArchetypeIds[entity.id];
    const unsigned int componentId = registerComponent<T>();
    std::bitset<1024> componentMask = _archetypeComponentMasks[srcArchetypeId];
    componentMask.set(componentId, false);
    unsigned int dstArchetypeId;
    if (_archetypeMap.contains(componentMask))
        dstArchetypeId = _archetypeMap.at(componentMask).id;
    else {
        std::vector<unsigned int> componentIds = _archetypeComponentIdArrays[srcArchetypeId];
        std::vector<size_t> componentSizes = _archetypeComponentSizeArrays[srcArchetypeId];
        std::vector<size_t> componentAligns = _archetypeComponentAlignArrays[srcArchetypeId];
        for (unsigned int i = 0; i < componentIds.size(); i++) {
            if (componentIds[i] == componentId) {
                componentIds.erase(componentIds.begin() + i);
                componentSizes.erase(componentSizes.begin() + i);
                componentAligns.erase(componentAligns.begin() + 1);
                break;
            }
        }
        dstArchetypeId = createArchetype(std::move(componentIds), std::move(componentMask), std::move(componentSizes), std::move(componentAligns)).id;
    }
    Combination* const srcCombination = _archetypeCombinations[srcArchetypeId].get();
    Combination* const dstCombination = _archetypeCombinations[dstArchetypeId].get();
    const unsigned int entityIndexInSrcCombination = _entityIndicesInCombination[entity.id];

    unsigned int entityIndexInDstCombination = dstCombination->copyEntity(entityIndexInSrcCombination, srcCombination);
    _entityArchetypeIds[entity.id] = dstArchetypeId;
    _entityIndicesInCombination[entity.id] = entityIndexInDstCombination;

    Entity movedEntity = dstCombination->removeEntity(entityIndexInSrcCombination);
    _entityIndicesInCombination[movedEntity.id] = entityIndexInSrcCombination;
}

template <typename T>
void EntityManager::setComponentImmediately(const Entity& entity, const T& component) {
    const unsigned int archetypeId = _entityArchetypeIds[entity.id];
    const unsigned int componentId = _componentIdMap.at(typeid(T));
    _archetypeCombinations[archetypeId]->setComponent(_entityIndicesInCombination[entity.id], componentId, static_cast<const void*>(&component));
}

}  // namespace Melon

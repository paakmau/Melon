#include <MelonCore/Entity.h>
#include <MelonCore/EntityManager.h>

#include <typeindex>

namespace Melon {

EntityCommandBuffer::EntityCommandBuffer(EntityManager* entityManager) : _entityManager(entityManager) {}

const Entity EntityCommandBuffer::createEntity(const Archetype& archetype) {
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

const Entity EntityManager::createEntity(const Archetype& archetype) {
    return _mainEntityCommandBuffer.createEntity(archetype);
}

void EntityManager::destroyEntity(const Entity& entity) {
    _mainEntityCommandBuffer.destroyEntity(entity);
}

std::vector<ChunkAccessor> EntityManager::filterEntities(const EntityFilter& entityFilter) {
    std::vector<ChunkAccessor> accessors;
    for (const std::pair<std::bitset<1024>, Archetype>& entry : _archetypeMap) {
        if ((entityFilter.componentMask | entry.first) == entry.first) {
            const std::vector<ChunkAccessor> accessorsInCombination = _archetypeCombinations[entry.second.id]->chunkAccessors();
            for (const ChunkAccessor& accessor : accessorsInCombination)
                accessors.push_back(accessor);
        }
    }
    return accessors;
}

unsigned int EntityManager::registerComponent(const std::type_index& typeIndex) {
    if (_componentIdMap.contains(typeIndex)) return _componentIdMap.at(typeIndex);
    return _componentIdMap.emplace(typeIndex, _componentIdMap.size()).first->second;
}

const Archetype EntityManager::createArchetype(std::vector<unsigned int>&& componentIds, std::bitset<1024>&& componentMask, std::vector<size_t>&& componentSizes, std::vector<size_t>&& componentAligns) {
    const Archetype archetype = Archetype{_archetypeIdCounter++};
    _archetypeMap.insert({componentMask, archetype});
    _archetypeCombinations.emplace_back(std::make_unique<Combination>(
        componentIds, componentSizes, componentAligns, &_chunkPool));
    _archetypeComponentIdArrays.emplace_back(std::move(componentIds));
    _archetypeComponentMasks.emplace_back(std::move(componentMask));
    _archetypeComponentSizeArrays.emplace_back(std::move(componentSizes));
    _archetypeComponentAlignArrays.emplace_back(std::move(componentAligns));
    return archetype;
}

const Entity EntityManager::assignEntity() {
    std::lock_guard lock(_entityIdMutex);
    if (!_availableEntityIds.empty()) {
        unsigned int entityId = _availableEntityIds.back();
        _availableEntityIds.pop();
        return Entity{entityId};
    }
    _entityArchetypeIds.push_back(0);
    _entityIndicesInCombination.push_back(0);
    return Entity{_entityIdCounter++};
}

void EntityManager::createEntityImmediately(const Entity& entity, const Archetype& archetype) {
    const unsigned int entityIndexInCombination = _archetypeCombinations.at(archetype.id)->addEntity(entity);
    _entityIndicesInCombination[entity.id] = entityIndexInCombination;
}

void EntityManager::destroyEntityImmediately(const Entity& entity) {
    const unsigned int archetypeId = _entityArchetypeIds[entity.id];
    const unsigned int entityIndex = _entityIndicesInCombination[entity.id];
    const Entity movedEntity = _archetypeCombinations[archetypeId]->removeEntity(entityIndex);
    _entityIndicesInCombination[movedEntity.id] = entityIndex;
}

void EntityManager::executeEntityCommandBuffers() {
    _mainEntityCommandBuffer.execute();
    for (EntityCommandBuffer& buffer : _taskEntityCommandBuffers)
        buffer.execute();
}

}  // namespace Melon

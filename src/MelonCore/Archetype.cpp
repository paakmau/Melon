#include <MelonCore/Archetype.h>

#include <algorithm>

namespace MelonCore {

Archetype::Archetype(
    const unsigned int& id,
    const ArchetypeMask& mask,
    const std::vector<unsigned int>& componentIds,
    const std::vector<std::size_t>& componentSizes,
    const std::vector<std::size_t>& componentAligns,
    const std::vector<unsigned int>& sharedComponentIds,
    ObjectPool<Chunk>* chunkPool)
    : _id(id), _mask(mask), _componentIds(componentIds), _componentSizes(componentSizes), _componentAligns(componentAligns), _sharedComponentIds(sharedComponentIds), _chunkPool(chunkPool) {
    _chunkLayout.componentSizes = componentSizes;
    std::size_t totalSize = sizeof(Entity);
    for (const std::size_t& size : _chunkLayout.componentSizes)
        totalSize += size;
    _chunkLayout.capacity = sizeof(Chunk) / totalSize;

    std::vector<std::pair<std::size_t, unsigned int>> alignAndIndices(componentIds.size() + 1);
    for (unsigned int i = 0; i < componentAligns.size(); i++)
        alignAndIndices[i] = {componentAligns[i], i};
    alignAndIndices.back() = {alignof(Entity), -1};
    std::sort(alignAndIndices.begin(), alignAndIndices.end(), std::greater<std::pair<unsigned int, unsigned int>>());

    _chunkLayout.componentIndexMap.reserve(componentIds.size());
    _chunkLayout.componentOffsets.resize(componentIds.size());
    std::size_t offset{};
    for (const std::pair<unsigned int, unsigned int>& alignAndIndex : alignAndIndices) {
        if (alignAndIndex.second == -1) {
            _chunkLayout.entityOffset = offset;
            offset += sizeof(Entity) * _chunkLayout.capacity;
        } else {
            _chunkLayout.componentIndexMap.emplace(componentIds[alignAndIndex.second], alignAndIndex.second);
            _chunkLayout.componentOffsets[alignAndIndex.second] = offset;
            offset += _chunkLayout.componentSizes[alignAndIndex.second] * _chunkLayout.capacity;
        }
    }

    std::sort(_sharedComponentIds.begin(), _sharedComponentIds.end());
}

void Archetype::addEntity(const Entity& entity, EntityLocation& location) {
    const unsigned int combinationIndex = createCombination();
    unsigned int entityIndexInCombination;
    bool chunkCountAdded;
    _combinations[combinationIndex]->addEntity(entity, entityIndexInCombination, chunkCountAdded);
    _entityCount++;
    if (chunkCountAdded) _chunkCount++;
    location = EntityLocation{
        _id,
        combinationIndex,
        entityIndexInCombination};
}

void Archetype::moveEntityAddingComponent(const EntityLocation& srcEntityLocation, Archetype* srcArchetype, const unsigned int& componentId, const void* component, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* srcCombination = srcArchetype->_combinations[srcEntityLocation.combinationIndex].get();
    const std::vector<unsigned int>& sharedComponentIndices = srcCombination->sharedComponentIndices();
    const unsigned int dstCombinationIndex = createCombination(sharedComponentIndices);
    Combination* dstCombination = _combinations[dstCombinationIndex].get();

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityAddingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, componentId, component, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        _chunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->_chunkCount--;
    _entityCount++;
    srcArchetype->_entityCount--;
    dstLocation = EntityLocation{_id, dstCombinationIndex, entityIndexInDstCombination};
}

void Archetype::moveEntityRemovingComponent(const EntityLocation& srcEntityLocation, Archetype* srcArchetype, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* srcCombination = srcArchetype->_combinations[srcEntityLocation.combinationIndex].get();
    const std::vector<unsigned int>& sharedComponentIndices = srcCombination->sharedComponentIndices();
    const unsigned int dstCombinationIndex = createCombination(sharedComponentIndices);
    Combination* dstCombination = _combinations[dstCombinationIndex].get();

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        _chunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->_chunkCount--;
    _entityCount++;
    srcArchetype->_entityCount--;
    dstLocation = EntityLocation{_id, dstCombinationIndex, entityIndexInDstCombination};
}

void Archetype::moveEntityAddingSharedComponent(const EntityLocation& srcEntityLocation, Archetype* srcArchetype, const unsigned int& sharedComponentId, const unsigned int& sharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* srcCombination = srcArchetype->_combinations[srcEntityLocation.combinationIndex].get();
    const std::vector<unsigned int>& srcSharedComponentIds = srcArchetype->_sharedComponentIds;
    const std::vector<unsigned int>& srcSharedComponentIndices = srcCombination->sharedComponentIndices();

    const std::vector<unsigned int>& dstSharedComponentIds = _sharedComponentIds;
    std::vector<unsigned int> dstSharedComponentIndices(_sharedComponentIds.size());
    // Assert SharedComponent ids are in ascending order
    for (unsigned int i = 0, j = 0; i < srcSharedComponentIndices.size(); i++, j++)
        if (srcSharedComponentIds[i] == dstSharedComponentIds[j])
            dstSharedComponentIndices[j] = srcSharedComponentIndices[i];
        else
            dstSharedComponentIndices[j] = sharedComponentId, i--;

    const unsigned int dstCombinationIndex = createCombination(dstSharedComponentIndices);
    Combination* dstCombination = _combinations[dstCombinationIndex].get();

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        _chunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->_chunkCount--;
    _entityCount++;
    srcArchetype->_entityCount--;
    dstLocation = EntityLocation{_id, dstCombinationIndex, entityIndexInDstCombination};
}

void Archetype::moveEntityRemovingSharedComponent(const EntityLocation& srcEntityLocation, Archetype* srcArchetype, unsigned int& originalSharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* srcCombination = srcArchetype->_combinations[srcEntityLocation.combinationIndex].get();
    const std::vector<unsigned int>& srcSharedComponentIds = srcArchetype->_sharedComponentIds;
    const std::vector<unsigned int>& srcSharedComponentIndices = srcCombination->sharedComponentIndices();

    const std::vector<unsigned int>& dstSharedComponentIds = _sharedComponentIds;
    std::vector<unsigned int> dstSharedComponentIndices(_sharedComponentIds.size());
    // Assert SharedComponent ids are in ascending order
    for (unsigned int i = 0, j = 0; i < srcSharedComponentIndices.size(); i++, j++)
        if (srcSharedComponentIds[i] == dstSharedComponentIds[j])
            dstSharedComponentIndices[j] = srcSharedComponentIndices[i];
        else {
            originalSharedComponentIndex = dstSharedComponentIndices[i];
            j--;
        }

    const unsigned int dstCombinationIndex = createCombination(dstSharedComponentIndices);
    Combination* dstCombination = _combinations[dstCombinationIndex].get();

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        _chunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->_chunkCount--;
    _entityCount++;
    srcArchetype->_entityCount--;
    dstLocation = EntityLocation{_id, dstCombinationIndex, entityIndexInDstCombination};
}

void Archetype::removeEntity(const EntityLocation& location, std::vector<unsigned int>& sharedComponentIndices, Entity& swappedEntity) {
    // Need to copy SharedComponent indices because Combination may be destroyed
    sharedComponentIndices = _combinations[location.entityIndexInCombination]->sharedComponentIndices();
    bool chunkCountMinused;
    _combinations[location.entityIndexInCombination]->removeEntity(location.entityIndexInCombination, swappedEntity, chunkCountMinused);
    if (chunkCountMinused) _chunkCount--;
}

void Archetype::setComponent(const EntityLocation& location, const unsigned int& componentId, const void* component) {
    _combinations[location.combinationIndex]->setComponent(location.entityIndexInCombination, componentId, component);
}

void Archetype::setSharedComponent(const EntityLocation& location, const unsigned int& sharedComponentId, const unsigned int& sharedComponentIndex, unsigned int& originalSharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* srcCombination = _combinations[location.combinationIndex].get();
    std::vector<unsigned int> dstSharedComponentIndices = srcCombination->sharedComponentIndices();
    for (unsigned int i = 0; i < _sharedComponentIds.size(); i++)
        if (_sharedComponentIds[i] == sharedComponentId) {
            originalSharedComponentIndex = dstSharedComponentIndices[i];
            dstSharedComponentIndices[i] = sharedComponentIndex;
            break;
        }

    const unsigned int dstCombinationIndex = createCombination(dstSharedComponentIndices);
    Combination* dstCombination = _combinations[dstCombinationIndex].get();

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(location.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        _chunkCount++;
    if (srcChunkCountMinused)
        _chunkCount--;
    dstLocation = EntityLocation{_id, dstCombinationIndex, entityIndexInDstCombination};
}

void Archetype::filterEntities(const EntityFilter& entityFilter, const ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount>& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const {
    for (const std::unique_ptr<Combination>& combination : _combinations)
        if (combination != nullptr)
            combination->filterEntities(sharedComponentStore, chunkAccessors);
}

}  // namespace MelonCore

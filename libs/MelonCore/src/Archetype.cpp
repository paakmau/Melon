#include <MelonCore/Archetype.h>

#include <algorithm>

namespace MelonCore {

Archetype::Archetype(
    unsigned int const& id,
    ArchetypeMask const& mask,
    std::vector<unsigned int> const& componentIds,
    std::vector<std::size_t> const& componentSizes,
    std::vector<std::size_t> const& componentAligns,
    std::vector<unsigned int> const& sharedComponentIds,
    ObjectPool<Chunk>* chunkPool)
    : _id(id), _mask(mask), _componentIds(componentIds), _componentSizes(componentSizes), _componentAligns(componentAligns), _sharedComponentIds(sharedComponentIds), _chunkPool(chunkPool) {
    _chunkLayout.componentSizes = componentSizes;
    std::size_t totalSize = sizeof(Entity);
    for (std::size_t const& size : _chunkLayout.componentSizes)
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
    for (std::pair<unsigned int, unsigned int> const& alignAndIndex : alignAndIndices) {
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

void Archetype::addEntity(Entity const& entity, EntityLocation& location) {
    Combination* const combination = createCombination();
    unsigned int entityIndexInCombination;
    bool chunkCountAdded;
    combination->addEntity(entity, entityIndexInCombination, chunkCountAdded);
    if (chunkCountAdded) _chunkCount++;
    _entityCount++;
    location = EntityLocation{
        _id,
        combination->index(),
        entityIndexInCombination};
}

void Archetype::moveEntityAddingComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, unsigned int const& componentId, void const* component, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* const srcCombination = srcArchetype->_combinations[srcEntityLocation.combinationIndex].get();
    std::vector<unsigned int> const& sharedComponentIndices = srcCombination->sharedComponentIndices();
    Combination* const dstCombination = createCombination(sharedComponentIndices);

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityAddingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, componentId, component, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        _chunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->_chunkCount--;
    _entityCount++;
    srcArchetype->_entityCount--;
    dstLocation = EntityLocation{_id, dstCombination->index(), entityIndexInDstCombination};
}

void Archetype::moveEntityRemovingComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* const srcCombination = srcArchetype->_combinations[srcEntityLocation.combinationIndex].get();
    std::vector<unsigned int> const& sharedComponentIndices = srcCombination->sharedComponentIndices();
    Combination* dstCombination = createCombination(sharedComponentIndices);

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        _chunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->_chunkCount--;
    _entityCount++;
    srcArchetype->_entityCount--;
    dstLocation = EntityLocation{_id, dstCombination->index(), entityIndexInDstCombination};
}

void Archetype::moveEntityAddingSharedComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, unsigned int const& sharedComponentId, unsigned int const& sharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* const srcCombination = srcArchetype->_combinations[srcEntityLocation.combinationIndex].get();
    std::vector<unsigned int> const& srcSharedComponentIds = srcArchetype->_sharedComponentIds;
    std::vector<unsigned int> const& srcSharedComponentIndices = srcCombination->sharedComponentIndices();

    std::vector<unsigned int> const& dstSharedComponentIds = _sharedComponentIds;
    std::vector<unsigned int> dstSharedComponentIndices(_sharedComponentIds.size());
    // Assert SharedComponent ids are in ascending order
    for (unsigned int i = 0, j = 0; i < dstSharedComponentIndices.size(); i++, j++)
        if (dstSharedComponentIds[i] == srcSharedComponentIds[j])
            dstSharedComponentIndices[i] = srcSharedComponentIndices[j];
        else
            dstSharedComponentIndices[i] = sharedComponentIndex, j--;

    Combination* const dstCombination = createCombination(dstSharedComponentIndices);

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        _chunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->_chunkCount--;
    _entityCount++;
    srcArchetype->_entityCount--;
    dstLocation = EntityLocation{_id, dstCombination->index(), entityIndexInDstCombination};
}

void Archetype::moveEntityRemovingSharedComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, unsigned int& originalSharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* const srcCombination = srcArchetype->_combinations[srcEntityLocation.combinationIndex].get();
    std::vector<unsigned int> const& srcSharedComponentIds = srcArchetype->_sharedComponentIds;
    std::vector<unsigned int> const& srcSharedComponentIndices = srcCombination->sharedComponentIndices();

    std::vector<unsigned int> const& dstSharedComponentIds = _sharedComponentIds;
    std::vector<unsigned int> dstSharedComponentIndices(_sharedComponentIds.size());
    // Assert SharedComponent ids are in ascending order
    for (unsigned int i = 0, j = 0; i < srcSharedComponentIndices.size(); i++, j++)
        if (srcSharedComponentIds[i] == dstSharedComponentIds[j])
            dstSharedComponentIndices[j] = srcSharedComponentIndices[i];
        else {
            originalSharedComponentIndex = dstSharedComponentIndices[i];
            j--;
        }

    Combination* const dstCombination = createCombination(dstSharedComponentIndices);

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        _chunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->_chunkCount--;
    _entityCount++;
    srcArchetype->_entityCount--;
    dstLocation = EntityLocation{_id, dstCombination->index(), entityIndexInDstCombination};
}

void Archetype::removeEntity(EntityLocation const& location, std::vector<unsigned int>& sharedComponentIndices, Entity& swappedEntity) {
    // Need to copy SharedComponent indices because Combination may be destroyed
    sharedComponentIndices = _combinations[location.combinationIndex]->sharedComponentIndices();
    bool chunkCountMinused;
    _combinations[location.combinationIndex]->removeEntity(location.entityIndexInCombination, swappedEntity, chunkCountMinused);
    if (chunkCountMinused) _chunkCount--;
    _entityCount--;
}

void Archetype::setComponent(EntityLocation const& location, unsigned int const& componentId, void const* component) {
    _combinations[location.combinationIndex]->setComponent(location.entityIndexInCombination, componentId, component);
}

void Archetype::setSharedComponent(EntityLocation const& location, unsigned int const& sharedComponentId, unsigned int const& sharedComponentIndex, unsigned int& originalSharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* const srcCombination = _combinations[location.combinationIndex].get();
    std::vector<unsigned int> dstSharedComponentIndices = srcCombination->sharedComponentIndices();
    for (unsigned int i = 0; i < _sharedComponentIds.size(); i++)
        if (_sharedComponentIds[i] == sharedComponentId) {
            originalSharedComponentIndex = dstSharedComponentIndices[i];
            dstSharedComponentIndices[i] = sharedComponentIndex;
            break;
        }

    Combination* const dstCombination = createCombination(dstSharedComponentIndices);

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(location.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        _chunkCount++;
    if (srcChunkCountMinused)
        _chunkCount--;
    dstLocation = EntityLocation{_id, dstCombination->index(), entityIndexInDstCombination};
}

void Archetype::filterEntities(EntityFilter const& entityFilter, ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount> const& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const {
    for (std::unique_ptr<Combination> const& combination : _combinations)
        if (combination != nullptr)
            combination->filterEntities(sharedComponentStore, chunkAccessors);
}

}  // namespace MelonCore

#include <libMelonCore/Archetype.h>

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
    : m_Id(id), m_Mask(mask), m_ComponentIds(componentIds), m_ComponentSizes(componentSizes), m_ComponentAligns(componentAligns), m_SharedComponentIds(sharedComponentIds), m_ChunkPool(chunkPool) {
    m_ChunkLayout.componentSizes = componentSizes;
    std::size_t totalSize = sizeof(Entity);
    for (std::size_t const& size : m_ChunkLayout.componentSizes)
        totalSize += size;
    m_ChunkLayout.capacity = sizeof(Chunk) / totalSize;

    std::vector<std::pair<std::size_t, unsigned int>> alignAndIndices(componentIds.size() + 1);
    for (unsigned int i = 0; i < componentAligns.size(); i++)
        alignAndIndices[i] = {componentAligns[i], i};
    alignAndIndices.back() = {alignof(Entity), -1};
    std::sort(alignAndIndices.begin(), alignAndIndices.end(), std::greater<std::pair<unsigned int, unsigned int>>());

    m_ChunkLayout.componentIndexMap.reserve(componentIds.size());
    m_ChunkLayout.componentOffsets.resize(componentIds.size());
    std::size_t offset{};
    for (std::pair<unsigned int, unsigned int> const& alignAndIndex : alignAndIndices) {
        if (alignAndIndex.second == -1) {
            m_ChunkLayout.entityOffset = offset;
            offset += sizeof(Entity) * m_ChunkLayout.capacity;
        } else {
            m_ChunkLayout.componentIndexMap.emplace(componentIds[alignAndIndex.second], alignAndIndex.second);
            m_ChunkLayout.componentOffsets[alignAndIndex.second] = offset;
            offset += m_ChunkLayout.componentSizes[alignAndIndex.second] * m_ChunkLayout.capacity;
        }
    }

    std::sort(m_SharedComponentIds.begin(), m_SharedComponentIds.end());
}

void Archetype::addEntity(Entity const& entity, EntityLocation& location) {
    Combination* const combination = createCombination();
    unsigned int entityIndexInCombination;
    bool chunkCountAdded;
    combination->addEntity(entity, entityIndexInCombination, chunkCountAdded);
    if (chunkCountAdded) m_ChunkCount++;
    m_EntityCount++;
    location = EntityLocation{
        m_Id,
        combination->index(),
        entityIndexInCombination};
}

void Archetype::moveEntityAddingComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, unsigned int const& componentId, void const* component, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* const srcCombination = srcArchetype->m_Combinations[srcEntityLocation.combinationIndex].get();
    std::vector<unsigned int> const& sharedComponentIndices = srcCombination->sharedComponentIndices();
    Combination* const dstCombination = createCombination(sharedComponentIndices);

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityAddingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, componentId, component, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        m_ChunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->m_ChunkCount--;
    m_EntityCount++;
    srcArchetype->m_EntityCount--;
    dstLocation = EntityLocation{m_Id, dstCombination->index(), entityIndexInDstCombination};
}

void Archetype::moveEntityRemovingComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* const srcCombination = srcArchetype->m_Combinations[srcEntityLocation.combinationIndex].get();
    std::vector<unsigned int> const& sharedComponentIndices = srcCombination->sharedComponentIndices();
    Combination* dstCombination = createCombination(sharedComponentIndices);

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        m_ChunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->m_ChunkCount--;
    m_EntityCount++;
    srcArchetype->m_EntityCount--;
    dstLocation = EntityLocation{m_Id, dstCombination->index(), entityIndexInDstCombination};
}

void Archetype::moveEntityAddingSharedComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, unsigned int const& sharedComponentId, unsigned int const& sharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* const srcCombination = srcArchetype->m_Combinations[srcEntityLocation.combinationIndex].get();
    std::vector<unsigned int> const& srcSharedComponentIds = srcArchetype->m_SharedComponentIds;
    std::vector<unsigned int> const& srcSharedComponentIndices = srcCombination->sharedComponentIndices();

    std::vector<unsigned int> const& dstSharedComponentIds = m_SharedComponentIds;
    std::vector<unsigned int> dstSharedComponentIndices(m_SharedComponentIds.size());
    // Assert SharedComponent ids are in ascending order
    for (unsigned int i = 0, j = 0; i < dstSharedComponentIndices.size(); i++, j++)
        if (j < srcSharedComponentIds.size() && dstSharedComponentIds[i] == srcSharedComponentIds[j])
            dstSharedComponentIndices[i] = srcSharedComponentIndices[j];
        else
            dstSharedComponentIndices[i] = sharedComponentIndex, j--;

    Combination* const dstCombination = createCombination(dstSharedComponentIndices);

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(srcEntityLocation.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        m_ChunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->m_ChunkCount--;
    m_EntityCount++;
    srcArchetype->m_EntityCount--;
    dstLocation = EntityLocation{m_Id, dstCombination->index(), entityIndexInDstCombination};
}

void Archetype::moveEntityRemovingSharedComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, unsigned int& originalSharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* const srcCombination = srcArchetype->m_Combinations[srcEntityLocation.combinationIndex].get();
    std::vector<unsigned int> const& srcSharedComponentIds = srcArchetype->m_SharedComponentIds;
    std::vector<unsigned int> const& srcSharedComponentIndices = srcCombination->sharedComponentIndices();

    std::vector<unsigned int> const& dstSharedComponentIds = m_SharedComponentIds;
    std::vector<unsigned int> dstSharedComponentIndices(m_SharedComponentIds.size());
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
        m_ChunkCount++;
    if (srcChunkCountMinused)
        srcArchetype->m_ChunkCount--;
    m_EntityCount++;
    srcArchetype->m_EntityCount--;
    dstLocation = EntityLocation{m_Id, dstCombination->index(), entityIndexInDstCombination};
}

void Archetype::removeEntity(EntityLocation const& location, std::vector<unsigned int>& sharedComponentIndices, Entity& swappedEntity) {
    // Need to copy SharedComponent indices because Combination may be destroyed
    sharedComponentIndices = m_Combinations[location.combinationIndex]->sharedComponentIndices();
    bool chunkCountMinused;
    m_Combinations[location.combinationIndex]->removeEntity(location.entityIndexInCombination, swappedEntity, chunkCountMinused);
    if (chunkCountMinused) m_ChunkCount--;
    m_EntityCount--;
}

void Archetype::setComponent(EntityLocation const& location, unsigned int const& componentId, void const* component) {
    m_Combinations[location.combinationIndex]->setComponent(location.entityIndexInCombination, componentId, component);
}

void Archetype::setSharedComponent(EntityLocation const& location, unsigned int const& sharedComponentId, unsigned int const& sharedComponentIndex, unsigned int& originalSharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity) {
    Combination* const srcCombination = m_Combinations[location.combinationIndex].get();
    std::vector<unsigned int> dstSharedComponentIndices = srcCombination->sharedComponentIndices();
    for (unsigned int i = 0; i < m_SharedComponentIds.size(); i++)
        if (m_SharedComponentIds[i] == sharedComponentId) {
            originalSharedComponentIndex = dstSharedComponentIndices[i];
            dstSharedComponentIndices[i] = sharedComponentIndex;
            break;
        }

    Combination* const dstCombination = createCombination(dstSharedComponentIndices);

    unsigned int entityIndexInDstCombination;
    bool dstChunkCountAdded, srcChunkCountMinused;
    dstCombination->moveEntityRemovingComponent(location.entityIndexInCombination, srcCombination, entityIndexInDstCombination, dstChunkCountAdded, srcSwappedEntity, srcChunkCountMinused);
    if (dstChunkCountAdded)
        m_ChunkCount++;
    if (srcChunkCountMinused)
        m_ChunkCount--;
    dstLocation = EntityLocation{m_Id, dstCombination->index(), entityIndexInDstCombination};
}

void Archetype::filterEntities(EntityFilter const& entityFilter, ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const {
    for (std::unique_ptr<Combination> const& combination : m_Combinations)
        if (combination != nullptr)
            combination->filterEntities(sharedComponentStore, chunkAccessors);
}

}  // namespace MelonCore

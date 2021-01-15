#include <libMelonCore/Combination.h>

#include <algorithm>
#include <cstring>
#include <functional>

namespace Melon {

Combination::Combination(
    const unsigned int& index,
    const ChunkLayout& chunkLayout,
    std::vector<unsigned int> const& sharedComponentIds,
    std::vector<unsigned int> const& sharedComponentIndices,
    ObjectPool<Chunk>* chunkPool)
    : m_Index(index),
      m_ChunkLayout(chunkLayout),
      m_SharedComponentIds(sharedComponentIds),
      m_SharedComponentIndices(sharedComponentIndices),
      m_ChunkPool(chunkPool),
      m_EntityCountInCurrentChunk(chunkLayout.capacity) {
}

void Combination::addEntity(const Entity& entity, unsigned int& entityIndexInCombination, bool& chunkCountAdded) {
    chunkCountAdded = m_EntityCountInCurrentChunk == m_ChunkLayout.capacity;
    if (chunkCountAdded)
        requestChunk();
    Chunk* chunk = m_Chunks.back();
    unsigned int entityIndexInChunk = m_EntityCountInCurrentChunk++;
    memcpy(entityAddress(chunk, entityIndexInChunk), &entity, sizeof(Entity));
    entityIndexInCombination = m_EntityCount++;
}

void Combination::moveEntityAddingComponent(const unsigned int& entityIndexInSrcCombination, Combination* srcCombination, const unsigned int& componentId, const void* component, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused) {
    const Entity& srcEntity = *srcCombination->entityAddress(entityIndexInSrcCombination);
    addEntity(srcEntity, entityIndexInDstCombination, dstChunkCountAdded);

    Chunk* dstChunk = m_Chunks.back();
    unsigned int entityIndexInDstChunk = m_EntityCountInCurrentChunk - 1;

    for (const auto& [id, index] : m_ChunkLayout.componentIndexMap) {
        if (id == componentId) continue;
        const std::size_t& size = m_ChunkLayout.componentSizes[index];
        void* dstAddress = componentAddress(dstChunk, index, entityIndexInDstChunk);
        void* srcAddress = srcCombination->componentAddress(id, entityIndexInSrcCombination);
        memcpy(dstAddress, srcAddress, size);
    }

    setComponent(entityIndexInDstCombination, componentId, component);

    srcCombination->removeEntity(entityIndexInSrcCombination, swappedEntity, srcChunkCountMinused);
}

void Combination::moveEntityRemovingComponent(const unsigned int& entityIndexInSrcCombination, Combination* srcCombination, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused) {
    const Entity& srcEntity = *srcCombination->entityAddress(entityIndexInSrcCombination);
    addEntity(srcEntity, entityIndexInDstCombination, dstChunkCountAdded);

    Chunk* dstChunk = m_Chunks.back();
    unsigned int entityIndexInDstChunk = m_EntityCountInCurrentChunk - 1;

    for (const auto& [id, index] : m_ChunkLayout.componentIndexMap) {
        const std::size_t& size = m_ChunkLayout.componentSizes[index];
        void* dstAddress = componentAddress(dstChunk, index, entityIndexInDstChunk);
        void* srcAddress = srcCombination->componentAddress(id, entityIndexInSrcCombination);
        memcpy(dstAddress, srcAddress, size);
    }

    srcCombination->removeEntity(entityIndexInSrcCombination, swappedEntity, srcChunkCountMinused);
}

void Combination::removeEntity(const unsigned int& entityIndexInCombination, Entity& swappedEntity, bool& chunkCountMinused) {
    Chunk* dstChunk = m_Chunks[entityIndexInCombination / m_ChunkLayout.capacity];
    const unsigned int dstEntityIndexInChunk = entityIndexInCombination % m_ChunkLayout.capacity;

    Chunk* srcChunk = m_Chunks.back();
    const unsigned int srcEntityIndexInChunk = m_EntityCountInCurrentChunk - 1;

    for (const auto& [id, index] : m_ChunkLayout.componentIndexMap) {
        const std::size_t& size = m_ChunkLayout.componentSizes[index];
        void* dstAddress = componentAddress(dstChunk, index, dstEntityIndexInChunk);
        void* srcAddress = componentAddress(srcChunk, index, srcEntityIndexInChunk);
        if (dstChunk != srcChunk || dstEntityIndexInChunk != srcEntityIndexInChunk)
            memcpy(dstAddress, srcAddress, size);
        memset(srcAddress, 0, size);
    }

    void* dstEntityAddress = static_cast<void*>(entityAddress(dstChunk, dstEntityIndexInChunk));
    void* srcEntityAddress = static_cast<void*>(entityAddress(srcChunk, srcEntityIndexInChunk));
    if (dstChunk != srcChunk || dstEntityIndexInChunk != srcEntityIndexInChunk)
        memcpy(dstEntityAddress, srcEntityAddress, sizeof(Entity));
    memset(srcEntityAddress, 0, sizeof(Entity));

    m_EntityCountInCurrentChunk--;
    m_EntityCount--;

    chunkCountMinused = m_EntityCountInCurrentChunk == 0;
    if (chunkCountMinused) recycleChunk();

    if (dstChunk != srcChunk || dstEntityIndexInChunk != srcEntityIndexInChunk)
        swappedEntity = *static_cast<Entity*>(dstEntityAddress);
    else
        swappedEntity = Entity::invalidEntity();
}

void Combination::setComponent(const unsigned int& entityIndexInCombination, const unsigned int& componentId, const void* component) {
    Chunk* chunk = m_Chunks[entityIndexInCombination / m_ChunkLayout.capacity];
    const unsigned int entityIndexInChunk = entityIndexInCombination % m_ChunkLayout.capacity;
    const unsigned int componentIndex = m_ChunkLayout.componentIndexMap.at(componentId);
    void* address = componentAddress(chunk, componentIndex, entityIndexInChunk);

    memcpy(address, component, m_ChunkLayout.componentSizes[componentIndex]);
}

void Combination::requestChunk() {
    m_Chunks.emplace_back(m_ChunkPool->request());
    m_EntityCountInCurrentChunk = 0;
}

void Combination::recycleChunk() {
    Chunk* chunk = m_Chunks.back();
    m_Chunks.pop_back();
    m_ChunkPool->recycle(chunk);
    m_EntityCountInCurrentChunk = m_ChunkLayout.capacity;
}

}  // namespace Melon

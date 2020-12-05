#include <MelonCore/Combination.h>

#include <algorithm>
#include <cstring>
#include <functional>

namespace MelonCore {

Combination::Combination(unsigned int const& index, ChunkLayout const& chunkLayout, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectPool<Chunk>* chunkPool) : m_Index(index), m_ChunkLayout(chunkLayout), m_SharedComponentIds(sharedComponentIds), m_SharedComponentIndices(sharedComponentIndices), m_ChunkPool(chunkPool), m_EntityCountInCurrentChunk(0) {
    // TODO: We should not request a Chunk here
    requestChunk();
}

void Combination::addEntity(Entity const& entity, unsigned int& entityIndexInCombination, bool& chunkCountAdded) {
    Chunk* chunk = m_Chunks.back();
    unsigned int entityIndexInChunk = m_EntityCountInCurrentChunk++;
    memcpy(entityAddress(chunk, entityIndexInChunk), &entity, sizeof(Entity));
    chunkCountAdded = m_EntityCountInCurrentChunk == m_ChunkLayout.capacity;
    if (chunkCountAdded)
        requestChunk();
    entityIndexInCombination = m_EntityCount++;
}

void Combination::moveEntityAddingComponent(unsigned int const& entityIndexInSrcCombination, Combination* srcCombination, unsigned int const& componentId, void const* component, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused) {
    Entity const& srcEntity = *srcCombination->entityAddress(entityIndexInSrcCombination);
    addEntity(srcEntity, entityIndexInDstCombination, dstChunkCountAdded);

    Chunk* dstChunk = m_Chunks.back();
    unsigned int entityIndexInDstChunk = m_EntityCountInCurrentChunk - 1;

    for (std::pair<unsigned int, unsigned int> const& idAndIndex : m_ChunkLayout.componentIndexMap) {
        if (idAndIndex.first == componentId) continue;
        std::size_t const& size = m_ChunkLayout.componentSizes[idAndIndex.second];
        void* dstAddress = componentAddress(dstChunk, idAndIndex.second, entityIndexInDstChunk);
        void* srcAddress = srcCombination->componentAddress(idAndIndex.first, entityIndexInSrcCombination);
        memcpy(dstAddress, srcAddress, size);
    }

    setComponent(entityIndexInDstCombination, componentId, component);

    srcCombination->removeEntity(entityIndexInSrcCombination, swappedEntity, srcChunkCountMinused);
}

void Combination::moveEntityRemovingComponent(unsigned int const& entityIndexInSrcCombination, Combination* srcCombination, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused) {
    Entity const& srcEntity = *srcCombination->entityAddress(entityIndexInSrcCombination);
    addEntity(srcEntity, entityIndexInDstCombination, dstChunkCountAdded);

    Chunk* dstChunk = m_Chunks.back();
    unsigned int entityIndexInDstChunk = m_EntityCountInCurrentChunk - 1;

    for (std::pair<unsigned int, unsigned int> const& idAndIndex : m_ChunkLayout.componentIndexMap) {
        std::size_t const& size = m_ChunkLayout.componentSizes[idAndIndex.second];
        void* dstAddress = componentAddress(dstChunk, idAndIndex.second, entityIndexInDstChunk);
        void* srcAddress = srcCombination->componentAddress(idAndIndex.first, entityIndexInSrcCombination);
        memcpy(dstAddress, srcAddress, size);
    }

    srcCombination->removeEntity(entityIndexInSrcCombination, swappedEntity, srcChunkCountMinused);
}

void Combination::removeEntity(unsigned int const& entityIndexInCombination, Entity& swappedEntity, bool& chunkCountMinused) {
    Chunk* dstChunk = m_Chunks[entityIndexInCombination / m_ChunkLayout.capacity];
    unsigned int const dstEntityIndexInChunk = entityIndexInCombination % m_ChunkLayout.capacity;

    chunkCountMinused = m_EntityCountInCurrentChunk == 0;
    if (chunkCountMinused) recycleChunk();

    Chunk* srcChunk = m_Chunks.back();
    unsigned int const srcEntityIndexInChunk = m_EntityCountInCurrentChunk - 1;

    for (std::pair<unsigned int, unsigned int> const& idAndIndex : m_ChunkLayout.componentIndexMap) {
        std::size_t const& size = m_ChunkLayout.componentSizes[idAndIndex.second];
        void* dstAddress = componentAddress(dstChunk, idAndIndex.second, dstEntityIndexInChunk);
        void* srcAddress = componentAddress(srcChunk, idAndIndex.second, srcEntityIndexInChunk);
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

    if (dstChunk != srcChunk || dstEntityIndexInChunk != srcEntityIndexInChunk)
        swappedEntity = *static_cast<Entity*>(dstEntityAddress);
    else
        swappedEntity = Entity::invalidEntity();
}

void Combination::setComponent(unsigned int const& entityIndexInCombination, unsigned int const& componentId, void const* component) {
    Chunk* chunk = m_Chunks[entityIndexInCombination / m_ChunkLayout.capacity];
    unsigned int const entityIndexInChunk = entityIndexInCombination % m_ChunkLayout.capacity;
    unsigned int const componentIndex = m_ChunkLayout.componentIndexMap.at(componentId);
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

}  // namespace MelonCore

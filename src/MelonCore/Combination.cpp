#include <MelonCore/Combination.h>

#include <algorithm>
#include <cstring>
#include <functional>

namespace MelonCore {

Combination::Combination(const unsigned int& index, const ChunkLayout& chunkLayout, const std::vector<unsigned int>& sharedComponentIds, const std::vector<unsigned int>& sharedComponentIndices, ObjectPool<Chunk>* chunkPool) : _index(index), _chunkLayout(chunkLayout), _sharedComponentIds(sharedComponentIds), _sharedComponentIndices(sharedComponentIndices), _chunkPool(chunkPool), _entityCountInCurrentChunk(0) {
    // TODO: We should not request a Chunk here
    requestChunk();
}

void Combination::addEntity(const Entity& entity, unsigned int& entityIndexInCombination, bool& chunkCountAdded) {
    Chunk* chunk = _chunks.back();
    unsigned int entityIndexInChunk = _entityCountInCurrentChunk++;
    memcpy(entityAddress(chunk, entityIndexInChunk), &entity, sizeof(Entity));
    chunkCountAdded = _entityCountInCurrentChunk == _chunkLayout.capacity;
    if (chunkCountAdded)
        requestChunk();
    entityIndexInCombination = _entityCount++;
}

void Combination::moveEntityAddingComponent(const unsigned int& entityIndexInSrcCombination, Combination* srcCombination, const unsigned int& componentId, const void* component, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused) {
    const Entity& srcEntity = *srcCombination->entityAddress(entityIndexInSrcCombination);
    addEntity(srcEntity, entityIndexInDstCombination, dstChunkCountAdded);

    Chunk* dstChunk = _chunks.back();
    unsigned int entityIndexInDstChunk = _entityCountInCurrentChunk;

    for (const std::pair<unsigned int, unsigned int>& idAndIndex : _chunkLayout.componentIndexMap) {
        if (idAndIndex.first == componentId) continue;
        const std::size_t& size = _chunkLayout.componentSizes[idAndIndex.second];
        void* dstAddress = componentAddress(dstChunk, idAndIndex.second, entityIndexInDstChunk);
        void* srcAddress = srcCombination->componentAddress(idAndIndex.first, entityIndexInSrcCombination);
        memcpy(dstAddress, srcAddress, size);
    }

    setComponent(entityIndexInDstCombination, componentId, component);

    srcCombination->removeEntity(entityIndexInSrcCombination, swappedEntity, srcChunkCountMinused);
}

void Combination::moveEntityRemovingComponent(const unsigned int& entityIndexInSrcCombination, Combination* srcCombination, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused) {
    const Entity& srcEntity = *srcCombination->entityAddress(entityIndexInSrcCombination);
    addEntity(srcEntity, entityIndexInDstCombination, dstChunkCountAdded);

    Chunk* dstChunk = _chunks.back();
    unsigned int entityIndexInDstChunk = _entityCountInCurrentChunk;

    for (const std::pair<unsigned int, unsigned int>& idAndIndex : _chunkLayout.componentIndexMap) {
        const std::size_t& size = _chunkLayout.componentSizes[idAndIndex.second];
        void* dstAddress = componentAddress(dstChunk, idAndIndex.second, entityIndexInDstChunk);
        void* srcAddress = srcCombination->componentAddress(idAndIndex.first, entityIndexInSrcCombination);
        memcpy(dstAddress, srcAddress, size);
    }

    srcCombination->removeEntity(entityIndexInSrcCombination, swappedEntity, srcChunkCountMinused);
}

void Combination::removeEntity(const unsigned int& entityIndexInCombination, Entity& swappedEntity, bool& chunkCountMinused) {
    Chunk* dstChunk = _chunks[entityIndexInCombination / _chunkLayout.capacity];
    const unsigned int dstEntityIndexInChunk = entityIndexInCombination % _chunkLayout.capacity;

    chunkCountMinused = _entityCountInCurrentChunk == 0;
    if (chunkCountMinused) recycleChunk();

    Chunk* srcChunk = _chunks.back();
    const unsigned int srcEntityIndexInChunk = _entityCountInCurrentChunk - 1;

    for (const std::pair<unsigned int, unsigned int>& idAndIndex : _chunkLayout.componentIndexMap) {
        const std::size_t& size = _chunkLayout.componentSizes[idAndIndex.second];
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

    _entityCountInCurrentChunk--;
    _entityCount--;

    if (dstChunk != srcChunk || dstEntityIndexInChunk != srcEntityIndexInChunk)
        swappedEntity = *static_cast<Entity*>(dstEntityAddress);
    else
        swappedEntity = Entity::invalidEntity();
}

void Combination::setComponent(const unsigned int& entityIndexInCombination, const unsigned int& componentId, const void* component) {
    Chunk* chunk = _chunks[entityIndexInCombination / _chunkLayout.capacity];
    const unsigned int entityIndexInChunk = entityIndexInCombination % _chunkLayout.capacity;
    const unsigned int componentIndex = _chunkLayout.componentIndexMap.at(componentId);
    void* address = componentAddress(chunk, componentIndex, entityIndexInChunk);

    memcpy(address, component, _chunkLayout.componentSizes[componentIndex]);
}

void Combination::requestChunk() {
    _chunks.emplace_back(_chunkPool->request());
    _entityCountInCurrentChunk = 0;
}

void Combination::recycleChunk() {
    Chunk* chunk = _chunks.back();
    _chunks.pop_back();
    _chunkPool->recycle(chunk);
    _entityCountInCurrentChunk = _chunkLayout.capacity;
}

}  // namespace MelonCore

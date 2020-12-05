#include <MelonCore/Combination.h>

#include <algorithm>
#include <cstring>
#include <functional>

namespace MelonCore {

Combination::Combination(unsigned int const& index, ChunkLayout const& chunkLayout, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectPool<Chunk>* chunkPool) : _index(index), _chunkLayout(chunkLayout), _sharedComponentIds(sharedComponentIds), _sharedComponentIndices(sharedComponentIndices), _chunkPool(chunkPool), _entityCountInCurrentChunk(0) {
    // TODO: We should not request a Chunk here
    requestChunk();
}

void Combination::addEntity(Entity const& entity, unsigned int& entityIndexInCombination, bool& chunkCountAdded) {
    Chunk* chunk = _chunks.back();
    unsigned int entityIndexInChunk = _entityCountInCurrentChunk++;
    memcpy(entityAddress(chunk, entityIndexInChunk), &entity, sizeof(Entity));
    chunkCountAdded = _entityCountInCurrentChunk == _chunkLayout.capacity;
    if (chunkCountAdded)
        requestChunk();
    entityIndexInCombination = _entityCount++;
}

void Combination::moveEntityAddingComponent(unsigned int const& entityIndexInSrcCombination, Combination* srcCombination, unsigned int const& componentId, void const* component, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused) {
    Entity const& srcEntity = *srcCombination->entityAddress(entityIndexInSrcCombination);
    addEntity(srcEntity, entityIndexInDstCombination, dstChunkCountAdded);

    Chunk* dstChunk = _chunks.back();
    unsigned int entityIndexInDstChunk = _entityCountInCurrentChunk - 1;

    for (std::pair<unsigned int, unsigned int> const& idAndIndex : _chunkLayout.componentIndexMap) {
        if (idAndIndex.first == componentId) continue;
        std::size_t const& size = _chunkLayout.componentSizes[idAndIndex.second];
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

    Chunk* dstChunk = _chunks.back();
    unsigned int entityIndexInDstChunk = _entityCountInCurrentChunk - 1;

    for (std::pair<unsigned int, unsigned int> const& idAndIndex : _chunkLayout.componentIndexMap) {
        std::size_t const& size = _chunkLayout.componentSizes[idAndIndex.second];
        void* dstAddress = componentAddress(dstChunk, idAndIndex.second, entityIndexInDstChunk);
        void* srcAddress = srcCombination->componentAddress(idAndIndex.first, entityIndexInSrcCombination);
        memcpy(dstAddress, srcAddress, size);
    }

    srcCombination->removeEntity(entityIndexInSrcCombination, swappedEntity, srcChunkCountMinused);
}

void Combination::removeEntity(unsigned int const& entityIndexInCombination, Entity& swappedEntity, bool& chunkCountMinused) {
    Chunk* dstChunk = _chunks[entityIndexInCombination / _chunkLayout.capacity];
    unsigned int const dstEntityIndexInChunk = entityIndexInCombination % _chunkLayout.capacity;

    chunkCountMinused = _entityCountInCurrentChunk == 0;
    if (chunkCountMinused) recycleChunk();

    Chunk* srcChunk = _chunks.back();
    unsigned int const srcEntityIndexInChunk = _entityCountInCurrentChunk - 1;

    for (std::pair<unsigned int, unsigned int> const& idAndIndex : _chunkLayout.componentIndexMap) {
        std::size_t const& size = _chunkLayout.componentSizes[idAndIndex.second];
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

void Combination::setComponent(unsigned int const& entityIndexInCombination, unsigned int const& componentId, void const* component) {
    Chunk* chunk = _chunks[entityIndexInCombination / _chunkLayout.capacity];
    unsigned int const entityIndexInChunk = entityIndexInCombination % _chunkLayout.capacity;
    unsigned int const componentIndex = _chunkLayout.componentIndexMap.at(componentId);
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

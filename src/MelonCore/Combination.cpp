#include <MelonCore/Combination.h>
#include <MelonCore/ObjectPool.h>

#include <algorithm>
#include <cstring>
#include <functional>

namespace MelonCore {

Combination::Combination(
    const std::vector<unsigned int>& componentIds,
    const std::vector<std::size_t>& componentSizes,
    const std::vector<std::size_t>& componentAligns,
    ObjectPool<Chunk>* chunkPool)
    : _chunkPool(chunkPool),
      _countInCurrentChunk(0) {
    _chunkLayout.componentSizes = componentSizes;
    std::size_t totalSize = sizeof(Entity);
    for (const std::size_t& size : _chunkLayout.componentSizes)
        totalSize += size;
    _chunkCapacity = sizeof(Chunk) / totalSize;

    std::vector<std::pair<size_t, unsigned int>> alignAndIndices(componentIds.size() + 1);
    for (unsigned int i = 0; i < componentAligns.size(); i++)
        alignAndIndices[i] = {componentAligns[i], i};
    alignAndIndices.back() = {alignof(Entity), -1};
    std::sort(alignAndIndices.begin(), alignAndIndices.end(), std::greater<std::pair<unsigned int, unsigned int>>());

    _chunkLayout.componentIndexMap.reserve(componentIds.size());
    _chunkLayout.componentOffsets.resize(componentIds.size());
    size_t offset{};
    for (const std::pair<unsigned int, unsigned int>& alignAndIndex : alignAndIndices) {
        if (alignAndIndex.second == -1) {
            _chunkLayout.entityOffset = offset;
            offset += sizeof(Entity) * _chunkCapacity;
        } else {
            _chunkLayout.componentIndexMap.emplace(componentIds[alignAndIndex.second], alignAndIndex.second);
            _chunkLayout.componentOffsets[alignAndIndex.second] = offset;
            offset += _chunkLayout.componentSizes[alignAndIndex.second] * _chunkCapacity;
        }
    }

    requestChunk();
}

unsigned int Combination::addEntity(const Entity& entity) {
    Chunk* chunk = _chunks.back();
    unsigned int entityIndexInChunk = _countInCurrentChunk++;
    unsigned int entityIndexInCombination = _count++;
    memcpy(entityAddress(chunk, entityIndexInChunk), &entity, sizeof(Entity));
    if (_countInCurrentChunk == _chunkCapacity)
        requestChunk();
    return entityIndexInCombination;
}

unsigned int Combination::copyEntity(const unsigned int& entityIndexInSrcCombination, Combination* const& srcCombination) {
    Chunk* dstChunk = _chunks.back();
    unsigned int entityIndexInDstChunk = _countInCurrentChunk++;
    unsigned int entityIndexInDstCombination = _count++;

    for (const std::pair<unsigned int, unsigned int>& idAndIndex : _chunkLayout.componentIndexMap) {
        if (!srcCombination->hasComponent(idAndIndex.first)) continue;
        const size_t& size = _chunkLayout.componentSizes[idAndIndex.second];
        void* dstAddress = componentAddress(dstChunk, idAndIndex.second, entityIndexInDstChunk);
        void* srcAddress = srcCombination->componentAddress(idAndIndex.first, entityIndexInSrcCombination);
        memcpy(dstAddress, srcAddress, size);
    }

    void* dstEntityAddress = entityAddress(dstChunk, entityIndexInDstChunk);
    void* srcEntityAddress = srcCombination->entityAddress(entityIndexInSrcCombination);
    memcpy(dstEntityAddress, srcEntityAddress, sizeof(Entity));

    return entityIndexInDstCombination;
}

Entity Combination::removeEntity(const unsigned int& entityIndex) {
    Chunk* dstChunk = _chunks[entityIndex / _chunkCapacity];
    const unsigned int dstEntityIndexInChunk = entityIndex % _chunkCapacity;

    if (_countInCurrentChunk == 0) recycleChunk();

    Chunk* srcChunk = _chunks.back();
    const unsigned int srcEntityIndexInChunk = _countInCurrentChunk - 1;

    for (const std::pair<unsigned int, unsigned int>& idAndIndex : _chunkLayout.componentIndexMap) {
        const size_t& size = _chunkLayout.componentSizes[idAndIndex.second];
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

    _countInCurrentChunk--;
    _count--;

    return *static_cast<Entity*>(dstEntityAddress);
}

void Combination::setComponent(const unsigned int& entityIndex, const unsigned int& componentId, const void* component) {
    Chunk* chunk = _chunks[entityIndex / _chunkCapacity];
    const unsigned int entityIndexInChunk = entityIndex % _chunkCapacity;
    const unsigned int componentIndex = _chunkLayout.componentIndexMap.at(componentId);
    void* address = componentAddress(chunk, componentIndex, entityIndexInChunk);

    memcpy(address, component, _chunkLayout.componentSizes[componentIndex]);
}

void Combination::requestChunk() {
    _chunks.emplace_back(_chunkPool->request());
    _countInCurrentChunk = 0;
}

void Combination::recycleChunk() {
    Chunk* chunk = _chunks.back();
    _chunks.pop_back();
    _chunkPool->recycle(chunk);
    _countInCurrentChunk = _chunkCapacity;
}

}  // namespace MelonCore

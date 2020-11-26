#pragma once

#include <MelonCore/ArchetypeMask.h>
#include <MelonCore/Chunk.h>
#include <MelonCore/ChunkAccessor.h>
#include <MelonCore/Entity.h>
#include <MelonCore/ObjectPool.h>
#include <MelonCore/ObjectStore.h>

#include <cstddef>
#include <cstdlib>
#include <unordered_map>
#include <vector>

namespace MelonCore {

class Combination {
   public:
    Combination(const ChunkLayout& chunkLayout, const std::vector<unsigned int>& sharedComponentIds, const std::vector<unsigned int>& sharedComponentIndices, ObjectPool<Chunk>* chunkPool);

    void addEntity(const Entity& entity, unsigned int& entityIndexInCombination, bool& chunkCountAdded);
    // Move an Entity when adding one component
    void moveEntityAddingComponent(const unsigned int& entityIndexInSrcCombination, Combination* srcCombination, const unsigned int& componentId, const void* component, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused);
    // Move an Entity when removing zero or more components
    void moveEntityRemovingComponent(const unsigned int& entityIndexInSrcCombination, Combination* srcCombination, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused);
    void removeEntity(const unsigned int& entityIndexInCombination, Entity& swappedEntity, bool& chunkCountMinused);
    void setComponent(const unsigned int& entityIndexInCombination, const unsigned int& componentId, const void* component);

    void filterEntities(const ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount>& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const;

    unsigned int chunkCount() const { return _chunks.size(); }
    bool hasComponent(const unsigned int& componentId) const { return _chunkLayout.componentIndexMap.contains(componentId); }

    const std::vector<unsigned int>& sharedComponentIndices() const { return _sharedComponentIndices; }
    const unsigned int& entityCount() const { return _entityCount; }

   private:
    void requestChunk();
    void recycleChunk();

    Entity* entityAddress(const unsigned int& entityIndex) const;
    Entity* entityAddress(Chunk* chunk, const unsigned int& entityIndexInChunk) const;
    void* componentAddress(const unsigned int& componentId, const unsigned int& entityIndex) const;
    void* componentAddress(Chunk* chunk, const unsigned int& componentIndex, const unsigned int& entityIndexInChunk) const;

    const ChunkLayout& _chunkLayout;

    const std::vector<unsigned int>& _sharedComponentIds;
    const std::vector<unsigned int> _sharedComponentIndices;

    ObjectPool<Chunk>* _chunkPool;
    std::vector<Chunk*> _chunks;

    unsigned int _entityCount{};
    unsigned int _entityCountInCurrentChunk{};
};

inline void Combination::filterEntities(const ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount>& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const {
    chunkAccessors.reserve(chunkAccessors.size() + chunkCount());
    for (Chunk* chunk : _chunks)
        chunkAccessors.emplace_back(ChunkAccessor{reinterpret_cast<std::byte*>(chunk), _chunkLayout, chunk != _chunks.back() ? _chunkLayout.capacity : _entityCountInCurrentChunk, _sharedComponentIds, _sharedComponentIndices, sharedComponentStore});
}

inline Entity* Combination::entityAddress(const unsigned int& entityIndex) const {
    Chunk* chunk = _chunks[entityIndex / _chunkLayout.capacity];
    const unsigned int entityIndexInChunk = entityIndex % _chunkLayout.capacity;
    return entityAddress(chunk, entityIndexInChunk);
}

inline Entity* Combination::entityAddress(Chunk* chunk, const unsigned int& entityIndexInChunk) const {
    return reinterpret_cast<Entity*>(reinterpret_cast<std::byte*>(chunk) + _chunkLayout.entityOffset + sizeof(Entity) * entityIndexInChunk);
}

inline void* Combination::componentAddress(const unsigned int& componentId, const unsigned int& entityIndex) const {
    Chunk* chunk = _chunks[entityIndex / _chunkLayout.capacity];
    const unsigned int entityIndexInChunk = entityIndex % _chunkLayout.capacity;
    const unsigned int componentIndex = _chunkLayout.componentIndexMap.at(componentId);
    return componentAddress(chunk, componentIndex, entityIndexInChunk);
}

inline void* Combination::componentAddress(Chunk* chunk, const unsigned int& componentIndex, const unsigned int& entityIndexInChunk) const {
    return static_cast<void*>(reinterpret_cast<std::byte*>(chunk) + _chunkLayout.componentOffsets[componentIndex] + _chunkLayout.componentSizes[componentIndex] * entityIndexInChunk);
}

}  // namespace MelonCore

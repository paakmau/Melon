#pragma once

#include <MelonCore/ArchetypeMask.h>
#include <MelonCore/Chunk.h>
#include <MelonCore/ChunkAccessor.h>
#include <MelonCore/Entity.h>
#include <MelonCore/ObjectPool.h>
#include <MelonCore/ObjectStore.h>

#include <climits>
#include <cstddef>
#include <cstdlib>
#include <unordered_map>
#include <vector>

namespace MelonCore {

class Combination {
  public:
    static constexpr unsigned int kInvalidIndex = std::numeric_limits<unsigned int>::max();
    static constexpr unsigned int kInvalidEntityIndex = std::numeric_limits<unsigned int>::max();

    Combination(unsigned int const& index, ChunkLayout const& chunkLayout, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectPool<Chunk>* chunkPool);
    Combination(Combination const&) = delete;

    void addEntity(Entity const& entity, unsigned int& entityIndexInCombination, bool& chunkCountAdded);
    // Move an Entity when adding one component
    void moveEntityAddingComponent(unsigned int const& entityIndexInSrcCombination, Combination* srcCombination, unsigned int const& componentId, void const* component, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused);
    // Move an Entity when removing zero or more components
    void moveEntityRemovingComponent(unsigned int const& entityIndexInSrcCombination, Combination* srcCombination, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused);
    void removeEntity(unsigned int const& entityIndexInCombination, Entity& swappedEntity, bool& chunkCountMinused);
    void setComponent(unsigned int const& entityIndexInCombination, unsigned int const& componentId, void const* component);

    void filterEntities(ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount> const& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const;

    unsigned int chunkCount() const { return _chunks.size(); }
    bool hasComponent(unsigned int const& componentId) const { return _chunkLayout.componentIndexMap.contains(componentId); }

    unsigned int const& index() const { return _index; }

    std::vector<unsigned int> const& sharedComponentIndices() const { return _sharedComponentIndices; }
    unsigned int const& entityCount() const { return _entityCount; }

  private:
    void requestChunk();
    void recycleChunk();

    Entity* entityAddress(unsigned int const& entityIndex) const;
    Entity* entityAddress(Chunk* chunk, unsigned int const& entityIndexInChunk) const;
    void* componentAddress(unsigned int const& componentId, unsigned int const& entityIndex) const;
    void* componentAddress(Chunk* chunk, unsigned int const& componentIndex, unsigned int const& entityIndexInChunk) const;

    unsigned int const _index;

    ChunkLayout const& _chunkLayout;

    std::vector<unsigned int> const& _sharedComponentIds;
    std::vector<unsigned int> const _sharedComponentIndices;

    ObjectPool<Chunk>* _chunkPool;
    std::vector<Chunk*> _chunks;

    unsigned int _entityCount{};
    unsigned int _entityCountInCurrentChunk{};
};

inline void Combination::filterEntities(ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount> const& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const {
    chunkAccessors.reserve(chunkAccessors.size() + chunkCount());
    for (Chunk* chunk : _chunks)
        chunkAccessors.emplace_back(ChunkAccessor{reinterpret_cast<std::byte*>(chunk), _chunkLayout, chunk != _chunks.back() ? _chunkLayout.capacity : _entityCountInCurrentChunk, _sharedComponentIds, _sharedComponentIndices, sharedComponentStore});
}

inline Entity* Combination::entityAddress(unsigned int const& entityIndex) const {
    Chunk* chunk = _chunks[entityIndex / _chunkLayout.capacity];
    unsigned int const entityIndexInChunk = entityIndex % _chunkLayout.capacity;
    return entityAddress(chunk, entityIndexInChunk);
}

inline Entity* Combination::entityAddress(Chunk* chunk, unsigned int const& entityIndexInChunk) const {
    return reinterpret_cast<Entity*>(reinterpret_cast<std::byte*>(chunk) + _chunkLayout.entityOffset + sizeof(Entity) * entityIndexInChunk);
}

inline void* Combination::componentAddress(unsigned int const& componentId, unsigned int const& entityIndex) const {
    Chunk* chunk = _chunks[entityIndex / _chunkLayout.capacity];
    unsigned int const entityIndexInChunk = entityIndex % _chunkLayout.capacity;
    unsigned int const componentIndex = _chunkLayout.componentIndexMap.at(componentId);
    return componentAddress(chunk, componentIndex, entityIndexInChunk);
}

inline void* Combination::componentAddress(Chunk* chunk, unsigned int const& componentIndex, unsigned int const& entityIndexInChunk) const {
    return static_cast<void*>(reinterpret_cast<std::byte*>(chunk) + _chunkLayout.componentOffsets[componentIndex] + _chunkLayout.componentSizes[componentIndex] * entityIndexInChunk);
}

}  // namespace MelonCore

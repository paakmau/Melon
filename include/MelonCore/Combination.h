#pragma once

#include <MelonCore/Entity.h>

#include <cstddef>
#include <cstdlib>
#include <unordered_map>
#include <vector>

namespace MelonCore {

template <typename T>
class ObjectPool;

struct ChunkLayout {
    std::size_t entityOffset{};
    std::unordered_map<unsigned int, unsigned int> componentIndexMap;
    std::vector<std::size_t> componentSizes;
    std::vector<std::size_t> componentOffsets;
};

class ChunkAccessor {
   public:
    Entity* entityArray() const;
    template <typename T>
    T* componentArray(const unsigned int& componentId) const;

    const unsigned int& entityCount() const { return _entityCount; }

   private:
    ChunkAccessor(std::byte* chunk, const ChunkLayout& chunkLayout, const unsigned int& entityCount);
    std::byte* const _chunk;
    const ChunkLayout& _chunkLayout;
    const unsigned int _entityCount;

    friend class Combination;
};

class Combination {
   public:
    static constexpr std::size_t kAlign = 64;
    static constexpr std::size_t kSize = 16 << 10;

    typedef std::aligned_storage_t<kSize, kAlign> Chunk;

    Combination(
        const std::vector<unsigned int>& componentIds,
        const std::vector<std::size_t>& componentSizes,
        const std::vector<std::size_t>& componentAligns,
        ObjectPool<Chunk>* chunkPool);

    unsigned int addEntity(const Entity& entity);
    unsigned int copyEntity(const unsigned int& entityIndexInSrcCombination, Combination* srdCombination);
    Entity removeEntity(const unsigned int& entityIndex);
    void setComponent(const unsigned int& entityIndex, const unsigned int& componentId, const void* component);

    std::vector<ChunkAccessor> chunkAccessors() const;

    unsigned int chunkCount() const;
    bool hasComponent(const unsigned int& componentId) const;

    const unsigned int& count() const { return _count; }

   private:
    void requestChunk();
    void recycleChunk();

    Entity* entityAddress(const unsigned int& entityIndex) const;
    Entity* entityAddress(Chunk* chunk, const unsigned int& entityIndexInChunk) const;
    void* componentAddress(const unsigned int& componentId, const unsigned int& entityIndex) const;
    void* componentAddress(Chunk* chunk, const unsigned int& componentIndex, const unsigned int& entityIndexInChunk) const;

    ChunkLayout _chunkLayout;

    ObjectPool<Chunk>* const _chunkPool;
    std::vector<Chunk*> _chunks;

    unsigned int _count{};
    unsigned int _chunkCapacity;
    unsigned int _countInCurrentChunk;
};

template <typename T>
inline T* ChunkAccessor::componentArray(const unsigned int& componentId) const {
    return reinterpret_cast<T*>(reinterpret_cast<std::byte*>(_chunk) + _chunkLayout.componentOffsets[_chunkLayout.componentIndexMap.at(componentId)]);
}

inline Entity* ChunkAccessor::entityArray() const {
    return reinterpret_cast<Entity*>(reinterpret_cast<std::byte*>(_chunk) + _chunkLayout.entityOffset);
}

inline ChunkAccessor::ChunkAccessor(std::byte* chunk, const ChunkLayout& chunkLayout, const unsigned int& entityCount) : _chunk(chunk), _chunkLayout(chunkLayout), _entityCount(entityCount) {}

inline std::vector<ChunkAccessor> Combination::chunkAccessors() const {
    std::vector<ChunkAccessor> accessors;
    accessors.reserve(_chunks.size());
    for (Chunk* chunk : _chunks)
        accessors.emplace_back(ChunkAccessor(reinterpret_cast<std::byte*>(chunk), _chunkLayout, chunk != _chunks.back() ? _chunkCapacity : _countInCurrentChunk));
    return accessors;
}

inline unsigned int Combination::chunkCount() const { return _chunks.size(); }

inline bool Combination::hasComponent(const unsigned int& componentId) const { return _chunkLayout.componentIndexMap.contains(componentId); }

inline Entity* Combination::entityAddress(const unsigned int& entityIndex) const {
    Chunk* chunk = _chunks[entityIndex / _chunkCapacity];
    const unsigned int entityIndexInChunk = entityIndex % _chunkCapacity;
    return entityAddress(chunk, entityIndexInChunk);
}

inline Entity* Combination::entityAddress(Chunk* chunk, const unsigned int& entityIndexInChunk) const {
    return reinterpret_cast<Entity*>(reinterpret_cast<std::byte*>(chunk) + _chunkLayout.entityOffset + sizeof(Entity) * entityIndexInChunk);
}

inline void* Combination::componentAddress(const unsigned int& componentId, const unsigned int& entityIndex) const {
    Chunk* chunk = _chunks[entityIndex / _chunkCapacity];
    const unsigned int entityIndexInChunk = entityIndex % _chunkCapacity;
    const unsigned int componentIndex = _chunkLayout.componentIndexMap.at(componentId);
    return componentAddress(chunk, componentIndex, entityIndexInChunk);
}

inline void* Combination::componentAddress(Chunk* chunk, const unsigned int& componentIndex, const unsigned int& entityIndexInChunk) const {
    return static_cast<void*>(reinterpret_cast<std::byte*>(chunk) + _chunkLayout.componentOffsets[componentIndex] + _chunkLayout.componentSizes[componentIndex] * entityIndexInChunk);
}

}  // namespace MelonCore

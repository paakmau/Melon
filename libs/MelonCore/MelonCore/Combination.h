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

namespace Melon {

class Combination {
  public:
    static constexpr unsigned int k_InvalidIndex = std::numeric_limits<unsigned int>::max();
    static constexpr unsigned int k_InvalidEntityIndex = std::numeric_limits<unsigned int>::max();

    Combination(const unsigned int& index, const ChunkLayout& chunkLayout, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectPool<Chunk>* chunkPool);
    Combination(const Combination&) = delete;

    void addEntity(const Entity& entity, unsigned int& entityIndexInCombination, bool& chunkCountAdded);
    // Move an Entity when adding one component
    void moveEntityAddingComponent(const unsigned int& entityIndexInSrcCombination, Combination* srcCombination, const unsigned int& componentId, const void* component, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused);
    // Move an Entity when removing zero or more components
    void moveEntityRemovingComponent(const unsigned int& entityIndexInSrcCombination, Combination* srcCombination, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused);
    void removeEntity(const unsigned int& entityIndexInCombination, Entity& swappedEntity, bool& chunkCountMinused);
    void setComponent(const unsigned int& entityIndexInCombination, const unsigned int& componentId, const void* component);

    void filterEntities(ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const;

    bool empty() const { return m_EntityCount == 0; }
    unsigned int chunkCount() const { return m_Chunks.size(); }
    bool hasComponent(const unsigned int& componentId) const { return m_ChunkLayout.componentIndexMap.contains(componentId); }

    const unsigned int& index() const { return m_Index; }

    const std::vector<unsigned int>& sharedComponentIndices() const { return m_SharedComponentIndices; }
    const unsigned int& entityCount() const { return m_EntityCount; }

  private:
    void requestChunk();
    void recycleChunk();

    Entity* entityAddress(const unsigned int& entityIndex) const;
    Entity* entityAddress(Chunk* chunk, const unsigned int& entityIndexInChunk) const;
    void* componentAddress(const unsigned int& componentId, const unsigned int& entityIndex) const;
    void* componentAddress(Chunk* chunk, const unsigned int& componentIndex, const unsigned int& entityIndexInChunk) const;

    const unsigned int m_Index;

    const ChunkLayout& m_ChunkLayout;

    std::vector<unsigned int> const& m_SharedComponentIds;
    std::vector<unsigned int> const m_SharedComponentIndices;

    ObjectPool<Chunk>* m_ChunkPool;
    std::vector<Chunk*> m_Chunks;

    unsigned int m_EntityCount{};
    unsigned int m_EntityCountInCurrentChunk{};
};

inline void Combination::filterEntities(ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const {
    chunkAccessors.reserve(chunkAccessors.size() + chunkCount());
    for (Chunk* chunk : m_Chunks)
        chunkAccessors.emplace_back(ChunkAccessor{reinterpret_cast<std::byte*>(chunk), m_ChunkLayout, chunk != m_Chunks.back() ? m_ChunkLayout.capacity : m_EntityCountInCurrentChunk, m_SharedComponentIds, m_SharedComponentIndices, sharedComponentStore});
}

inline Entity* Combination::entityAddress(const unsigned int& entityIndex) const {
    Chunk* chunk = m_Chunks[entityIndex / m_ChunkLayout.capacity];
    const unsigned int entityIndexInChunk = entityIndex % m_ChunkLayout.capacity;
    return entityAddress(chunk, entityIndexInChunk);
}

inline Entity* Combination::entityAddress(Chunk* chunk, const unsigned int& entityIndexInChunk) const {
    return reinterpret_cast<Entity*>(reinterpret_cast<std::byte*>(chunk) + m_ChunkLayout.entityOffset + sizeof(Entity) * entityIndexInChunk);
}

inline void* Combination::componentAddress(const unsigned int& componentId, const unsigned int& entityIndex) const {
    Chunk* chunk = m_Chunks[entityIndex / m_ChunkLayout.capacity];
    const unsigned int entityIndexInChunk = entityIndex % m_ChunkLayout.capacity;
    const unsigned int componentIndex = m_ChunkLayout.componentIndexMap.at(componentId);
    return componentAddress(chunk, componentIndex, entityIndexInChunk);
}

inline void* Combination::componentAddress(Chunk* chunk, const unsigned int& componentIndex, const unsigned int& entityIndexInChunk) const {
    return static_cast<void*>(reinterpret_cast<std::byte*>(chunk) + m_ChunkLayout.componentOffsets[componentIndex] + m_ChunkLayout.componentSizes[componentIndex] * entityIndexInChunk);
}

}  // namespace Melon

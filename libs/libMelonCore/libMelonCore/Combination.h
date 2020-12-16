#pragma once

#include <libMelonCore/ArchetypeMask.h>
#include <libMelonCore/Chunk.h>
#include <libMelonCore/ChunkAccessor.h>
#include <libMelonCore/Entity.h>
#include <libMelonCore/ObjectPool.h>
#include <libMelonCore/ObjectStore.h>

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

    Combination(unsigned int const& index, ChunkLayout const& chunkLayout, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectPool<Chunk>* chunkPool);
    Combination(Combination const&) = delete;

    void addEntity(Entity const& entity, unsigned int& entityIndexInCombination, bool& chunkCountAdded);
    // Move an Entity when adding one component
    void moveEntityAddingComponent(unsigned int const& entityIndexInSrcCombination, Combination* srcCombination, unsigned int const& componentId, void const* component, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused);
    // Move an Entity when removing zero or more components
    void moveEntityRemovingComponent(unsigned int const& entityIndexInSrcCombination, Combination* srcCombination, unsigned int& entityIndexInDstCombination, bool& dstChunkCountAdded, Entity& swappedEntity, bool& srcChunkCountMinused);
    void removeEntity(unsigned int const& entityIndexInCombination, Entity& swappedEntity, bool& chunkCountMinused);
    void setComponent(unsigned int const& entityIndexInCombination, unsigned int const& componentId, void const* component);

    void filterEntities(ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const;

    unsigned int chunkCount() const { return m_Chunks.size(); }
    bool hasComponent(unsigned int const& componentId) const { return m_ChunkLayout.componentIndexMap.contains(componentId); }

    unsigned int const& index() const { return m_Index; }

    std::vector<unsigned int> const& sharedComponentIndices() const { return m_SharedComponentIndices; }
    unsigned int const& entityCount() const { return m_EntityCount; }

  private:
    void requestChunk();
    void recycleChunk();

    Entity* entityAddress(unsigned int const& entityIndex) const;
    Entity* entityAddress(Chunk* chunk, unsigned int const& entityIndexInChunk) const;
    void* componentAddress(unsigned int const& componentId, unsigned int const& entityIndex) const;
    void* componentAddress(Chunk* chunk, unsigned int const& componentIndex, unsigned int const& entityIndexInChunk) const;

    unsigned int const m_Index;

    ChunkLayout const& m_ChunkLayout;

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

inline Entity* Combination::entityAddress(unsigned int const& entityIndex) const {
    Chunk* chunk = m_Chunks[entityIndex / m_ChunkLayout.capacity];
    unsigned int const entityIndexInChunk = entityIndex % m_ChunkLayout.capacity;
    return entityAddress(chunk, entityIndexInChunk);
}

inline Entity* Combination::entityAddress(Chunk* chunk, unsigned int const& entityIndexInChunk) const {
    return reinterpret_cast<Entity*>(reinterpret_cast<std::byte*>(chunk) + m_ChunkLayout.entityOffset + sizeof(Entity) * entityIndexInChunk);
}

inline void* Combination::componentAddress(unsigned int const& componentId, unsigned int const& entityIndex) const {
    Chunk* chunk = m_Chunks[entityIndex / m_ChunkLayout.capacity];
    unsigned int const entityIndexInChunk = entityIndex % m_ChunkLayout.capacity;
    unsigned int const componentIndex = m_ChunkLayout.componentIndexMap.at(componentId);
    return componentAddress(chunk, componentIndex, entityIndexInChunk);
}

inline void* Combination::componentAddress(Chunk* chunk, unsigned int const& componentIndex, unsigned int const& entityIndexInChunk) const {
    return static_cast<void*>(reinterpret_cast<std::byte*>(chunk) + m_ChunkLayout.componentOffsets[componentIndex] + m_ChunkLayout.componentSizes[componentIndex] * entityIndexInChunk);
}

}  // namespace Melon

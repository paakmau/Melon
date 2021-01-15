#pragma once

#include <libMelonCore/ArchetypeMask.h>
#include <libMelonCore/Chunk.h>
#include <libMelonCore/Component.h>
#include <libMelonCore/Entity.h>
#include <libMelonCore/ObjectStore.h>
#include <libMelonCore/SharedComponent.h>

namespace Melon {

class ChunkAccessor {
  public:
    const Entity* entityArray() const;
    template <typename Type>
    Type* componentArray(const unsigned int& componentId) const;

    unsigned int sharedComponentIndex(const unsigned int& sharedComponentId) const;
    template <typename Type>
    const Type* sharedComponent(const unsigned int& sharedComponentId) const;

    const unsigned int& entityCount() const { return m_EntityCount; }

  private:
    ChunkAccessor(std::byte* chunk, const ChunkLayout& chunkLayout, const unsigned int& entityCount, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& sharedComponentStore);
    std::byte* const m_Chunk;
    const ChunkLayout& m_ChunkLayout;
    const unsigned int& m_EntityCount;
    std::vector<unsigned int> const& m_SharedComponentIds;
    std::vector<unsigned int> const& m_SharedComponentIndices;

    ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& m_SharedComponentStore;

    friend class Combination;
};

inline const Entity* ChunkAccessor::entityArray() const {
    return reinterpret_cast<const Entity*>(reinterpret_cast<std::byte*>(m_Chunk) + m_ChunkLayout.entityOffset);
}

template <typename Type>
inline Type* ChunkAccessor::componentArray(const unsigned int& componentId) const {
    static_assert(std::is_base_of_v<Component, Type>);
    return reinterpret_cast<Type*>(reinterpret_cast<std::byte*>(m_Chunk) + m_ChunkLayout.componentOffsets[m_ChunkLayout.componentIndexMap.at(componentId)]);
}

inline unsigned int ChunkAccessor::sharedComponentIndex(const unsigned int& sharedComponentId) const {
    for (unsigned int i = 0; i < m_SharedComponentIds.size(); i++)
        if (m_SharedComponentIds[i] == sharedComponentId)
            return m_SharedComponentIndices[i];
    return 0;
}

template <typename Type>
inline const Type* ChunkAccessor::sharedComponent(const unsigned int& sharedComponentId) const {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    return m_SharedComponentStore.object<Type>(sharedComponentIndex(sharedComponentId));
}

inline ChunkAccessor::ChunkAccessor(std::byte* chunk, const ChunkLayout& chunkLayout, const unsigned int& entityCount, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& sharedComponentStore) : m_Chunk(chunk), m_ChunkLayout(chunkLayout), m_EntityCount(entityCount), m_SharedComponentIds(sharedComponentIds), m_SharedComponentIndices(sharedComponentIndices), m_SharedComponentStore(sharedComponentStore) {}

}  // namespace Melon

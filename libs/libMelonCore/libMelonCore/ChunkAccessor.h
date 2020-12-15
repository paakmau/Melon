#pragma once

#include <libMelonCore/ArchetypeMask.h>
#include <libMelonCore/Chunk.h>
#include <libMelonCore/Component.h>
#include <libMelonCore/Entity.h>
#include <libMelonCore/ObjectStore.h>
#include <libMelonCore/SharedComponent.h>

namespace MelonCore {

class ChunkAccessor {
  public:
    Entity const* entityArray() const;
    template <typename Type>
    Type* componentArray(unsigned int const& componentId) const;

    unsigned int sharedComponentIndex(unsigned int const& sharedComponentId) const;
    template <typename Type>
    Type const* sharedComponent(unsigned int const& sharedComponentId) const;

    unsigned int const& entityCount() const { return m_EntityCount; }

  private:
    ChunkAccessor(std::byte* chunk, ChunkLayout const& chunkLayout, unsigned int const& entityCount, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& sharedComponentStore);
    std::byte* const m_Chunk;
    ChunkLayout const& m_ChunkLayout;
    unsigned int const& m_EntityCount;
    std::vector<unsigned int> const& m_SharedComponentIds;
    std::vector<unsigned int> const& m_SharedComponentIndices;

    ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& m_SharedComponentStore;

    friend class Combination;
};

inline Entity const* ChunkAccessor::entityArray() const {
    return reinterpret_cast<Entity const*>(reinterpret_cast<std::byte*>(m_Chunk) + m_ChunkLayout.entityOffset);
}

template <typename Type>
inline Type* ChunkAccessor::componentArray(unsigned int const& componentId) const {
    static_assert(std::is_base_of_v<Component, Type>);
    return reinterpret_cast<Type*>(reinterpret_cast<std::byte*>(m_Chunk) + m_ChunkLayout.componentOffsets[m_ChunkLayout.componentIndexMap.at(componentId)]);
}

inline unsigned int ChunkAccessor::sharedComponentIndex(unsigned int const& sharedComponentId) const {
    for (unsigned int i = 0; i < m_SharedComponentIds.size(); i++)
        if (m_SharedComponentIds[i] == sharedComponentId)
            return m_SharedComponentIndices[i];
    return 0;
}

template <typename Type>
inline Type const* ChunkAccessor::sharedComponent(unsigned int const& sharedComponentId) const {
    static_assert(std::is_base_of_v<SharedComponent, Type>);
    return m_SharedComponentStore.object<Type>(sharedComponentIndex(sharedComponentId));
}

inline ChunkAccessor::ChunkAccessor(std::byte* chunk, ChunkLayout const& chunkLayout, unsigned int const& entityCount, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& sharedComponentStore) : m_Chunk(chunk), m_ChunkLayout(chunkLayout), m_EntityCount(entityCount), m_SharedComponentIds(sharedComponentIds), m_SharedComponentIndices(sharedComponentIndices), m_SharedComponentStore(sharedComponentStore) {}

}  // namespace MelonCore

#pragma once

#include <MelonCore/ArchetypeMask.h>
#include <MelonCore/Chunk.h>
#include <MelonCore/Component.h>
#include <MelonCore/Entity.h>
#include <MelonCore/ObjectStore.h>
#include <MelonCore/SharedComponent.h>

namespace MelonCore {

class ChunkAccessor {
  public:
    Entity const* entityArray() const;
    template <typename T>
    T* componentArray(unsigned int const& componentId) const;

    unsigned int sharedComponentIndex(unsigned int const& sharedComponentId) const;
    template <typename T>
    T const* sharedComponent(unsigned int const& sharedComponentId) const;

    unsigned int const& entityCount() const { return _entityCount; }

  private:
    ChunkAccessor(std::byte* chunk, ChunkLayout const& chunkLayout, unsigned int const& entityCount, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount> const& sharedComponentStore);
    std::byte* const _chunk;
    ChunkLayout const& _chunkLayout;
    unsigned int const& _entityCount;
    std::vector<unsigned int> const& _sharedComponentIds;
    std::vector<unsigned int> const& _sharedComponentIndices;

    ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount> const& _sharedComponentStore;

    friend class Combination;
};

inline Entity const* ChunkAccessor::entityArray() const {
    return reinterpret_cast<Entity const*>(reinterpret_cast<std::byte*>(_chunk) + _chunkLayout.entityOffset);
}

template <typename T>
inline T* ChunkAccessor::componentArray(unsigned int const& componentId) const {
    static_assert(std::is_base_of_v<Component, T>);
    return reinterpret_cast<T*>(reinterpret_cast<std::byte*>(_chunk) + _chunkLayout.componentOffsets[_chunkLayout.componentIndexMap.at(componentId)]);
}

inline unsigned int ChunkAccessor::sharedComponentIndex(unsigned int const& sharedComponentId) const {
    for (unsigned int i = 0; i < _sharedComponentIds.size(); i++)
        if (_sharedComponentIds[i] == sharedComponentId)
            return _sharedComponentIndices[i];
    return 0;
}

template <typename T>
inline T const* ChunkAccessor::sharedComponent(unsigned int const& sharedComponentId) const {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    return _sharedComponentStore.object<T>(sharedComponentIndex(sharedComponentId));
}

inline ChunkAccessor::ChunkAccessor(std::byte* chunk, ChunkLayout const& chunkLayout, unsigned int const& entityCount, std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices, ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount> const& sharedComponentStore) : _chunk(chunk), _chunkLayout(chunkLayout), _entityCount(entityCount), _sharedComponentIds(sharedComponentIds), _sharedComponentIndices(sharedComponentIndices), _sharedComponentStore(sharedComponentStore) {}

}  // namespace MelonCore

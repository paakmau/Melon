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
    const Entity* entityArray() const;
    template <typename T>
    T* componentArray(const unsigned int& componentId) const;

    unsigned int sharedComponentIndex(const unsigned int& sharedComponentId) const;
    template <typename T>
    const T* sharedComponent(const unsigned int& sharedComponentId) const;

    const unsigned int& entityCount() const { return _entityCount; }

  private:
    ChunkAccessor(std::byte* chunk, const ChunkLayout& chunkLayout, const unsigned int& entityCount, const std::vector<unsigned int>& sharedComponentIds, const std::vector<unsigned int>& sharedComponentIndices, const ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount>& sharedComponentStore);
    std::byte* const _chunk;
    const ChunkLayout& _chunkLayout;
    const unsigned int& _entityCount;
    const std::vector<unsigned int>& _sharedComponentIds;
    const std::vector<unsigned int>& _sharedComponentIndices;

    const ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount>& _sharedComponentStore;

    friend class Combination;
};

inline const Entity* ChunkAccessor::entityArray() const {
    return reinterpret_cast<const Entity*>(reinterpret_cast<std::byte*>(_chunk) + _chunkLayout.entityOffset);
}

template <typename T>
inline T* ChunkAccessor::componentArray(const unsigned int& componentId) const {
    static_assert(std::is_base_of_v<Component, T>);
    return reinterpret_cast<T*>(reinterpret_cast<std::byte*>(_chunk) + _chunkLayout.componentOffsets[_chunkLayout.componentIndexMap.at(componentId)]);
}

inline unsigned int ChunkAccessor::sharedComponentIndex(const unsigned int& sharedComponentId) const {
    for (unsigned int i = 0; i < _sharedComponentIds.size(); i++)
        if (_sharedComponentIds[i] == sharedComponentId)
            return _sharedComponentIndices[i];
    return 0;
}

template <typename T>
inline const T* ChunkAccessor::sharedComponent(const unsigned int& sharedComponentId) const {
    static_assert(std::is_base_of_v<SharedComponent, T>);
    return _sharedComponentStore.object<T>(sharedComponentIndex(sharedComponentId));
}

inline ChunkAccessor::ChunkAccessor(std::byte* chunk, const ChunkLayout& chunkLayout, const unsigned int& entityCount, const std::vector<unsigned int>& sharedComponentIds, const std::vector<unsigned int>& sharedComponentIndices, const ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount>& sharedComponentStore) : _chunk(chunk), _chunkLayout(chunkLayout), _entityCount(entityCount), _sharedComponentIds(sharedComponentIds), _sharedComponentIndices(sharedComponentIndices), _sharedComponentStore(sharedComponentStore) {}

}  // namespace MelonCore

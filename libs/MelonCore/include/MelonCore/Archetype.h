#pragma once

#include <MelonCore/ArchetypeMask.h>
#include <MelonCore/Chunk.h>
#include <MelonCore/ChunkAccessor.h>
#include <MelonCore/Combination.h>
#include <MelonCore/Entity.h>
#include <MelonCore/EntityFilter.h>
#include <MelonCore/ObjectPool.h>
#include <MelonCore/ObjectStore.h>

#include <bitset>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace MelonCore {

class Archetype {
  public:
    static constexpr unsigned int kInvalidId = std::numeric_limits<unsigned int>::max();

    struct EntityLocation {
        static constexpr EntityLocation invalidEntityLocation() { return EntityLocation{Archetype::kInvalidId, Combination::kInvalidIndex, Combination::kInvalidEntityIndex}; }

        bool valid() const { return archetypeId != Archetype::kInvalidId; }

        unsigned int archetypeId;
        unsigned int combinationIndex;
        unsigned int entityIndexInCombination;
    };

    struct SharedComponentIndexHash {
        std::size_t operator()(std::vector<unsigned int> const& sharedComponentIndices) const {
            std::size_t hash = sharedComponentIndices.size();
            for (unsigned int const& sharedComponentIndex : sharedComponentIndices)
                hash ^= (hash << 8) + sharedComponentIndex;
            return hash;
        }
    };

    Archetype(
        unsigned int const& id,
        ArchetypeMask const& mask,
        std::vector<unsigned int> const& componentIds,
        std::vector<std::size_t> const& componentSizes,
        std::vector<std::size_t> const& componentAligns,
        std::vector<unsigned int> const& sharedComponentIds,
        ObjectPool<Chunk>* chunkPool);
    Archetype(Archetype const&) = delete;

    void addEntity(Entity const& entity, EntityLocation& location);
    // Move an Entity when adding a Component
    void moveEntityAddingComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, unsigned int const& componentId, void const* component, EntityLocation& dstLocation, Entity& srcSwappedEntity);
    // Move an Entity when removing a Component
    void moveEntityRemovingComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, EntityLocation& dstLocation, Entity& srcSwappedEntity);
    // Move an Entity when adding a SharedComponent
    void moveEntityAddingSharedComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, unsigned int const& sharedComponentId, unsigned int const& sharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity);
    // Move an Entity when removing a SharedComponent
    void moveEntityRemovingSharedComponent(EntityLocation const& srcEntityLocation, Archetype* srcArchetype, unsigned int& originalSharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity);
    void removeEntity(EntityLocation const& location, std::vector<unsigned int>& sharedComponentIndices, Entity& swappedEntity);
    void setComponent(EntityLocation const& location, unsigned int const& componentId, void const* component);
    void setSharedComponent(EntityLocation const& location, unsigned int const& sharedComponentId, unsigned int const& sharedComponentIndex, unsigned int& originalSharedComponentIndex, EntityLocation& dstLocation, Entity& swappedEntity);

    void filterEntities(EntityFilter const& entityFilter, ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount> const& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const;

    bool single() const { return _mask.single(); }
    bool fullyManual() const { return _mask.fullyManual(); }
    bool partiallyManual() const { return _mask.partiallyManual(); }

    std::vector<unsigned int> notManualComponentIds() const;
    std::vector<unsigned int> notManualSharedComponentIds() const;

    unsigned int componentCount() const { return _mask.componentCount(); }
    unsigned int sharedComponentCount() const { return _mask.sharedComponentCount(); }

    unsigned int const& id() const { return _id; }
    ArchetypeMask const& mask() const { return _mask; }

    std::vector<unsigned int> const& componentIds() const { return _componentIds; }
    std::vector<std::size_t> const& componentSizes() const { return _componentSizes; }
    std::vector<std::size_t> const& componentAligns() const { return _componentAligns; }

    std::vector<unsigned int> const& sharedComponentIds() const { return _sharedComponentIds; }

    unsigned int const& chunkCount() const { return _chunkCount; }
    unsigned int const& entityCount() const { return _entityCount; }

  private:
    Combination* createCombination();
    Combination* createCombination(std::vector<unsigned int> const& sharedComponentIndices);
    void destroyCombination(unsigned int const& combinationIndex, std::vector<unsigned int> const& sharedComponentIndices);

    unsigned int const _id;
    ArchetypeMask const _mask;

    ChunkLayout _chunkLayout;
    std::vector<unsigned int> _componentIds;
    std::vector<std::size_t> _componentSizes;
    std::vector<std::size_t> _componentAligns;

    // Shared component ids should be in ascending order
    std::vector<unsigned int> _sharedComponentIds;

    unsigned int _chunkCount{};
    unsigned int _entityCount{};

    ObjectPool<Chunk>* _chunkPool;
    std::vector<std::unique_ptr<Combination>> _combinations;
    std::unordered_map<std::vector<unsigned int>, unsigned int, SharedComponentIndexHash> _combinationIndexMap;
    std::vector<unsigned int> _freeCombinationIndices;

    friend class EntityManager;
};

inline std::vector<unsigned int> Archetype::notManualComponentIds() const {
    std::vector<unsigned int> componentIds;
    for (unsigned int const& componentId : _componentIds)
        if (!_mask.manualComponent(componentId))
            componentIds.push_back(componentId);
    return componentIds;
}

inline std::vector<unsigned int> Archetype::notManualSharedComponentIds() const {
    std::vector<unsigned int> sharedComponentIds;
    for (unsigned int const& sharedComponentId : _sharedComponentIds)
        if (!_mask.manualSharedComponent(sharedComponentId))
            sharedComponentIds.push_back(sharedComponentId);
    return sharedComponentIds;
}

inline Combination* Archetype::createCombination() {
    return createCombination(std::vector<unsigned int>(sharedComponentCount(), ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount>::kInvalidIndex));
}

inline Combination* Archetype::createCombination(std::vector<unsigned int> const& sharedComponentIndices) {
    if (_combinationIndexMap.contains(sharedComponentIndices))
        return _combinations[_combinationIndexMap[sharedComponentIndices]].get();
    unsigned int combinationIndex;
    if (_freeCombinationIndices.empty()) {
        combinationIndex = _combinations.size();
        _combinations.emplace_back(std::make_unique<Combination>(combinationIndex, _chunkLayout, _sharedComponentIds, sharedComponentIndices, _chunkPool));
    } else {
        combinationIndex = _freeCombinationIndices.back(), _freeCombinationIndices.pop_back();
        _combinations[combinationIndex] = std::make_unique<Combination>(combinationIndex, _chunkLayout, _sharedComponentIds, sharedComponentIndices, _chunkPool);
    }
    _combinationIndexMap.emplace(sharedComponentIndices, combinationIndex);
    // TODO: A new Combination should not have a Chunk
    _chunkCount++;
    return _combinations[combinationIndex].get();
}

// TODO: Consider when to destroy a Combination
inline void Archetype::destroyCombination(unsigned int const& combinationIndex, std::vector<unsigned int> const& sharedComponentIndices) {
    _combinations[combinationIndex].reset();
    _combinationIndexMap.erase(sharedComponentIndices);
    _freeCombinationIndices.push_back(combinationIndex);
    // TODO: A new Combination should not have a Chunk
    _chunkCount--;
}

}  // namespace MelonCore

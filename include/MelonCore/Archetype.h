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

struct EntityLocation {
    unsigned int archetypeId;
    unsigned int combinationIndex;
    unsigned int entityIndexInCombination;
};

class Archetype {
   public:
    struct SharedComponentIndexHash {
        std::size_t operator()(const std::vector<unsigned int>& sharedComponentIndices) const {
            std::size_t hash = sharedComponentIndices.size();
            for (const unsigned int& sharedComponentIndex : sharedComponentIndices)
                hash ^= (hash << 8) + sharedComponentIndex;
            return hash;
        }
    };

    Archetype(
        const unsigned int& id,
        const ArchetypeMask& mask,
        const std::vector<unsigned int>& componentIds,
        const std::vector<std::size_t>& componentSizes,
        const std::vector<std::size_t>& componentAligns,
        const std::vector<unsigned int>& sharedComponentIds,
        ObjectPool<Chunk>* chunkPool);
    Archetype(const Archetype&) = delete;

    void addEntity(const Entity& entity, EntityLocation& location);
    // Move an Entity when adding a Component
    void moveEntityAddingComponent(const EntityLocation& srcEntityLocation, Archetype* srcArchetype, const unsigned int& componentId, const void* component, EntityLocation& dstLocation, Entity& srcSwappedEntity);
    // Move an Entity when removing a Component
    void moveEntityRemovingComponent(const EntityLocation& srcEntityLocation, Archetype* srcArchetype, EntityLocation& dstLocation, Entity& srcSwappedEntity);
    // Move an Entity when adding a SharedComponent
    void moveEntityAddingSharedComponent(const EntityLocation& srcEntityLocation, Archetype* srcArchetype, const unsigned int& sharedComponentId, const unsigned int& sharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity);
    // Move an Entity when removing a SharedComponent
    void moveEntityRemovingSharedComponent(const EntityLocation& srcEntityLocation, Archetype* srcArchetype, unsigned int& originalSharedComponentIndex, EntityLocation& dstLocation, Entity& srcSwappedEntity);
    void removeEntity(const EntityLocation& location, std::vector<unsigned int>& sharedComponentIndices, Entity& swappedEntity);
    void setComponent(const EntityLocation& location, const unsigned int& componentId, const void* component);
    void setSharedComponent(const EntityLocation& location, const unsigned int& sharedComponentId, const unsigned int& sharedComponentIndex, unsigned int& originalSharedComponentIndex, EntityLocation& dstLocation, Entity& swappedEntity);

    void filterEntities(const EntityFilter& entityFilter, const ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount>& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const;

    bool singleAndManual() const { return _mask.singleAndManual(); }
    bool fullyManual() const { return _mask.fullyManual(); }
    bool partiallyManual() const { return _mask.paritiallyManual(); }

    std::vector<unsigned int> notManualComponentIds() const;
    std::vector<unsigned int> notManualSharedComponentIds() const;

    unsigned int componentCount() const { return _mask.componentCount(); }
    unsigned int sharedComponentCount() const { return _mask.sharedComponentCount(); }

    const unsigned int& id() const { return _id; }
    const ArchetypeMask& mask() const { return _mask; }

    const std::vector<unsigned int>& componentIds() const { return _componentIds; }
    const std::vector<std::size_t>& componentSizes() const { return _componentSizes; }
    const std::vector<std::size_t>& componentAligns() const { return _componentAligns; }

    const std::vector<unsigned int>& sharedComponentIds() const { return _sharedComponentIds; }

    const unsigned int& chunkCount() const { return _chunkCount; }
    const unsigned int& entityCount() const { return _entityCount; }

   private:
    Combination* createCombination();
    Combination* createCombination(const std::vector<unsigned int>& sharedComponentIndices);
    void destroyCombination(const unsigned int& combinationIndex, const std::vector<unsigned int>& sharedComponentIndices);

    const unsigned int _id;
    const ArchetypeMask _mask;

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
    for (const unsigned int& componentId : _componentIds)
        if (!_mask.manualComponent(componentId))
            componentIds.push_back(componentId);
    return componentIds;
}

inline std::vector<unsigned int> Archetype::notManualSharedComponentIds() const {
    std::vector<unsigned int> sharedComponentIds;
    for (const unsigned int& sharedComponentId : _sharedComponentIds)
        if (!_mask.manualSharedComponent(sharedComponentId))
            sharedComponentIds.push_back(sharedComponentId);
    return sharedComponentIds;
}

inline Combination* Archetype::createCombination() {
    return createCombination(std::vector<unsigned int>(sharedComponentCount(), ObjectStore<ArchetypeMask::kMaxSharedComponentIdCount>::kNullPointerIndex));
}

inline Combination* Archetype::createCombination(const std::vector<unsigned int>& sharedComponentIndices) {
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
inline void Archetype::destroyCombination(const unsigned int& combinationIndex, const std::vector<unsigned int>& sharedComponentIndices) {
    _combinations[combinationIndex].reset();
    _combinationIndexMap.erase(sharedComponentIndices);
    _freeCombinationIndices.push_back(combinationIndex);
    // TODO: A new Combination should not have a Chunk
    _chunkCount--;
}

}  // namespace MelonCore

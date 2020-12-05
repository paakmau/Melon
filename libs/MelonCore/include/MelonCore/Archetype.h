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
    static constexpr unsigned int k_InvalidId = std::numeric_limits<unsigned int>::max();

    struct EntityLocation {
        static constexpr EntityLocation invalidEntityLocation() { return EntityLocation{Archetype::k_InvalidId, Combination::k_InvalidIndex, Combination::k_InvalidEntityIndex}; }

        bool valid() const { return archetypeId != Archetype::k_InvalidId; }

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

    void filterEntities(EntityFilter const& entityFilter, ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const;

    bool single() const { return m_Mask.single(); }
    bool fullyManual() const { return m_Mask.fullyManual(); }
    bool partiallyManual() const { return m_Mask.partiallyManual(); }

    std::vector<unsigned int> notManualComponentIds() const;
    std::vector<unsigned int> notManualSharedComponentIds() const;

    unsigned int componentCount() const { return m_Mask.componentCount(); }
    unsigned int sharedComponentCount() const { return m_Mask.sharedComponentCount(); }

    unsigned int const& id() const { return m_Id; }
    ArchetypeMask const& mask() const { return m_Mask; }

    std::vector<unsigned int> const& componentIds() const { return m_ComponentIds; }
    std::vector<std::size_t> const& componentSizes() const { return m_ComponentSizes; }
    std::vector<std::size_t> const& componentAligns() const { return m_ComponentAligns; }

    std::vector<unsigned int> const& sharedComponentIds() const { return m_SharedComponentIds; }

    unsigned int const& chunkCount() const { return m_ChunkCount; }
    unsigned int const& entityCount() const { return m_EntityCount; }

  private:
    Combination* createCombination();
    Combination* createCombination(std::vector<unsigned int> const& sharedComponentIndices);
    void destroyCombination(unsigned int const& combinationIndex, std::vector<unsigned int> const& sharedComponentIndices);

    unsigned int const m_Id;
    ArchetypeMask const m_Mask;

    ChunkLayout m_ChunkLayout;
    std::vector<unsigned int> m_ComponentIds;
    std::vector<std::size_t> m_ComponentSizes;
    std::vector<std::size_t> m_ComponentAligns;

    // Shared component ids should be in ascending order
    std::vector<unsigned int> m_SharedComponentIds;

    unsigned int m_ChunkCount{};
    unsigned int m_EntityCount{};

    ObjectPool<Chunk>* m_ChunkPool;
    std::vector<std::unique_ptr<Combination>> m_Combinations;
    std::unordered_map<std::vector<unsigned int>, unsigned int, SharedComponentIndexHash> m_CombinationIndexMap;
    std::vector<unsigned int> m_FreeCombinationIndices;

    friend class EntityManager;
};

inline std::vector<unsigned int> Archetype::notManualComponentIds() const {
    std::vector<unsigned int> componentIds;
    for (unsigned int const& componentId : m_ComponentIds)
        if (!m_Mask.manualComponent(componentId))
            componentIds.push_back(componentId);
    return componentIds;
}

inline std::vector<unsigned int> Archetype::notManualSharedComponentIds() const {
    std::vector<unsigned int> sharedComponentIds;
    for (unsigned int const& sharedComponentId : m_SharedComponentIds)
        if (!m_Mask.manualSharedComponent(sharedComponentId))
            sharedComponentIds.push_back(sharedComponentId);
    return sharedComponentIds;
}

inline Combination* Archetype::createCombination() {
    return createCombination(std::vector<unsigned int>(sharedComponentCount(), ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount>::k_InvalidIndex));
}

inline Combination* Archetype::createCombination(std::vector<unsigned int> const& sharedComponentIndices) {
    if (m_CombinationIndexMap.contains(sharedComponentIndices))
        return m_Combinations[m_CombinationIndexMap[sharedComponentIndices]].get();
    unsigned int combinationIndex;
    if (m_FreeCombinationIndices.empty()) {
        combinationIndex = m_Combinations.size();
        m_Combinations.emplace_back(std::make_unique<Combination>(combinationIndex, m_ChunkLayout, m_SharedComponentIds, sharedComponentIndices, m_ChunkPool));
    } else {
        combinationIndex = m_FreeCombinationIndices.back(), m_FreeCombinationIndices.pop_back();
        m_Combinations[combinationIndex] = std::make_unique<Combination>(combinationIndex, m_ChunkLayout, m_SharedComponentIds, sharedComponentIndices, m_ChunkPool);
    }
    m_CombinationIndexMap.emplace(sharedComponentIndices, combinationIndex);
    // TODO: A new Combination should not have a Chunk
    m_ChunkCount++;
    return m_Combinations[combinationIndex].get();
}

// TODO: Consider when to destroy a Combination
inline void Archetype::destroyCombination(unsigned int const& combinationIndex, std::vector<unsigned int> const& sharedComponentIndices) {
    m_Combinations[combinationIndex].reset();
    m_CombinationIndexMap.erase(sharedComponentIndices);
    m_FreeCombinationIndices.push_back(combinationIndex);
    // TODO: A new Combination should not have a Chunk
    m_ChunkCount--;
}

}  // namespace MelonCore
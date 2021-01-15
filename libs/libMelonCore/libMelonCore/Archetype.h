#pragma once

#include <libMelonCore/ArchetypeMask.h>
#include <libMelonCore/Chunk.h>
#include <libMelonCore/ChunkAccessor.h>
#include <libMelonCore/Combination.h>
#include <libMelonCore/Entity.h>
#include <libMelonCore/EntityFilter.h>
#include <libMelonCore/ObjectPool.h>
#include <libMelonCore/ObjectStore.h>

#include <bitset>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Melon {

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
            for (const unsigned int& sharedComponentIndex : sharedComponentIndices)
                hash ^= (hash << 8) + sharedComponentIndex;
            return hash;
        }
    };

    Archetype(
        const unsigned int& id,
        const ArchetypeMask& mask,
        std::vector<unsigned int> const& componentIds,
        std::vector<std::size_t> const& componentSizes,
        std::vector<std::size_t> const& componentAligns,
        std::vector<unsigned int> const& sharedComponentIds,
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

    void filterEntities(const EntityFilter& entityFilter, ObjectStore<ArchetypeMask::k_MaxSharedComponentIdCount> const& sharedComponentStore, std::vector<ChunkAccessor>& chunkAccessors) const;

    bool single() const { return m_Mask.single(); }
    bool fullyManual() const { return m_Mask.fullyManual(); }
    bool partiallyManual() const { return m_Mask.partiallyManual(); }

    std::vector<unsigned int> notManualComponentIds() const;
    std::vector<unsigned int> notManualSharedComponentIds() const;

    unsigned int componentCount() const { return m_Mask.componentCount(); }
    unsigned int sharedComponentCount() const { return m_Mask.sharedComponentCount(); }

    const unsigned int& id() const { return m_Id; }
    const ArchetypeMask& mask() const { return m_Mask; }

    std::vector<unsigned int> const& componentIds() const { return m_ComponentIds; }
    std::vector<std::size_t> const& componentSizes() const { return m_ComponentSizes; }
    std::vector<std::size_t> const& componentAligns() const { return m_ComponentAligns; }

    std::vector<unsigned int> const& sharedComponentIds() const { return m_SharedComponentIds; }

    const unsigned int& chunkCount() const { return m_ChunkCount; }
    const unsigned int& entityCount() const { return m_EntityCount; }

  private:
    Combination* createCombination();
    Combination* createCombination(std::vector<unsigned int> const& sharedComponentIndices);
    void destroyCombination(const Combination* combination);

    const unsigned int m_Id;
    const ArchetypeMask m_Mask;

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
    for (const unsigned int& componentId : m_ComponentIds)
        if (!m_Mask.manualComponent(componentId))
            componentIds.push_back(componentId);
    return componentIds;
}

inline std::vector<unsigned int> Archetype::notManualSharedComponentIds() const {
    std::vector<unsigned int> sharedComponentIds;
    for (const unsigned int& sharedComponentId : m_SharedComponentIds)
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
    return m_Combinations[combinationIndex].get();
}

inline void Archetype::destroyCombination(const Combination* combination) {
    m_CombinationIndexMap.erase(combination->sharedComponentIndices());
    m_FreeCombinationIndices.push_back(combination->index());
    m_Combinations[combination->index()].reset();
}

}  // namespace Melon

#pragma once

#include <libMelonCore/ArchetypeMask.h>

#include <bitset>
#include <vector>

namespace Melon {

struct EntityFilter {
    bool satisfied(const ArchetypeMask& mask) const;
    // SharedComponents should be in ascending order
    bool satisfied(std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices) const;

    ArchetypeMask::ComponentMask requiredComponentMask;
    ArchetypeMask::ComponentMask rejectedComponentMask;

    ArchetypeMask::SharedComponentMask requiredSharedComponentMask;
    ArchetypeMask::SharedComponentMask rejectedSharedComponentMask;

    // FIXME: It will crash if SharedComponents are destroyed
    // SharedComponents should be in ascending order
    std::vector<std::pair<unsigned int, unsigned int>> requiredSharedComponentIdAndIndices;
    // SharedComponents should be in ascending order
    std::vector<std::pair<unsigned int, unsigned int>> rejectedSharedComponentIdAndIndices;
};

inline bool EntityFilter::satisfied(const ArchetypeMask& mask) const {
    const bool requiredSatisfied = (mask.componentMask | requiredComponentMask) == mask.componentMask && (mask.sharedComponentMask | requiredSharedComponentMask) == mask.sharedComponentMask;
    const bool rejectedSatisfied = (mask.componentMask & rejectedComponentMask).none() && (mask.sharedComponentMask & rejectedSharedComponentMask).none();
    return requiredSatisfied && rejectedSatisfied;
}

inline bool EntityFilter::satisfied(std::vector<unsigned int> const& sharedComponentIds, std::vector<unsigned int> const& sharedComponentIndices) const {
    unsigned int i = 0, j = 0;
    // Check if required SharedComponent indicess satisfied
    while (true) {
        if (i >= requiredSharedComponentIdAndIndices.size()) break;
        while (j < sharedComponentIds.size() && sharedComponentIds[j] < requiredSharedComponentIdAndIndices[i].first) j++;
        if (j >= sharedComponentIds.size()) return false;
        if (sharedComponentIds[j] > requiredSharedComponentIdAndIndices[i].first) return false;
        bool found = false;
        while (requiredSharedComponentIdAndIndices[i].first == sharedComponentIds[j])
            found |= requiredSharedComponentIdAndIndices[i++].second == sharedComponentIndices[j];
        if (!found) return false;
    }
    // Check if rejected SharedComponent indices satisfied
    i = 0, j = 0;
    while (true) {
        if (i >= rejectedSharedComponentIdAndIndices.size()) break;
        while (j < sharedComponentIds.size() && sharedComponentIds[j] < rejectedSharedComponentIdAndIndices[i].first) j++;
        if (j >= sharedComponentIds.size()) return false;
        if (sharedComponentIds[j] > rejectedSharedComponentIdAndIndices[i].first) return false;
        bool found = false;
        while (rejectedSharedComponentIdAndIndices[i].first == sharedComponentIds[j])
            found |= rejectedSharedComponentIdAndIndices[i++].second == sharedComponentIndices[j];
        if (found) return false;
    }
    return true;
}

}  // namespace Melon

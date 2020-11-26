#pragma once

#include <bitset>
#include <cstddef>

namespace MelonCore {

struct ArchetypeMask {
    static constexpr unsigned int kMaxComponentIdCount = 1024U;
    static constexpr unsigned int kMaxSharedComponentIdCount = 256U;

    struct Hash {
        std::size_t operator()(const MelonCore::ArchetypeMask& mask) const {
            return std::hash<std::bitset<MelonCore::ArchetypeMask::kMaxComponentIdCount>>()(mask.componentMask) ^ std::hash<std::bitset<MelonCore::ArchetypeMask::kMaxSharedComponentIdCount>>()(mask.sharedComponentMask);
        }
    };

    bool operator==(const ArchetypeMask& other) const {
        return componentMask == other.componentMask && sharedComponentMask == other.sharedComponentMask;
    }

    unsigned int componentCount() const { return componentMask.count(); }
    unsigned int sharedComponentCount() const { return sharedComponentMask.count(); }

    std::bitset<kMaxComponentIdCount> componentMask;
    std::bitset<kMaxSharedComponentIdCount> sharedComponentMask;
};

struct EntityFilter {
    bool satisfied(const ArchetypeMask& mask) const;

    std::bitset<ArchetypeMask::kMaxComponentIdCount> componentMask;
    std::bitset<ArchetypeMask::kMaxSharedComponentIdCount> sharedComponentMask;
};

inline bool EntityFilter::satisfied(const ArchetypeMask& mask) const {
    return (mask.componentMask | componentMask) == mask.componentMask && (mask.sharedComponentMask | sharedComponentMask) == mask.sharedComponentMask;
}

}  // namespace MelonCore

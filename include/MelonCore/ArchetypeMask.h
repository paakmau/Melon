#pragma once

#include <bitset>
#include <cstddef>
#include <vector>

namespace MelonCore {

struct ArchetypeMask {
    static constexpr unsigned int kMaxComponentIdCount = 1024U;
    static constexpr unsigned int kMaxSharedComponentIdCount = 256U;

    typedef std::bitset<ArchetypeMask::kMaxComponentIdCount> ComponentMask;
    typedef std::bitset<ArchetypeMask::kMaxSharedComponentIdCount> SharedComponentMask;

    struct Hash {
        std::size_t operator()(const MelonCore::ArchetypeMask& mask) const {
            return std::hash<ComponentMask>()(mask.componentMask) ^ std::hash<SharedComponentMask>()(mask.sharedComponentMask);
        }
    };

    ArchetypeMask(const std::vector<unsigned int>& componentIds, const std::vector<unsigned int>& sharedComponentIds) {
        for (unsigned int componentId : componentIds)
            componentMask.set(componentId);
        for (unsigned int sharedComponentId : sharedComponentIds)
            sharedComponentMask.set(sharedComponentId);
    }

    bool operator==(const ArchetypeMask& other) const {
        return componentMask == other.componentMask && sharedComponentMask == other.sharedComponentMask;
    }

    unsigned int componentCount() const { return componentMask.count(); }
    unsigned int sharedComponentCount() const { return sharedComponentMask.count(); }

    ComponentMask componentMask;
    SharedComponentMask sharedComponentMask;
};

}  // namespace MelonCore

#pragma once

#include <bitset>
#include <cstddef>
#include <vector>

namespace MelonCore {

struct ArchetypeMask {
    static constexpr unsigned int kMaxComponentIdCount = 1024U;
    static constexpr unsigned int kMaxSharedComponentIdCount = 256U;

    typedef std::bitset<kMaxComponentIdCount> ComponentMask;
    typedef std::bitset<kMaxComponentIdCount> ManualComponentMask;
    typedef std::bitset<kMaxSharedComponentIdCount> SharedComponentMask;
    typedef std::bitset<kMaxSharedComponentIdCount> ManualSharedComponentMask;

    struct Hash {
        std::size_t operator()(MelonCore::ArchetypeMask const& mask) const {
            return std::hash<ComponentMask>()(mask.componentMask) ^ std::hash<SharedComponentMask>()(mask.sharedComponentMask);
        }
    };

    ArchetypeMask() {}

    void markComponent(unsigned int const& componentId, bool const& manual, bool const& value = true) {
        componentMask.set(componentId, value);
        if (manual)
            manualComponentMask.set(componentId, value);
        if (value)
            count++;
        else
            count--;
    }

    void markComponents(std::vector<unsigned int> const& componentIds, std::vector<bool> const& manuals) {
        for (unsigned int i = 0; i < componentIds.size(); i++)
            markComponent(componentIds[i], manuals[i]);
    }

    void markSharedComponent(unsigned int const& sharedComponentId, bool const& manual, bool const& value = true) {
        sharedComponentMask.set(sharedComponentId, value);
        if (manual)
            manualSharedComponentMask.set(sharedComponentId, value);
        if (value)
            count++;
        else
            count--;
    }

    void markSharedComponents(std::vector<unsigned int> const& sharedComponentIds, std::vector<bool> const& manuals) {
        for (unsigned int i = 0; i < sharedComponentIds.size(); i++)
            markSharedComponent(sharedComponentIds[i], manuals[i]);
    }

    bool operator==(ArchetypeMask const& other) const {
        return componentMask == other.componentMask && sharedComponentMask == other.sharedComponentMask;
    }

    bool manualComponent(unsigned int const& componentId) const { return manualComponentMask.test(componentId); }
    bool manualSharedComponent(unsigned int const& sharedComponentId) const { return manualSharedComponentMask.test(sharedComponentId); }

    bool none() const { return count == 0; }

    bool single() const { return count == 1; }
    bool fullyManual() const { return !none() && componentMask == manualComponentMask && sharedComponentMask == manualSharedComponentMask; }
    bool partiallyManual() const { return !fullyManual() && (manualComponentMask.any() || manualSharedComponentMask.any()); }

    unsigned int componentCount() const { return componentMask.count(); }
    unsigned int sharedComponentCount() const { return sharedComponentMask.count(); }

    unsigned int count{};
    ComponentMask componentMask;
    ManualComponentMask manualComponentMask;
    SharedComponentMask sharedComponentMask;
    ManualSharedComponentMask manualSharedComponentMask;
};

}  // namespace MelonCore

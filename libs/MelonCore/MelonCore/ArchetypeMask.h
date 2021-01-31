#pragma once

#include <bitset>
#include <cstddef>
#include <vector>

namespace Melon {

struct ArchetypeMask {
    static constexpr unsigned int k_MaxComponentIdCount = 1024U;
    static constexpr unsigned int k_MaxSharedComponentIdCount = 256U;

    using ComponentMask = std::bitset<k_MaxComponentIdCount>;
    using ManualComponentMask = std::bitset<k_MaxComponentIdCount>;
    using SharedComponentMask = std::bitset<k_MaxSharedComponentIdCount>;
    using ManualSharedComponentMask = std::bitset<k_MaxSharedComponentIdCount>;

    struct Hash {
        std::size_t operator()(const ArchetypeMask& mask) const {
            return std::hash<ComponentMask>()(mask.componentMask) ^ std::hash<SharedComponentMask>()(mask.sharedComponentMask);
        }
    };

    ArchetypeMask() {}

    void markComponent(const unsigned int& componentId, const bool& manual, const bool& value = true) {
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

    void markSharedComponent(const unsigned int& sharedComponentId, const bool& manual, const bool& value = true) {
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

    bool operator==(const ArchetypeMask& other) const {
        return componentMask == other.componentMask && sharedComponentMask == other.sharedComponentMask;
    }

    bool manualComponent(const unsigned int& componentId) const { return manualComponentMask.test(componentId); }
    bool manualSharedComponent(const unsigned int& sharedComponentId) const { return manualSharedComponentMask.test(sharedComponentId); }

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

}  // namespace Melon

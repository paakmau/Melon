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
        std::size_t operator()(const MelonCore::ArchetypeMask& mask) const {
            return std::hash<ComponentMask>()(mask.componentMask) ^ std::hash<SharedComponentMask>()(mask.sharedComponentMask);
        }
    };

    ArchetypeMask() {}

    void markComponent(const unsigned int& componentId, const bool& manual, const bool& value = true) {
        componentMask.set(componentId, value);
        if (manual)
            manualComponentMask.set(componentId, value);
    }

    void markComponents(const std::vector<unsigned int>& componentIds, const std::vector<bool>& manuals) {
        for (unsigned int i = 0; i < componentIds.size(); i++)
            markComponent(componentIds[i], manuals[i]);
    }

    void markSharedComponent(const unsigned int& sharedComponentId, const bool& manual, const bool& value = true) {
        sharedComponentMask.set(sharedComponentId, value);
        if (manual)
            manualSharedComponentMask.set(sharedComponentId, value);
    }

    void markSharedComponents(const std::vector<unsigned int>& sharedComponentIds, const std::vector<bool>& manuals) {
        for (unsigned int i = 0; i < sharedComponentIds.size(); i++)
            markSharedComponent(sharedComponentIds[i], manuals[i]);
    }

    bool operator==(const ArchetypeMask& other) const {
        return componentMask == other.componentMask && sharedComponentMask == other.sharedComponentMask;
    }

    bool manualComponent(const unsigned int& componentId) const { return manualComponentMask.test(componentId); }
    bool manualSharedComponent(const unsigned int& sharedComponentId) const { return manualSharedComponentMask.test(sharedComponentId); }

    bool none() const { return componentMask.none() && sharedComponentMask.none(); }

    bool singleAndManual() const { return componentCount() + sharedComponentCount() == 1 && fullyManual(); }
    bool fullyManual() const { return !none() && componentMask == manualComponentMask && sharedComponentMask == manualSharedComponentMask; }
    bool paritiallyManual() const { return !fullyManual() && (componentMask.any() || sharedComponentMask.any()); }

    unsigned int componentCount() const { return componentMask.count(); }
    unsigned int sharedComponentCount() const { return sharedComponentMask.count(); }

    ComponentMask componentMask;
    ManualComponentMask manualComponentMask;
    SharedComponentMask sharedComponentMask;
    ManualSharedComponentMask manualSharedComponentMask;
};

}  // namespace MelonCore

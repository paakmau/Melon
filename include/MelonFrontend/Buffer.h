#pragma once

#include <MelonFrontend/VulkanPlatform.h>

#include <cstddef>
#include <functional>

namespace MelonFrontend {

struct Buffer {
    bool operator==(const Buffer& other) const {
        return buffer == other.buffer && allocation == other.allocation;
    }

    VkBuffer buffer;
    VmaAllocation allocation;
};

}  // namespace MelonFrontend

template <>
struct std::hash<MelonFrontend::Buffer> {
    std::size_t operator()(const MelonFrontend::Buffer& buffer) {
        return std::hash<void*>()(static_cast<void*>(buffer.buffer)) ^ std::hash<void*>()(static_cast<void*>(buffer.allocation));
    }
};

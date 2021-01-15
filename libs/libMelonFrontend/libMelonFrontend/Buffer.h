#pragma once

#include <libMelonFrontend/VulkanPlatform.h>

#include <cstddef>
#include <functional>

namespace Melon {

struct Buffer {
    bool operator==(const Buffer& other) const {
        return buffer == other.buffer && allocation == other.allocation;
    }

    VkBuffer buffer;
    VmaAllocation allocation;
};

}  // namespace Melon

template <>
struct std::hash<Melon::Buffer> {
    std::size_t operator()(const Melon::Buffer& buffer) {
        return std::hash<void*>()(static_cast<void*>(buffer.buffer)) ^ std::hash<void*>()(static_cast<void*>(buffer.allocation));
    }
};

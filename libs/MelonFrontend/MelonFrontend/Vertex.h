#pragma once

#include <MelonFrontend/VulkanPlatform.h>

#include <array>
#include <cstddef>
#include <glm/vec3.hpp>

namespace Melon {

struct Vertex {
    static constexpr unsigned int k_AttributeCount = 2;

    static std::array<VkFormat, k_AttributeCount> formats() {
        return std::array<VkFormat, k_AttributeCount>{
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32B32_SFLOAT};
    }

    static std::array<uint32_t, k_AttributeCount> offsets() {
        return std::array<uint32_t, k_AttributeCount>{
            offsetof(Vertex, position),
            offsetof(Vertex, normal)};
    }

    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal;
    }

    glm::vec3 position;
    glm::vec3 normal;
};

}  // namespace Melon

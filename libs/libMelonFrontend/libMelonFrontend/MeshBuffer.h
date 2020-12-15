#pragma once

#include <libMelonFrontend/Buffer.h>
#include <libMelonFrontend/VulkanPlatform.h>

#include <array>
#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

namespace MelonFrontend {

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

    bool operator==(Vertex const& other) const {
        return position == other.position && normal == other.normal;
    }

    glm::vec3 position;
    glm::vec3 normal;
};

struct MeshBuffer {
    bool operator==(MeshBuffer const& other) const {
        return vertexBuffer == other.vertexBuffer && indexBuffer == other.indexBuffer;
    }

    Buffer vertexBuffer;
    Buffer indexBuffer;
    uint32_t vertexCount;
    uint32_t indexCount;
};

}  // namespace MelonFrontend

template <>
struct std::hash<MelonFrontend::Vertex> {
    std::size_t operator()(MelonFrontend::Vertex const& vertex) {
        return std::hash<glm::vec3>()(vertex.position) ^ std::hash<glm::vec3>()(vertex.normal);
    }
};

template <>
struct std::hash<MelonFrontend::MeshBuffer> {
    std::size_t operator()(MelonFrontend::MeshBuffer const& meshBuffer) {
        return std::hash<MelonFrontend::Buffer>()(meshBuffer.vertexBuffer) ^ std::hash<MelonFrontend::Buffer>()(meshBuffer.indexBuffer);
    }
};

#pragma once

#include <MelonFrontend/Buffer.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>

namespace MelonFrontend {

struct Vertex {
    static constexpr unsigned int kAttributeCount = 2;

    static std::array<VkFormat, kAttributeCount> formats() {
        return std::array<VkFormat, kAttributeCount>{
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R32G32B32_SFLOAT};
    }

    static std::array<uint32_t, kAttributeCount> offsets() {
        return std::array<uint32_t, kAttributeCount>{
            offsetof(Vertex, position),
            offsetof(Vertex, normal)};
    }

    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal;
    }

    glm::vec3 position;
    glm::vec3 normal;
};

struct MeshBuffer {
    bool operator==(const MeshBuffer& other) const {
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
    std::size_t operator()(const MelonFrontend::Vertex& vertex) {
        return std::hash<glm::vec3>()(vertex.position) ^ std::hash<glm::vec3>()(vertex.normal);
    }
};

template <>
struct std::hash<MelonFrontend::MeshBuffer> {
    std::size_t operator()(const MelonFrontend::MeshBuffer& meshBuffer) {
        return std::hash<MelonFrontend::Buffer>()(meshBuffer.vertexBuffer) ^ std::hash<MelonFrontend::Buffer>()(meshBuffer.indexBuffer);
    }
};

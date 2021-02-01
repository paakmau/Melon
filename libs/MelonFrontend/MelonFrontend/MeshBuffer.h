#pragma once

#include <MelonFrontend/Buffer.h>
#include <MelonFrontend/Vertex.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

namespace Melon {

struct MeshBuffer {
    bool operator==(const MeshBuffer& other) const {
        return vertexBuffer == other.vertexBuffer && indexBuffer == other.indexBuffer;
    }

    Buffer vertexBuffer;
    Buffer indexBuffer;
    uint32_t vertexCount;
    uint32_t indexCount;
};

}  // namespace Melon

template <>
struct std::hash<Melon::Vertex> {
    std::size_t operator()(const Melon::Vertex& vertex) {
        return std::hash<glm::vec3>()(vertex.position) ^ std::hash<glm::vec3>()(vertex.normal);
    }
};

template <>
struct std::hash<Melon::MeshBuffer> {
    std::size_t operator()(const Melon::MeshBuffer& meshBuffer) {
        return std::hash<Melon::Buffer>()(meshBuffer.vertexBuffer) ^ std::hash<Melon::Buffer>()(meshBuffer.indexBuffer);
    }
};

#pragma once

#include <MelonFrontend/Buffer.h>
#include <MelonFrontend/VulkanPlatform.h>

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
struct std::hash<Melon::MeshBuffer> {
    std::size_t operator()(const Melon::MeshBuffer& meshBuffer) {
        return std::hash<Melon::Buffer>()(meshBuffer.vertexBuffer) ^ std::hash<Melon::Buffer>()(meshBuffer.indexBuffer);
    }
};

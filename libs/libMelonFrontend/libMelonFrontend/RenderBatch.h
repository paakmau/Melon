#pragma once

#include <libMelonFrontend/MeshBuffer.h>
#include <libMelonFrontend/UniformBuffer.h>
#include <libMelonFrontend/VulkanPlatform.h>

#include <vector>

namespace Melon {

struct RenderBatch {
    const Buffer& vertexBuffer() const { return meshBuffer.vertexBuffer; }
    const Buffer& indexBuffer() const { return meshBuffer.indexBuffer; }
    const uint32_t& vertexCount() const { return meshBuffer.vertexCount; }
    const uint32_t& indexCount() const { return meshBuffer.indexCount; }

    std::vector<UniformBuffer> entityUniformMemories;
    MeshBuffer meshBuffer;
};

}  // namespace Melon

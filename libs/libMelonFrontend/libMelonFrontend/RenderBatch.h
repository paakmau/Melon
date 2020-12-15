#pragma once

#include <libMelonFrontend/MeshBuffer.h>
#include <libMelonFrontend/UniformBuffer.h>
#include <libMelonFrontend/VulkanPlatform.h>

#include <vector>

namespace MelonFrontend {

struct RenderBatch {
    Buffer const& vertexBuffer() const { return meshBuffer.vertexBuffer; }
    Buffer const& indexBuffer() const { return meshBuffer.indexBuffer; }
    uint32_t const& vertexCount() const { return meshBuffer.vertexCount; }
    uint32_t const& indexCount() const { return meshBuffer.indexCount; }

    std::vector<UniformBuffer> entityUniformMemories;
    MeshBuffer meshBuffer;
};

}  // namespace MelonFrontend

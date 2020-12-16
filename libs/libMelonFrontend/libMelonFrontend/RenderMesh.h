#pragma once

#include <libMelonCore/SharedComponent.h>
#include <libMelonFrontend/MeshBuffer.h>

#include <cstddef>
#include <functional>
#include <glm/vec3.hpp>
#include <vector>

namespace Melon {

struct RenderMesh : public SharedComponent {
    bool operator==(RenderMesh const& other) const {
        return vertices == other.vertices && indices == other.indices;
    }

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

struct ManualRenderMesh : public ManualSharedComponent {
    bool operator==(ManualRenderMesh const& other) const {
        return meshBuffer == other.meshBuffer;
    }

    unsigned int renderMeshIndex;

    MeshBuffer meshBuffer;
};

}  // namespace Melon

template <>
struct std::hash<Melon::RenderMesh> {
    std::size_t operator()(Melon::RenderMesh const& renderMesh) {
        std::size_t hash{};
        for (Melon::Vertex const& vertex : renderMesh.vertices)
            hash ^= std::hash<Melon::Vertex>()(vertex);
        for (unsigned int const& index : renderMesh.indices)
            hash ^= std::hash<unsigned int>()(index);
        return hash;
    }
};

template <>
struct std::hash<Melon::ManualRenderMesh> {
    std::size_t operator()(Melon::ManualRenderMesh const& manualRenderMesh) {
        return std::hash<Melon::MeshBuffer>()(manualRenderMesh.meshBuffer);
    }
};

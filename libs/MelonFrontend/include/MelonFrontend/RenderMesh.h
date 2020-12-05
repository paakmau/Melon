#pragma once

#include <MelonCore/SharedComponent.h>
#include <MelonFrontend/MeshBuffer.h>

#include <cstddef>
#include <functional>
#include <glm/vec3.hpp>
#include <vector>

namespace MelonFrontend {

struct RenderMesh : public MelonCore::SharedComponent {
    bool operator==(RenderMesh const& other) const {
        return vertices == other.vertices && indices == other.indices;
    }

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

struct ManualRenderMesh : public MelonCore::ManualSharedComponent {
    bool operator==(ManualRenderMesh const& other) const {
        return meshBuffer == other.meshBuffer;
    }

    unsigned int renderMeshIndex;

    MeshBuffer meshBuffer;
};

}  // namespace MelonFrontend

template <>
struct std::hash<MelonFrontend::RenderMesh> {
    std::size_t operator()(MelonFrontend::RenderMesh const& renderMesh) {
        std::size_t hash{};
        for (MelonFrontend::Vertex const& vertex : renderMesh.vertices)
            hash ^= std::hash<MelonFrontend::Vertex>()(vertex);
        for (unsigned int const& index : renderMesh.indices)
            hash ^= std::hash<unsigned int>()(index);
        return hash;
    }
};

template <>
struct std::hash<MelonFrontend::ManualRenderMesh> {
    std::size_t operator()(MelonFrontend::ManualRenderMesh const& manualRenderMesh) {
        return std::hash<MelonFrontend::MeshBuffer>()(manualRenderMesh.meshBuffer);
    }
};

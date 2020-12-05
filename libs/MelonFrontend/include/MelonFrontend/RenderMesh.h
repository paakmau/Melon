#pragma once

#include <MelonCore/SharedComponent.h>
#include <MelonFrontend/MeshBuffer.h>

#include <cstddef>
#include <functional>
#include <glm/vec3.hpp>
#include <vector>

namespace MelonFrontend {

struct RenderMesh : public MelonCore::SharedComponent {
    bool operator==(const RenderMesh& other) const {
        return vertices == other.vertices && indices == other.indices;
    }

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

struct ManualRenderMesh : public MelonCore::ManualSharedComponent {
    bool operator==(const ManualRenderMesh& other) const {
        return meshBuffer == other.meshBuffer;
    }

    unsigned int renderMeshIndex;

    MeshBuffer meshBuffer;
};

}  // namespace MelonFrontend

template <>
struct std::hash<MelonFrontend::RenderMesh> {
    std::size_t operator()(const MelonFrontend::RenderMesh& renderMesh) {
        std::size_t hash{};
        for (const MelonFrontend::Vertex& vertex : renderMesh.vertices)
            hash ^= std::hash<MelonFrontend::Vertex>()(vertex);
        for (const unsigned int& index : renderMesh.indices)
            hash ^= std::hash<unsigned int>()(index);
        return hash;
    }
};

template <>
struct std::hash<MelonFrontend::ManualRenderMesh> {
    std::size_t operator()(const MelonFrontend::ManualRenderMesh& manualRenderMesh) {
        return std::hash<MelonFrontend::MeshBuffer>()(manualRenderMesh.meshBuffer);
    }
};

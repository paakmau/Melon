#pragma once

#include <MelonCore/SharedComponent.h>
#include <MelonFrontend/MeshBuffer.h>

#include <cstddef>
#include <functional>
#include <glm/vec3.hpp>
#include <vector>

namespace Melon {

struct RenderMesh : public SharedComponent {
    bool operator==(const RenderMesh& other) const {
        return vertices == other.vertices && indices == other.indices;
    }

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

struct ManualRenderMesh : public ManualSharedComponent {
    bool operator==(const ManualRenderMesh& other) const {
        return meshBuffer == other.meshBuffer;
    }

    unsigned int renderMeshIndex;

    MeshBuffer meshBuffer;
};

}  // namespace Melon

template <>
struct std::hash<Melon::RenderMesh> {
    std::size_t operator()(const Melon::RenderMesh& renderMesh) {
        std::size_t hash{};
        for (const Melon::Vertex& vertex : renderMesh.vertices)
            hash ^= std::hash<Melon::Vertex>()(vertex);
        for (const unsigned int& index : renderMesh.indices)
            hash ^= std::hash<unsigned int>()(index);
        return hash;
    }
};

template <>
struct std::hash<Melon::ManualRenderMesh> {
    std::size_t operator()(const Melon::ManualRenderMesh& manualRenderMesh) {
        return std::hash<Melon::MeshBuffer>()(manualRenderMesh.meshBuffer);
    }
};

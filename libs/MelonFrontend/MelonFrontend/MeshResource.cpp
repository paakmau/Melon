#include <MelonFrontend/MeshResource.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>

namespace Melon {

std::unique_ptr<MeshResource> MeshResource::create(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);

    if (!scene)
        return nullptr;

    unsigned int vertexCount = 0;
    unsigned int indexCount = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        vertexCount += scene->mMeshes[i]->mNumVertices;
        indexCount += scene->mMeshes[i]->mNumFaces * 3;
    }

    std::vector<Vertex> vertices(vertexCount);
    std::vector<uint16_t> indices(indexCount);

    unsigned int firstVertexIndex = 0;
    unsigned int firstIndexIndex = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const auto& mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            const auto& vertex = mesh->mVertices[j];
            const auto& normal = mesh->mNormals[j];
            vertices[firstVertexIndex + j] = Vertex{
                .position = {vertex.x, vertex.y, vertex.z},
                .normal = {normal.x, normal.y, normal.z}};
        }
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
            for (unsigned int k = 0; k < 3; k++)
                indices[firstIndexIndex + j * 3 + k] = mesh->mFaces[j].mIndices[k];

        firstVertexIndex += mesh->mNumVertices;
        firstIndexIndex += mesh->mNumFaces * 3;
    }

    return std::make_unique<MeshResource>(path, std::move(vertices), std::move(indices));
}

}  // namespace Melon

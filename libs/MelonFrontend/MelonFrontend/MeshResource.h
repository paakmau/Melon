#pragma once

#include <MelonCore/Resource.h>
#include <MelonFrontend/Vertex.h>

#include <memory>
#include <string>

namespace Melon {

class MeshResource : public Resource {
  public:
    static std::unique_ptr<MeshResource> create(const std::string& path);

    MeshResource(const std::string& path, std::vector<Vertex>&& vertices, std::vector<uint16_t>&& indices) : Resource(path), m_Vertices(vertices), m_Indices(indices) {}
    virtual ~MeshResource() {}

    const std::vector<Vertex>& vertices() const { return m_Vertices; }
    const std::vector<uint16_t>& indices() const { return m_Indices; }

  private:
    std::vector<Vertex> m_Vertices;
    std::vector<uint16_t> m_Indices;
};

}  // namespace Melon

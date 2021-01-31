#pragma once

#include <MelonCore/Resource.h>

#include <unordered_map>
#include <memory>

namespace Melon {

class ResourceManager {
  public:
    void addResource(std::unique_ptr<Resource>&& resource);
    void removeResource(const std::string& path);
    Resource* resource(const std::string& path);

  private:
    std::unordered_map<std::string, std::unique_ptr<Resource>> m_ResourceMap;
};

}  // namespace Melon

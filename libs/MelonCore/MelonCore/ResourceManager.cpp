#include <MelonCore/ResourceManager.h>

namespace Melon {

void ResourceManager::addResource(std::unique_ptr<Resource>&& resource) {
    m_ResourceMap.try_emplace(resource->path(), std::move(resource));
}

void ResourceManager::removeResource(const std::string& path) {
    m_ResourceMap.erase(path);
}

Resource* ResourceManager::resource(const std::string& path) {
    return m_ResourceMap.at(path).get();
}

}  // namespace Melon

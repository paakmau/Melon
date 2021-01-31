#pragma once

#include <string>

namespace Melon {

class Resource {
  public:
    Resource(const std::string& path) : m_Path(path) {}
    virtual ~Resource(){};

    const std::string& path() const { return m_Path; }

  protected:
    std::string m_Path;
};

}  // namespace Melon

#pragma once

#include <MelonCore/DataComponent.h>

#include <glm/vec3.hpp>

namespace Melon {

struct Scale : public DataComponent {
    glm::vec3 value;
};

}  // namespace Melon

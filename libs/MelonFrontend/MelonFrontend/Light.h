#pragma once

#include <MelonCore/DataComponent.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Melon {

struct Light : public DataComponent {
    glm::vec3 direction;
};

}  // namespace Melon

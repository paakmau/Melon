#pragma once

#include <libMelonCore/Component.h>

#include <glm/vec3.hpp>

namespace Melon {

struct Scale : public Component {
    glm::vec3 value;
};

}  // namespace Melon

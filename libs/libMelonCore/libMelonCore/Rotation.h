#pragma once

#include <libMelonCore/Component.h>

#include <glm/gtc/quaternion.hpp>

namespace Melon {

struct Rotation : public Component {
    glm::quat value;
};

}  // namespace Melon

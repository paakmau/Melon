#pragma once

#include <libMelonCore/Component.h>

#include <glm/gtc/quaternion.hpp>

namespace MelonCore {

struct Rotation : public Component {
    glm::quat value;
};

}  // namespace MelonCore

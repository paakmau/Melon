#pragma once

#include <MelonCore/DataComponent.h>

#include <glm/gtc/quaternion.hpp>

namespace Melon {

struct Rotation : public DataComponent {
    glm::quat value;
};

}  // namespace Melon

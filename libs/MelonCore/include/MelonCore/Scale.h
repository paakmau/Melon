#pragma once

#include <MelonCore/Component.h>

#include <glm/vec3.hpp>

namespace MelonCore {

struct Scale : public Component {
    glm::vec3 value;
};

}  // namespace MelonCore

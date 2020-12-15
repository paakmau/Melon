#pragma once

#include <libMelonCore/Component.h>

#include <glm/vec3.hpp>

namespace MelonCore {

struct Translation : public Component {
    glm::vec3 value;
};

}  // namespace MelonCore

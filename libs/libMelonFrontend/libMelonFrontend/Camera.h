#pragma once

#include <glm/mat4x4.hpp>

namespace Melon {

struct Camera : public Component {
};

struct PerspectiveProjection : public Component {
    float fovy;
    float zNear;
    float zFar;
};

}  // namespace Melon

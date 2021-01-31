#pragma once

#include <glm/mat4x4.hpp>

namespace Melon {

struct Camera : public DataComponent {
};

struct PerspectiveProjection : public DataComponent {
    float fovy;
    float zNear;
    float zFar;
};

}  // namespace Melon

#include <cstdint>
#include <vector>

constexpr const char* k_VertexShader =
    "#version 450\n"
    "\n"
    "    layout(set = 0, binding = 0) uniform CameraUniformObject {\n"
    "    mat4 vp;\n"
    "}\n"
    "cameraUniformObject;\n"
    "\n"
    "layout(set = 1, binding = 0) uniform EntityUniformObject {\n"
    "    mat4 model;\n"
    "}\n"
    "entityUniformObbject;\n"
    "\n"
    "layout(location = 0) in vec3 position;\n"
    "\n"
    "layout(location = 0) out vec3 fragColor;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = cameraUniformObject.vp * entityUniformObbject.model * vec4(position, 1.0);\n"
    "    fragColor = vec3(1.0, 1.0, 1.0);\n"
    "}\n";

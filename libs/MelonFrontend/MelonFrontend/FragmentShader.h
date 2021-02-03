#include <cstdint>
#include <vector>

constexpr const char* k_FragmentShader =
    "#version 450\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "\n"
    "layout(set = 2, binding = 0) uniform LightUniformObject {\n"
    "    vec3 direction;\n"
    "}\n"
    "lightUniformObject;\n"
    "\n"
    "layout(location = 0) in vec3 fragColor;\n"
    "layout(location = 1) in vec3 normal;\n"
    "\n"
    "layout(location = 0) out vec4 outColor;\n"
    "\n"
    "void main() {\n"
    "    float diffuse = max(0, -dot(normal, lightUniformObject.direction));\n"
    "    outColor = vec4(fragColor * diffuse, 1.0);\n"
    "    outColor += vec4(0.1, 0.1, 0.1, 0.0);"
    "}\n";

#include <cstdint>
#include <vector>

constexpr const char* k_FragmentShader =
    "#version 450\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "\n"
    "layout(location = 0) in vec3 fragColor;\n"
    "\n"
    "layout(location = 0) out vec4 outColor;\n"
    "\n"
    "void main() {\n"
    "    outColor = vec4(fragColor, 1.0);\n"
    "}\n";

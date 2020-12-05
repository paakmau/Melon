#include <vector>

/*

GLSL code

#version 450

layout(set = 0, binding = 0) uniform CameraUniformObject {
    mat4 vp;
}cameraUniformObject;

layout(set = 1, binding = 0) uniform EntityUniformObject {
    mat4 model;
}entityUniformObbject;

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = cameraUniformObject.vp * entityUniformObbject.model * vec4(position, 1.0);
    fragColor = vec3(1.0, 1.0, 1.0);
}

*/

inline std::vector<uint32_t> k_VertexShader{
    0x07230203, 0x00010000, 0x000d000a, 0x0000002c,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0008000f, 0x00000000, 0x00000004, 0x6e69616d,
    0x00000000, 0x0000000d, 0x0000001f, 0x0000002a,
    0x00030003, 0x00000002, 0x000001c2, 0x000a0004,
    0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70,
    0x5f656c79, 0x656e696c, 0x7269645f, 0x69746365,
    0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f,
    0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572,
    0x00657669, 0x00040005, 0x00000004, 0x6e69616d,
    0x00000000, 0x00060005, 0x0000000b, 0x505f6c67,
    0x65567265, 0x78657472, 0x00000000, 0x00060006,
    0x0000000b, 0x00000000, 0x505f6c67, 0x7469736f,
    0x006e6f69, 0x00070006, 0x0000000b, 0x00000001,
    0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000,
    0x00070006, 0x0000000b, 0x00000002, 0x435f6c67,
    0x4470696c, 0x61747369, 0x0065636e, 0x00070006,
    0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75,
    0x61747369, 0x0065636e, 0x00030005, 0x0000000d,
    0x00000000, 0x00070005, 0x00000011, 0x656d6143,
    0x6e556172, 0x726f6669, 0x6a624f6d, 0x00746365,
    0x00040006, 0x00000011, 0x00000000, 0x00007076,
    0x00070005, 0x00000013, 0x656d6163, 0x6e556172,
    0x726f6669, 0x6a624f6d, 0x00746365, 0x00070005,
    0x00000017, 0x69746e45, 0x6e557974, 0x726f6669,
    0x6a624f6d, 0x00746365, 0x00050006, 0x00000017,
    0x00000000, 0x65646f6d, 0x0000006c, 0x00080005,
    0x00000019, 0x69746e65, 0x6e557974, 0x726f6669,
    0x62624f6d, 0x7463656a, 0x00000000, 0x00050005,
    0x0000001f, 0x69736f70, 0x6e6f6974, 0x00000000,
    0x00050005, 0x0000002a, 0x67617266, 0x6f6c6f43,
    0x00000072, 0x00050048, 0x0000000b, 0x00000000,
    0x0000000b, 0x00000000, 0x00050048, 0x0000000b,
    0x00000001, 0x0000000b, 0x00000001, 0x00050048,
    0x0000000b, 0x00000002, 0x0000000b, 0x00000003,
    0x00050048, 0x0000000b, 0x00000003, 0x0000000b,
    0x00000004, 0x00030047, 0x0000000b, 0x00000002,
    0x00040048, 0x00000011, 0x00000000, 0x00000005,
    0x00050048, 0x00000011, 0x00000000, 0x00000023,
    0x00000000, 0x00050048, 0x00000011, 0x00000000,
    0x00000007, 0x00000010, 0x00030047, 0x00000011,
    0x00000002, 0x00040047, 0x00000013, 0x00000022,
    0x00000000, 0x00040047, 0x00000013, 0x00000021,
    0x00000000, 0x00040048, 0x00000017, 0x00000000,
    0x00000005, 0x00050048, 0x00000017, 0x00000000,
    0x00000023, 0x00000000, 0x00050048, 0x00000017,
    0x00000000, 0x00000007, 0x00000010, 0x00030047,
    0x00000017, 0x00000002, 0x00040047, 0x00000019,
    0x00000022, 0x00000001, 0x00040047, 0x00000019,
    0x00000021, 0x00000000, 0x00040047, 0x0000001f,
    0x0000001e, 0x00000000, 0x00040047, 0x0000002a,
    0x0000001e, 0x00000000, 0x00020013, 0x00000002,
    0x00030021, 0x00000003, 0x00000002, 0x00030016,
    0x00000006, 0x00000020, 0x00040017, 0x00000007,
    0x00000006, 0x00000004, 0x00040015, 0x00000008,
    0x00000020, 0x00000000, 0x0004002b, 0x00000008,
    0x00000009, 0x00000001, 0x0004001c, 0x0000000a,
    0x00000006, 0x00000009, 0x0006001e, 0x0000000b,
    0x00000007, 0x00000006, 0x0000000a, 0x0000000a,
    0x00040020, 0x0000000c, 0x00000003, 0x0000000b,
    0x0004003b, 0x0000000c, 0x0000000d, 0x00000003,
    0x00040015, 0x0000000e, 0x00000020, 0x00000001,
    0x0004002b, 0x0000000e, 0x0000000f, 0x00000000,
    0x00040018, 0x00000010, 0x00000007, 0x00000004,
    0x0003001e, 0x00000011, 0x00000010, 0x00040020,
    0x00000012, 0x00000002, 0x00000011, 0x0004003b,
    0x00000012, 0x00000013, 0x00000002, 0x00040020,
    0x00000014, 0x00000002, 0x00000010, 0x0003001e,
    0x00000017, 0x00000010, 0x00040020, 0x00000018,
    0x00000002, 0x00000017, 0x0004003b, 0x00000018,
    0x00000019, 0x00000002, 0x00040017, 0x0000001d,
    0x00000006, 0x00000003, 0x00040020, 0x0000001e,
    0x00000001, 0x0000001d, 0x0004003b, 0x0000001e,
    0x0000001f, 0x00000001, 0x0004002b, 0x00000006,
    0x00000021, 0x3f800000, 0x00040020, 0x00000027,
    0x00000003, 0x00000007, 0x00040020, 0x00000029,
    0x00000003, 0x0000001d, 0x0004003b, 0x00000029,
    0x0000002a, 0x00000003, 0x0006002c, 0x0000001d,
    0x0000002b, 0x00000021, 0x00000021, 0x00000021,
    0x00050036, 0x00000002, 0x00000004, 0x00000000,
    0x00000003, 0x000200f8, 0x00000005, 0x00050041,
    0x00000014, 0x00000015, 0x00000013, 0x0000000f,
    0x0004003d, 0x00000010, 0x00000016, 0x00000015,
    0x00050041, 0x00000014, 0x0000001a, 0x00000019,
    0x0000000f, 0x0004003d, 0x00000010, 0x0000001b,
    0x0000001a, 0x00050092, 0x00000010, 0x0000001c,
    0x00000016, 0x0000001b, 0x0004003d, 0x0000001d,
    0x00000020, 0x0000001f, 0x00050051, 0x00000006,
    0x00000022, 0x00000020, 0x00000000, 0x00050051,
    0x00000006, 0x00000023, 0x00000020, 0x00000001,
    0x00050051, 0x00000006, 0x00000024, 0x00000020,
    0x00000002, 0x00070050, 0x00000007, 0x00000025,
    0x00000022, 0x00000023, 0x00000024, 0x00000021,
    0x00050091, 0x00000007, 0x00000026, 0x0000001c,
    0x00000025, 0x00050041, 0x00000027, 0x00000028,
    0x0000000d, 0x0000000f, 0x0003003e, 0x00000028,
    0x00000026, 0x0003003e, 0x0000002a, 0x0000002b,
    0x000100fd, 0x00010038};

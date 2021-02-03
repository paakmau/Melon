#pragma once

#include <MelonFrontend/Buffer.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace Melon {

struct CameraUniformObject {
    glm::mat4 vp;
};

struct EntityUniformObject {
    glm::mat4 model;
};

struct LightUniformObject {
    glm::vec3 direction;
};

struct UniformBuffer {
    VkDeviceSize size;
    Buffer stagingBuffer;
    Buffer buffer;
    VkDescriptorSet descriptorSet;
};

}  // namespace Melon

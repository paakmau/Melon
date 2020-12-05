#pragma once

#include <MelonFrontend/Buffer.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <glm/mat4x4.hpp>

namespace MelonFrontend {

struct CameraUniformObject {
    glm::mat4 vp;
};

struct EntityUniformObject {
    glm::mat4 model;
};

struct UniformBuffer {
    VkDeviceSize size;
    Buffer stagingBuffer;
    Buffer buffer;
    VkDescriptorSet descriptorSet;
};

}  // namespace MelonFrontend

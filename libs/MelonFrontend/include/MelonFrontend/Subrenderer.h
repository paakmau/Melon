#pragma once

#include <MelonFrontend/RenderBatch.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <glm/mat4x4.hpp>
#include <vector>

namespace MelonFrontend {

class Subrenderer {
   public:
    void initialize(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout entityDescriptorSetLayout, VkRenderPass renderPass, const unsigned int& swapChainImageCount);
    void terminate();

    void draw(VkCommandBuffer commandBuffer, const unsigned int& swapChainImageIndex, VkDescriptorSet cameraDescriptorSet, const RenderBatch& renderBatch);

   protected:
    VkDevice _device;
    VkExtent2D _swapChainExtent;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;
};

}  // namespace MelonFrontend

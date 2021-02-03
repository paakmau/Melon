#pragma once

#include <MelonFrontend/RenderBatch.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <glm/mat4x4.hpp>
#include <vector>

namespace Melon {

class Subrenderer {
  public:
    void initialize(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout entityDescriptorSetLayout, VkDescriptorSetLayout lightDescriptorSetLayout, VkRenderPass renderPass, const unsigned int& swapChainImageCount);
    void terminate();

    void draw(VkCommandBuffer commandBuffer, const unsigned int& swapChainImageIndex, VkDescriptorSet cameraDescriptorSet, VkDescriptorSet lightDescriptorSet, const RenderBatch& renderBatch);

  protected:
    VkDevice m_Device;
    VkExtent2D m_SwapChainExtent;
    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_Pipeline;
};

}  // namespace Melon

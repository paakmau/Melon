#pragma once

#include <MelonFrontend/RenderBatch.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <glm/mat4x4.hpp>
#include <vector>

namespace MelonFrontend {

class Subrenderer {
  public:
    void initialize(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout entityDescriptorSetLayout, VkRenderPass renderPass, unsigned int const& swapChainImageCount);
    void terminate();

    void draw(VkCommandBuffer commandBuffer, unsigned int const& swapChainImageIndex, VkDescriptorSet cameraDescriptorSet, RenderBatch const& renderBatch);

  protected:
    VkDevice m_Device;
    VkExtent2D m_SwapChainExtent;
    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_Pipeline;
};

}  // namespace MelonFrontend

#include <MelonFrontend/FragmentShader.h>
#include <MelonFrontend/Subrenderer.h>
#include <MelonFrontend/UniformBuffer.h>
#include <MelonFrontend/VertexShader.h>
#include <MelonFrontend/VulkanUtil.h>

#include <glm/mat4x4.hpp>

namespace MelonFrontend {

void Subrenderer::initialize(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout entityDescriptorSetLayout, VkRenderPass renderPass, unsigned int const& swapChainImageCount) {
    m_Device = device;
    m_SwapChainExtent = swapChainExtent;

    createPipelineLayout<2>(m_Device, {cameraDescriptorSetLayout, entityDescriptorSetLayout}, m_PipelineLayout);
    createGraphicsPipeline(m_Device, k_VertexShader, k_FragmentShader, sizeof(Vertex), Vertex::formats(), Vertex::offsets(), m_SwapChainExtent, m_PipelineLayout, renderPass, m_Pipeline);
}

void Subrenderer::terminate() {
    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
}

void Subrenderer::draw(VkCommandBuffer commandBuffer, unsigned int const& swapChainImageIndex, VkDescriptorSet cameraDescriptorSet, RenderBatch const& renderBatch) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &cameraDescriptorSet, 0, nullptr);
    VkDeviceSize offset = 0;
    for (UniformBuffer const& entityDescriptorSet : renderBatch.entityUniformMemories) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 1, 1, &entityDescriptorSet.descriptorSet, 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &renderBatch.vertexBuffer().buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, renderBatch.indexBuffer().buffer, offset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(commandBuffer, renderBatch.indexCount(), 1, 0, 0, 0);
    }
}

}  // namespace MelonFrontend

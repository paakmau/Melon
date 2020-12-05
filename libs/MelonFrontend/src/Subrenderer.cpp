#include <MelonFrontend/FragmentShader.h>
#include <MelonFrontend/Subrenderer.h>
#include <MelonFrontend/UniformBuffer.h>
#include <MelonFrontend/VertexShader.h>
#include <MelonFrontend/VulkanUtil.h>

#include <glm/mat4x4.hpp>

namespace MelonFrontend {

void Subrenderer::initialize(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout entityDescriptorSetLayout, VkRenderPass renderPass, const unsigned int& swapChainImageCount) {
    _device = device;
    _swapChainExtent = swapChainExtent;

    createPipelineLayout<2>(_device, {cameraDescriptorSetLayout, entityDescriptorSetLayout}, _pipelineLayout);
    createGraphicsPipeline(_device, kVertexShader, kFragmentShader, sizeof(Vertex), Vertex::formats(), Vertex::offsets(), _swapChainExtent, _pipelineLayout, renderPass, _pipeline);
}

void Subrenderer::terminate() {
    vkDestroyPipeline(_device, _pipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
}

void Subrenderer::draw(VkCommandBuffer commandBuffer, const unsigned int& swapChainImageIndex, VkDescriptorSet cameraDescriptorSet, const RenderBatch& renderBatch) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &cameraDescriptorSet, 0, nullptr);
    VkDeviceSize offset = 0;
    for (const UniformBuffer& entityDescriptorSet : renderBatch.entityUniformMemories) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &entityDescriptorSet.descriptorSet, 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &renderBatch.vertexBuffer().buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, renderBatch.indexBuffer().buffer, offset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(commandBuffer, renderBatch.indexCount(), 1, 0, 0, 0);
    }
}

}  // namespace MelonFrontend

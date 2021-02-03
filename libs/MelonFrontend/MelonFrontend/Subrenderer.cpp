#include <MelonFrontend/FragmentShader.h>
#include <MelonFrontend/SpirvUtils.h>
#include <MelonFrontend/Subrenderer.h>
#include <MelonFrontend/UniformBuffer.h>
#include <MelonFrontend/Vertex.h>
#include <MelonFrontend/VertexShader.h>
#include <MelonFrontend/VulkanUtils.h>

#include <cassert>
#include <glm/mat4x4.hpp>

namespace Melon {

void Subrenderer::initialize(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout entityDescriptorSetLayout, VkDescriptorSetLayout lightDescriptorSetLayout, VkRenderPass renderPass, const unsigned int& swapChainImageCount) {
    m_Device = device;
    m_SwapChainExtent = swapChainExtent;

    std::vector<uint32_t> vertexShaderSpirv;
    bool result = glslToSpirv(VK_SHADER_STAGE_VERTEX_BIT, k_VertexShader, vertexShaderSpirv);
    assert(result);
    std::vector<uint32_t> fragmentShaderSpirv;
    result = glslToSpirv(VK_SHADER_STAGE_FRAGMENT_BIT, k_FragmentShader, fragmentShaderSpirv);
    assert(result);

    createPipelineLayout<3>(m_Device, {cameraDescriptorSetLayout, entityDescriptorSetLayout, lightDescriptorSetLayout}, m_PipelineLayout);
    createGraphicsPipeline(m_Device, vertexShaderSpirv, fragmentShaderSpirv, sizeof(Vertex), Vertex::formats(), Vertex::offsets(), m_SwapChainExtent, m_PipelineLayout, renderPass, m_Pipeline);
}

void Subrenderer::terminate() {
    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
}

void Subrenderer::draw(VkCommandBuffer commandBuffer, const unsigned int& swapChainImageIndex, VkDescriptorSet cameraDescriptorSet, VkDescriptorSet lightDescriptorSet, const RenderBatch& renderBatch) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &cameraDescriptorSet, 0, nullptr);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 2, 1, &lightDescriptorSet, 0, nullptr);
    VkDeviceSize offset = 0;
    for (const UniformBuffer& entityDescriptorSet : renderBatch.entityUniformMemories) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 1, 1, &entityDescriptorSet.descriptorSet, 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &renderBatch.vertexBuffer().buffer, &offset);
        vkCmdBindIndexBuffer(commandBuffer, renderBatch.indexBuffer().buffer, offset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(commandBuffer, renderBatch.indexCount(), 1, 0, 0, 0);
    }
}

}  // namespace Melon

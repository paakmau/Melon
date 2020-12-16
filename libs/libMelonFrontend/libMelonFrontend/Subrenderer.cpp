#include <libMelonFrontend/FragmentShader.h>
#include <libMelonFrontend/SpirvUtils.h>
#include <libMelonFrontend/Subrenderer.h>
#include <libMelonFrontend/UniformBuffer.h>
#include <libMelonFrontend/VertexShader.h>
#include <libMelonFrontend/VulkanUtils.h>

#include <glm/mat4x4.hpp>

namespace Melon {

void Subrenderer::initialize(VkDevice device, VkExtent2D swapChainExtent, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout entityDescriptorSetLayout, VkRenderPass renderPass, unsigned int const& swapChainImageCount) {
    m_Device = device;
    m_SwapChainExtent = swapChainExtent;

    std::vector<uint32_t> vertexShaderSpirv;
    bool result = glslToSpirv(VK_SHADER_STAGE_VERTEX_BIT, k_VertexShader, vertexShaderSpirv);
    assert(result);
    std::vector<uint32_t> fragmentShaderSpirv;
    result = glslToSpirv(VK_SHADER_STAGE_FRAGMENT_BIT, k_FragmentShader, fragmentShaderSpirv);
    assert(result);

    createPipelineLayout<2>(m_Device, {cameraDescriptorSetLayout, entityDescriptorSetLayout}, m_PipelineLayout);
    createGraphicsPipeline(m_Device, vertexShaderSpirv, fragmentShaderSpirv, sizeof(Vertex), Vertex::formats(), Vertex::offsets(), m_SwapChainExtent, m_PipelineLayout, renderPass, m_Pipeline);
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

}  // namespace Melon

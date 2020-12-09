#include <MelonFrontend/Renderer.h>
#include <MelonFrontend/SwapChain.h>
#include <MelonFrontend/VulkanUtil.h>
#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

#include <algorithm>
#include <array>
#include <climits>
#include <cstring>
#include <memory>

// GLFW header
#include <GLFW/glfw3.h>

namespace MelonFrontend {

void Renderer::initialize(Window* window) {
    m_Window = window;

    createInstance(window->requiredVulkanInstanceExtensions(), m_VulkanInstance);
    glfwCreateWindowSurface(m_VulkanInstance, window->window(), nullptr, &m_Surface);
    selectPhysicalDevice(m_VulkanInstance, m_Surface, m_PhysicalDevice, m_GraphicsQueueFamilyIndex, m_PresentQueueFamilyIndex, m_PhysicalDeviceFeatures);
    createLogicalDevice(m_PhysicalDevice, m_GraphicsQueueFamilyIndex, m_PresentQueueFamilyIndex, m_PhysicalDeviceFeatures, m_Device, m_GraphicsQueue, m_PresentQueue);

    m_SwapChain.initialize(window->extent(), m_Surface, m_PhysicalDevice, m_Device, m_GraphicsQueueFamilyIndex, m_PresentQueueFamilyIndex, m_PresentQueue);

    createCommandPool(m_Device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, m_GraphicsQueueFamilyIndex, m_CommandPool);
    for (unsigned int i = 0; i < k_MaxTaskCount; i++)
        createCommandPool(m_Device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, m_GraphicsQueueFamilyIndex, m_CommandPools[i]);
    createRenderPass(m_Device, m_SwapChain.imageFormat(), m_RenderPassClear);
    m_Framebuffers.resize(m_SwapChain.imageCount());
    for (unsigned int i = 0; i < m_Framebuffers.size(); i++)
        createFramebuffer(m_Device, m_SwapChain.imageViews()[i], m_RenderPassClear, m_SwapChain.imageExtent(), m_Framebuffers[i]);
    createSemaphores(m_Device, k_MaxInFlightFrameCount, m_ImageAvailableSemaphores);
    createSemaphores(m_Device, k_MaxInFlightFrameCount, m_RenderFinishedSemaphores);
    createFences(m_Device, VK_FENCE_CREATE_SIGNALED_BIT, k_MaxInFlightFrameCount, m_InFlightFences);

    createDescriptorSetLayout<1>(m_Device, {1}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, {VK_SHADER_STAGE_VERTEX_BIT}, m_CameraDescriptorSetLayout);
    createDescriptorSetLayout<1>(m_Device, {1}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, {VK_SHADER_STAGE_VERTEX_BIT}, m_EntityDescriptorSetLayout);
    m_Subrenderer = std::make_unique<Subrenderer>();
    m_Subrenderer->initialize(m_Device, m_SwapChain.imageExtent(), m_CameraDescriptorSetLayout, m_EntityDescriptorSetLayout, m_RenderPassClear, m_SwapChain.imageCount());

    createAllocator(m_VulkanInstance, m_PhysicalDevice, m_Device, m_Allocator);
    createDescriptorPool<1>(m_Device, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, {k_MaxUniformDescriptorCount}, {k_MaxUniformDescriptorCount}, m_UniformDescriptorPool);
    m_UniformMemoryPool.initialize(m_Device, m_Allocator, m_CameraDescriptorSetLayout, m_UniformDescriptorPool);
    m_UniformMemoryPool.registerUniformObjectSize(sizeof(CameraUniformObject));
    m_UniformMemoryPool.registerUniformObjectSize(sizeof(EntityUniformObject));

    for (unsigned int i = 0; i < m_CameraUniformMemories.size(); i++)
        m_CameraUniformMemories[i] = m_UniformMemoryPool.request(sizeof(CameraUniformObject));
}

void Renderer::terminate() {
    vkDeviceWaitIdle(m_Device);

    for (std::vector<MeshBuffer> destoryingMeshBuffers : m_DestroyingMeshBufferArrays) {
        for (MeshBuffer const& meshBuffer : destoryingMeshBuffers) {
            vmaDestroyBuffer(m_Allocator, meshBuffer.vertexBuffer.buffer, meshBuffer.vertexBuffer.allocation);
            vmaDestroyBuffer(m_Allocator, meshBuffer.indexBuffer.buffer, meshBuffer.indexBuffer.allocation);
        }
        destoryingMeshBuffers.clear();
    }

    // Recycle the buffers
    for (std::vector<RenderBatch> const& renderBatches : m_RenderBatchArrays)
        for (RenderBatch const& renderBatch : renderBatches)
            for (UniformBuffer const& buffer : renderBatch.entityUniformMemories)
                m_UniformMemoryPool.recycle(buffer);
    m_UniformMemoryPool.terminate();
    vkDestroyDescriptorPool(m_Device, m_UniformDescriptorPool, nullptr);
    vmaDestroyAllocator(m_Allocator);

    m_Subrenderer->terminate();
    vkDestroyDescriptorSetLayout(m_Device, m_EntityDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_Device, m_CameraDescriptorSetLayout, nullptr);

    for (unsigned int i = 0; i < k_MaxInFlightFrameCount; i++) {
        vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
    }

    for (VkFramebuffer framebuffer : m_Framebuffers)
        vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
    vkDestroyRenderPass(m_Device, m_RenderPassClear, nullptr);
    for (unsigned int i = 0; i < k_MaxTaskCount; i++)
        vkDestroyCommandPool(m_Device, m_CommandPools[i], nullptr);
    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

    m_SwapChain.terminate();

    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);
    vkDestroyInstance(m_VulkanInstance, nullptr);
}

void Renderer::beginFrame() {
    vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, std::numeric_limits<uint32_t>::max());

    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &m_CommandBuffers[m_CurrentFrame]);
    for (SecondaryCommandBuffer const& secondaryCommandBuffer : m_SecondaryCommandBufferArrays[m_CurrentFrame])
        vkFreeCommandBuffers(m_Device, secondaryCommandBuffer.pool, 1, &secondaryCommandBuffer.buffer);
    m_SecondaryCommandBufferArrays[m_CurrentFrame].clear();

    for (MeshBuffer const& meshBuffer : m_DestroyingMeshBufferArrays[m_CurrentFrame]) {
        vmaDestroyBuffer(m_Allocator, meshBuffer.vertexBuffer.buffer, meshBuffer.vertexBuffer.allocation);
        vmaDestroyBuffer(m_Allocator, meshBuffer.indexBuffer.buffer, meshBuffer.indexBuffer.allocation);
    }
    m_DestroyingMeshBufferArrays[m_CurrentFrame].clear();

    allocateCommandBuffer(m_Device, m_CommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, m_CommandBuffers[m_CurrentFrame]);

    VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(m_CommandBuffers[m_CurrentFrame], &beginInfo);
}

MeshBuffer Renderer::createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices) {
    // TODO: Use VMA_MEMORY_USAGE_GPU_ONLY instead
    VkDeviceSize bufferSize = vertices.size() * sizeof(Vertex);
    Buffer vertexBuffer;
    createBuffer(m_Allocator, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, vertexBuffer.buffer, vertexBuffer.allocation);
    void* mappedData;
    vmaMapMemory(m_Allocator, vertexBuffer.allocation, &mappedData);
    memcpy(mappedData, vertices.data(), bufferSize);
    vmaUnmapMemory(m_Allocator, vertexBuffer.allocation);

    bufferSize = indices.size() * sizeof(uint16_t);
    Buffer indexBuffer;
    createBuffer(m_Allocator, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, indexBuffer.buffer, indexBuffer.allocation);
    vmaMapMemory(m_Allocator, indexBuffer.allocation, &mappedData);
    memcpy(mappedData, indices.data(), bufferSize);
    vmaUnmapMemory(m_Allocator, indexBuffer.allocation);

    return MeshBuffer{
        .vertexBuffer = vertexBuffer,
        .indexBuffer = indexBuffer,
        .vertexCount = static_cast<uint32_t>(vertices.size()),
        .indexCount = static_cast<uint32_t>(indices.size())};
}

void Renderer::destroyMeshBuffer(MeshBuffer const& meshBuffer) {
    m_DestroyingMeshBufferArrays[m_CurrentFrame].emplace_back(meshBuffer);
}

void Renderer::beginBatches() {
    for (RenderBatch const& renderBatch : m_RenderBatchArrays[m_CurrentFrame])
        for (UniformBuffer const& buffer : renderBatch.entityUniformMemories)
            m_UniformMemoryPool.recycle(buffer);
    m_RenderBatchArrays[m_CurrentFrame].clear();
}

void Renderer::addBatch(std::vector<glm::mat4> const& models, MeshBuffer const& meshBuffer) {
    std::vector<UniformBuffer> memories;
    for (glm::mat4 const& model : models) {
        UniformBuffer const& buffer = memories.emplace_back(m_UniformMemoryPool.request(sizeof(EntityUniformObject)));
        EntityUniformObject uniformObject{model};
        recordCommandBufferCopyUniformObject(&uniformObject, buffer, sizeof(EntityUniformObject));
    }
    m_RenderBatchArrays[m_CurrentFrame].emplace_back(RenderBatch{std::move(memories), meshBuffer});
}

void Renderer::endBatches() {}

void Renderer::renderFrame(/* TODO: Implement Camera */ glm::mat4 const& vp) {
    bool result = m_SwapChain.acquireNextImageContext(m_ImageAvailableSemaphores[m_CurrentFrame], m_CurrentImageIndex);
    if (!result) {
        recreateSwapChain();
        return;
    }

    CameraUniformObject uniformObject{vp};
    recordCommandBufferCopyUniformObject(&uniformObject, m_CameraUniformMemories[m_CurrentFrame], sizeof(CameraUniformObject));

    recordCommandBufferDraw(m_RenderBatchArrays[m_CurrentFrame], m_CameraUniformMemories[m_CurrentFrame]);

    vkEndCommandBuffer(m_CommandBuffers[m_CurrentFrame]);

    // Submit current command buffer
    vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_ImageAvailableSemaphores[m_CurrentFrame],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_CommandBuffers[m_CurrentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_RenderFinishedSemaphores[m_CurrentFrame]};
    vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);

    result = m_SwapChain.presentImage(m_CurrentImageIndex, m_RenderFinishedSemaphores[m_CurrentFrame]);
    if (!result) {
        recreateSwapChain();
        return;
    }
}

void Renderer::endFrame() {
    m_CurrentFrame = (m_CurrentFrame + 1) % k_MaxInFlightFrameCount;
}

void Renderer::recreateSwapChain() {
    m_Window->waitForResized();
    vkDeviceWaitIdle(m_Device);
    m_Subrenderer->terminate();
    for (VkFramebuffer framebuffer : m_Framebuffers)
        vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
    vkDestroyRenderPass(m_Device, m_RenderPassClear, nullptr);
    m_SwapChain.recreateSwapchain(m_Window->extent());
    createRenderPass(m_Device, m_SwapChain.imageFormat(), m_RenderPassClear);
    m_Framebuffers.resize(m_SwapChain.imageCount());
    for (unsigned int i = 0; i < m_Framebuffers.size(); i++)
        createFramebuffer(m_Device, m_SwapChain.imageViews()[i], m_RenderPassClear, m_SwapChain.imageExtent(), m_Framebuffers[i]);
    m_Subrenderer->initialize(m_Device, m_SwapChain.imageExtent(), m_CameraDescriptorSetLayout, m_EntityDescriptorSetLayout, m_RenderPassClear, m_SwapChain.imageCount());
}

void Renderer::recordCommandBufferCopyUniformObject(void const* data, UniformBuffer buffer, VkDeviceSize size) {
    copyBuffer(m_Allocator, m_CommandBuffers[m_CurrentFrame], buffer.stagingBuffer.buffer, buffer.stagingBuffer.allocation, data, size, buffer.buffer.buffer);
}

void Renderer::recordCommandBufferDraw(std::vector<RenderBatch> const& renderBatches, UniformBuffer const& cameraUniformBuffer) {
    uint32_t taskCount = std::min(k_MaxTaskCount, static_cast<unsigned int>(renderBatches.size()));
    m_SecondaryCommandBufferArrays[m_CurrentFrame].resize(taskCount);
    unsigned int batchCountPerTask = taskCount == 0 ? 0 : (renderBatches.size() / taskCount + ((renderBatches.size() % taskCount == 0) ? 0 : 1));

    std::vector<std::shared_ptr<MelonTask::TaskHandle>> subrendererHandles(taskCount);
    for (uint32_t i = 0; i < taskCount; i++) {
        VkDevice device = m_Device;
        VkFramebuffer framebuffer = m_Framebuffers[m_CurrentImageIndex];
        VkRenderPass renderPass = m_RenderPassClear;
        SecondaryCommandBuffer* secondaryCommandBuffer = &m_SecondaryCommandBufferArrays[m_CurrentFrame][i];
        secondaryCommandBuffer->pool = m_CommandPools[i];
        Subrenderer* subrenderer = m_Subrenderer.get();
        unsigned int swapChainImageIndex = m_CurrentImageIndex;
        subrendererHandles[i] = MelonTask::TaskManager::instance()->schedule(
            [device, framebuffer, renderPass, secondaryCommandBuffer, subrenderer, swapChainImageIndex, i, &cameraUniformBuffer, &renderBatches, batchCountPerTask]() {
                allocateCommandBuffer(device, secondaryCommandBuffer->pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1, secondaryCommandBuffer->buffer);
                VkCommandBufferInheritanceInfo inheritanceInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                    .renderPass = renderPass,
                    .subpass = 0,
                    .framebuffer = framebuffer};
                VkCommandBufferBeginInfo beginInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                    .pInheritanceInfo = &inheritanceInfo};
                vkBeginCommandBuffer(secondaryCommandBuffer->buffer, &beginInfo);
                for (unsigned int j = 0; j < batchCountPerTask && batchCountPerTask * i + j < renderBatches.size(); j++)
                    subrenderer->draw(secondaryCommandBuffer->buffer, swapChainImageIndex, cameraUniformBuffer.descriptorSet, renderBatches[batchCountPerTask * i + j]);
                vkEndCommandBuffer(secondaryCommandBuffer->buffer);
            });
    }
    MelonTask::TaskManager::instance()->activateWaitingTasks();
    for (std::shared_ptr<MelonTask::TaskHandle> taskHandle : subrendererHandles)
        taskHandle->complete();

    VkClearValue clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
    VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_RenderPassClear,
        .framebuffer = m_Framebuffers[m_CurrentImageIndex],
        .renderArea = VkRect2D{
            .offset = {0, 0},
            .extent = m_SwapChain.imageExtent()},
        .clearValueCount = 1,
        .pClearValues = &clearValue};
    vkCmdBeginRenderPass(m_CommandBuffers[m_CurrentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    for (SecondaryCommandBuffer const& secondaryCommandBuffer : m_SecondaryCommandBufferArrays[m_CurrentFrame])
        vkCmdExecuteCommands(m_CommandBuffers[m_CurrentFrame], 1, &secondaryCommandBuffer.buffer);
    vkCmdEndRenderPass(m_CommandBuffers[m_CurrentFrame]);
}

}  // namespace MelonFrontend

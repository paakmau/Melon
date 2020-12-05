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

namespace MelonFrontend {

void Renderer::initialize(Window* window) {
    _window = window;

    volkInitialize();
    createInstance(window->requiredVulkanInstanceExtensions(), _vulkanInstance);
    volkLoadInstance(_vulkanInstance);
    glfwCreateWindowSurface(_vulkanInstance, window->window(), nullptr, &_surface);
    selectPhysicalDevice(_vulkanInstance, _surface, _physicalDevice, _graphicsQueueFamilyIndex, _presentQueueFamilyIndex, _physicalDeviceFeatures);
    createLogicalDevice(_physicalDevice, _graphicsQueueFamilyIndex, _presentQueueFamilyIndex, _physicalDeviceFeatures, _device, _graphicsQueue, _presentQueue);

    _swapChain.initialize(window->extent(), _surface, _physicalDevice, _device, _graphicsQueueFamilyIndex, _presentQueueFamilyIndex, _presentQueue);

    createCommandPool(_device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, _graphicsQueueFamilyIndex, _commandPool);
    for (unsigned int i = 0; i < kMaxTaskCount; i++)
        createCommandPool(_device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, _graphicsQueueFamilyIndex, _commandPools[i]);
    createRenderPass(_device, _swapChain.imageFormat(), _renderPassClear);
    _framebuffers.resize(_swapChain.imageCount());
    for (unsigned int i = 0; i < _framebuffers.size(); i++)
        createFramebuffer(_device, _swapChain.imageViews()[i], _renderPassClear, _swapChain.imageExtent(), _framebuffers[i]);
    createSemaphores(_device, kMaxInFlightFrameCount, _imageAvailableSemaphores);
    createSemaphores(_device, kMaxInFlightFrameCount, _renderFinishedSemaphores);
    createFences(_device, VK_FENCE_CREATE_SIGNALED_BIT, kMaxInFlightFrameCount, _inFlightFences);

    createDescriptorSetLayout<1>(_device, {1}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, {VK_SHADER_STAGE_VERTEX_BIT}, _cameraDescriptorSetLayout);
    createDescriptorSetLayout<1>(_device, {1}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, {VK_SHADER_STAGE_VERTEX_BIT}, _entityDescriptorSetLayout);
    _subrenderer = std::make_unique<Subrenderer>();
    _subrenderer->initialize(_device, _swapChain.imageExtent(), _cameraDescriptorSetLayout, _entityDescriptorSetLayout, _renderPassClear, _swapChain.imageCount());

    createAllocator(_vulkanInstance, _physicalDevice, _device, _allocator);
    createDescriptorPool<1>(_device, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}, {kMaxUniformDescriptorCount}, {kMaxUniformDescriptorCount}, _uniformDescriptorPool);
    _uniformMemoryPool.initialize(_device, _allocator, _cameraDescriptorSetLayout, _uniformDescriptorPool);
    _uniformMemoryPool.registerUniformObjectSize(sizeof(CameraUniformObject));
    _uniformMemoryPool.registerUniformObjectSize(sizeof(EntityUniformObject));

    for (unsigned int i = 0; i < _cameraUniformMemories.size(); i++)
        _cameraUniformMemories[i] = _uniformMemoryPool.request(sizeof(CameraUniformObject));
}

void Renderer::terminate() {
    vkDeviceWaitIdle(_device);

    for (std::vector<MeshBuffer> destoryingMeshBuffers : _destroyingMeshBufferArrays) {
        for (const MeshBuffer& meshBuffer : destoryingMeshBuffers) {
            vmaDestroyBuffer(_allocator, meshBuffer.vertexBuffer.buffer, meshBuffer.vertexBuffer.allocation);
            vmaDestroyBuffer(_allocator, meshBuffer.indexBuffer.buffer, meshBuffer.indexBuffer.allocation);
        }
        destoryingMeshBuffers.clear();
    }

    // Recycle the buffers
    for (const std::vector<RenderBatch> renderBatches : _renderBatchArrays)
        for (const RenderBatch& renderBatch : renderBatches)
            for (const UniformBuffer& buffer : renderBatch.entityUniformMemories)
                _uniformMemoryPool.recycle(buffer);
    _uniformMemoryPool.terminate();
    vkDestroyDescriptorPool(_device, _uniformDescriptorPool, nullptr);
    vmaDestroyAllocator(_allocator);

    _subrenderer->terminate();
    vkDestroyDescriptorSetLayout(_device, _entityDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(_device, _cameraDescriptorSetLayout, nullptr);

    for (unsigned int i = 0; i < kMaxInFlightFrameCount; i++) {
        vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(_device, _inFlightFences[i], nullptr);
    }

    for (VkFramebuffer framebuffer : _framebuffers)
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    vkDestroyRenderPass(_device, _renderPassClear, nullptr);
    for (unsigned int i = 0; i < kMaxTaskCount; i++)
        vkDestroyCommandPool(_device, _commandPools[i], nullptr);
    vkDestroyCommandPool(_device, _commandPool, nullptr);

    _swapChain.terminate();

    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_vulkanInstance, _surface, nullptr);
    vkDestroyInstance(_vulkanInstance, nullptr);
}

void Renderer::beginFrame() {
    vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, std::numeric_limits<uint32_t>::max());

    vkFreeCommandBuffers(_device, _commandPool, 1, &_commandBuffers[_currentFrame]);
    for (const SecondaryCommandBuffer& secondaryCommandBuffer : _secondaryCommandBufferArrays[_currentFrame])
        vkFreeCommandBuffers(_device, secondaryCommandBuffer.pool, 1, &secondaryCommandBuffer.buffer);
    _secondaryCommandBufferArrays[_currentFrame].clear();

    for (const MeshBuffer& meshBuffer : _destroyingMeshBufferArrays[_currentFrame]) {
        vmaDestroyBuffer(_allocator, meshBuffer.vertexBuffer.buffer, meshBuffer.vertexBuffer.allocation);
        vmaDestroyBuffer(_allocator, meshBuffer.indexBuffer.buffer, meshBuffer.indexBuffer.allocation);
    }
    _destroyingMeshBufferArrays[_currentFrame].clear();

    allocateCommandBuffer(_device, _commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, _commandBuffers[_currentFrame]);

    VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(_commandBuffers[_currentFrame], &beginInfo);
}

MeshBuffer Renderer::createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices) {
    // TODO: Use VMA_MEMORY_USAGE_GPU_ONLY instead
    VkDeviceSize bufferSize = vertices.size() * sizeof(Vertex);
    Buffer vertexBuffer;
    createBuffer(_allocator, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, vertexBuffer.buffer, vertexBuffer.allocation);
    void* mappedData;
    vmaMapMemory(_allocator, vertexBuffer.allocation, &mappedData);
    memcpy(mappedData, vertices.data(), bufferSize);
    vmaUnmapMemory(_allocator, vertexBuffer.allocation);

    bufferSize = indices.size() * sizeof(uint16_t);
    Buffer indexBuffer;
    createBuffer(_allocator, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, indexBuffer.buffer, indexBuffer.allocation);
    vmaMapMemory(_allocator, indexBuffer.allocation, &mappedData);
    memcpy(mappedData, indices.data(), bufferSize);
    vmaUnmapMemory(_allocator, indexBuffer.allocation);

    return MeshBuffer{
        .vertexBuffer = vertexBuffer,
        .vertexCount = static_cast<uint32_t>(vertices.size()),
        .indexBuffer = indexBuffer,
        .indexCount = static_cast<uint32_t>(indices.size())};
}

void Renderer::destroyMeshBuffer(const MeshBuffer& meshBuffer) {
    _destroyingMeshBufferArrays[_currentFrame].emplace_back(meshBuffer);
}

void Renderer::beginBatches() {
    for (const RenderBatch& renderBatch : _renderBatchArrays[_currentFrame])
        for (const UniformBuffer& buffer : renderBatch.entityUniformMemories)
            _uniformMemoryPool.recycle(buffer);
    _renderBatchArrays[_currentFrame].clear();
}

void Renderer::addBatch(const std::vector<glm::mat4>& models, const MeshBuffer& meshBuffer) {
    std::vector<UniformBuffer> memories;
    for (const glm::mat4& model : models) {
        const UniformBuffer& buffer = memories.emplace_back(_uniformMemoryPool.request(sizeof(EntityUniformObject)));
        EntityUniformObject uniformObject{model};
        recordCommandBufferCopyUniformObject(&uniformObject, buffer, sizeof(EntityUniformObject));
    }
    _renderBatchArrays[_currentFrame].emplace_back(RenderBatch{std::move(memories), meshBuffer});
}

void Renderer::endBatches() {}

void Renderer::renderFrame(/* TODO: Implement Camera */ const glm::mat4& vp) {
    bool result = _swapChain.acquireNextImageContext(_imageAvailableSemaphores[_currentFrame], _currentImageIndex);
    if (!result) {
        recreateSwapChain();
        return;
    }

    CameraUniformObject uniformObject{vp};
    recordCommandBufferCopyUniformObject(&uniformObject, _cameraUniformMemories[_currentFrame], sizeof(CameraUniformObject));

    recordCommandBufferDraw(_renderBatchArrays[_currentFrame], _cameraUniformMemories[_currentFrame]);

    vkEndCommandBuffer(_commandBuffers[_currentFrame]);

    // Submit current command buffer
    vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &_imageAvailableSemaphores[_currentFrame],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &_commandBuffers[_currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &_renderFinishedSemaphores[_currentFrame]};
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]);

    result = _swapChain.presentImage(_currentImageIndex, _renderFinishedSemaphores[_currentFrame]);
    if (!result) {
        recreateSwapChain();
        return;
    }
}

void Renderer::endFrame() {
    _currentFrame = (_currentFrame + 1) % kMaxInFlightFrameCount;
}

void Renderer::recreateSwapChain() {
    _window->waitForResized();
    vkDeviceWaitIdle(_device);
    _subrenderer->terminate();
    for (VkFramebuffer framebuffer : _framebuffers)
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    vkDestroyRenderPass(_device, _renderPassClear, nullptr);
    _swapChain.recreateSwapchain(_window->extent());
    createRenderPass(_device, _swapChain.imageFormat(), _renderPassClear);
    _framebuffers.resize(_swapChain.imageCount());
    for (unsigned int i = 0; i < _framebuffers.size(); i++)
        createFramebuffer(_device, _swapChain.imageViews()[i], _renderPassClear, _swapChain.imageExtent(), _framebuffers[i]);
    _subrenderer->initialize(_device, _swapChain.imageExtent(), _cameraDescriptorSetLayout, _entityDescriptorSetLayout, _renderPassClear, _swapChain.imageCount());
}

void Renderer::recordCommandBufferCopyUniformObject(const void* data, UniformBuffer buffer, VkDeviceSize size) {
    copyBuffer(_allocator, _commandBuffers[_currentFrame], buffer.stagingBuffer.buffer, buffer.stagingBuffer.allocation, data, size, buffer.buffer.buffer);
}

void Renderer::recordCommandBufferDraw(const std::vector<RenderBatch>& renderBatches, const UniformBuffer& cameraUniformBuffer) {
    uint32_t taskCount = std::min(kMaxTaskCount, static_cast<unsigned int>(renderBatches.size()));
    _secondaryCommandBufferArrays[_currentFrame].resize(taskCount);
    unsigned int batchCountPerTask = taskCount == 0 ? 0 : (renderBatches.size() / taskCount + ((renderBatches.size() % taskCount == 0) ? 0 : 1));

    std::vector<std::shared_ptr<MelonTask::TaskHandle>> subrendererHandles(taskCount);
    for (uint32_t i = 0; i < taskCount; i++) {
        VkDevice device = _device;
        VkFramebuffer framebuffer = _framebuffers[_currentImageIndex];
        VkRenderPass renderPass = _renderPassClear;
        SecondaryCommandBuffer* secondaryCommandBuffer = &_secondaryCommandBufferArrays[_currentFrame][i];
        secondaryCommandBuffer->pool = _commandPools[i];
        Subrenderer* subrenderer = _subrenderer.get();
        unsigned int swapChainImageIndex = _currentImageIndex;
        subrendererHandles[i] = MelonTask::TaskManager::instance()->schedule(
            [device, framebuffer, renderPass, secondaryCommandBuffer, subrenderer, swapChainImageIndex, i, &cameraUniformBuffer, &renderBatches, batchCountPerTask]() {
                allocateCommandBuffer(device, secondaryCommandBuffer->pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1, secondaryCommandBuffer->buffer);
                VkCommandBufferInheritanceInfo inheritanceInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                    .framebuffer = framebuffer,
                    .renderPass = renderPass,
                    .subpass = 0};
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
        .renderPass = _renderPassClear,
        .framebuffer = _framebuffers[_currentImageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = _swapChain.imageExtent(),
        .clearValueCount = 1,
        .pClearValues = &clearValue};
    vkCmdBeginRenderPass(_commandBuffers[_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    for (const SecondaryCommandBuffer& secondaryCommandBuffer : _secondaryCommandBufferArrays[_currentFrame])
        vkCmdExecuteCommands(_commandBuffers[_currentFrame], 1, &secondaryCommandBuffer.buffer);
    vkCmdEndRenderPass(_commandBuffers[_currentFrame]);
}

}  // namespace MelonFrontend

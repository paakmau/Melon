#pragma once

#include <MelonFrontend/MeshBuffer.h>
#include <MelonFrontend/RenderBatch.h>
#include <MelonFrontend/Subrenderer.h>
#include <MelonFrontend/SwapChain.h>
#include <MelonFrontend/UniformBuffer.h>
#include <MelonFrontend/UniformBufferPool.h>
#include <MelonFrontend/VulkanPlatform.h>
#include <MelonFrontend/Window.h>

#include <array>
#include <glm/mat4x4.hpp>
#include <vector>

namespace MelonFrontend {

class SwapChain;
struct SwapchainImageContext;

class Renderer {
  public:
    static constexpr unsigned int kMaxInFlightFrameCount = 2U;
    static constexpr unsigned int kMaxTaskCount = 4U;
    static constexpr unsigned int kMaxUniformDescriptorCount = 2048U;

    void initialize(Window* window);
    void terminate();

    void beginFrame();
    MeshBuffer createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices);
    void destroyMeshBuffer(MeshBuffer const& meshBuffer);
    void beginBatches();
    void addBatch(std::vector<glm::mat4> const& models, MeshBuffer const& meshBuffer);
    void endBatches();
    void renderFrame(glm::mat4 const& vp);
    void endFrame();

  private:
    struct SecondaryCommandBuffer {
        VkCommandPool pool;
        VkCommandBuffer buffer;
    };

    void recreateSwapChain();

    void recordCommandBufferCopyUniformObject(void const* data, UniformBuffer memory, VkDeviceSize size);
    void recordCommandBufferDraw(std::vector<RenderBatch> const& renderBatches, UniformBuffer const& cameraUniformBuffer);

    Window* _window;

    VkInstance _vulkanInstance;
    VkSurfaceKHR _surface;
    VkPhysicalDevice _physicalDevice;
    VkPhysicalDeviceFeatures _physicalDeviceFeatures;
    uint32_t _graphicsQueueFamilyIndex;
    uint32_t _presentQueueFamilyIndex;
    VkDevice _device;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    SwapChain _swapChain;
    VkCommandPool _commandPool;
    VkCommandPool _commandPools[kMaxTaskCount];
    uint32_t _currentImageIndex;

    VkRenderPass _renderPassClear;
    std::vector<VkFramebuffer> _framebuffers;

    unsigned int _currentFrame{};
    std::array<VkSemaphore, kMaxInFlightFrameCount> _imageAvailableSemaphores;
    std::array<VkSemaphore, kMaxInFlightFrameCount> _renderFinishedSemaphores;
    std::array<VkFence, kMaxInFlightFrameCount> _inFlightFences;
    std::array<VkCommandBuffer, kMaxInFlightFrameCount> _commandBuffers;
    std::array<std::vector<SecondaryCommandBuffer>, kMaxInFlightFrameCount> _secondaryCommandBufferArrays;
    std::array<std::vector<MeshBuffer>, kMaxInFlightFrameCount> _destroyingMeshBufferArrays;

    VkDescriptorSetLayout _cameraDescriptorSetLayout;
    VkDescriptorSetLayout _entityDescriptorSetLayout;
    std::unique_ptr<Subrenderer> _subrenderer;

    VmaAllocator _allocator;
    VkDescriptorPool _uniformDescriptorPool;
    UniformBufferPool _uniformMemoryPool;
    std::array<std::vector<RenderBatch>, kMaxInFlightFrameCount> _renderBatchArrays;
    std::array<UniformBuffer, kMaxInFlightFrameCount> _cameraUniformMemories;
};

}  // namespace MelonFrontend

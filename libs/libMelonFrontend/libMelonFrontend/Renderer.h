#pragma once

#include <libMelonFrontend/MeshBuffer.h>
#include <libMelonFrontend/RenderBatch.h>
#include <libMelonFrontend/Subrenderer.h>
#include <libMelonFrontend/SwapChain.h>
#include <libMelonFrontend/UniformBuffer.h>
#include <libMelonFrontend/UniformBufferPool.h>
#include <libMelonFrontend/VulkanPlatform.h>
#include <libMelonFrontend/Window.h>
#include <libMelonTask/TaskManager.h>

#include <array>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>

namespace MelonFrontend {

class SwapChain;
struct SwapchainImageContext;

class Renderer {
  public:
    static constexpr unsigned int k_MaxInFlightFrameCount = 2U;
    static constexpr unsigned int k_MaxTaskCount = 4U;
    static constexpr unsigned int k_MaxUniformDescriptorCount = 2048U;

    void initialize(MelonTask::TaskManager* taskManager, Window* window);
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

    MelonTask::TaskManager* m_TaskManager;

    Window* m_Window;

    VkInstance m_VulkanInstance;
    VkSurfaceKHR m_Surface;
    VkPhysicalDevice m_PhysicalDevice;
    VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
    uint32_t m_GraphicsQueueFamilyIndex;
    uint32_t m_PresentQueueFamilyIndex;
    VkDevice m_Device;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;

    SwapChain m_SwapChain;
    VkCommandPool m_CommandPool;
    VkCommandPool m_CommandPools[k_MaxTaskCount];
    uint32_t m_CurrentImageIndex;

    VkRenderPass m_RenderPassClear;
    std::vector<VkFramebuffer> m_Framebuffers;

    unsigned int m_CurrentFrame{};
    std::array<VkSemaphore, k_MaxInFlightFrameCount> m_ImageAvailableSemaphores;
    std::array<VkSemaphore, k_MaxInFlightFrameCount> m_RenderFinishedSemaphores;
    std::array<VkFence, k_MaxInFlightFrameCount> m_InFlightFences;
    std::array<VkCommandBuffer, k_MaxInFlightFrameCount> m_CommandBuffers;
    std::array<std::vector<SecondaryCommandBuffer>, k_MaxInFlightFrameCount> m_SecondaryCommandBufferArrays;
    std::array<std::vector<MeshBuffer>, k_MaxInFlightFrameCount> m_DestroyingMeshBufferArrays;

    VkDescriptorSetLayout m_CameraDescriptorSetLayout;
    VkDescriptorSetLayout m_EntityDescriptorSetLayout;
    std::unique_ptr<Subrenderer> m_Subrenderer;

    VmaAllocator m_Allocator;
    VkDescriptorPool m_UniformDescriptorPool;
    UniformBufferPool m_UniformMemoryPool;
    std::array<std::vector<RenderBatch>, k_MaxInFlightFrameCount> m_RenderBatchArrays;
    std::array<UniformBuffer, k_MaxInFlightFrameCount> m_CameraUniformMemories;
};

}  // namespace MelonFrontend

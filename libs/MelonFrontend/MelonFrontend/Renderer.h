#pragma once

#include <MelonFrontend/Buffer.h>
#include <MelonFrontend/Light.h>
#include <MelonFrontend/MeshBuffer.h>
#include <MelonFrontend/RenderBatch.h>
#include <MelonFrontend/StagingMeshBufferPool.h>
#include <MelonFrontend/Subrenderer.h>
#include <MelonFrontend/SwapChain.h>
#include <MelonFrontend/UniformBuffer.h>
#include <MelonFrontend/UniformBufferPool.h>
#include <MelonFrontend/Vertex.h>
#include <MelonFrontend/VulkanPlatform.h>
#include <MelonFrontend/Window.h>
#include <MelonTask/TaskManager.h>

#include <array>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <vector>

namespace Melon {

class SwapChain;
struct SwapchainImageContext;

class Renderer {
  public:
    static constexpr unsigned int k_MaxInFlightFrameCount = 2U;
    static constexpr unsigned int k_MaxTaskCount = 4U;
    static constexpr unsigned int k_MaxUniformDescriptorCount = 2048U;

    void initialize(TaskManager* taskManager, Window* window);
    void terminate();

    void beginFrame();
    MeshBuffer createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices);
    void destroyMeshBuffer(const MeshBuffer& meshBuffer);
    void beginBatches();
    void addBatch(std::vector<glm::mat4> const& models, const MeshBuffer& meshBuffer);
    void endBatches();
    void renderFrame(const glm::mat4& projection, const glm::vec3& cameraTranslation, const glm::quat& cameraRotation, const glm::vec3& lightDirection);
    void endFrame();

  private:
    struct SecondaryCommandBuffer {
        VkCommandPool pool;
        VkCommandBuffer buffer;
    };

    void recreateSwapChain();

    void recordCommandBufferCopyMeshData(std::vector<Vertex> const& vertices, std::vector<uint16_t> const& indices, StagingMeshBuffer stagingMeshBuffer, MeshBuffer meshBuffer);
    void recordCommandBufferCopyUniformObject(VkDeviceSize size, UniformBuffer memory);
    void recordCommandBufferDraw(std::vector<RenderBatch> const& renderBatches, const UniformBuffer& cameraUniformBuffer, const UniformBuffer& lightUniformBuffer);

    TaskManager* m_TaskManager;

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

    std::array<std::vector<StagingMeshBuffer>, k_MaxInFlightFrameCount> m_RecyclingStagingMeshBufferArrays;
    std::array<std::vector<Buffer>, k_MaxInFlightFrameCount> m_DestroyingBufferArrays;

    VkDescriptorSetLayout m_CameraDescriptorSetLayout;
    VkDescriptorSetLayout m_EntityDescriptorSetLayout;
    VkDescriptorSetLayout m_LightDescriptorSetLayout;
    std::unique_ptr<Subrenderer> m_Subrenderer;

    VmaAllocator m_Allocator;
    VkDescriptorPool m_UniformDescriptorPool;
    UniformBufferPool m_UniformMemoryPool;
    StagingMeshBufferPool m_StagingMeshBufferPool;

    std::array<std::vector<RenderBatch>, k_MaxInFlightFrameCount> m_RenderBatchArrays;
    std::array<UniformBuffer, k_MaxInFlightFrameCount> m_CameraUniformMemories;
    std::array<UniformBuffer, k_MaxInFlightFrameCount> m_LightUniformMemories;
};

}  // namespace Melon

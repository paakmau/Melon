#pragma once

#include <MelonFrontend/VulkanPlatform.h>
#include <MelonFrontend/Window.h>

#include <vector>

namespace MelonFrontend {

class SwapChain {
  public:
    void initialize(VkExtent2D extent, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex, VkQueue presentQueue);
    void terminate();

    bool acquireNextImageContext(VkSemaphore imageAvailableSemaphore, uint32_t& imageIndex);
    bool presentImage(uint32_t const& imageIndex, VkSemaphore waitSemaphore);

    void recreateSwapchain(VkExtent2D extent);

    unsigned int imageCount() const;

    VkFormat const& imageFormat() const { return m_ImageFormat; }
    VkExtent2D const& imageExtent() const { return m_ImageExtent; }
    std::vector<VkImage> const& images() const { return m_Images; }
    std::vector<VkImageView> const& imageViews() const { return m_ImageViews; }

  private:
    VkSurfaceKHR m_Surface;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    uint32_t m_GraphicsQueueFamilyIndex;
    uint32_t m_PresentQueueFamilyIndex;
    VkQueue m_PresentQueue;
    VkSwapchainKHR m_Swapchain;
    VkFormat m_ImageFormat;
    VkExtent2D m_ImageExtent;
    std::vector<VkImage> m_Images;
    std::vector<VkImageView> m_ImageViews;
};

inline unsigned int SwapChain::imageCount() const { return m_Images.size(); }

}  // namespace MelonFrontend

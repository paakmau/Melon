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
    bool presentImage(const uint32_t& imageIndex, VkSemaphore waitSemaphore);

    void recreateSwapchain(VkExtent2D extent);

    unsigned int imageCount() const;

    const VkFormat& imageFormat() const { return _imageFormat; }
    const VkExtent2D& imageExtent() const { return _imageExtent; }
    const std::vector<VkImage>& images() const { return _images; }
    const std::vector<VkImageView>& imageViews() const { return _imageViews; }

  private:
    VkSurfaceKHR _surface;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    uint32_t _graphicsQueueFamilyIndex;
    uint32_t _presentQueueFamilyIndex;
    VkQueue _presentQueue;
    VkSwapchainKHR _swapchain;
    VkFormat _imageFormat;
    VkExtent2D _imageExtent;
    std::vector<VkImage> _images;
    std::vector<VkImageView> _imageViews;
};

inline unsigned int SwapChain::imageCount() const { return _images.size(); }

}  // namespace MelonFrontend

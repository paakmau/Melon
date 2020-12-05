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

    VkFormat const& imageFormat() const { return _imageFormat; }
    VkExtent2D const& imageExtent() const { return _imageExtent; }
    std::vector<VkImage> const& images() const { return _images; }
    std::vector<VkImageView> const& imageViews() const { return _imageViews; }

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

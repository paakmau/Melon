#include <MelonFrontend/SwapChain.h>
#include <MelonFrontend/VulkanUtil.h>

#include <climits>

namespace MelonFrontend {

void SwapChain::initialize(VkExtent2D extent, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex, VkQueue presentQueue) {
    _surface = surface;
    _physicalDevice = physicalDevice;
    _device = device;
    _graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    _presentQueueFamilyIndex = presentQueueFamilyIndex;
    _presentQueue = presentQueue;

    createSwapchain(device, extent, surface, physicalDevice, graphicsQueueFamilyIndex, presentQueueFamilyIndex, _swapchain, _imageFormat, _imageExtent, _images, _imageViews);
}

void SwapChain::terminate() {
    for (VkImageView imageView : _imageViews)
        vkDestroyImageView(_device, imageView, nullptr);
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

bool SwapChain::acquireNextImageContext(VkSemaphore imageAvailableSemaphore, uint32_t& imageIndex) {
    VkResult result = vkAcquireNextImageKHR(_device, _swapchain, std::numeric_limits<uint32_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        return false;
    else
        assert(result == VK_SUCCESS);

    return true;
}

bool SwapChain::presentImage(const uint32_t& imageIndex, VkSemaphore waitSemaphore) {
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &waitSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &_swapchain,
        .pImageIndices = &imageIndex};
    VkResult result = vkQueuePresentKHR(_presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        return false;
    else
        assert(result == VK_SUCCESS);
    return true;
}

void SwapChain::recreateSwapchain(VkExtent2D extent) {
    for (VkImageView imageView : _imageViews)
        vkDestroyImageView(_device, imageView, nullptr);
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    createSwapchain(_device, extent, _surface, _physicalDevice, _graphicsQueueFamilyIndex, _presentQueueFamilyIndex, _swapchain, _imageFormat, _imageExtent, _images, _imageViews);
}

}  // namespace MelonFrontend
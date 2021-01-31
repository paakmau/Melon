#include <MelonFrontend/SwapChain.h>
#include <MelonFrontend/VulkanUtils.h>

#include <cassert>
#include <limits>

namespace Melon {

void SwapChain::initialize(VkExtent2D extent, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex, VkQueue presentQueue) {
    m_Surface = surface;
    m_PhysicalDevice = physicalDevice;
    m_Device = device;
    m_GraphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    m_PresentQueueFamilyIndex = presentQueueFamilyIndex;
    m_PresentQueue = presentQueue;

    createSwapchain(device, extent, surface, physicalDevice, graphicsQueueFamilyIndex, presentQueueFamilyIndex, m_Swapchain, m_ImageFormat, m_ImageExtent, m_Images, m_ImageViews);
}

void SwapChain::terminate() {
    for (VkImageView imageView : m_ImageViews)
        vkDestroyImageView(m_Device, imageView, nullptr);
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
}

bool SwapChain::acquireNextImageContext(VkSemaphore imageAvailableSemaphore, uint32_t& imageIndex) {
    VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain, (std::numeric_limits<uint32_t>::max)(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
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
        .pSwapchains = &m_Swapchain,
        .pImageIndices = &imageIndex};
    VkResult result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        return false;
    else
        assert(result == VK_SUCCESS);
    return true;
}

void SwapChain::recreateSwapchain(VkExtent2D extent) {
    for (VkImageView imageView : m_ImageViews)
        vkDestroyImageView(m_Device, imageView, nullptr);
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

    createSwapchain(m_Device, extent, m_Surface, m_PhysicalDevice, m_GraphicsQueueFamilyIndex, m_PresentQueueFamilyIndex, m_Swapchain, m_ImageFormat, m_ImageExtent, m_Images, m_ImageViews);
}

}  // namespace Melon
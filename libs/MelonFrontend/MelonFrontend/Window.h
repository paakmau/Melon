#pragma once

#include <MelonFrontend/VulkanPlatform.h>

#include <vector>

struct GLFWwindow;

namespace Melon {

// TODO: Process window events
class Window {
  public:
    void initialize(const char* title, const unsigned int& width, const unsigned int& height);
    void terminate();

    void pollEvents();

    void waitForResized();

    std::vector<const char*> requiredVulkanInstanceExtensions() const;

    float aspectRatio() const { return static_cast<float>(m_Extent.width) / static_cast<float>(m_Extent.height); }

    GLFWwindow* const& window() const { return m_Window; }

    const VkExtent2D& extent() const { return m_Extent; }

    bool resized() const { return m_Resized; }
    bool closed() const { return m_Closed; }

  private:
    static void framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height);
    static void windowCloseCallback(GLFWwindow* glfwWindow);

    void notifyResized();
    void notifyClosed();

    GLFWwindow* m_Window;

    VkExtent2D m_Extent;

    bool m_Resized{};
    bool m_Closed{};
};

}  // namespace Melon

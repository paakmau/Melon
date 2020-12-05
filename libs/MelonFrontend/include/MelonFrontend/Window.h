#pragma once

#include <GLFW/glfw3.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <vector>

namespace MelonFrontend {

// TODO: Process window events
class Window {
  public:
    void initialize(char const* title, unsigned int const& width, unsigned int const& height);
    void terminate();

    void pollEvents();

    void waitForResized();

    std::vector<char const*> requiredVulkanInstanceExtensions() const;

    float aspectRatio() const { return static_cast<float>(m_Extent.width) / static_cast<float>(m_Extent.height); }

    GLFWwindow* const& window() const { return m_Window; }

    VkExtent2D const& extent() const { return m_Extent; }

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

}  // namespace MelonFrontend

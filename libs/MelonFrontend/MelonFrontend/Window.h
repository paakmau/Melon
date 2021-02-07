#pragma once

#include <MelonFrontend/InputEvents.h>
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

    const std::vector<KeyDownEvent>& keyDownEvents() { return m_KeyDownEvents; }
    const std::vector<KeyUpEvent>& keyUpEvents() { return m_KeyUpEvents; }
    const std::vector<MouseButtonDownEvent>& mouseButtonDownEvents() { return m_MouseButtonDownEvents; }
    const std::vector<MouseButtonUpEvent>& mouseButtonUpEvents() { return m_MouseButtonUpEvents; }
    const std::vector<MouseScrollEvent>& mouseScrollEvents() { return m_MouseScrollEvents; }

  private:
    static void framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height);
    static void windowCloseCallback(GLFWwindow* glfwWindow);
    static void keyCallback(GLFWwindow* glfwWindow, int key, int scanCode, int action, int mods);
    static void cursorPosCallback(GLFWwindow* glfwWindow, double xPos, double yPos);
    static void mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* glfwWindow, double xOffset, double yOffset);

    void notifyResized();
    void notifyClosed();

    GLFWwindow* m_Window;

    VkExtent2D m_Extent;

    bool m_Resized{};
    bool m_Closed{};

    std::vector<KeyDownEvent> m_KeyDownEvents;
    std::vector<KeyUpEvent> m_KeyUpEvents;
    std::vector<MouseButtonDownEvent> m_MouseButtonDownEvents;
    std::vector<MouseButtonUpEvent> m_MouseButtonUpEvents;
    std::vector<MouseScrollEvent> m_MouseScrollEvents;
};

}  // namespace Melon

#pragma once

#include <MelonFrontend/VulkanPlatform.h>
#include <GLFW/glfw3.h>

#include <vector>

namespace MelonFrontend {

// TODO: Process window events
class Window {
   public:
    void initialize(const char* title, const int& width, const int& height);
    void terminate();

    void pollEvents();

    void waitForResized();

    std::vector<const char*> requiredVulkanInstanceExtensions() const;

    float aspectRatio() const { return static_cast<float>(_extent.width) / static_cast<float>(_extent.height); }

    GLFWwindow* const& window() const { return _window; }

    const VkExtent2D& extent() const { return _extent; }

    bool resized() const { return _resized; }
    bool closed() const { return _closed; }

   private:
    static void framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height);
    static void windowCloseCallback(GLFWwindow* glfwWindow);

    void notifyResized();
    void notifyClosed();

    GLFWwindow* _window;

    VkExtent2D _extent;

    bool _resized{};
    bool _closed{};
};

}  // namespace MelonFrontend

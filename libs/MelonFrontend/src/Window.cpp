#include <MelonFrontend/Window.h>

namespace MelonFrontend {

void Window::initialize(char const* title, unsigned int const& width, unsigned int const& height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    _extent = VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    glfwSetWindowUserPointer(_window, this);

    // Callbacks
    glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
    glfwSetWindowCloseCallback(_window, windowCloseCallback);
}

void Window::terminate() {
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::waitForResized() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_window, &width, &height);
        glfwWaitEvents();
    }
    _extent = VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    _resized = false;
}

std::vector<char const*> Window::requiredVulkanInstanceExtensions() const {
    uint32_t extensionCount;
    char const** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    return std::vector<char const*>(extensions, extensions + extensionCount);
}

void Window::framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->notifyResized();
}

void Window::windowCloseCallback(GLFWwindow* glfwWindow) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->notifyClosed();
}

void Window::notifyResized() { _resized = true; }

void Window::notifyClosed() { _closed = true; }

}  // namespace MelonFrontend

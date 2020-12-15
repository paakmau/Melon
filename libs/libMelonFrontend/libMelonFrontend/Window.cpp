#include <GLFW/glfw3.h>
#include <libMelonFrontend/Window.h>

namespace MelonFrontend {

void Window::initialize(char const* title, unsigned int const& width, unsigned int const& height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    m_Extent = VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    glfwSetWindowUserPointer(m_Window, this);

    // Callbacks
    glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
    glfwSetWindowCloseCallback(m_Window, windowCloseCallback);
}

void Window::terminate() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::waitForResized() {
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_Window, &width, &height);
        glfwWaitEvents();
    }
    m_Extent = VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    m_Resized = false;
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

void Window::notifyResized() { m_Resized = true; }

void Window::notifyClosed() { m_Closed = true; }

}  // namespace MelonFrontend

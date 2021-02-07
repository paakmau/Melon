#include <GLFW/glfw3.h>
#include <MelonFrontend/Window.h>

namespace Melon {

void Window::initialize(const char* title, const unsigned int& width, const unsigned int& height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    m_Extent = VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    glfwSetWindowUserPointer(m_Window, this);

    // Callbacks
    glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
    glfwSetWindowCloseCallback(m_Window, windowCloseCallback);
    glfwSetKeyCallback(m_Window, keyCallback);
    glfwSetCursorPosCallback(m_Window, cursorPosCallback);
    glfwSetMouseButtonCallback(m_Window, mouseButtonCallback);
    glfwSetScrollCallback(m_Window, scrollCallback);
}

void Window::terminate() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::pollEvents() {
    m_KeyDownEvents.clear();
    m_KeyUpEvents.clear();
    m_MouseButtonDownEvents.clear();
    m_MouseButtonUpEvents.clear();
    m_MouseScrollEvents.clear();
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

std::vector<const char*> Window::requiredVulkanInstanceExtensions() const {
    uint32_t extensionCount;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    return std::vector<const char*>(extensions, extensions + extensionCount);
}

void Window::framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->notifyResized();
}

void Window::windowCloseCallback(GLFWwindow* glfwWindow) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->notifyClosed();
}

void Window::keyCallback(GLFWwindow* glfwWindow, int key, int scanCode, int action, int mods) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    switch (action) {
        case GLFW_PRESS:
            window->m_KeyDownEvents.emplace_back(KeyDownEvent{.key = key});
            break;
        case GLFW_RELEASE:
            window->m_KeyUpEvents.emplace_back(KeyUpEvent{.key = key});
            break;
    }
}

void Window::cursorPosCallback(GLFWwindow* glfwWindow, double xPos, double yPos) {
    // TODO: Deal with cursor position Event
}

void Window::mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    switch (action) {
        case GLFW_PRESS:
            window->m_MouseButtonDownEvents.emplace_back(MouseButtonDownEvent{.button = button});
            break;
        case GLFW_RELEASE:
            window->m_MouseButtonUpEvents.emplace_back(MouseButtonUpEvent{.button = button});
            break;
    }
}

void Window::scrollCallback(GLFWwindow* glfwWindow, double xOffset, double yOffset) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->m_MouseScrollEvents.emplace_back(MouseScrollEvent{.xOffset = static_cast<float>(xOffset), .yOffset = static_cast<float>(yOffset)});
}

void Window::notifyResized() { m_Resized = true; }

void Window::notifyClosed() { m_Closed = true; }

}  // namespace Melon

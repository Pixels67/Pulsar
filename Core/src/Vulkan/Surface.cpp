#include "Surface.hpp"

namespace Pulsar::Vulkan {
    Surface Surface::Create(Instance &instance, const Glfw::Window &window) {
        Surface surface;

        GLFWwindow *glfwWindow = window.GetGlfwWindowPtr();
        if (glfwCreateWindowSurface(instance.GetVkInstance(), glfwWindow, nullptr, &surface.m_Surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to initialize window surface: Unknown error");
        }

        surface.m_Instance = &instance;
        return surface;
    }

    Surface::~Surface() {
        if (m_Surface != nullptr) {
            vkDestroySurfaceKHR(m_Instance->GetVkInstance(), m_Surface, nullptr);
            m_Surface = nullptr;
        }
    }

    VkSurfaceKHR Surface::GetVkSurface() const {
        return m_Surface;
    }
}
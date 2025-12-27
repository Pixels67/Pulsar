#ifndef PULSAR_SURFACE_HPP
#define PULSAR_SURFACE_HPP

#include "Instance.hpp"
#include "Glfw/Window.hpp"

namespace Pulsar::Vulkan {
    class Surface {
    public:
        static Surface Create(Instance &instance, const Glfw::Window &window);
        ~Surface();

        Surface(const Surface &other) = delete;
        Surface(Surface &&other) noexcept = default;

        Surface &operator=(const Surface &other) = delete;
        Surface &operator=(Surface &&other) noexcept = default;

        VkSurfaceKHR GetVkSurface() const;

    private:
        VkSurfaceKHR m_Surface = nullptr;
        Instance *m_Instance = nullptr;
        Glfw::Window *m_Window = nullptr;

        Surface() = default;
    };
}

#endif //PULSAR_SURFACE_HPP
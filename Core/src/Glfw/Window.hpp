#ifndef PULSAR_WINDOW_HPP
#define PULSAR_WINDOW_HPP

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GraphicsApi.hpp"

namespace Pulsar::Glfw {
    struct WindowConfig {
        std::string title = "Pulsar";
        uint16_t width = 800;
        uint16_t height = 600;
        GraphicsApi api = GraphicsApi::Vulkan;
    };

    void PollEvents();

    class Window {
    public:
        static Window Create(const WindowConfig &config = {});

        static Window *GetCurrent();

        ~Window();

        Window(const Window &other) = delete;

        Window(Window &&other) noexcept = default;

        Window &operator=(const Window &other) = delete;

        Window &operator=(Window &&other) noexcept = default;

        GLFWwindow *GetGlfwWindowPtr() const;

        void SetCurrent();

        bool ShouldClose() const;

        uint16_t GetWidth() const;

        uint16_t GetHeight() const;

        std::string GetTitle() const;

        void SetWidth(uint16_t value) const;

        void SetHeight(uint16_t value) const;

        void SetTitle(const std::string &value) const;

    private:
        inline static unsigned int s_WindowCount = 0;
        inline static Window *s_CurrentWindow = nullptr;

        GLFWwindow *m_GlfwWindowPtr = nullptr;
        WindowConfig m_Config = {};

        Window() = default;
    };
}

#endif //PULSAR_WINDOW_HPP

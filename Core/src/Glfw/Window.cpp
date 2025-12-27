#include "Window.hpp"

#include <stdexcept>
#include <string>

namespace Pulsar::Glfw {
    static constexpr int s_OpenGlVersionMajor = 3;
    static constexpr int s_OpenGlVersionMinor = 3;

    void PollEvents() {
        glfwPollEvents();
    }


    Window Window::Create(const WindowConfig &config) {
        if (s_WindowCount == 0) {
            if (glfwInit() == 0) {
                throw std::runtime_error("Failed to create window: Could not initialize GLFW");
            }
        }

        if (config.api == GraphicsApi::OpenGl) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, s_OpenGlVersionMajor);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, s_OpenGlVersionMinor);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        } else if (config.api == GraphicsApi::Vulkan) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        GLFWwindow *glfwWindowPtr = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr,
                                                     nullptr);

        if (glfwWindowPtr == nullptr) {
            if (s_WindowCount == 0) {
                glfwTerminate();
            }

            throw std::runtime_error("Failed to create window");
        }

        if (config.api == GraphicsApi::OpenGl) {
            GLFWwindow *currentWindow = glfwGetCurrentContext();

            glfwMakeContextCurrent(glfwWindowPtr);

            if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
                if (s_WindowCount == 0) {
                    glfwTerminate();
                    glfwDestroyWindow(glfwWindowPtr);
                }

                throw std::runtime_error("Failed to initialize GLAD");
            }

            glfwMakeContextCurrent(currentWindow);
        }

        s_WindowCount++;

        Window window;

        window.m_GlfwWindowPtr = glfwWindowPtr;
        window.m_Config        = config;

        return window;
    }

    Window *Window::GetCurrent() {
        return s_CurrentWindow;
    }

    Window::~Window() {
        if (m_GlfwWindowPtr == nullptr) {
            return;
        }

        s_WindowCount--;

        glfwDestroyWindow(m_GlfwWindowPtr);

        if (s_WindowCount == 0) {
            glfwTerminate();\
        }
    }

    GLFWwindow *Window::GetGlfwWindowPtr() const {
        return m_GlfwWindowPtr;
    }

    void Window::SetCurrent() {
        if (m_Config.api == GraphicsApi::OpenGl) {
            glfwMakeContextCurrent(m_GlfwWindowPtr);
        }

        s_CurrentWindow = this;
    }

    bool Window::ShouldClose() const {
        return glfwWindowShouldClose(m_GlfwWindowPtr);
    }

    uint16_t Window::GetWidth() const {
        int width = 0;

        glfwGetWindowSize(m_GlfwWindowPtr, &width, nullptr);
        return width;
    }

    uint16_t Window::GetHeight() const {
        int height = 0;

        glfwGetWindowSize(m_GlfwWindowPtr, nullptr, &height);
        return height;
    }

    std::string Window::GetTitle() const {
        return glfwGetWindowTitle(m_GlfwWindowPtr);
    }

    void Window::SetWidth(const uint16_t value) const {
        glfwSetWindowSize(m_GlfwWindowPtr, value, GetHeight());
    }

    void Window::SetHeight(const uint16_t value) const {
        glfwSetWindowSize(m_GlfwWindowPtr, GetWidth(), value);
    }

    void Window::SetTitle(const std::string &value) const {
        glfwSetWindowTitle(m_GlfwWindowPtr, value.c_str());
    }
}
#include "Glfw/Window.hpp"
#include "Vulkan/Instance.hpp"

int main() {
    Pulsar::Glfw::Window window = Pulsar::Glfw::Window::Create();
    window.SetCurrent();

    Pulsar::Vulkan::Instance instance = Pulsar::Vulkan::Instance::Create();

    while (!window.ShouldClose()) {
        Pulsar::Glfw::PollEvents();
    }
}

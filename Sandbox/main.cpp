#include <iostream>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "Vulkan/Instance.hpp"

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Window", nullptr, nullptr);

    Pulsar::Vulkan::Instance instance = Pulsar::Vulkan::Instance::Create();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

#include "Glfw/Window.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/ImageViews.hpp"
#include "Vulkan/Instance.hpp"
#include "Vulkan/Pipeline.hpp"
#include "Vulkan/Surface.hpp"
#include "Vulkan/SwapChain.hpp"

static constexpr auto s_VertShader = R"(
#version 450

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
)";

static constexpr auto s_FragShader = R"(
#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
)";

int main() {
    using namespace Pulsar;
    using namespace Pulsar::Vulkan;

    Glfw::Window window     = Glfw::Window::Create();
    Instance     instance   = Instance::Create();
    Surface      surface    = Surface::Create(instance, window);
    Device       device     = Device::Create(instance, surface);
    SwapChain    swapChain  = SwapChain::Create(surface, device, window);
    ImageViews   imageViews = ImageViews::Create(device, swapChain);
    Pipeline     pipeline   = Pipeline::Create(device, s_VertShader, s_FragShader);

    while (!window.ShouldClose()) {
        Glfw::PollEvents();
    }
}

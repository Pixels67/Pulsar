#ifndef PULSAR_COMMON_HPP
#define PULSAR_COMMON_HPP

#include <array>

#include <vulkan/vulkan_core.h>

#include "Version.hpp"

namespace Pulsar::Vulkan {
    constexpr auto g_EngineName = "Pulsar";
    constexpr Version g_EngineVersion = {0, 0, 1};
    constexpr uint32_t g_VulkanVersion = VK_API_VERSION_1_0;

    constexpr std::array g_DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    constexpr std::array g_ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifndef NDEBUG
    constexpr bool g_ValidationLayerEnabled = true;
#else
    constexpr bool g_ValidationLayerEnabled = false;
#endif
}

#endif //PULSAR_COMMON_HPP
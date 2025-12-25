#include "Instance.hpp"

#include <stdexcept>
#include <vector>

#include <GLFW/glfw3.h>

namespace Pulsar::Vulkan {
    static constexpr auto s_EngineName = "Pulsar";
    static constexpr Version s_EngineVersion = {0, 0, 1};

    Instance Instance::Create(const ApplicationInfo &info) {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = info.name.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(info.version.major, info.version.minor, info.version.hotfix);
        appInfo.pEngineName = s_EngineName;
        appInfo.engineVersion = VK_MAKE_VERSION(s_EngineVersion.major, s_EngineVersion.minor, s_EngineVersion.hotfix);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        if (glfwVulkanSupported() == GL_FALSE) {
            throw std::runtime_error("Failed to create Vulkan instance: Vulkan unsupported on this machine");
        }

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> requiredExtensions;

        requiredExtensions.reserve(glfwExtensionCount);
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            requiredExtensions.emplace_back(glfwExtensions[i]);
        }

#if __APPLE__
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = 0;

        Instance instance;
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance.m_Instance);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance: Unknown error");
        }

        return instance;
    }

    Instance::~Instance() {
        if (m_Instance != nullptr) {
            vkDestroyInstance(m_Instance, nullptr);
        }
    }

    Instance::Instance(Instance &&other) noexcept {
        m_Instance = other.m_Instance;
        other.m_Instance = nullptr;
    }

    Instance &Instance::operator=(Instance &&other) noexcept {
        if (m_Instance != nullptr) {
            vkDestroyInstance(m_Instance, nullptr);
        }

        m_Instance = other.m_Instance;
        other.m_Instance = nullptr;

        return *this;
    }
}

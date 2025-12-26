#include "Instance.hpp"

#include <array>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <GLFW/glfw3.h>

#include "Extensions.hpp"

namespace Pulsar::Vulkan {
    static constexpr auto s_EngineName = "Pulsar";
    static constexpr Version s_EngineVersion = {0, 0, 1};
    static constexpr uint32_t s_VulkanVersion = VK_API_VERSION_1_0;

    static constexpr std::array s_ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifndef NDEBUG
    static constexpr bool s_ValidationLayerEnabled = true;
#else
    static constexpr bool s_ValidationLayerEnabled = false;
#endif

    Instance Instance::Create(const ApplicationInfo &info) {
#ifndef NDEBUG
        uint32_t extensionCount;
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
        VkExtensionProperties *extensions = (VkExtensionProperties *)
                malloc(extensionCount * sizeof(VkExtensionProperties));
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

        std::cout << extensionCount << " extensions supported\n";
        std::cout << "=======================\n";
        for (uint32_t i = 0; i < extensionCount; i++) {
            std::cout << extensions[i].extensionName << '\n';
        }

        std::cout << "=======================\n";
#endif

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = info.name.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(info.version.major, info.version.minor, info.version.hotfix);
        appInfo.pEngineName = s_EngineName;
        appInfo.engineVersion = VK_MAKE_VERSION(s_EngineVersion.major, s_EngineVersion.minor, s_EngineVersion.hotfix);
        appInfo.apiVersion = s_VulkanVersion;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        if (glfwVulkanSupported() == GL_FALSE) {
            throw std::runtime_error("Failed to create Vulkan instance: Vulkan unsupported on this machine");
        }

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (s_ValidationLayerEnabled) {
            if (!IsValidationLayerSupported()) {
                throw std::runtime_error("Failed to create Vulkan instance: Validation layers not supported");
            }

            createInfo.enabledLayerCount = s_ValidationLayers.size();
            createInfo.ppEnabledLayerNames = s_ValidationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        std::vector<const char *> requiredExtensions = GetRequiredExtensions();

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

        instance.InitDebugMessenger();
        return instance;
    }

    void Instance::SetMessageCallback(const std::function<void(std::string)> &callback) {
        s_MessageCallback = callback;
    }

    Instance::~Instance() {
        if (m_Instance != nullptr) {
            DeinitDebugMessenger();
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

    void Instance::InitDebugMessenger() {
        if (!s_ValidationLayerEnabled) {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to initialize debug messenger: Unknown error");
        }
    }

    void Instance::DeinitDebugMessenger() const {
        if (m_DebugMessenger != nullptr) {
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }
    }

    VkBool32 Instance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                     void *pUserData) {
        if (s_MessageCallback) {
            s_MessageCallback.value()(pCallbackData->pMessage);
        } else {
            std::cout << pCallbackData->pMessage << '\n';
        }

        return VK_FALSE;
    }

    bool Instance::IsValidationLayerSupported() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName: s_ValidationLayers) {
            bool layerFound = false;

            for (const auto &layerProperties: availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    void Instance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;
    }

    std::vector<const char *> Instance::GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (s_ValidationLayerEnabled) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }
}

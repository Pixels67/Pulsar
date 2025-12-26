#include "Instance.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "Extensions.hpp"
#include "Shader.hpp"
#include "Glfw/Window.hpp"

namespace Pulsar::Vulkan {
    static constexpr auto s_EngineName = "Pulsar";
    static constexpr Version s_EngineVersion = {0, 0, 1};
    static constexpr uint32_t s_VulkanVersion = VK_API_VERSION_1_0;

    static constexpr std::array s_DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    static constexpr std::array s_ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifndef NDEBUG
    static constexpr bool s_ValidationLayerEnabled = true;
#else
    static constexpr bool s_ValidationLayerEnabled = false;
#endif

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

    Instance Instance::Create(const ApplicationInfo &info) {
#ifndef NDEBUG
        uint32_t extensionCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        auto *extensions = static_cast<VkExtensionProperties *>(malloc(extensionCount * sizeof(VkExtensionProperties)));
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);

        std::cout << extensionCount << " extensions supported\n";
        std::cout << "=======================\n";
        for (uint32_t i = 0; i < extensionCount; i++) {
            std::cout << extensions[i].extensionName << '\n';
        }

        std::cout << "=======================\n";

        free(extensions);
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
        if constexpr (s_ValidationLayerEnabled) {
            if (!AreValidationLayersSupported()) {
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
        instance.InitSurface();
        instance.SelectPhysicalDevice();
        instance.InitLogicalDevice();
        instance.InitSwapChain();
        instance.InitImageViews();
        instance.InitGraphicsPipeline();

        return instance;
    }

    void Instance::SetMessageCallback(const std::function<void(std::string)> &callback) {
        s_MessageCallback = callback;
    }

    Instance::~Instance() {
        if (m_Instance != nullptr) {
            DeinitGraphicsPipeline();
            DeinitImageViews();
            DeinitSwapChain();
            DeinitLogicalDevice();
            DeinitSurface();
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

    VkBool32 Instance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                     void *pUserData) {
        if (s_MessageCallback) {
            s_MessageCallback.value()(pCallbackData->pMessage);
        } else {
            std::cout << "[VK] " << pCallbackData->pMessage << '\n';
        }

        return VK_FALSE;
    }

    VkSurfaceFormatKHR Instance::SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR Instance::SelectSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
                return availablePresentMode;
            }
        }

        return availablePresentModes[0];
    }

    VkExtent2D Instance::SelectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(Glfw::Window::GetCurrent()->GetGlfwWindowPtr(), &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
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

    std::string Instance::GetDeviceName(const VkPhysicalDevice &device) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        return properties.deviceName;
    }

    bool Instance::AreValidationLayersSupported() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName : s_ValidationLayers) {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers) {
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

    bool Instance::AreDeviceExtensionsSupported(const VkPhysicalDevice &device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(s_DeviceExtensions.begin(), s_DeviceExtensions.end());

        for (const auto &[extensionName, specVersion] : availableExtensions) {
            requiredExtensions.erase(extensionName);
        }

        return requiredExtensions.empty();
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

    QueueFamilyIndices Instance::FindQueueFamilies(const VkPhysicalDevice &device) const {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        if (m_Surface == nullptr) {
            throw std::runtime_error("Failed to find queue families: Surface not initialized");
        }

        int i = 0;
        for (const auto &queueFamily : queueFamilies) {
            if (indices.IsValid()) {
                break;
            }

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            i++;
        }

        return indices;
    }

    SwapChainSupportInfo Instance::QuerySwapChainSupport(const VkPhysicalDevice &device) const {
        SwapChainSupportInfo info;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &info.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

        if (formatCount != 0) {
            info.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, info.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            info.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, info.presentModes.data());
        }

        return info;
    }

    VkShaderModule Instance::CreateShaderModule(const ShaderType type, const std::string &source) const {
        const std::vector<uint32_t> spirv = CompileShader(type, source);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = spirv.size();
        createInfo.pCode = spirv.data();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_LogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module: Unknown error");
        }

        return shaderModule;
    }

    // Returns 0 when the device is not supported
    uint16_t Instance::RateDevice(const VkPhysicalDevice &device) const {
        if (!FindQueueFamilies(device).IsValid()) {
            return 0;
        }

        if (!AreDeviceExtensionsSupported(device)) {
            return 0;
        }

        const SwapChainSupportInfo swapChainSupport = QuerySwapChainSupport(device);
        if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
            return 0;
        }

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        uint16_t score = 0;

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 32;
        } else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            score += 1;
        }

        score += properties.limits.maxImageDimension2D / 1024;

        return score;
    }

    void Instance::InitDebugMessenger() {
        if constexpr (!s_ValidationLayerEnabled) {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to initialize debug messenger: Unknown error");
        }
    }

    void Instance::DeinitDebugMessenger() {
        if (m_DebugMessenger != nullptr) {
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
            m_DebugMessenger = nullptr;
        }
    }

    void Instance::SelectPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to select physical device: No device found with vulkan support");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        for (const auto &device : devices) {
            if (RateDevice(device) != 0 &&
                (m_PhysicalDevice == nullptr || RateDevice(device) > RateDevice(m_PhysicalDevice))) {
                m_PhysicalDevice = device;
                break;
            }
        }

        if (m_PhysicalDevice == nullptr) {
            throw std::runtime_error("Failed to select physical device: No suitable device found");
        }

        std::cout << "[PS] Selected physical device: " << GetDeviceName(m_PhysicalDevice) << '\n';
    }

    void Instance::InitLogicalDevice() {
        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(s_DeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = s_DeviceExtensions.data();

        if (s_ValidationLayerEnabled) {
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(s_ValidationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = s_ValidationLayers.data();
        } else {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

        std::set uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        float queuePriority = 1.0F;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

        if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device: Unknown error");
        }

        vkGetDeviceQueue(m_LogicalDevice, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &m_PresentQueue);

        std::cout << "[PS] " << "Initialized logical device successfully\n";
    }

    void Instance::DeinitLogicalDevice() {
        if (m_LogicalDevice != nullptr) {
            vkDestroyDevice(m_LogicalDevice, nullptr);
            m_LogicalDevice = nullptr;
        }
    }

    void Instance::InitSurface() {
        GLFWwindow *currentWindow = Glfw::Window::GetCurrent()->GetGlfwWindowPtr();
        if (glfwCreateWindowSurface(m_Instance, currentWindow, nullptr, &m_Surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to initialize window surface: Unknown error");
        }
    }

    void Instance::DeinitSurface() {
        if (m_Surface != nullptr) {
            vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
            m_Surface = nullptr;
        }
    }

    void Instance::InitSwapChain() {
        auto [capabilities, formats, presentModes] = QuerySwapChainSupport(m_PhysicalDevice);

        VkSurfaceFormatKHR surfaceFormat = SelectSwapSurfaceFormat(formats);
        VkPresentModeKHR presentMode = SelectSwapPresentMode(presentModes);
        VkExtent2D extent = SelectSwapExtent(capabilities);

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto [graphicsFamily, presentFamily] = FindQueueFamilies(m_PhysicalDevice);
        const uint32_t queueFamilyIndices[] = {graphicsFamily.value(), presentFamily.value()};

        if (graphicsFamily != presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = nullptr;

        if (vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to initialize swap chain: Unknown error");
        }

        vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, nullptr);
        m_SwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, m_SwapChainImages.data());

        m_SwapChainExtent = extent;
        m_SwapChainImageFormat = surfaceFormat.format;
    }

    void Instance::DeinitSwapChain() {
        if (m_SwapChain != nullptr) {
            vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);
            m_SwapChain = nullptr;
        }
    }

    void Instance::InitImageViews() {
        m_SwapChainImageViews.resize(m_SwapChainImages.size());

        for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_SwapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_SwapChainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_LogicalDevice, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image views: Unknown error");
            }
        }
    }

    void Instance::DeinitImageViews() {
        for (const auto &imageView : m_SwapChainImageViews) {
            vkDestroyImageView(m_LogicalDevice, imageView, nullptr);
        }

        m_SwapChainImageViews.clear();
    }

    void Instance::InitGraphicsPipeline() {
        VkShaderModule vertShaderModule = CreateShaderModule(ShaderType::Vertex, s_VertShader);
        VkShaderModule fragShaderModule = CreateShaderModule(ShaderType::Fragment, s_FragShader);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        vertShaderStageInfo.module = fragShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        vkDestroyShaderModule(m_LogicalDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(m_LogicalDevice, vertShaderModule, nullptr);
    }

    void Instance::DeinitGraphicsPipeline() {
        if (m_GraphicsPipeline != nullptr) {
            vkDestroyPipeline(m_LogicalDevice, m_GraphicsPipeline, nullptr);
            m_GraphicsPipeline = nullptr;
        }
    }
}

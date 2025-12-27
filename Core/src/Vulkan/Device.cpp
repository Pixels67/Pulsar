#include "Device.hpp"

#include "Common.hpp"

namespace Pulsar::Vulkan {
    Device Device::Create(Instance &instance, Surface &surface) {
        Device device;
        device.m_Instance = &instance;
        device.m_Surface  = &surface;

        device.SelectPhysicalDevice();

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pEnabledFeatures     = &deviceFeatures;

        deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(g_DeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = g_DeviceExtensions.data();

        if (g_ValidationLayerEnabled) {
            deviceCreateInfo.enabledLayerCount   = static_cast<uint32_t>(g_ValidationLayers.size());
            deviceCreateInfo.ppEnabledLayerNames = g_ValidationLayers.data();
        } else {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        auto [graphicsFamily, presentFamily] = FindQueueFamilies(device.m_PhysicalDevice, *device.m_Surface);

        std::set                             uniqueQueueFamilies = {graphicsFamily.value(), presentFamily.value()};
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        float queuePriority = 1.0F;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount       = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
        deviceCreateInfo.pQueueCreateInfos    = queueCreateInfos.data();

        if (vkCreateDevice(device.m_PhysicalDevice, &deviceCreateInfo, nullptr, &device.m_LogicalDevice) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device: Unknown error");
        }

        vkGetDeviceQueue(device.m_LogicalDevice, graphicsFamily.value(), 0, &device.m_GraphicsQueue);
        vkGetDeviceQueue(device.m_LogicalDevice, presentFamily.value(), 0, &device.m_PresentQueue);

        std::cout << "[PS] " << "Initialized logical device successfully\n";

        return device;
    }

    Device::~Device() {
        if (m_LogicalDevice != nullptr) {
            vkDestroyDevice(m_LogicalDevice, nullptr);
            m_LogicalDevice = nullptr;
        }
    }

    QueueFamilyIndices Device::FindQueueFamilies() const {
        return FindQueueFamilies(m_PhysicalDevice, *m_Surface);
    }

    SwapChainSupportInfo Device::QuerySwapChainSupport() const {
        return QuerySwapChainSupport(m_PhysicalDevice, *m_Surface);
    }

    uint16_t Device::RateDevice() const {
        return RateDevice(m_PhysicalDevice, *m_Surface);
    }

    bool Device::AreDeviceExtensionsSupported() const {
        return AreDeviceExtensionsSupported(m_PhysicalDevice);
    }

    VkPhysicalDevice Device::GetVkPhysicalDevice() const {
        return m_PhysicalDevice;
    }

    VkDevice Device::GetVkLogicalDevice() const {
        return m_LogicalDevice;
    }

    VkQueue Device::GetVkGraphicsQueue() const {
        return m_GraphicsQueue;
    }

    VkQueue Device::GetVkPresentQueue() const {
        return m_PresentQueue;
    }

    QueueFamilyIndices Device::FindQueueFamilies(const VkPhysicalDevice &device, const Surface &surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        if (surface.GetVkSurface() == nullptr) {
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
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface.GetVkSurface(), &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            i++;
        }

        return indices;
    }

    SwapChainSupportInfo Device::QuerySwapChainSupport(const VkPhysicalDevice &device, Surface &surface) {
        SwapChainSupportInfo info;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.GetVkSurface(), &info.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.GetVkSurface(), &formatCount, nullptr);

        if (formatCount != 0) {
            info.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.GetVkSurface(), &formatCount, info.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.GetVkSurface(), &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            info.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.GetVkSurface(), &presentModeCount,
                                                      info.presentModes.data());
        }

        return info;
    }

    uint16_t Device::RateDevice(const VkPhysicalDevice &device, Surface &surface) {
        if (!FindQueueFamilies(device, surface).IsValid()) {
            return 0;
        }

        if (!AreDeviceExtensionsSupported(device)) {
            return 0;
        }

        const SwapChainSupportInfo swapChainSupport = QuerySwapChainSupport(device, surface);
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

    bool Device::AreDeviceExtensionsSupported(const VkPhysicalDevice &device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(g_DeviceExtensions.begin(), g_DeviceExtensions.end());

        for (const auto &[extensionName, specVersion] : availableExtensions) {
            requiredExtensions.erase(extensionName);
        }

        return requiredExtensions.empty();
    }

    void Device::SelectPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance->GetVkInstance(), &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to select physical device: No device found with vulkan support");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance->GetVkInstance(), &deviceCount, devices.data());

        for (const auto &device : devices) {
            if (RateDevice(device, *m_Surface) != 0 &&
                (m_PhysicalDevice == nullptr || RateDevice(device, *m_Surface) >
                    RateDevice(m_PhysicalDevice, *m_Surface))) {
                m_PhysicalDevice = device;
                break;
            }
        }

        if (m_PhysicalDevice == nullptr) {
            throw std::runtime_error("Failed to select physical device: No suitable device found");
        }
    }
}
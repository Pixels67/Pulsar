#include "SwapChain.hpp"

namespace Pulsar::Vulkan {
    SwapChain SwapChain::Create(const Surface &surface, Device &device, const Glfw::Window &window) {
        auto [capabilities, formats, presentModes] = device.QuerySwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = SelectSwapSurfaceFormat(formats);
        VkPresentModeKHR   presentMode   = SelectSwapPresentMode(presentModes);
        VkExtent2D         extent        = SelectSwapExtent(capabilities, window);

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface          = surface.GetVkSurface();
        createInfo.minImageCount    = imageCount;
        createInfo.imageFormat      = surfaceFormat.format;
        createInfo.imageColorSpace  = surfaceFormat.colorSpace;
        createInfo.imageExtent      = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto           [graphicsFamily, presentFamily] = device.FindQueueFamilies();
        const uint32_t queueFamilyIndices[]            = {graphicsFamily.value(), presentFamily.value()};

        if (graphicsFamily != presentFamily) {
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform   = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode    = presentMode;
        createInfo.clipped        = VK_TRUE;
        createInfo.oldSwapchain   = nullptr;

        SwapChain swapChain{};
        swapChain.m_Device = &device;

        if (vkCreateSwapchainKHR(device.GetVkLogicalDevice(), &createInfo, nullptr, &swapChain.m_SwapChain) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to initialize swap chain: Unknown error");
        }

        vkGetSwapchainImagesKHR(device.GetVkLogicalDevice(), swapChain.m_SwapChain, &imageCount, nullptr);
        swapChain.m_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(device.GetVkLogicalDevice(), swapChain.m_SwapChain, &imageCount,
                                swapChain.m_Images.data());

        swapChain.m_SwapChainExtent = extent;
        swapChain.m_ImageFormat     = surfaceFormat.format;

        return swapChain;
    }

    SwapChain::~SwapChain() {
        if (m_SwapChain != nullptr) {
            vkDestroySwapchainKHR(m_Device->GetVkLogicalDevice(), m_SwapChain, nullptr);
            m_SwapChain = nullptr;
        }
    }

    VkSwapchainKHR SwapChain::GetVkSwapChain() const {
        return m_SwapChain;
    }

    std::vector<VkImage> SwapChain::GetVkImages() const {
        return m_Images;
    }

    VkFormat SwapChain::GetVkImageFormat() const {
        return m_ImageFormat;
    }

    VkSurfaceFormatKHR SwapChain::SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::SelectSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
                return availablePresentMode;
            }
        }

        return availablePresentModes[0];
    }

    VkExtent2D SwapChain::SelectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, const Glfw::Window &window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(window.GetGlfwWindowPtr(), &width, &height);

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
}
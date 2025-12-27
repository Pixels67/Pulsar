#include "ImageViews.hpp"

namespace Pulsar::Vulkan {
    ImageViews ImageViews::Create(Device &device, const SwapChain &swapChain) {
        ImageViews imageViews;
        imageViews.m_Device = &device;

        imageViews.m_SwapChainImageViews.resize(swapChain.GetVkImages().size());

        for (size_t i = 0; i < swapChain.GetVkImages().size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image    = swapChain.GetVkImages()[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format   = swapChain.GetVkImageFormat();

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel   = 0;
            createInfo.subresourceRange.levelCount     = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount     = 1;

            if (vkCreateImageView(device.GetVkLogicalDevice(), &createInfo, nullptr,
                                  &imageViews.m_SwapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image views: Unknown error");
            }
        }

        return imageViews;
    }

    ImageViews::~ImageViews() {
        for (const auto &imageView : m_SwapChainImageViews) {
            vkDestroyImageView(m_Device->GetVkLogicalDevice(), imageView, nullptr);
        }

        m_SwapChainImageViews.clear();
    }

    std::vector<VkImageView> ImageViews::GetVkImageViews() const {
        return m_SwapChainImageViews;
    }
}
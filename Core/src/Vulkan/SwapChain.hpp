#ifndef PULSAR_SWAPCHAIN_HPP
#define PULSAR_SWAPCHAIN_HPP

#include <vulkan/vulkan_core.h>

#include "Device.hpp"

namespace Pulsar::Vulkan {
    class SwapChain {
    public:
        static SwapChain Create(const Surface &surface, Device &device, const Glfw::Window &window);
        ~SwapChain();

        SwapChain(const SwapChain &other)     = delete;
        SwapChain(SwapChain &&other) noexcept = default;

        SwapChain &operator=(const SwapChain &other)     = delete;
        SwapChain &operator=(SwapChain &&other) noexcept = default;

        [[nodiscard]] VkSwapchainKHR       GetVkSwapChain() const;
        [[nodiscard]] std::vector<VkImage> GetVkImages() const;
        [[nodiscard]] VkFormat             GetVkImageFormat() const;

    private:
        VkSwapchainKHR       m_SwapChain;
        std::vector<VkImage> m_Images;
        VkFormat             m_ImageFormat{};
        VkExtent2D           m_SwapChainExtent{};
        Device *             m_Device;

        [[nodiscard]] static VkSurfaceFormatKHR SelectSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats);
        [[nodiscard]] static VkPresentModeKHR SelectSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes);
        [[nodiscard]] static VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                                                         const Glfw::Window &            window);

        SwapChain() = default;
    };
}

#endif //PULSAR_SWAPCHAIN_HPP
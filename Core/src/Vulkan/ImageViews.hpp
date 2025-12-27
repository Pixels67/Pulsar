#ifndef PULSAR_IMAGEVIEWS_HPP
#define PULSAR_IMAGEVIEWS_HPP

#include "Device.hpp"
#include "SwapChain.hpp"

namespace Pulsar::Vulkan {
    class ImageViews {
    public:
        static ImageViews Create(Device &device, const SwapChain &swapChain);
        ~ImageViews();

        ImageViews(const ImageViews &other)     = delete;
        ImageViews(ImageViews &&other) noexcept = default;

        ImageViews &operator=(const ImageViews &other)     = delete;
        ImageViews &operator=(ImageViews &&other) noexcept = default;

        [[nodiscard]] std::vector<VkImageView> GetVkImageViews() const;

    private:
        std::vector<VkImageView> m_SwapChainImageViews;
        Device *                 m_Device = nullptr;

        ImageViews() = default;
    };
}

#endif //PULSAR_IMAGEVIEWS_HPP
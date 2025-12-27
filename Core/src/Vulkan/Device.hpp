#ifndef PULSAR_DEVICE_HPP
#define PULSAR_DEVICE_HPP

#include "Instance.hpp"
#include "Surface.hpp"

namespace Pulsar::Vulkan {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily = std::nullopt;
        std::optional<uint32_t> presentFamily = std::nullopt;

        [[nodiscard]] bool IsValid() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportInfo {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> presentModes{};
    };

    class Device {
    public:
        static Device Create(Instance &instance, Surface &surface);

        [[nodiscard]] static QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice &device,
                                                                  const Surface &surface);
        [[nodiscard]] static SwapChainSupportInfo QuerySwapChainSupport(const VkPhysicalDevice &device,
                                                                        Surface &surface);
        [[nodiscard]] static uint16_t RateDevice(const VkPhysicalDevice &device, Surface &surface);
        [[nodiscard]] static bool AreDeviceExtensionsSupported(const VkPhysicalDevice &device);

        ~Device();

        Device(const Device &other) = delete;
        Device(Device &&other) noexcept = default;

        Device &operator=(const Device &other) = delete;
        Device &operator=(Device &&other) noexcept = default;

        [[nodiscard]] QueueFamilyIndices FindQueueFamilies() const;
        [[nodiscard]] SwapChainSupportInfo QuerySwapChainSupport() const;
        [[nodiscard]] uint16_t RateDevice() const;
        [[nodiscard]] bool AreDeviceExtensionsSupported() const;

        [[nodiscard]] VkPhysicalDevice GetVkPhysicalDevice() const;
        [[nodiscard]] VkDevice GetVkLogicalDevice() const;
        [[nodiscard]] VkQueue GetVkGraphicsQueue() const;
        [[nodiscard]] VkQueue GetVkPresentQueue() const;

    private:
        VkPhysicalDevice m_PhysicalDevice = nullptr;
        VkDevice m_LogicalDevice = nullptr;
        VkQueue m_GraphicsQueue = nullptr;
        VkQueue m_PresentQueue = nullptr;
        Instance *m_Instance = nullptr;
        Surface *m_Surface = nullptr;

        Device() = default;

        void SelectPhysicalDevice();
    };
}

#endif //PULSAR_DEVICE_HPP
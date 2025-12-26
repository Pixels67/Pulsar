#ifndef PULSAR_INSTANCE_HPP
#define PULSAR_INSTANCE_HPP

#include <functional>
#include <string>
#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

#include "Version.hpp"

namespace Pulsar::Vulkan {
    struct ApplicationInfo {
        std::string name = "Vulkan";
        Version version = {1, 0, 0};
    };

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

    class Instance {
    public:
        static Instance Create(const ApplicationInfo &info = {});
        static void SetMessageCallback(const std::function<void(std::string)> &callback);

        ~Instance();

        Instance(const Instance &other) = delete;
        Instance(Instance &&other) noexcept;

        Instance &operator=(const Instance &other) = delete;
        Instance &operator=(Instance &&other) noexcept;

    private:
        inline static std::optional<std::function<void(std::string)>> s_MessageCallback = std::nullopt;

        VkInstance m_Instance = nullptr;
        VkSurfaceKHR m_Surface = nullptr;
        VkDebugUtilsMessengerEXT m_DebugMessenger = nullptr;
        VkPhysicalDevice m_PhysicalDevice = nullptr;
        VkDevice m_LogicalDevice = nullptr;
        VkQueue m_GraphicsQueue = nullptr;
        VkQueue m_PresentQueue = nullptr;
        VkSwapchainKHR m_SwapChain = nullptr;
        std::vector<VkImage> m_SwapChainImages;
        VkFormat m_SwapChainImageFormat{};
        VkExtent2D m_SwapChainExtent{};
        std::vector<VkImageView> m_SwapChainImageViews;

        Instance() = default;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);

        static VkSurfaceFormatKHR SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        static VkPresentModeKHR SelectSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        static VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        static std::vector<const char *> GetRequiredExtensions();
        static std::string GetDeviceName(const VkPhysicalDevice &device);

        static bool AreValidationLayersSupported();
        static bool AreDeviceExtensionsSupported(const VkPhysicalDevice &device);

        static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        [[nodiscard]] QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice &device) const;
        [[nodiscard]] SwapChainSupportInfo QuerySwapChainSupport(const VkPhysicalDevice &device) const;
        [[nodiscard]] uint16_t RateDevice(const VkPhysicalDevice &device) const;

        void InitDebugMessenger();
        void DeinitDebugMessenger();

        void SelectPhysicalDevice();

        void InitLogicalDevice();
        void DeinitLogicalDevice();

        void InitSurface();
        void DeinitSurface();

        void InitSwapChain();
        void DeinitSwapChain();

        void InitImageViews();
        void DeinitImageViews();
    };
} //Pulsar::Vulkan

#endif //PULSAR_INSTANCE_HPP

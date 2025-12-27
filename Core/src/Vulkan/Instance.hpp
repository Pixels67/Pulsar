#ifndef PULSAR_INSTANCE_HPP
#define PULSAR_INSTANCE_HPP

#include <vulkan/vulkan.h>

#include "Version.hpp"

namespace Pulsar::Vulkan {
    struct ApplicationInfo {
        std::string name    = "Vulkan";
        Version     version = {1, 0, 0};
    };

    class Instance {
    public:
        static Instance Create(const ApplicationInfo &info = {});
        static void     SetMessageCallback(const std::function<void(std::string)> &callback);

        ~Instance();

        Instance(const Instance &other) = delete;
        Instance(Instance &&other) noexcept;

        Instance &operator=(const Instance &other) = delete;
        Instance &operator=(Instance &&other) noexcept;

        [[nodiscard]] VkInstance GetVkInstance() const;

    private:
        inline static std::optional<std::function<void(std::string)>> s_MessageCallback = std::nullopt;

        VkInstance               m_Instance       = nullptr;
        VkDebugUtilsMessengerEXT m_DebugMessenger = nullptr;

        Instance() = default;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT             messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *                                      pUserData);

        [[nodiscard]] static std::vector<const char *> GetRequiredExtensions();
        [[nodiscard]] static bool                      AreValidationLayersSupported();

        static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        void InitDebugMessenger();
        void DeinitDebugMessenger();
    };
} //Pulsar::Vulkan

#endif //PULSAR_INSTANCE_HPP
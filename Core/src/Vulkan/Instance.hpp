#ifndef PULSAR_INSTANCE_H
#define PULSAR_INSTANCE_H

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
        VkInstance m_Instance = nullptr;
        VkDebugUtilsMessengerEXT m_DebugMessenger = nullptr;
        inline static std::optional<std::function<void(std::string)> > s_MessageCallback = std::nullopt;

        Instance() = default;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);

        static bool IsValidationLayerSupported();

        static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        static std::vector<const char *> GetRequiredExtensions();

        void InitDebugMessenger();

        void DeinitDebugMessenger() const;
    };
} //Pulsar::Vulkan

#endif //PULSAR_INSTANCE_H

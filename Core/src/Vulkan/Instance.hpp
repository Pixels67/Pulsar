#ifndef PULSAR_INSTANCE_H
#define PULSAR_INSTANCE_H

#include <string>

#include <vulkan/vulkan.h>

#include "Version.hpp"

namespace Pulsar::Vulkan {
    struct ApplicationInfo {
        std::string name = "Vulkan";
        Version version = {1, 0, 0};
    };

    class Instance {
    public:
        static Instance Create(const ApplicationInfo& info = {});
        ~Instance();

        Instance(const Instance &other) = delete;
        Instance(Instance &&other) noexcept;

        Instance &operator=(const Instance &other) = delete;
        Instance &operator=(Instance &&other) noexcept;

    private:
        VkInstance m_Instance = nullptr;

        Instance() = default;
    };
} //Pulsar::Vulkan

#endif //PULSAR_INSTANCE_H

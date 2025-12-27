#ifndef PULSAR_PIPELINE_HPP
#define PULSAR_PIPELINE_HPP

#include "Device.hpp"
#include "Shader.hpp"

namespace Pulsar::Vulkan {
    class Pipeline {
    public:
        static Pipeline Create(Device &device, const std::string &vertexShader, const std::string &fragmentShader);
        ~Pipeline();

        Pipeline(const Pipeline &other)     = delete;
        Pipeline(Pipeline &&other) noexcept = default;

        Pipeline &operator=(const Pipeline &other)     = delete;
        Pipeline &operator=(Pipeline &&other) noexcept = default;

        [[nodiscard]] VkPipeline GetVkPipeline() const;

    private:
        VkPipeline m_Pipeline = nullptr;
        Device *   m_Device   = nullptr;

        Pipeline() = default;

        [[nodiscard]] VkShaderModule CreateShaderModule(ShaderType type, const std::string &source) const;
    };
}

#endif //PULSAR_PIPELINE_HPP
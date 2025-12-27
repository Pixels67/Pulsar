#ifndef PULSAR_SHADER_HPP
#define PULSAR_SHADER_HPP

namespace Pulsar::Vulkan {
    enum class ShaderType : uint8_t {
        Vertex,
        Fragment
    };

    std::vector<uint32_t> CompileShader(ShaderType type, const std::string &source);
}

#endif //PULSAR_SHADER_HPP
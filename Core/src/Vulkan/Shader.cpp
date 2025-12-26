#include "Shader.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <shaderc/shaderc.hpp>

namespace Pulsar::Vulkan {
    std::vector<uint32_t> CompileShader(const ShaderType type, const std::string &source) {
        const shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
        options.SetTargetSpirv(shaderc_spirv_version_1_0);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        shaderc_shader_kind shaderType = {};

        switch (type) {
        case ShaderType::Vertex:
            shaderType = shaderc_glsl_vertex_shader;
            break;
        case ShaderType::Fragment:
            shaderType = shaderc_glsl_fragment_shader;
            break;
        }

        const shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
            source,
            shaderType,
            "UNSPECIFIED",
            options
        );

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error(result.GetErrorMessage());\
        }

        std::vector spirv(result.cbegin(), result.cend());

        return spirv;
    }
}

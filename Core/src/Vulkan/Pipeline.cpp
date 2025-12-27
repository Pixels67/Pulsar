#include "Pipeline.hpp"

namespace Pulsar::Vulkan {
    Pipeline Pipeline::Create(Device &device, const std::string &vertexShader, const std::string &fragmentShader) {
        Pipeline pipeline;
        pipeline.m_Device = &device;

        VkShaderModule vertShaderModule = pipeline.CreateShaderModule(ShaderType::Vertex, vertexShader);
        VkShaderModule fragShaderModule = pipeline.CreateShaderModule(ShaderType::Fragment, fragmentShader);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName  = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        vertShaderStageInfo.module = fragShaderModule;
        vertShaderStageInfo.pName  = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        vkDestroyShaderModule(device.GetVkLogicalDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(device.GetVkLogicalDevice(), vertShaderModule, nullptr);

        return pipeline;
    }

    Pipeline::~Pipeline() {
        if (m_Pipeline != nullptr) {
            vkDestroyPipeline(m_Device->GetVkLogicalDevice(), m_Pipeline, nullptr);
            m_Pipeline = nullptr;
        }
    }

    VkPipeline Pipeline::GetVkPipeline() const {
        return m_Pipeline;
    }

    VkShaderModule Pipeline::CreateShaderModule(const ShaderType type, const std::string &source) const {
        const std::vector<uint32_t> spirv = CompileShader(type, source);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = spirv.size();
        createInfo.pCode    = spirv.data();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_Device->GetVkLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module: Unknown error");
        }

        return shaderModule;
    }
}
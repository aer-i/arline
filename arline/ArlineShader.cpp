#include "ArlineShader.hpp"
#include <format>
#include <fstream>

arline::Shader::Shader(std::filesystem::path const& path) noexcept
    : m{}
{
    if (!std::filesystem::exists(path))
    {
        VkContext::Get()->errorCallback(std::format("File not found: {}", path.string()));
        return;
    }

    VkShaderStageFlagBits stage;
    if (path.string().ends_with(".vert.spv"))
    {
        stage = VK_SHADER_STAGE_VERTEX_BIT;
    }
    else if (path.string().ends_with(".frag.spv"))
    {
        stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    else if (path.string().ends_with(".comp.spv"))
    {
        stage = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    else
    {
        VkContext::Get()->errorCallback(std::format("Unsupported file format: {}", path.string()));
        return;
    }

    auto file{ std::ifstream{path, std::ios::ate | std::ios::binary} };

    if (!file)
    {
        VkContext::Get()->errorCallback(std::format("Can't open file: {}", path.string()));
        return;
    }

    auto fileSize{ static_cast<std::streamsize>(file.tellg()) };
    auto buffer{ std::vector<c8>(fileSize) };

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    m.shaderStage = VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage,
        .module = VkContext::CreateShaderModule(buffer),
        .pName = "main"
    };
}

arline::Shader::~Shader() noexcept
{
    vkDestroyShaderModule(VkContext::Get()->device, m.shaderStage.module, nullptr);
}

arline::Shader::Shader(Shader&& other) noexcept
    : m{ other.m }
{
    other.m = {};
}

auto arline::Shader::operator=(Shader&& other) noexcept -> Shader&
{
    vkDestroyShaderModule(VkContext::Get()->device, m.shaderStage.module, nullptr);

    this->m = other.m;
    other.m = {};

    return *this;
}
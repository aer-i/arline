#include "ArlineShader.hpp"
#include <format>
#include <fstream>
#include <cstring>

arline::Shader::Shader(std::filesystem::path const& path, SpecializationInfo const& specializationInfo) noexcept
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
        .module = VkContext::CreateShaderModule(buffer.data(), buffer.size()),
        .pName = "main"
    };

    if (specializationInfo.entries.size() > 0)
    {
        m.mapEntries.reserve(specializationInfo.entries.size());

        for (auto const& entry : specializationInfo.entries)
        {
            m.mapEntries.emplace_back(VkSpecializationMapEntry{
                .constantID = entry.id,
                .offset = entry.offset,
                .size = entry.size
            });
        }

        m.specializationInfo = {
            .mapEntryCount = static_cast<u32>(m.mapEntries.size()),
            .pMapEntries = m.mapEntries.data(),
            .dataSize = specializationInfo.dataSize,
            .pData = specializationInfo.pData
        };

        m.shaderStage.pSpecializationInfo = &m.specializationInfo;
    }
}

arline::Shader::Shader(v0 const* pCode, u64 size, ShaderStage stage, SpecializationInfo const& specializationInfo) noexcept
{
    VkShaderStageFlagBits constexpr stages[] = { 
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        VK_SHADER_STAGE_COMPUTE_BIT
    };

    auto selectedStage{ stages[static_cast<u32>(stage)] };

    m.shaderStage = VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = selectedStage,
        .module = VkContext::CreateShaderModule(pCode, size),
        .pName = "main"
    };

    if (specializationInfo.entries.size() > 0)
    {
        m.mapEntries.reserve(specializationInfo.entries.size());

        for (auto const& entry : specializationInfo.entries)
        {
            m.mapEntries.emplace_back(VkSpecializationMapEntry{
                .constantID = entry.id,
                .offset = entry.offset,
                .size = entry.size
            });
        }

        m.specializationInfo = {
            .mapEntryCount = static_cast<u32>(m.mapEntries.size()),
            .pMapEntries = m.mapEntries.data(),
            .dataSize = specializationInfo.dataSize,
            .pData = specializationInfo.pData
        };

        m.shaderStage.pSpecializationInfo = &m.specializationInfo;
    }
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
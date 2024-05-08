#pragma once
#include "ArlineVkContext.hpp"
#include <filesystem>

namespace arline
{
    struct EntryInfo
    {
        u32 id;
        u32 offset;
        u64 size;
    };

    struct SpecializationInfo
    {
        v0 const* pData;
        u64 dataSize;
        std::initializer_list<EntryInfo> entries;
    };

    class Shader
    {
    public:
        Shader() = delete;
        Shader(std::filesystem::path const& path, SpecializationInfo const& specializationInfo = {}) noexcept;
        ~Shader() noexcept;
        Shader(Shader const&) = delete;
        Shader(Shader&& other) noexcept;
        auto operator=(Shader const&) -> Shader& = delete;
        auto operator=(Shader&& other) noexcept -> Shader&;

    public:
        inline operator VkPipelineShaderStageCreateInfo() const noexcept { return m.shaderStage; }

    private:
        struct Members
        {
            std::vector<VkSpecializationMapEntry> mapEntries;
            VkSpecializationInfo specializationInfo;
            VkPipelineShaderStageCreateInfo shaderStage;
        } m;
    };
}
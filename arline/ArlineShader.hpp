#pragma once
#include "ArlineVkContext.hpp"
#include <filesystem>

namespace arline
{
    class Shader
    {
    public:
        Shader() = delete;
        Shader(std::filesystem::path const& path) noexcept;
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
            VkPipelineShaderStageCreateInfo shaderStage;
        } m;
    };
}
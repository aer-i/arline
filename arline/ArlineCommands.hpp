#pragma once
#include "ArlineTypes.hpp"
#include "ArlineVkContext.hpp"
#include "ArlinePipeline.hpp"

namespace arline
{
    struct Color
    {
        u8 r, g, b, a;
    };

    enum class LoadOp : u8
    {
        eLoad     = 0x00000000,
        eClear    = 0x00000001,
        eDontCare = 0x00000002
    };

    enum class StoreOp : u8
    {
        eStore    = 0x00000000,
        eDontCare = 0x00000001
    };

    struct RenderPass
    {
        Color color;
        LoadOp loadOp;
        StoreOp storeOp;
    };

    class Commands
    {
    public:
        Commands() = default;
        ~Commands() = default;
        Commands(Commands const&) = default;
        Commands(Commands&&) = default;
        auto operator=(Commands const&) -> Commands& = default;
        auto operator=(Commands&&) -> Commands& = default;

    public:
        inline auto begin() noexcept -> v0;
        inline auto end() noexcept -> v0;
        inline auto beginPresent(RenderPass renderPass = {}) const noexcept -> v0;
        inline auto endPresent() const noexcept -> v0;
        inline auto setScissor(i32 x, i32 y, u32 width, u32 height) const noexcept -> v0;
        inline auto bindPipeline(Pipeline const& pipeline) const noexcept -> v0;
        inline auto draw(u32 vertexCount, u32 instanceCount = 1, u32 vertex = 0, u32 instance = 0) const noexcept -> v0;
        inline auto drawImGui() const noexcept -> v0;
        inline auto pushConstant(auto const* pData) const noexcept -> v0;
        inline auto pushConstant(v0 const* pData, u32 dataSize) const noexcept -> v0;

    private:
        struct Members
        {
            mutable VkContext::Members* ctx = VkContext::Get();
            mutable VkCommandBuffer cmd = VkCommandBuffer{ };
        } m;
    };
}

#include "ArlineCommands.inl"
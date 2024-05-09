#pragma once
#include "ArlineTypes.hpp"
#include "ArlineBuffer.hpp"
#include "ArlinePipeline.hpp"
#include "ArlineImage.hpp"
#include <imgui.h>

namespace arline
{
    class ImGuiContext
    {
    public:
        ImGuiContext() = delete;
        ~ImGuiContext() = delete;
        ImGuiContext(ImGuiContext const&) = delete;
        ImGuiContext(ImGuiContext&&) = delete;
        auto operator=(ImGuiContext const&) -> ImGuiContext& = delete;
        auto operator=(ImGuiContext&&) -> ImGuiContext& = delete;

    public:
        static auto Create() noexcept -> v0;
        static auto Teardown() noexcept -> v0;
        static auto NewFrame() noexcept -> v0;
        static auto EndFrame() noexcept -> v0;
        static auto CreatePipeline() noexcept -> v0;

    public:
        static inline auto Get() noexcept { return &m; }

    private:
        static inline struct Members
        {
            DynamicBuffer vbo;
            DynamicBuffer ibo;
            Image fontTexture;
            Pipeline pipeline;
        } m;
    };
}
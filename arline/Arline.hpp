#pragma once
#include "ArlineWindow.hpp"
#include "ArlineVkContext.hpp"
#include "ArlineCommands.hpp"
#include "ArlinePipeline.hpp"
#include "ArlineBuffer.hpp"
#include "ArlineImage.hpp"
#include "ArlineTime.hpp"
#include "ArlineImgui.hpp"
#include <concepts>

namespace arline
{
    template<class T>
    concept Engine = requires(T t)
    {
        { t.update() } -> std::same_as<v0>;
        { t.renderGui() } -> std::same_as<v0>;
        { t.recordCommands(Commands{}) } -> std::same_as<v0>;
        { T::UseImgui() } -> std::same_as<u32>;
    };

    class Context
    {
    public:
        Context(WindowInfo const& windowInfo, ContextInfo const& contextInfo = {}) noexcept
        {
            Window::Create(windowInfo, contextInfo.infoCallback, contextInfo.errorCallback);
            VkContext::Create(contextInfo);
            ImGuiContext::Create();
        }

        ~Context() noexcept
        {
            m.commands.m = {};

            ImGuiContext::Teardown();
            VkContext::Teardown();
            Window::Teardown();
        }

        inline auto initEngine(Engine auto&& engine) const noexcept -> v0
        {
            auto prevWidth{ Window::GetFramebufferWidth() };
            auto prevHeight{ Window::GetFramebufferHeight() };

            while (Window::IsAvailable()) [[likely]]
            {
                engine.update();

                if (prevHeight != Window::GetFramebufferHeight() || prevWidth != Window::GetFramebufferWidth()) [[unlikely]]
                {
                    prevWidth  = Window::GetFramebufferWidth();
                    prevHeight = Window::GetFramebufferHeight();

                    while (!prevHeight || !prevWidth)
                    {
                        Window::WaitEvents();
                        prevWidth  = Window::GetFramebufferWidth();
                        prevHeight = Window::GetFramebufferHeight();

                        if (!Window::IsAvailable())
                        {
                            goto exit;
                        }
                    }

                    VkContext::RecreateSwapchain();
                }

                if constexpr (engine.UseImgui())
                {
                    ImGuiContext::NewFrame();
                    engine.renderGui();
                    ImGuiContext::EndFrame();
                }

                m.commands.begin();
                engine.recordCommands(m.commands);
                m.commands.end();
            }

            exit:
            VkContext::WaitForDevice();
        }

        Context(Context const&) = delete;
        Context(Context&&) = delete;
        auto operator=(Context const&) = delete;
        auto operator=(Context&&) = delete;

    private:
        struct Members
        {
            Commands commands;
        } m;
    };
}

namespace ar = arline;
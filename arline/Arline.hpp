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
        { t.recordCommands(Commands{}) } -> std::same_as<v0>;
    };

    template<Engine engine_t, typename... Args>
    auto InitEngine(Args&&... args)
    {
        {
            WindowInfo windowInfo{ [&]
            {
                if constexpr (requires {{ engine_t::WindowInfo() } -> std::same_as<WindowInfo>; })
                {
                    return engine_t::WindowInfo();
                }
                else
                {
                    return WindowInfo{
                        .width = 1280,
                        .height = 720,
                        .minWidth = 400,
                        .minHeight = 300,
                        .title = "Arline application"
                    };
                }
            }()};

            ContextInfo contextInfo{ [&]
            {
                if constexpr (requires {{ engine_t::ContextInfo() } -> std::same_as<ContextInfo>; })
                {
                    return engine_t::ContextInfo();
                }
                else
                {
                    return ContextInfo{
                        .infoCallback = [](std::string_view){},
                        .errorCallback = [](std::string_view){},
                        .enableValidationLayers = false
                    };
                }
            }()};

            Window::Create(windowInfo, contextInfo.infoCallback, contextInfo.errorCallback);
            VkContext::Create(contextInfo);
            ImGuiContext::Create();
        }
        {
            auto commands{ Commands{} };
            auto prevWidth{ Window::GetFramebufferWidth() };
            auto prevHeight{ Window::GetFramebufferHeight() };

            engine_t engine(args...);

            while (Window::IsAvailable()) [[likely]]
            {
                engine.update();

                if (prevHeight != Window::GetFramebufferHeight() || prevWidth != Window::GetFramebufferWidth()) [[unlikely]]
                {
                    prevWidth = Window::GetFramebufferWidth();
                    prevHeight = Window::GetFramebufferHeight();

                    while (!prevHeight || !prevWidth)
                    {
                        Window::WaitEvents();
                        prevWidth = Window::GetFramebufferWidth();
                        prevHeight = Window::GetFramebufferHeight();

                        if (!Window::IsAvailable())
                        {
                            goto exitLoop;
                        }
                    }

                    VkContext::RecreateSwapchain();
                }

                //            requires { engine.renderGui(); }) is not working for some reason
                if constexpr (requires {{engine_t(args...).renderGui() } -> std::same_as<v0>; })
                {
                    ImGuiContext::NewFrame();
                    engine.renderGui();
                    ImGuiContext::EndFrame();
                }

                commands.begin();
                engine.recordCommands(commands);
                commands.end();
            }
            exitLoop:
            VkContext::WaitForDevice();
        }
        {
            ImGuiContext::Teardown();
            VkContext::Teardown();
            Window::Teardown();
        }
    }
}

namespace ar = arline;
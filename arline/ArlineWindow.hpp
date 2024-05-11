#pragma once
#define GLFW_INCLUDE_NONE
#include "ArlineTypes.hpp"
#include <GLFW/glfw3.h>
#include <string_view>

struct GLFWwindow;

namespace arline
{
    struct WindowInfo
    {
        u32 width;
        u32 height;
        u32 minWidth;
        u32 minHeight;
        u32 maxWidth;
        u32 maxHeight;
        std::string_view title;
    };

    class Window
    {
    public:
        Window() = delete;
        ~Window() = delete;
        Window(Window const&) = delete;
        Window(Window&&) = delete;
        auto operator=(Window const&) -> Window& = delete;
        auto operator=(Window&&) -> Window& = delete;

    public:
        static auto Create(
            WindowInfo const& info,
            v0 (*infoCallback)(std::string_view),
            v0 (*errorCallback)(std::string_view)
        ) -> v0;
        static auto Teardown() -> v0;
        static auto IsAvailable() noexcept -> b8;
        static auto PollEvents() noexcept -> v0;
        static auto WaitEvents() noexcept -> v0;
        static auto GetTitle() noexcept -> c8 const*;
        static auto SetTitle(std::string_view title) noexcept -> v0;

    private:
        static auto SizeCallback(GLFWwindow* pWindow, i32 width, i32 height) -> v0;
        static auto FramebufferSizeCallback(GLFWwindow* pWindow, i32 width, i32 height) -> v0;

    public:
        static inline auto GetHandle() noexcept -> GLFWwindow* { return m.handle; }
        template<typename T = i32>
        static inline auto GetWidth() noexcept -> T { return static_cast<T>(m.width); }
        template<typename T = i32>
        static inline auto GetHeight() noexcept -> T { return static_cast<T>(m.height); }
        template<typename T = i32>
        static inline auto GetFramebufferWidth() noexcept -> T { return static_cast<T>(m.framebufferWidth); }
        template<typename T = i32>
        static inline auto GetFramebufferHeight() noexcept -> T { return static_cast<T>(m.framebufferHeight); }

    private:
        static inline struct Members
        {
            v0 (*infoCallback)(std::string_view);
            GLFWwindow* handle;
            i32 width, height;
            i32 framebufferWidth;
            i32 framebufferHeight;
        } m;
    };
}
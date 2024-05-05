#pragma once
#include "ArlineTypes.hpp"
#include <string_view>

struct GLFWwindow;

namespace arline
{
    class Window
    {
    public:
        Window() = delete;
        ~Window() = delete;
        Window(Window const&) = delete;
        Window(Window&&) = delete;
        auto operator=(Window const&) -> Window& = delete;
        auto operator=(Window&&) -> Window& = delete;

    private:
        friend class Context;
        static auto Create(u32 width, u32 height, std::string_view title) -> v0;
        static auto Teardown() -> v0;

    public:
        static auto IsAvailable() noexcept -> b8;
        static auto PollEvents() noexcept -> v0;
        static auto WaitEvents() noexcept -> v0;

    private:
        static inline struct Members
        {
            GLFWwindow* handle;
        } m;
    };
}
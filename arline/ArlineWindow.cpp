#define GLFW_INCLUDE_NONE
#include "ArlineWindow.hpp"
#include <GLFW/glfw3.h>

#ifdef _WIN32

#   define GLFW_EXPOSE_NATIVE_WIN32
#   include <GLFW/glfw3native.h>
#   include <dwmapi.h>
#   pragma comment(lib, "Dwmapi")

#endif

auto arline::Window::Create(u32 width, u32 height, std::string_view title) -> void
{
    m = {};

    if (!glfwInit())
    {

    }

    m.handle = glfwCreateWindow(
        static_cast<i32>(width), static_cast<i32>(height),
        title.data(), nullptr, nullptr
    );

    if (!m.handle)
    {

    }

#ifdef _WIN32

    auto const useDarkMode{ BOOL{1} };

    DwmSetWindowAttribute(
        glfwGetWin32Window(m.handle), DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
        &useDarkMode, sizeof(useDarkMode)
    );

#endif
}

auto arline::Window::Teardown() -> v0
{
    glfwTerminate();
}

auto arline::Window::IsAvailable() noexcept -> b8
{
    return !glfwWindowShouldClose(m.handle);
}

auto arline::Window::PollEvents() noexcept -> v0
{
    glfwPollEvents();
}

auto arline::Window::WaitEvents() noexcept -> v0
{
    glfwWaitEvents();
}

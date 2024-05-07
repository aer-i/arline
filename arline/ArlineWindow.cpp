#include "ArlineWindow.hpp"
#include <format>

#ifdef _WIN32

#   define GLFW_EXPOSE_NATIVE_WIN32
#   include <GLFW/glfw3native.h>
#   include <dwmapi.h>
#   pragma comment(lib, "Dwmapi")

#endif

auto arline::Window::Create(
    WindowInfo const& info,
    v0 (*infoCallback)(std::string_view),
    v0 (*errorCallback)(std::string_view)
) -> v0
{
    m = {
        .infoCallback = infoCallback
    };

    if (!glfwInit())
    {
        c8 const* error;
        glfwGetError(&error);

        errorCallback(std::format("Failed to initialize GLFW: {}", error));
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m.handle = glfwCreateWindow(
        static_cast<i32>(info.width), static_cast<i32>(info.height),
        info.title.data(), nullptr, nullptr
    );

    if (!m.handle)
    {
        c8 const* error;
        glfwGetError(&error);

        errorCallback(std::format("Failed to create window: {}", error));
    }

#ifdef _WIN32
    {
        auto const useDarkMode{BOOL{1}};

        DwmSetWindowAttribute(
            glfwGetWin32Window(m.handle), DWMWA_USE_IMMERSIVE_DARK_MODE,
            &useDarkMode, sizeof(useDarkMode)
        );

        // Update window to apply new theme
        glfwGetWindowSize(m.handle, &m.width, &m.height);
        glfwSetWindowSize(m.handle, m.width + 1, m.height + 1);
        glfwGetWindowSize(m.handle, &m.width, &m.height);
        glfwSetWindowSize(m.handle, m.width - 1, m.height - 1);
    }
#endif

    glfwSetWindowSizeLimits(
        m.handle,
        info.minWidth  ? static_cast<i32>(info.minWidth)  : GLFW_DONT_CARE,
        info.minHeight ? static_cast<i32>(info.minHeight) : GLFW_DONT_CARE,
        info.maxWidth  ? static_cast<i32>(info.maxWidth)  : GLFW_DONT_CARE,
        info.maxHeight ? static_cast<i32>(info.maxHeight) : GLFW_DONT_CARE
    );

    auto primaryMonitor{ glfwGetPrimaryMonitor() };
    auto videoMode{ glfwGetVideoMode(primaryMonitor) };
    glfwGetWindowSize(m.handle, &m.width, &m.height);
    glfwGetFramebufferSize(m.handle, &m.framebufferWidth, &m.framebufferHeight);

    glfwSetWindowSizeCallback(m.handle, SizeCallback);
    glfwSetFramebufferSizeCallback(m.handle, FramebufferSizeCallback);

    infoCallback(std::format("Created window: width[ {} ], height[ {} ]", m.width, m.height));
    infoCallback(std::format(
        "Primary monitor: width[ {} ], height[ {} ], refresh rate[ {} ]",
        videoMode->width, videoMode->height, videoMode->refreshRate
    ));
}

auto arline::Window::Teardown() -> v0
{
    glfwTerminate();

    m.infoCallback("Destroyed window");
    m.infoCallback("Terminated GLFW context");
}

auto arline::Window::SizeCallback([[maybe_unused]] GLFWwindow* pWindow, i32 width, i32 height) -> v0
{
    m.width = width;
    m.height = height;
}

auto arline::Window::FramebufferSizeCallback([[maybe_unused]]GLFWwindow* pWindow, i32 width, i32 height) -> v0
{
    m.framebufferWidth = width;
    m.framebufferHeight = height;
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

auto arline::Window::GetTitle() noexcept -> const c8*
{
    return glfwGetWindowTitle(m.handle);
}

auto arline::Window::SetTitle(std::string_view title) noexcept -> v0
{
    glfwSetWindowTitle(m.handle, title.data());
}
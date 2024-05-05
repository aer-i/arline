#pragma once
#include "ArlineWindow.hpp"
#include "ArlineTypes.hpp"
#include <string_view>

namespace arline
{
    struct WindowInfo
    {
        u32 width;
        u32 height;
        std::string_view title;
    };

    class Context
    {
    public:
        Context(WindowInfo const& windowInfo)
        {
            Window::Create(
                windowInfo.width,
                windowInfo.height,
                windowInfo.title
            );
        }

        ~Context()
        {
            Window::Teardown();
        }

        Context(Context const&) = delete;
        Context(Context&&) = delete;
        auto operator=(Context const&) = delete;
        auto operator=(Context&&) = delete;
    };
}

namespace ar = arline;
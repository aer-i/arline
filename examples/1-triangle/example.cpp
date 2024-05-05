#include <Arline.hpp>

auto main() -> ar::i32
{
    auto const arlineContext{ ar::Context{
        ar::WindowInfo{
            .width = 1280,
            .height = 720,
            .title = "Example - Triangle"
        }
    }};

    while (ar::Window::IsAvailable())
    {
        ar::Window::PollEvents();
    }
}
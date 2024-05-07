#include <Arline.hpp>

using namespace ar::types;

struct Engine
{
    ar::Pipeline pipeline;
    f64 currentTime  = ar::time::get();
    f64 previousTime = ar::time::get();
    f64 timeDiff     = f64{ };
    u32 frameCounter = u32{ };

    inline Engine() noexcept
    {
        auto const vertShader{ ar::Shader{"shaders/main.vert.spv"} };
        auto const fragShader{ ar::Shader{"shaders/main.frag.spv"} };

        pipeline = ar::GraphicsPipeline{{
            .shaders = {
                vertShader, fragShader
            }
        }};
    }

    inline auto update() noexcept -> v0
    {
        ar::Window::PollEvents();

        currentTime = ar::time::get();
        timeDiff = currentTime - previousTime;
        ++frameCounter;

        if (timeDiff > 0.25)
        {
            ar::Window::SetTitle(std::format(
                "FPS: {}, MS: {:.3f}",
                static_cast<u32>((1.0 / timeDiff) * frameCounter),
                (timeDiff / frameCounter) * 1000.0
            ));

            previousTime = currentTime;
            frameCounter = {};
        }
    }

    inline auto recordCommands(ar::Commands const& commands) noexcept -> v0
    {
        commands.beginPresent();

        commands.bindPipeline(pipeline);
        commands.draw(3);

        commands.endPresent();
    }
};

auto main() -> i32
{
    ar::Context{
        ar::WindowInfo{
            .width = 1280,
            .height = 720,
            .title = "Example - Triangle"
        },
        ar::ContextInfo{
            .infoCallback = [](std::string_view message) { std::printf("INFO: %s\n", message.data()); },
            .errorCallback = [](std::string_view message) { std::printf("ERROR: %s\n", message.data()); exit(1); },
            .enableValidationLayers = true
        }
    }.initEngine(Engine{});
}
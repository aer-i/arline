#include <Arline.hpp>
#include <format>

using namespace ar::types;

struct Engine
{
    ar::StaticBuffer buffer;
    ar::Pipeline pipeline;
    f64 currentTime  = ar::time::get();
    f64 previousTime = ar::time::get();
    f64 timeDiff     = f64{ };
    u32 frameCounter = u32{ };

    inline Engine() noexcept
    {
        struct Vertex
        {
            f32 x, y, z;
            f32 r, g, b;
        } vertices[] = {
            { 0.5f, 0.5f, 0.0f, 1.f, 0.f, 0.f },
            { -.5f, 0.5f, 0.0f, 0.f, 1.f, 0.f },
            { 0.0f, -.5f, 0.0f, 0.f, 0.f, 1.f },
        };

        buffer = ar::StaticBuffer{ vertices, sizeof(vertices) };

        pipeline = ar::GraphicsPipeline{{
            .shaders = {
                ar::Shader{"shaders/main.vert.spv"},
                ar::Shader{"shaders/main.frag.spv"}
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
        commands.pushConstant(buffer.getAddress());
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
            .enableValidationLayers = false
        }
    }.initEngine(Engine{});
}
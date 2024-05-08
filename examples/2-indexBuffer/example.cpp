#include <Arline.hpp>
#include <format>
#include <cmath>

using namespace ar::types;

struct Engine
{
    ar::StaticBuffer vbo;
    ar::StaticBuffer ibo;
    ar::Pipeline pipeline;
    
    f64 currentTime  = ar::time::get();
    f64 previousTime = ar::time::get();
    f64 timeDiff     = f64{ };
    u32 frameCounter = u32{ };

    inline Engine() noexcept
    {
        struct{ f32 x, y, z; }
        vertices[] = {
            { 0.5f, 0.5f, 0.0f },
            { -.5f, 0.5f, 0.0f },
            { -.5f, -.5f, 0.0f },
            { 0.5f, -.5f, 0.0f }
        };

        u32 indices[] = { 0, 1, 2, 2, 3, 0 };

        vbo = ar::StaticBuffer{ vertices, sizeof(vertices) };
        ibo = ar::StaticBuffer{ indices, sizeof(indices) };

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
        struct{ u64 vbo, ibo; }
        pushConstant {
            .vbo = *vbo.getAddress(),
            .ibo = *ibo.getAddress()
        };

        commands.beginPresent();

        commands.bindPipeline(pipeline);
        commands.pushConstant(&pushConstant);
        commands.draw(6);

        commands.endPresent();
    }
};

auto main() -> i32
{
    ar::Context{
        ar::WindowInfo{
            .width = 1280,
            .height = 720,
            .minWidth = 400,
            .minHeight = 300,
            .title = "Example - Triangle"
        },
        ar::ContextInfo{
            .infoCallback = [](std::string_view message) { std::printf("INFO: %s\n", message.data()); },
            .errorCallback = [](std::string_view message) { std::printf("ERROR: %s\n", message.data()); exit(1); },
            .enableValidationLayers = true
        }
    }.initEngine(Engine{});
}
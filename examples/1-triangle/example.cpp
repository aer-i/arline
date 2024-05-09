#include <Arline.hpp>
#include <format>
#include <cmath>

using namespace ar::types;

struct Engine
{
    static consteval u32 UseImgui() { return 0; }

    ar::StaticBuffer vbo;
    ar::Pipeline pipeline;

    inline Engine() noexcept
    {
        struct{ f32 x, y, z; }
        vertices[] = {
            { 0.5f, 0.5f, 0.0f },
            { -.5f, 0.5f, 0.0f },
            { 0.0f, -.5f, 0.0f }
        };

        vbo = ar::StaticBuffer{ vertices, sizeof(vertices) };

        pipeline = ar::GraphicsPipeline{{
            .shaders = {
                ar::Shader{"shaders/main.vert.spv"},
                ar::Shader{"shaders/main.frag.spv"}
            }
        }};
    }

    inline auto renderGui() -> v0 {}
    inline auto update() noexcept -> v0
    {
        ar::Window::PollEvents();
    }

    inline auto recordCommands(ar::Commands const& commands) noexcept -> v0
    {
        struct{ u64 vbo; f32 color[3]; }
        pushConstant {
            .vbo = *vbo.getAddress(),
            .color = {
                static_cast<f32>(std::sin(ar::time::get() * 1.0)) * 0.5f + 0.5f,
                static_cast<f32>(std::sin(ar::time::get() * 2.0)) * 0.5f + 0.5f,
                static_cast<f32>(std::sin(ar::time::get() * 3.0)) * 0.5f + 0.5f
            }
        };

        commands.beginPresent();

        commands.bindPipeline(pipeline);
        commands.pushConstant(&pushConstant);
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
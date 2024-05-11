#include <Arline.hpp>
#include <cmath>

using namespace ar::types;

struct Engine
{
    ar::StaticBuffer vbo;
    ar::StaticBuffer ibo;
    ar::Pipeline pipeline;

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
    ar::InitEngine<Engine>();
}
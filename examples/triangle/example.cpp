#include <Arline.hpp>
#include <cmath>

using namespace ar::types;

struct{ f32 x, y, z; }
static constexpr vertices[] = {
    { 0.5f, 0.5f, 0.0f },
    { -.5f, 0.5f, 0.0f },
    { 0.0f, -.5f, 0.0f }
};

struct Engine
{
    ar::StaticBuffer vbo = ar::StaticBuffer{ vertices, sizeof(vertices) };
    ar::Pipeline pipeline = ar::Pipeline{{
        .shaders = {
            ar::Shader{"shaders/main.vert.spv"},
            ar::Shader{"shaders/main.frag.spv"}
        }
    }};

    inline void update() noexcept
    {
        ar::Window::PollEvents();
    }

    inline void recordCommands(ar::Commands const& commands) noexcept
    {
        struct{ u64 vbo; f32 color[3]; }
        const pushConstant {
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

int main()
{
    ar::InitEngine<Engine>();
}
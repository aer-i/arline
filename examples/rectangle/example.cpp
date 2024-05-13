#include <Arline.hpp>
#include <cmath>
#include <cstdint>

unsigned const indices[] = { 0, 1, 2, 2, 3, 0 };
float const vertices[] = {
    0.5f, 0.5f, 0.0f,
    -.5f, 0.5f, 0.0f,
    -.5f, -.5f, 0.0f,
    0.5f, -.5f, 0.0f
};

struct Engine
{
    ar::StaticBuffer vbo = ar::StaticBuffer{ vertices, sizeof(vertices) };
    ar::StaticBuffer ibo = ar::StaticBuffer{ indices, sizeof(indices) };
    ar::Pipeline pipeline = ar::Pipeline{{
        .shaders = {
            ar::Shader{"shaders/main.vert.spv"},
            ar::Shader{"shaders/main.frag.spv"}
        }
    }};

    void update()
    {
        ar::Window::PollEvents();
    }

    void recordCommands(ar::Commands commands)
    {
        struct{ uint64_t vbo, ibo; }
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

int main()
{
    ar::InitEngine<Engine>();
}
#include "shader.inl"
#include <Arline.hxx>

static constexpr float g_vertices[] = {
    0.5f, 0.5f, 0.f,
    -.5f, 0.5f, 0.f,
    0.0f, -.5f, 0.f
};

class Engine : public ar::Engine
{
public:
    void onCommandsRecord(ar::GraphicsCommands const& commands) final
    {
        commands.beginPresent();
        {
            auto address{ vertexBuffer.getAddress() };

            commands.pushConstant(&address, sizeof(address));
            commands.bindPipeline(pipeline);
            commands.draw(3u);
        }
        commands.endPresent();
    }

private:
    ar::StaticBuffer vertexBuffer{ g_vertices, sizeof(g_vertices) };
    ar::Pipeline pipeline{ ar::GraphicsConfig{
        .shaders = {
            ar::Shader{ shaders::vert, sizeof(shaders::vert), ar::ShaderStage::eVertex   },
            ar::Shader{ shaders::frag, sizeof(shaders::frag), ar::ShaderStage::eFragment }
        }
    }};
};

int main()
{
    return Engine{}.execute();
}
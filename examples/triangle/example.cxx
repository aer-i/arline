#include "shader.inl"
#include <Arline.hxx>

class Engine : public ar::Engine
{
public:
    void onCommandsRecord(ar::GraphicsCommands const& commands) final
    {
        float vertices[] = {
            0.5f, 0.5f, 0.f,
            -.5f, 0.5f, 0.f,
            0.0f, -.5f, 0.f
        };

        vertexBuffer.write(vertices);

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
    ar::Buffer vertexBuffer{ sizeof(float) * 3 * 3 };
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
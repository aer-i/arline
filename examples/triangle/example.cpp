#include "shader.inl"
#include <Arline.hxx>

using namespace ar::types;

class Engine : public ar::Engine
{
public:
    ar::Buffer vertexBuffer{ sizeof(f32_t) * 3 * 3 };
    ar::Pipeline pipeline{ ar::GraphicsConfig{
        .shaders = {
            ar::Shader{ shaders::vert, sizeof(shaders::vert), ar::ShaderStage::eVertex   },
            ar::Shader{ shaders::frag, sizeof(shaders::frag), ar::ShaderStage::eFragment }
        }
    }};

    Engine()
    {
        f32_t vertices[] = {
            0.5f, 0.5f, 0.f,
            -.5f, 0.5f, 0.f,
            0.0f, -.5f, 0.f
        };

        vertexBuffer.write(vertices);
    }

    void onResourcesUpdate() final
    {

    }

    void onUpdate() final
    {
        ar::Window::PollEvents();
    }

    void onCommandsRecord(ar::GraphicsCommands const& commands) final
    {
        commands.beginPresent({ ar::LoadOp::eClear, ar::StoreOp::eDontCare });
        {
            auto address{ vertexBuffer.getAddress() };

            commands.pushConstant(&address, sizeof(address));
            commands.bindPipeline(pipeline);
            commands.draw(3u);
        }
        commands.endPresent();
    }
};

int main()
{
    return Engine{}.execute();
}
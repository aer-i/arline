#include "shader.inl"
#include <Arline.hxx>

static ar::Pipeline pipeline;
static ar::Buffer vertexBuffer;

static auto recordCommands(ar::GraphicsCommands cmd) -> void
{
    cmd.beginPresent();
    {
        cmd.pushConstant(&vertexBuffer.address, sizeof(vertexBuffer.address));
        cmd.bindPipeline(pipeline);
        cmd.draw(3u);
    }
    cmd.endPresent();
}

static auto init() -> void
{
    constexpr float vertices[] = {
        0.5f, 0.5f, 0.f,
        -.5f, 0.5f, 0.f,
        0.0f, -.5f, 0.f
    };

    vertexBuffer.create(vertices, sizeof(vertices));

    ar::Shader vert, frag;
    vert.create(shaders::vert, sizeof(shaders::vert), ar::ShaderStage::eVertex);
    frag.create(shaders::frag, sizeof(shaders::frag), ar::ShaderStage::eFragment);

    pipeline.create(ar::GraphicsConfig{
        .shaders = { vert, frag }
    });

    vert.destroy();
    frag.destroy();
}

static auto teardown() -> void
{
    vertexBuffer.destroy();
    pipeline.destroy();
}

auto main() -> int
{
    return ar::execute(ar::AppInfo{
        .onInit = init,
        .onDestroy = teardown,
        .onCommandsRecord = recordCommands,
        .title = "Triangle",
        .width = 1280,
        .height = 720
    });
}
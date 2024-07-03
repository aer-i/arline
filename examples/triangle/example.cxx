#include <Arline.hxx>

static ar::Pipeline pipeline;
static ar::Buffer vertexBuffer;

void recordCommands(ar::GraphicsCommands cmd)
{
    cmd.beginPresent();
    {
        cmd.pushConstant(&vertexBuffer.address, sizeof(vertexBuffer.address));
        cmd.bindPipeline(pipeline);
        cmd.draw(3);
    }
    cmd.endPresent();
}

void init()
{
    ar::f32 vertices[] = {
        0.5f, 0.5f, 0.f,
        -.5f, 0.5f, 0.f,
        0.0f, -.5f, 0.f
    };

    vertexBuffer.create(vertices, sizeof(vertices));

    ar::Shader vert, frag;
    frag.create("shaders/main.frag.spv");
    vert.create("shaders/main.vert.spv");

    pipeline.create(ar::GraphicsConfig{
        .shaders = { vert, frag },
        .attachments = {
            ar::BlendAttachment{
                .colorComponent = ar::ColorComponent::eRGBA
            }
        }
    });

    vert.destroy();
    frag.destroy();
}

void teardown()
{
    vertexBuffer.destroy();
    pipeline.destroy();
}

int main()
{
    ar::execute(ar::AppInfo{
        .onInit = init,
        .onDestroy = teardown,
        .onCommandsRecord = recordCommands,
        .width = 1280,
        .height = 720,
        .enalbeVsync = true
    });
}
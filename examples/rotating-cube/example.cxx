#include <Arline.hxx>
#include "shader.inl"
#include "vertices.inl"
#include <cmath>
#include <cstdio>
#include <exception>

struct SceneData
{
    std::array<ar::f32_t, 16> cameraProjectionMatrix;
    std::array<ar::f32_t, 16> cubeMatrix;
};

static ar::Image color, resolve, depth;
static ar::Buffer vertexBuffer, colorBuffer, sceneBuffer;
static ar::Pipeline pipeline, finalImagePipeline;
static SceneData sceneData;

static auto
createPerSwapchainResources() noexcept -> void
{
    color.create({
        .layout = ar::ImageLayout::eColorAttachment,
        .usage = ar::ImageUsage::eColorAttachment,
        .useMsaa = true
    });

    resolve.create({
        .layout = ar::ImageLayout::eShaderReadOnly,
        .usage = ar::ImageUsage::eColorAttachment,
        .sampler = ar::Sampler::eNearestToEdge,
        .shaderArrayElement = 1u,
    });

    depth.create({
        .layout = ar::ImageLayout::eDepthAttachment,
        .usage = ar::ImageUsage::eDepthAttachment,
        .useMsaa = true
    });
}

static auto
destroyPerSwapchainResources() noexcept -> void
{
    depth.destroy();
    resolve.destroy();
    color.destroy();
}

static auto
init() noexcept -> void
{
    createPerSwapchainResources();

    vertexBuffer.create(g_cubeVertices, sizeof(g_cubeVertices));
    colorBuffer.create(g_cubeColors, sizeof(g_cubeColors));
    sceneBuffer.create(sizeof(sceneData));

    ar::Shader vert, frag;
    vert.create(shaders::vert, sizeof(shaders::vert), ar::ShaderStage::eVertex);
    frag.create(shaders::frag, sizeof(shaders::frag), ar::ShaderStage::eFragment);

    pipeline.create(ar::GraphicsConfig{
        .shaders = { vert, frag },
        .attachments = { ar::BlendAttachment{
            .colorComponent = ar::ColorComponent::eRGBA
        }},
        .depthStencilState = {
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .compareOp = ar::CompareOp::eLess
        },
        .useMsaa = true
    });

    frag.destroy();
    vert.destroy();

    vert.create(shaders::finalImageVert, sizeof(shaders::finalImageVert), ar::ShaderStage::eVertex);
    frag.create(shaders::finalImageFrag, sizeof(shaders::finalImageFrag), ar::ShaderStage::eFragment);

    finalImagePipeline.create(ar::GraphicsConfig{
        .shaders = { vert, frag },
        .attachments = { ar::BlendAttachment{
            .colorComponent = ar::ColorComponent::eRGBA
        }}
    });

    frag.destroy();
    vert.destroy();
}

static auto
teardown() noexcept -> void
{
    sceneBuffer.destroy();
    colorBuffer.destroy();
    vertexBuffer.destroy();
    finalImagePipeline.destroy();
    pipeline.destroy();

    destroyPerSwapchainResources();
}

static auto
resize() noexcept -> void
{
    destroyPerSwapchainResources();
    createPerSwapchainResources();
}

static auto
recordCommands(ar::GraphicsCommands cmd) noexcept -> void
{
    ar::u64_t const pushConstant[] = {
        vertexBuffer.address,
        colorBuffer.address,
        sceneBuffer.address
    };

    cmd.barrier({
        .image = resolve,
        .oldLayout = ar::ImageLayout::eShaderReadOnly,
        .newLayout = ar::ImageLayout::eColorAttachment
    });

    cmd.beginRendering({
        ar::ColorAttachment{
            .image = color,
            .pResolve = &resolve,
            .loadOp = ar::LoadOp::eClear,
            .storeOp = ar::StoreOp::eDontCare,
            .clearColor = ar::ClearColor{ 0.6f, 0.7f, 1.0f, 1.0f }
        }},
        ar::DepthAttachment{
            .pImage = &depth,
            .loadOp = ar::LoadOp::eClear,
            .storeOp = ar::StoreOp::eDontCare
        }
    );

    cmd.pushConstant(pushConstant, sizeof(pushConstant));
    cmd.bindPipeline(pipeline);
    cmd.draw(36u);

    cmd.endRendering();

    cmd.barrier({
        .image = resolve,
        .oldLayout = ar::ImageLayout::eColorAttachment,
        .newLayout = ar::ImageLayout::eShaderReadOnly
    });

    cmd.beginPresent();
    cmd.bindPipeline(finalImagePipeline);
    cmd.draw(3u);
    cmd.endPresent();
}

static auto
update() noexcept -> void
{
    ar::pollEvents();

    ar::f32_t aspectRatio = ar::getFramebufferAspectRatio();
    ar::f32_t time = ar::getTimef();
    ar::f32_t c = cosf(time), s = sinf(time);

    sceneData.cameraProjectionMatrix = {
        1.f / aspectRatio, 0.0f, 0.0f, 0.0f,
        0.0f,              1.0f, 0.0f, 0.0f,
        0.0f,              0.0f, 1.0f, 1.0f,
        0.0f,              0.0f, -.5f, 0.0f
    };

    sceneData.cubeMatrix = {
        + c, c * s, s * s, 0.f,
        - s, c * c, s * c, 0.f,
        0.f,   - s,     c, 0.f,
        0.f,   0.f,   5.f, 1.f
    };
}

static auto
resourcesUpdate() noexcept -> ar::Request
{
    sceneBuffer.write(&sceneData);

    return ar::Request::eNone;
}

static auto
infoCallback(std::string_view message) noexcept -> void
{
    std::printf("%s\n", message.data());
}

[[noreturn]] static auto
errorCallback(std::string_view message) noexcept -> void
{
    ar::messageBoxError(message);
    std::terminate();
}

[[nodiscard]] auto
main() noexcept -> ar::i32_t
{
    return ar::execute(ar::AppInfo{
        .onInit = init,
        .onDestroy = teardown,
        .onCommandsRecord = recordCommands,
        .onResourcesUpdate = resourcesUpdate,
        .onUpdate = update,
        .onResize = resize,
        .title = "Cube",
        .width = 1280,
        .height = 720,
        .infoCallback = infoCallback,
        .errorCallback = errorCallback,
        .enableValidationLayers = true,
        .enalbeVsync = true
    });
}
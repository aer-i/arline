#include <Arline.hxx>
#include "shader.inl"
#include "vertices.inl"

struct SceneData
{
    std::array<float, 16> cameraProjectionMatrix;
    std::array<float, 16> cubeMatrix;
};

class Engine : public ar::Engine
{
public:
    [[noexcept]] auto
    onResourcesUpdate() noexcept -> bool final
    {
        sceneBuffer.write(&sceneData);
        vertexBuffer.write(g_cubeVertices);
        colorBuffer.write(g_cubeColors);

        return false;
    }

    auto onUpdate() noexcept -> void final
    {
        ar::Window::PollEvents();

        const float time = ar::getTimef();
        const float aspect = ar::Window::GetFramebufferAspect();
        const float c = cosf(time);
        const float s = sinf(time);

        sceneData.cameraProjectionMatrix = {
            1.f / aspect, 0.0f, 0.0f, 0.0f,
            0.0f,         1.0f, 0.0f, 0.0f,
            0.0f,         0.0f, 1.0f, 1.0f,
            0.0f,         0.0f, -.5f, 0.0f
        };

        sceneData.cubeMatrix = {
            + c, c * s, s * s, 0.f,
            - s, c * c, s * c, 0.f,
            0.f,   - s,     c, 0.f,
            0.f,   0.f,   5.f, 1.f
        };
    }

    auto onCommandsRecord(ar::GraphicsCommands const& commands) noexcept -> void final
    {
        uint64_t const addresses[] = {
            vertexBuffer.getAddress(),
            colorBuffer.getAddress(),
            sceneBuffer.getAddress()
        };

        commands.barrier({
            .image = colorFramebuffer,
            .oldLayout = ar::ImageLayout::eShaderReadOnly,
            .newLayout = ar::ImageLayout::eColorAttachment
        });

        commands.barrier({
            .image = depthFramebuffer,
            .oldLayout = ar::ImageLayout::eDepthAttachment,
            .newLayout = ar::ImageLayout::eDepthAttachment
        });

        commands.beginRendering({
            ar::ColorAttachment{
                .image = colorFramebuffer,
                .loadOp = ar::LoadOp::eClear,
                .storeOp = ar::StoreOp::eStore,
                .clearColor = ar::ClearColor{ 0.6f, 0.7f, 1.f, 1.f },
            }},
            ar::DepthAttachment{
                .pImage = &depthFramebuffer,
                .loadOp = ar::LoadOp::eClear,
                .storeOp = ar::StoreOp::eDontCare
            }
        );

        commands.pushConstant(addresses, sizeof(addresses));
        commands.bindPipeline(pipeline);
        commands.draw(3u * 12u);

        commands.endRendering();

        commands.barrier({
            .image = colorFramebuffer,
            .oldLayout = ar::ImageLayout::eColorAttachment,
            .newLayout = ar::ImageLayout::eShaderReadOnly
        });

        commands.beginPresent();

        commands.bindPipeline(finalImagePipeline);
        commands.draw(3u);

        commands.endPresent();
    }

    auto onResize() noexcept -> void final
    {
        colorFramebuffer = ar::Image{ ar::ImageCreateInfo{
            .usage = ar::ImageUsage::eColorAttachment,
            .sampler = ar::Sampler::eNearestToEdge
        }};

        depthFramebuffer = ar::Image{ ar::ImageCreateInfo{
            .usage = ar::ImageUsage::eDepthAttachment
        }};
    }

private:
    SceneData sceneData{ SceneData{} };
    ar::Image colorFramebuffer{ ar::ImageCreateInfo{
        .usage = ar::ImageUsage::eColorAttachment,
        .sampler = ar::Sampler::eNearestToEdge
    }};
    ar::Image depthFramebuffer{ ar::ImageCreateInfo{
        .usage = ar::ImageUsage::eDepthAttachment
    }};
    ar::Buffer vertexBuffer{ sizeof(g_cubeVertices) };
    ar::Buffer colorBuffer{ sizeof(g_cubeColors) };
    ar::Buffer sceneBuffer{ sizeof(sceneData) };
    ar::Pipeline 
    pipeline{ ar::GraphicsConfig{
        .shaders = {
            ar::Shader{ shaders::vert, sizeof(shaders::vert), ar::ShaderStage::eVertex   },
            ar::Shader{ shaders::frag, sizeof(shaders::frag), ar::ShaderStage::eFragment },
        },
        .depthStencilState = {
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .compareOp = ar::CompareOp::eLess
        }
    }}, 
    finalImagePipeline{ ar::GraphicsConfig{
        .shaders = {
            ar::Shader{ shaders::finalImageVert, sizeof(shaders::finalImageVert), ar::ShaderStage::eVertex   },
            ar::Shader{ shaders::finalImageFrag, sizeof(shaders::finalImageFrag), ar::ShaderStage::eFragment }
        }
    }};
};

[[nodiscard]] auto
main() noexcept -> int
{
    return Engine{}.execute();
}
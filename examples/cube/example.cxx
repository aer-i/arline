#include <arline.h>
#include <string.h>
#include <math.h>
#include "vertices.inl"

struct mat4 { float data[16]; };

struct SceneData
{
    mat4 projectionMatrix;
    mat4 cubeMatrix;
};

static ArImage colorFb, depthFb;
static ArBuffer positionBuffer, colorBuffer, sceneBuffer;
static ArPipeline cubePipeline, finalImagePipeline;
static SceneData sceneData;

static void
createPerSwapchainResources()
{
    {
        ArImageCreateInfo const colorFbCreateInfo = {
            .usage = AR_IMAGE_USAGE_COLOR_ATTACHMENT,
            .sampler = AR_SAMPLER_NEAREST_TO_EDGE,
            .dstArrayElement = 1
        };

        arCreateImage(&colorFb, &colorFbCreateInfo);
    }
    {
        ArImageCreateInfo const depthFbCreateInfo = {
            .usage = AR_IMAGE_USAGE_DEPTH_ATTACHMENT
        };

        arCreateImage(&depthFb, &depthFbCreateInfo);
    }
}

static void
destroyPerSwapchainResources()
{
    arDestroyImage(&depthFb);
    arDestroyImage(&colorFb);
}

static void
init()
{
    createPerSwapchainResources();
    arCreateStaticBuffer(&positionBuffer, sizeof(cubePositions), cubePositions);
    arCreateStaticBuffer(&colorBuffer, sizeof(cubeColors), cubeColors);
    arCreateDynamicBuffer(&sceneBuffer, sizeof(sceneBuffer));

    ArShader vertShader;
    ArShader fragShader;
    arCreateShaderFromFile(&vertShader, "shaders/cube.vert.spv");
    arCreateShaderFromFile(&fragShader, "shaders/cube.frag.spv");

    ArBlendAttachment const blendAttachment = {
        .colorWriteMask = AR_COLOR_COMPONENT_RGBA_BITS
    };

    ArGraphicsPipelineCreateInfo const cubePipelineCreateInfo = {
        .blendAttachmentCount = 1,
        .pBlendAttachments = &blendAttachment,
        .depthState = {
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .compareOp = AR_COMPARE_OP_LESS
        },
        .vertShader = vertShader,
        .fragShader = fragShader,
        .topology = AR_TOPOLOGY_TRIANGLE_LIST
    };

    arCreateGraphicsPipeline(&cubePipeline, &cubePipelineCreateInfo);
    arDestroyShader(&fragShader);
    arDestroyShader(&vertShader);
    
    arCreateShaderFromFile(&vertShader, "shaders/finalImage.vert.spv");
    arCreateShaderFromFile(&fragShader, "shaders/finalImage.frag.spv");

    ArGraphicsPipelineCreateInfo const finalImagePipelineCreateInfo = {
        .blendAttachmentCount = 1,
        .pBlendAttachments = &blendAttachment,
        .vertShader = vertShader,
        .fragShader = fragShader,
        .topology = AR_TOPOLOGY_TRIANGLE_LIST
    };

    arCreateGraphicsPipeline(&finalImagePipeline, &finalImagePipelineCreateInfo);
    arDestroyShader(&fragShader);
    arDestroyShader(&vertShader);
}

static void
teardown()
{
    arDestroyPipeline(&finalImagePipeline);
    arDestroyPipeline(&cubePipeline);
    arDestroyBuffer(&sceneBuffer);
    arDestroyBuffer(&colorBuffer);
    arDestroyBuffer(&positionBuffer);
    destroyPerSwapchainResources();
}

static void
update()
{
    arPollEvents();

    float const aspr = 1.0f / arGetRenderAspectRatio();
    float const time = arGetTimef();
    float const c = cosf(time);
    float const s = sinf(time);

    sceneData.projectionMatrix = {
        aspr, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, -.5f, 0.0f
    };

    sceneData.cubeMatrix = {
        + c, c*s, s*s, 0.f,
        - s, c*c, s*c, 0.f,
        0.f,  -s,   c, 0.f,
        0.f, 0.f, 5.f, 1.f
    };
}

static ArRequest
updateResources()
{
    memcpy(sceneBuffer.pMapped, &sceneData, sizeof(sceneData));
    return AR_REQUEST_NONE;
}

static void
resize()
{
    destroyPerSwapchainResources();
    createPerSwapchainResources();
}

static void
recordCommands()
{
    uint64_t const pushConstant[] = {
        positionBuffer.address,
        colorBuffer.address,
        sceneBuffer.address
    };

    ArAttachment const colorAttachment = {
        .pImage = &colorFb,
        .loadOp = AR_LOAD_OP_CLEAR,
        .storeOp = AR_STORE_OP_STORE,
        .clearValue = {
            .color = ArClearColor{ 0.6f, 0.7f, 1.0f, 1.0f }
        }
    };

    ArAttachment const depthAttachment = {
        .pImage = &depthFb,
        .loadOp = AR_LOAD_OP_CLEAR,
        .storeOp = AR_STORE_OP_DONT_CARE,
        .clearValue = { .depth = 1.0f }
    };

    {
        ArBarrier const barriers[2] = {
            ArBarrier{
                .pImage = &colorFb,
                .oldLayout = AR_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = AR_IMAGE_LAYOUT_COLOR_ATTACHMENT
            },
            ArBarrier{
                .pImage = &depthFb,
                .oldLayout = AR_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = AR_IMAGE_LAYOUT_DEPTH_ATTACHMENT
            }
        };

        arCmdPipelineBarrier(2, barriers);
    }
    arCmdBeginRendering(1, &colorAttachment, &depthAttachment);
    {
        arCmdPushConstants(0, sizeof(pushConstant), pushConstant);
        arCmdBindGraphicsPipeline(&cubePipeline);
        arCmdDraw(36, 1, 0, 0);
    }
    arCmdEndRendering();
    {
        ArBarrier const barrier = {
            .pImage = &colorFb,
            .oldLayout = AR_IMAGE_LAYOUT_COLOR_ATTACHMENT,
            .newLayout = AR_IMAGE_LAYOUT_SHADER_READ
        };

        arCmdPipelineBarrier(1, &barrier);
    }

    arCmdBeginPresent();
    {
        arCmdBindGraphicsPipeline(&finalImagePipeline);
        arCmdDraw(3, 1, 0, 0);
    }
    arCmdEndPresent();
}

int
main()
{
    ArApplicationInfo const applicationInfo = {
        .pfnInit = init,
        .pfnTeardown = teardown,
        .pfnUpdate = update,
        .pfnUpdateResources = updateResources,
        .pfnResize = resize,
        .pfnRecordCommands = recordCommands,
        .width = 1280,
        .height = 720,
        .enableVsync = true
    };

    arExecute(&applicationInfo);
}
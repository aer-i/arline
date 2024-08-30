#include <arline.h>
#include <windows.h>

struct
{
    ArImage texture;
    ArBuffer vertexBuffer;
    ArPipeline pipeline;
}
static g;

static void
init()
{
    const uint32_t rawPixels[9] = {
        0xFFFFFA7E, 0x372549ff, 0x7722ff77,
        0x37B8B42D, 0xeacdc2ff, 0xb7C0FDFB,
        0x00000000, 0x1a702632, 0x00000000
    };

    ArImageCreateInfo imageCreateInfo;
    imageCreateInfo.usage = AR_IMAGE_USAGE_TEXTURE;
    imageCreateInfo.format = AR_FORMAT_RGBA8_UNORM;
    imageCreateInfo.sampler = AR_SAMPLER_NEAREST_TO_EDGE;
    imageCreateInfo.dstArrayElement = 0;
    imageCreateInfo.width = 3;
    imageCreateInfo.height = 3;
    imageCreateInfo.depth = 1;

    arCreateImage(&g.texture, &imageCreateInfo);
    arUpdateImage(&g.texture, sizeof(rawPixels), rawPixels);

    const float vertices[] = {
        /* positions */-0.5f, -0.5f, /* UVs */ 0.0f, 0.0f,
        /* positions */ 0.5f, -0.5f, /* UVs */ 1.0f, 0.0f,
        /* positions */ 0.0f,  0.5f, /* UVs */ 0.5f, 1.0f
    };

    arCreateStaticBuffer(&g.vertexBuffer, sizeof(vertices), vertices);

    ArBlendAttachment blend;
    blend.blendEnable = false;
    blend.colorWriteMask = AR_COLOR_COMPONENT_RGBA_BITS;

    ArGraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.blendAttachmentCount = 1;
    pipelineCreateInfo.pBlendAttachments = &blend;
    pipelineCreateInfo.depthState.depthTestEnable = false;
    pipelineCreateInfo.depthState.depthWriteEnable = false;
    pipelineCreateInfo.depthState.compareOp = AR_COMPARE_OP_NEVER;
    pipelineCreateInfo.polygonMode = AR_POLYGON_MODE_FILL;
    pipelineCreateInfo.topology = AR_TOPOLOGY_TRIANGLE_STRIP;
    pipelineCreateInfo.cullMode = AR_CULL_MODE_FRONT;
    pipelineCreateInfo.frontFace = AR_FRONT_FACE_COUNTER_CLOCKWISE;

    arCreateShaderFromFile(&pipelineCreateInfo.vertShader, "shaders/main.vert.spv");
    arCreateShaderFromFile(&pipelineCreateInfo.fragShader, "shaders/main.frag.spv");
    arCreateGraphicsPipeline(&g.pipeline, &pipelineCreateInfo);

    arDestroyShader(&pipelineCreateInfo.fragShader);
    arDestroyShader(&pipelineCreateInfo.vertShader);
}

static void
teardown()
{
    arDestroyPipeline(&g.pipeline);
    arDestroyBuffer(&g.vertexBuffer);
    arDestroyImage(&g.texture);
}

static void
update()
{
    arPollEvents();
}

static ArRequest
updateResources()
{
    return AR_REQUEST_NONE;
}

static void
resize() {}

static void
recordCommands()
{
    ArAttachment colorAttachment;
    colorAttachment.pImage = NULL;
    colorAttachment.loadOp = AR_LOAD_OP_CLEAR;
    colorAttachment.storeOp = AR_STORE_OP_STORE;
    colorAttachment.clearValue.color.float32[0] = 0.25f;
    colorAttachment.clearValue.color.float32[1] = 0.5f;
    colorAttachment.clearValue.color.float32[2] = 0.75f;
    colorAttachment.clearValue.color.float32[3] = 1.0f;

    ArBarrier barriers[2];
    barriers[0].pImage = NULL;
    barriers[0].oldLayout = AR_IMAGE_LAYOUT_UNDEFINED;
    barriers[0].newLayout = AR_IMAGE_LAYOUT_COLOR_ATTACHMENT;

    barriers[1].pImage = NULL;
    barriers[1].oldLayout = AR_IMAGE_LAYOUT_COLOR_ATTACHMENT;
    barriers[1].newLayout = AR_IMAGE_LAYOUT_PRESENT_SRC;

    arCmdPipelineBarrier(1, &barriers[0]);
    arCmdBeginRendering(1, &colorAttachment, NULL);

    arCmdBindGraphicsPipeline(&g.pipeline);
    arCmdPushConstants(0, sizeof(g.vertexBuffer.address), &g.vertexBuffer.address);
    arCmdDraw(3, 1, 0, 0);

    arCmdEndRendering();
    arCmdPipelineBarrier(1, &barriers[1]);
}

static int
main(void)
{
    ArApplicationInfo applicationInfo;
    applicationInfo.pfnInit = init;
    applicationInfo.pfnTeardown = teardown;
    applicationInfo.pfnResize = resize;
    applicationInfo.pfnRecordCommands = recordCommands;
    applicationInfo.pfnUpdate = update;
    applicationInfo.pfnUpdateResources = updateResources;
    applicationInfo.width = 1280;
    applicationInfo.height = 720;
    applicationInfo.enableVsync = true;

    arExecute(&applicationInfo);
    ExitProcess(0);
}

int _fltused;

#pragma function(memset)
void*
memset(void* dest, int c, size_t size)
{
    byte* bytes = (byte*)dest;

    while (size--)
    {
        *bytes++ = (byte)c;
    }
    
    return dest;
}

#pragma function(memcpy)
void*
memcpy(void* dst, const void* src, size_t size)
{
    byte* dst8 = (byte*)dst;
    byte const* src8 = (byte const*)src;

    while (size--)
    {
        *dst8++ = *src8++;
    }

    return dst;
}
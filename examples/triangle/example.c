#include <arline.h>
#include <windows.h>

int _fltused;

static ArPipeline g_pipeline;

static void
init()
{
    ArBlendAttachment blend;
    blend.blendEnable = AR_FALSE;
    blend.colorWriteMask = AR_COLOR_COMPONENT_RGBA_BITS;

    ArGraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.blendAttachmentCount = 1;
    pipelineCreateInfo.pBlendAttachments = &blend;
    pipelineCreateInfo.polygonMode = AR_POLYGON_MODE_FILL;
    pipelineCreateInfo.topology = AR_TOPOLOGY_TRIANGLE_STRIP;
    pipelineCreateInfo.cullMode = AR_CULL_MODE_BACK;

    arCreateShaderFromFile(&pipelineCreateInfo.vertShader, "shaders/main.vert.spv");
    arCreateShaderFromFile(&pipelineCreateInfo.fragShader, "shaders/main.frag.spv");
    arCreateGraphicsPipeline(&g_pipeline, &pipelineCreateInfo);

    arDestroyShader(pipelineCreateInfo.fragShader);
    arDestroyShader(pipelineCreateInfo.vertShader);
}

static void
teardown()
{
    arDestroyPipeline(g_pipeline);
}

static void
update()
{
    arWaitEvents();
}

static void
resize() {}

static void
recordCommands()
{
    arCmdBeginPresent();

    arCmdBindGraphicsPipeline(g_pipeline);
    arCmdDraw(3, 1, 0, 0);

    arCmdEndPresent();
}

static int
main(void)
{
    ArApplicationInfo applicationInfo;
    applicationInfo.pfnInit = init;
    applicationInfo.pfnTeardown = teardown;
    applicationInfo.pfnUpdate = update;
    applicationInfo.pfnResize = resize;
    applicationInfo.pfnRecordCommands = recordCommands;
    applicationInfo.width = 1280;
    applicationInfo.height = 720;
    applicationInfo.enableVsync = AR_TRUE;

    arExecute(&applicationInfo);
    ExitProcess(0);
}
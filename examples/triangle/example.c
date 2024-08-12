#include <arline.h>
#include <windows.h>

int _fltused;

static ArPipeline g_pipeline;

static void
init()
{
    ArGraphicsPipelineCreateInfo pipelineCreateInfo;
    arCreateShaderFromFile(&pipelineCreateInfo.vertShader, "shaders/main.vert.spv");
    arCreateShaderFromFile(&pipelineCreateInfo.fragShader, "shaders/main.frag.spv");

    pipelineCreateInfo;
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
recordCommands(ArCommandBuffer cmd)
{
    arCmdBeginPresent(cmd);

    arCmdBindGraphicsPipeline(cmd, g_pipeline);
    arCmdDraw(cmd, 3, 1, 0, 0);

    arCmdEndPresent(cmd);
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
    applicationInfo.enableVsync = true;

    arExecute(&applicationInfo);
    ExitProcess(0);
}
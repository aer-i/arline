#ifndef ARLINE_H
#define ARLINE_H

#include <stdint.h>
#include <stdbool.h>

typedef unsigned char ArShaderStage;
#define AR_SHADER_STAGE_VERTEX          0x01
#define AR_SHADER_STAGE_FRAGMENT        0x10
#define AR_SHADER_STAGE_COMPUTE         0x20

typedef unsigned char ArTopology;
#define AR_TOPOLOGY_POINT               0x00
#define AR_TOPOLOGY_LINE_LIST           0x01
#define AR_TOPOLOGY_LINE_STRIP          0x02
#define AR_TOPOLOGY_TRIANGLE_LIST       0x03
#define AR_TOPOLOGY_TRIANGLE_STRIP      0x04
#define AR_TOPOLOGY_TRIANGLE_FAN        0x04

typedef unsigned char ArPolygonMode;
#define AR_POLYGON_MODE_FILL            0x00
#define AR_POLYGON_MODE_LINE            0x01
#define AR_POLYGON_MODE_POINT           0x02

typedef unsigned char ArCullMode;
#define AR_CULL_MODE_NONE               0x00
#define AR_CULL_MODE_FRONT              0x01
#define AR_CULL_MODE_BACK               0x02

typedef unsigned char ArLoadOp;
#define AR_LOAD_OP_LOAD                 0x00
#define AR_LOAD_OP_CLEAR                0x01
#define AR_LOAD_OP_DONT_CARE            0x02

typedef unsigned char ArStoreOp;
#define AR_STORE_OP_STORE               0x00
#define AR_STORE_OP_DONT_CARE           0x01

typedef unsigned char ArImageUsage;
#define AR_IMAGE_USAGE_COLOR_ATTACH     0x00
#define AR_IMAGE_USAGE_DEPTH_ATTACH     0x01

typedef unsigned char ArImageLayout;
#define AR_IMAGE_LAYOUT_COLOR_ATTACH    0x02
#define AR_IMAGE_LAYOUT_DEPTH_ATTACH    0x03
#define AR_IMAGE_LAYOUT_DEPTH_READ      0x04
#define AR_IMAGE_LAYOUT_SHADER_READ     0x05

typedef unsigned char ArCompareOp;
#define AR_COMPARE_OP_NEVER             0x00
#define AR_COMPARE_OP_LESS              0x01
#define AR_COMPARE_OP_EQUAL             0x02
#define AR_COMPARE_OP_LEQUAL            0x03
#define AR_COMPARE_OP_GREATER           0x04
#define AR_COMPARE_OP_NEQUAL            0x05
#define AR_COMPARE_OP_GEQUAL            0x06
#define AR_COMPARE_OP_ALWAYS            0x07

typedef unsigned char ArRequest;
#define AR_REQUEST_NONE                 0x00
#define AR_REQUEST_RECORD_COMMANDS      0x01
#define AR_REQUEST_VSYNC_ENABLE         0x02
#define AR_REQUEST_VSYNC_DISABLE        0x03

typedef struct
{
    void* _data;
}
ArShader, ArPipeline, ArCommandBuffer;

typedef struct ArApplicationInfo
{
    void (*pfnInit)();
    void (*pfnTeardown)();
    void (*pfnUpdate)();
    void (*pfnResize)();
    void (*pfnRecordCommands)(ArCommandBuffer);
    int  width, height;
    bool enableVsync;
}
ArApplicationInfo;

typedef struct ArGraphicsPipelineCreateInfo
{
    ArShader vertShader;
    ArShader fragShader;
}
ArGraphicsPipelineCreateInfo;

#ifdef __cplusplus
extern "C" {
#endif

void arExecute(ArApplicationInfo const* pApplicationInfo);
void arPollEvents(void);
void arWaitEvents(void);
void arCreateShaderFromFile(ArShader* pShader, char const* filename);
void arCreateShaderFromMemory(ArShader* pShader, uint32_t const* pCode, size_t codeSize);
void arDestroyShader(ArShader shader);
void arCreateGraphicsPipeline(ArPipeline* pPipeline, ArGraphicsPipelineCreateInfo const* pPipelineCreateInfo);
void arDestroyPipeline(ArPipeline pipeline);
void arCmdBeginPresent(ArCommandBuffer cmd);
void arCmdEndPresent(ArCommandBuffer cmd);
void arCmdBindGraphicsPipeline(ArCommandBuffer cmd, ArPipeline pipeline);
void arCmdDraw(ArCommandBuffer cmd, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertex, uint32_t instance);

#ifdef __cplusplus
}
#endif
#endif
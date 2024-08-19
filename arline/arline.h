#ifndef ARLINE_H
#define ARLINE_H

#include <stdint.h>

typedef unsigned char ArKey;
#define AR_KEY_A                            0x41
#define AR_KEY_B                            0x42
#define AR_KEY_C                            0x43
#define AR_KEY_D                            0x44
#define AR_KEY_E                            0x45
#define AR_KEY_F                            0x46
#define AR_KEY_G                            0x47
#define AR_KEY_H                            0x48
#define AR_KEY_I                            0x49
#define AR_KEY_J                            0x4a
#define AR_KEY_K                            0x4b
#define AR_KEY_L                            0x4c
#define AR_KEY_M                            0x4d
#define AR_KEY_N                            0x4e
#define AR_KEY_O                            0x4f
#define AR_KEY_P                            0x50
#define AR_KEY_Q                            0x51
#define AR_KEY_R                            0x52
#define AR_KEY_S                            0x53
#define AR_KEY_T                            0x54
#define AR_KEY_U                            0x55
#define AR_KEY_V                            0x56
#define AR_KEY_W                            0x57
#define AR_KEY_X                            0x58
#define AR_KEY_Y                            0x59
#define AR_KEY_Z                            0x5a
#define AR_KEY_0                            0x30
#define AR_KEY_1                            0x31
#define AR_KEY_2                            0x32
#define AR_KEY_3                            0x33
#define AR_KEY_4                            0x34
#define AR_KEY_5                            0x35
#define AR_KEY_6                            0x36
#define AR_KEY_7                            0x37
#define AR_KEY_8                            0x38
#define AR_KEY_9                            0x39
#define AR_KEY_F1                           0x70
#define AR_KEY_F2                           0x71
#define AR_KEY_F3                           0x72
#define AR_KEY_F4                           0x73
#define AR_KEY_F5                           0x74
#define AR_KEY_F6                           0x75
#define AR_KEY_F7                           0x76
#define AR_KEY_F8                           0x77
#define AR_KEY_F9                           0x78
#define AR_KEY_F10                          0x79
#define AR_KEY_F11                          0x7a
#define AR_KEY_F12                          0x7b
#define AR_KEY_F13                          0x7c
#define AR_KEY_F14                          0x7d
#define AR_KEY_F15                          0x7e
#define AR_KEY_F16                          0x7f
#define AR_KEY_F17                          0x80
#define AR_KEY_F18                          0x81
#define AR_KEY_F19                          0x82
#define AR_KEY_F20                          0x83
#define AR_KEY_F21                          0x84
#define AR_KEY_F22                          0x85
#define AR_KEY_F23                          0x86
#define AR_KEY_F24                          0x87
#define AR_KEY_SPACE                        0x20
#define AR_KEY_APOSTROPHE                   0xde
#define AR_KEY_COMMA                        0xbc
#define AR_KEY_MINUS                        0xbd
#define AR_KEY_PERIOD                       0xbe
#define AR_KEY_SLASH                        0xbf
#define AR_KEY_SEMICOLON                    0xba
#define AR_KEY_PLUS                         0xbb
#define AR_KEY_LBRACKET                     0xdb
#define AR_KEY_RBRACKET                     0xdd
#define AR_KEY_BACKSLASH                    0xdc
#define AR_KEY_GRAVE                        0xc0
#define AR_KEY_ESCAPE                       0x1b
#define AR_KEY_ENTER                        0x0d
#define AR_KEY_TAB                          0x09
#define AR_KEY_BACKSPACE                    0x08
#define AR_KEY_INSERT                       0x2d
#define AR_KEY_DELETE                       0x2e
#define AR_KEY_LEFT                         0x25
#define AR_KEY_UP                           0x26
#define AR_KEY_RIGHT                        0x27
#define AR_KEY_DOWN                         0x28
#define AR_KEY_PAGE_UP                      0x21
#define AR_KEY_PAGE_DOWN                    0x22
#define AR_KEY_HOME                         0x24
#define AR_KEY_END                          0x23
#define AR_KEY_CAPS_LOCK                    0x14
#define AR_KEY_SCROLL_LOCK                  0x91
#define AR_KEY_PRINT_SCREEN                 0x2c
#define AR_KEY_PAUSE                        0x12
#define AR_KEY_SHIFT                        0x10
#define AR_KEY_CTRL                         0x11
#define AR_KEY_ALT                          0x12
#define AR_KEY_MENU                         0x5d

typedef unsigned char ArButton;
#define AR_BUTTON_LEFT                      0x00
#define AR_BUTTON_RIGHT                     0x01
#define AR_BUTTON_MIDDLE                    0x02
#define AR_BUTTON_BACKWARD                  0x03
#define AR_BUTTON_FORWARD                   0x04

typedef unsigned char ArBool8;
#define AR_FALSE                            0x00
#define AR_TRUE                             0x01

typedef unsigned char ArBlendOp;
#define AR_BLEND_OP_ADD                     0x00
#define AR_BLEND_OP_SUB                     0x01
#define AR_BLEND_OP_REVERSE_SUB             0x02
#define AR_BLEND_OP_MIN                     0x03
#define AR_BLEND_OP_MAX                     0x04

typedef unsigned char ArBlendFactor;
#define AR_BLEND_FACTOR_ZERO                0x00
#define AR_BLEND_FACTOR_ONE                 0x01
#define AR_BLEND_FACTOR_SRC_COLOR           0x02
#define AR_BLEND_FACTOR_ONE_MIN_SRC_COLOR   0x03
#define AR_BLEND_FACTOR_DST_COLOR           0x04
#define AR_BLEND_FACTOR_ONE_MIN_DST_COLOR   0x05
#define AR_BLEND_FACTOR_SRC_ALPHA           0x06
#define AR_BLEND_FACTOR_ONE_MIN_SRC_ALPHA   0x07
#define AR_BLEND_FACTOR_DST_ALPHA           0x08
#define AR_BLEND_FACTOR_ONE_MIN_DST_ALPHA   0x09

typedef unsigned char ArColorComponentFlags;
#define AR_COLOR_COMPONENT_R_BIT            0x01
#define AR_COLOR_COMPONENT_G_BIT            0x02
#define AR_COLOR_COMPONENT_B_BIT            0x04
#define AR_COLOR_COMPONENT_A_BIT            0x08
#define AR_COLOR_COMPONENT_RGB_BITS         0x07
#define AR_COLOR_COMPONENT_RGBA_BITS        0x0f

typedef unsigned char ArFrontFace;
#define AR_FRONT_FACE_COUNTER_CLOCKWISE     0x00
#define AR_FRONT_FACE_CLOCKWISE             0x01

typedef unsigned char ArShaderStage;
#define AR_SHADER_STAGE_VERTEX              0x01
#define AR_SHADER_STAGE_FRAGMENT            0x10
#define AR_SHADER_STAGE_COMPUTE             0x20

typedef unsigned char ArTopology;
#define AR_TOPOLOGY_POINT                   0x00
#define AR_TOPOLOGY_LINE_LIST               0x01
#define AR_TOPOLOGY_LINE_STRIP              0x02
#define AR_TOPOLOGY_TRIANGLE_LIST           0x03
#define AR_TOPOLOGY_TRIANGLE_STRIP          0x04
#define AR_TOPOLOGY_TRIANGLE_FAN            0x04

typedef unsigned char ArPolygonMode;
#define AR_POLYGON_MODE_FILL                0x00
#define AR_POLYGON_MODE_LINE                0x01
#define AR_POLYGON_MODE_POINT               0x02

typedef unsigned char ArCullMode;
#define AR_CULL_MODE_NONE                   0x00
#define AR_CULL_MODE_FRONT                  0x01
#define AR_CULL_MODE_BACK                   0x02
#define AR_CULL_MODE_BOTH                   0x03

typedef unsigned char ArLoadOp;
#define AR_LOAD_OP_LOAD                     0x00
#define AR_LOAD_OP_CLEAR                    0x01
#define AR_LOAD_OP_DONT_CARE                0x02

typedef unsigned char ArStoreOp;
#define AR_STORE_OP_STORE                   0x00
#define AR_STORE_OP_DONT_CARE               0x01

typedef unsigned char ArImageUsage;
#define AR_IMAGE_USAGE_COLOR_ATTACHMENT     0x00
#define AR_IMAGE_USAGE_DEPTH_ATTACHMENT     0x02
#define AR_IMAGE_USAGE_TEXTURE              0x04

typedef unsigned char ArImageLayout;
#define AR_IMAGE_LAYOUT_COLOR_ATTACHMENT    0x02
#define AR_IMAGE_LAYOUT_DEPTH_ATTACHMENT    0x03
#define AR_IMAGE_LAYOUT_DEPTH_READ          0x04
#define AR_IMAGE_LAYOUT_SHADER_READ         0x05
#define AR_IMAGE_LAYOUT_TRANSFER_SRC        0x06
#define AR_IMAGE_LAYOUT_TRANSFER_DST        0x07

typedef unsigned char ArSampler;
#define AR_SAMPLER_NONE                     0x00
#define AR_SAMPLER_LINEAR_TO_EDGE           0x01
#define AR_SAMPLER_LINEAR_REPEAT            0x02
#define AR_SAMPLER_NEAREST_TO_EDGE          0x03
#define AR_SAMPLER_NEAREST_REPEAT           0x04

typedef unsigned char ArCompareOp;
#define AR_COMPARE_OP_NEVER                 0x00
#define AR_COMPARE_OP_LESS                  0x01
#define AR_COMPARE_OP_EQUAL                 0x02
#define AR_COMPARE_OP_LEQUAL                0x03
#define AR_COMPARE_OP_GREATER               0x04
#define AR_COMPARE_OP_NEQUAL                0x05
#define AR_COMPARE_OP_GEQUAL                0x06
#define AR_COMPARE_OP_ALWAYS                0x07

typedef unsigned char ArRequest;
#define AR_REQUEST_NONE                     0x00
#define AR_REQUEST_RECORD_COMMANDS          0x01
#define AR_REQUEST_VSYNC_ENABLE             0x02
#define AR_REQUEST_VSYNC_DISABLE            0x03

typedef struct { void* data[3]; } ArImageHandle;
typedef struct { void* data[2]; } ArBufferHandle;
typedef struct { void* data;    } ArShaderHandle;
typedef struct { void* data;    } ArPipelineHandle;

typedef union ArClearColor
{
    float float32[4];
    uint32_t uint32[4];
    int32_t int32[4];
}
ArClearColor;

typedef union ArClearValue
{
    ArClearColor color;
    float depth;
}
ArClearValue;

typedef struct ArBuffer
{
    ArBufferHandle handle;
    uint64_t address;
    uint64_t size;
    void* pMapped;
}
ArBuffer;

typedef struct ArImage
{
    ArImageHandle handle;
    uint32_t index;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
}
ArImage;

typedef struct ArShader
{
    ArShaderHandle handle;
}
ArShader;

typedef struct ArPipeline
{
    ArPipelineHandle handle;
}
ArPipeline;

typedef struct ArApplicationInfo
{
    void (*pfnInit)();
    void (*pfnTeardown)();
    void (*pfnUpdate)();
    ArRequest (*pfnUpdateResources)();
    void (*pfnResize)();
    void (*pfnRecordCommands)();
    int32_t width, height;
    ArBool8 enableVsync;
}
ArApplicationInfo;

typedef struct ArAttachment
{
    ArImage const* pImage;
    ArLoadOp loadOp;
    ArStoreOp storeOp;
    ArClearValue clearValue;
}
ArAttachment;

typedef struct ArBlendAttachment
{
    ArBool8 blendEnable;
    ArBlendOp colorBlendOp;
    ArBlendOp alphaBlendOp;
    ArBlendFactor srcColorFactor;
    ArBlendFactor dstColorFactor;
    ArBlendFactor srcAlphaFactor;
    ArBlendFactor dstAlphaFactor;
    ArColorComponentFlags colorWriteMask;
}
ArBlendAttachment;

typedef struct ArDepthState
{
    ArBool8 depthTestEnable;
    ArBool8 depthWriteEnable;
    ArCompareOp compareOp;
}
ArDepthState;

typedef struct ArImageCreateInfo
{
    ArImageUsage usage;
    ArSampler sampler;
    uint32_t dstArrayElement;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
}
ArImageCreateInfo;

typedef struct ArGraphicsPipelineCreateInfo
{
    uint32_t blendAttachmentCount;
    ArBlendAttachment const* pBlendAttachments;
    ArDepthState depthState;
    ArShader vertShader;
    ArShader fragShader;
    ArPolygonMode polygonMode;
    ArTopology topology;
    ArCullMode cullMode;
    ArFrontFace frontFace;
}
ArGraphicsPipelineCreateInfo;

#ifdef __cplusplus
extern "C" {
#endif

ArBool8 arIsKeyDown(ArKey key);
ArBool8 arIsKeyPressed(ArKey key);
ArBool8 arIsKeyReleased(ArKey key);
ArBool8 arIsButtonDown(ArButton button);
ArBool8 arIsButtonPressed(ArButton button);
ArBool8 arIsButtonReleased(ArButton button);

double arGetTime();
float  arGetTimef();

uint32_t arGetWindowWidth();
uint32_t arGetWindowHeight();
uint32_t arGetRenderWidth();
uint32_t arGetRenderHeight();

float arGetWindowAspectRatio();
float arGetRenderAspectRatio();

void arExecute(ArApplicationInfo const* pApplicationInfo);
void arPollEvents(void);
void arWaitEvents(void);
void arCreateDynamicBuffer(ArBuffer* pBuffer, uint64_t capacity);
void arCreateStaticBuffer(ArBuffer* pBuffer, uint64_t size, void const* pData);
void arDestroyBuffer(ArBuffer* pBuffer);
void arCreateImage(ArImage* pImage, ArImageCreateInfo const* pImageCreateInfo);
void arUpdateImage(ArImage* pImage, size_t size, void const* pData);
void arDestroyImage(ArImage* pImage);
void arCreateShaderFromFile(ArShader* pShader, char const* filename);
void arCreateShaderFromMemory(ArShader* pShader, uint32_t const* pCode, size_t codeSize);
void arDestroyShader(ArShader* pShader);
void arCreateGraphicsPipeline(ArPipeline* pPipeline, ArGraphicsPipelineCreateInfo const* pPipelineCreateInfo);
void arDestroyPipeline(ArPipeline* pPipeline);
void arCmdBeginPresent(void);
void arCmdEndPresent(void);
void arCmdBeginRendering(uint32_t colorAttachmentCount, ArAttachment const* pColorAttachments, ArAttachment const* pDepthAttachment);
void arCmdEndRendering(void);
void arCmdPushConstants(uint32_t offset, uint32_t size, void const* pValues);
void arCmdBindGraphicsPipeline(ArPipeline const* pPipeline);
void arCmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertex, uint32_t instance);

#ifdef __cplusplus
}
#endif
#endif
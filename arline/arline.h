#ifndef ARLINE_H
#define ARLINE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum ArKey {
    AR_KEY_A                                = 0x41,
    AR_KEY_B                                = 0x42,
    AR_KEY_C                                = 0x43,
    AR_KEY_D                                = 0x44,
    AR_KEY_E                                = 0x45,
    AR_KEY_F                                = 0x46,
    AR_KEY_G                                = 0x47,
    AR_KEY_H                                = 0x48,
    AR_KEY_I                                = 0x49,
    AR_KEY_J                                = 0x4a,
    AR_KEY_K                                = 0x4b,
    AR_KEY_L                                = 0x4c,
    AR_KEY_M                                = 0x4d,
    AR_KEY_N                                = 0x4e,
    AR_KEY_O                                = 0x4f,
    AR_KEY_P                                = 0x50,
    AR_KEY_Q                                = 0x51,
    AR_KEY_R                                = 0x52,
    AR_KEY_S                                = 0x53,
    AR_KEY_T                                = 0x54,
    AR_KEY_U                                = 0x55,
    AR_KEY_V                                = 0x56,
    AR_KEY_W                                = 0x57,
    AR_KEY_X                                = 0x58,
    AR_KEY_Y                                = 0x59,
    AR_KEY_Z                                = 0x5a,
    AR_KEY_0                                = 0x30,
    AR_KEY_1                                = 0x31,
    AR_KEY_2                                = 0x32,
    AR_KEY_3                                = 0x33,
    AR_KEY_4                                = 0x34,
    AR_KEY_5                                = 0x35,
    AR_KEY_6                                = 0x36,
    AR_KEY_7                                = 0x37,
    AR_KEY_8                                = 0x38,
    AR_KEY_9                                = 0x39,
    AR_KEY_F1                               = 0x70,
    AR_KEY_F2                               = 0x71,
    AR_KEY_F3                               = 0x72,
    AR_KEY_F4                               = 0x73,
    AR_KEY_F5                               = 0x74,
    AR_KEY_F6                               = 0x75,
    AR_KEY_F7                               = 0x76,
    AR_KEY_F8                               = 0x77,
    AR_KEY_F9                               = 0x78,
    AR_KEY_F10                              = 0x79,
    AR_KEY_F11                              = 0x7a,
    AR_KEY_F12                              = 0x7b,
    AR_KEY_F13                              = 0x7c,
    AR_KEY_F14                              = 0x7d,
    AR_KEY_F15                              = 0x7e,
    AR_KEY_F16                              = 0x7f,
    AR_KEY_F17                              = 0x80,
    AR_KEY_F18                              = 0x81,
    AR_KEY_F19                              = 0x82,
    AR_KEY_F20                              = 0x83,
    AR_KEY_F21                              = 0x84,
    AR_KEY_F22                              = 0x85,
    AR_KEY_F23                              = 0x86,
    AR_KEY_F24                              = 0x87,
    AR_KEY_SPACE                            = 0x20,
    AR_KEY_APOSTROPHE                       = 0xde,
    AR_KEY_COMMA                            = 0xbc,
    AR_KEY_MINUS                            = 0xbd,
    AR_KEY_PERIOD                           = 0xbe,
    AR_KEY_SLASH                            = 0xbf,
    AR_KEY_SEMICOLON                        = 0xba,
    AR_KEY_PLUS                             = 0xbb,
    AR_KEY_LBRACKET                         = 0xdb,
    AR_KEY_RBRACKET                         = 0xdd,
    AR_KEY_BACKSLASH                        = 0xdc,
    AR_KEY_GRAVE                            = 0xc0,
    AR_KEY_ESCAPE                           = 0x1b,
    AR_KEY_ENTER                            = 0x0d,
    AR_KEY_TAB                              = 0x09,
    AR_KEY_BACKSPACE                        = 0x08,
    AR_KEY_INSERT                           = 0x2d,
    AR_KEY_DELETE                           = 0x2e,
    AR_KEY_LEFT                             = 0x25,
    AR_KEY_UP                               = 0x26,
    AR_KEY_RIGHT                            = 0x27,
    AR_KEY_DOWN                             = 0x28,
    AR_KEY_PAGE_UP                          = 0x21,
    AR_KEY_PAGE_DOWN                        = 0x22,
    AR_KEY_HOME                             = 0x24,
    AR_KEY_END                              = 0x23,
    AR_KEY_CAPS_LOCK                        = 0x14,
    AR_KEY_SCROLL_LOCK                      = 0x91,
    AR_KEY_PRINT_SCREEN                     = 0x2c,
    AR_KEY_PAUSE                            = 0x12,
    AR_KEY_SHIFT                            = 0x10,
    AR_KEY_CTRL                             = 0x11,
    AR_KEY_ALT                              = 0x12,
    AR_KEY_MENU                             = 0x5d
} ArKey;

typedef enum ArButton {
    AR_BUTTON_LEFT                          = 0x00,
    AR_BUTTON_RIGHT                         = 0x01,
    AR_BUTTON_MIDDLE                        = 0x02,
    AR_BUTTON_BACKWARD                      = 0x03,
    AR_BUTTON_FORWARD                       = 0x04
} ArButton;

typedef enum ArBlendOp {
    AR_BLEND_OP_ADD                         = 0x00,
    AR_BLEND_OP_SUB                         = 0x01,
    AR_BLEND_OP_REVERSE_SUB                 = 0x02,
    AR_BLEND_OP_MIN                         = 0x03,
    AR_BLEND_OP_MAX                         = 0x04
} ArBlendOp;

typedef enum ArBlendFactor {
    AR_BLEND_FACTOR_ZERO                    = 0x00,
    AR_BLEND_FACTOR_ONE                     = 0x01,
    AR_BLEND_FACTOR_SRC_COLOR               = 0x02,
    AR_BLEND_FACTOR_ONE_MIN_SRC_COLOR       = 0x03,
    AR_BLEND_FACTOR_DST_COLOR               = 0x04,
    AR_BLEND_FACTOR_ONE_MIN_DST_COLOR       = 0x05,
    AR_BLEND_FACTOR_SRC_ALPHA               = 0x06,
    AR_BLEND_FACTOR_ONE_MIN_SRC_ALPHA       = 0x07,
    AR_BLEND_FACTOR_DST_ALPHA               = 0x08,
    AR_BLEND_FACTOR_ONE_MIN_DST_ALPHA       = 0x09
} ArBlendFactor;

typedef enum ArColorComponentFlags {
    AR_COLOR_COMPONENT_R_BIT                = 0x01,
    AR_COLOR_COMPONENT_G_BIT                = 0x02,
    AR_COLOR_COMPONENT_B_BIT                = 0x04,
    AR_COLOR_COMPONENT_A_BIT                = 0x08,
    AR_COLOR_COMPONENT_RGB_BITS             = 0x07,
    AR_COLOR_COMPONENT_RGBA_BITS            = 0x0f
} ArColorComponentFlags;

typedef enum ArFrontFace {
    AR_FRONT_FACE_COUNTER_CLOCKWISE         = 0x00,
    AR_FRONT_FACE_CLOCKWISE                 = 0x01
} ArFrontFace;

typedef enum ArShaderStage {
    AR_SHADER_STAGE_VERTEX                  = 0x01,
    AR_SHADER_STAGE_FRAGMENT                = 0x10,
    AR_SHADER_STAGE_COMPUTE                 = 0x20
} ArShaderStage;

typedef enum ArTopology {
    AR_TOPOLOGY_POINT                       = 0x00,
    AR_TOPOLOGY_LINE_LIST                   = 0x01,
    AR_TOPOLOGY_LINE_STRIP                  = 0x02,
    AR_TOPOLOGY_TRIANGLE_LIST               = 0x03,
    AR_TOPOLOGY_TRIANGLE_STRIP              = 0x04,
    AR_TOPOLOGY_TRIANGLE_FAN                = 0x04
} ArTopology;

typedef enum ArPolygonMode {
    AR_POLYGON_MODE_FILL                    = 0x00,
    AR_POLYGON_MODE_LINE                    = 0x01,
    AR_POLYGON_MODE_POINT                   = 0x02
} ArPolygonMode;

typedef enum ArCullMode {
    AR_CULL_MODE_NONE                       = 0x00,
    AR_CULL_MODE_FRONT                      = 0x01,
    AR_CULL_MODE_BACK                       = 0x02,
    AR_CULL_MODE_BOTH                       = 0x03
} ArCullMode;

typedef enum ArLoadOp {
    AR_LOAD_OP_LOAD                         = 0x00,
    AR_LOAD_OP_CLEAR                        = 0x01,
    AR_LOAD_OP_DONT_CARE                    = 0x02
} ArLoadOp;

typedef enum ArStoreOp {
    AR_STORE_OP_STORE                       = 0x00,
    AR_STORE_OP_DONT_CARE                   = 0x01
} ArStoreOp;

typedef enum ArImageUsage {   
    AR_IMAGE_USAGE_COLOR_ATTACHMENT         = 0x00,
    AR_IMAGE_USAGE_DEPTH_ATTACHMENT         = 0x02,
    AR_IMAGE_USAGE_TEXTURE                  = 0x04
} ArImageUsage;

typedef enum ArImageLayout {
    AR_IMAGE_LAYOUT_UNDEFINED               = 0x00,
    AR_IMAGE_LAYOUT_COLOR_ATTACHMENT        = 0x02,
    AR_IMAGE_LAYOUT_DEPTH_ATTACHMENT        = 0x03,
    AR_IMAGE_LAYOUT_SHADER_READ             = 0x05,
    AR_IMAGE_LAYOUT_TRANSFER_SRC            = 0x06,
    AR_IMAGE_LAYOUT_TRANSFER_DST            = 0x07,
    AR_IMAGE_LAYOUT_PRESENT_SRC             = 0x08
} ArImageLayout;

typedef enum ArSampler {
    AR_SAMPLER_NONE                         = 0x00,
    AR_SAMPLER_LINEAR_TO_EDGE               = 0x01,
    AR_SAMPLER_LINEAR_REPEAT                = 0x02,
    AR_SAMPLER_NEAREST_TO_EDGE              = 0x03,
    AR_SAMPLER_NEAREST_REPEAT               = 0x04
} ArSampler;

typedef enum ArFormat {
    AR_FORMAT_UNDEFINED                     = 0x00,
    AR_FORMAT_R8_UNORM                      = 0x09,
    AR_FORMAT_R8_SNORM                      = 0x0a,
    AR_FORMAT_R8_UINT                       = 0x0d,
    AR_FORMAT_R8_SINT                       = 0x0e,
    AR_FORMAT_RG8_UNORM                     = 0x10,
    AR_FORMAT_RG8_SNORM                     = 0x11,
    AR_FORMAT_RG8_UINT                      = 0x14,
    AR_FORMAT_RG8_SINT                      = 0x15,
    AR_FORMAT_RGBA8_UNORM                   = 0x25,
    AR_FORMAT_RGBA8_SNORM                   = 0x26,
    AR_FORMAT_RGBA8_UINT                    = 0x29,
    AR_FORMAT_RGBA8_SINT                    = 0x2a
} ArFormat;

typedef enum ArCompareOp {
    AR_COMPARE_OP_NEVER                     = 0x00,
    AR_COMPARE_OP_LESS                      = 0x01,
    AR_COMPARE_OP_EQUAL                     = 0x02,
    AR_COMPARE_OP_LEQUAL                    = 0x03,
    AR_COMPARE_OP_GREATER                   = 0x04,
    AR_COMPARE_OP_NEQUAL                    = 0x05,
    AR_COMPARE_OP_GEQUAL                    = 0x06,
    AR_COMPARE_OP_ALWAYS                    = 0x07
} ArCompareOp;

typedef enum ArIndexType {
    AR_INDEX_TYPE_UINT16                    = 0x00,
    AR_INDEX_TYPE_UINT32                    = 0x01
} ArIndexType;

typedef enum ArRequest {
    AR_REQUEST_NONE                         = 0x00,
    AR_REQUEST_RECORD_COMMANDS              = 0x01,
    AR_REQUEST_VSYNC_ENABLE                 = 0x02,
    AR_REQUEST_VSYNC_DISABLE                = 0x03
} ArRequest;

typedef struct ArImageHandle {
    void*                                   data[3];
} ArImageHandle;

typedef struct ArBufferHandle {
    void*                                   data[2];
} ArBufferHandle;

typedef struct ArShaderHandle {
    void*                                   data;
} ArShaderHandle;

typedef struct ArPipelineHandle { 
    void*                                   data;
} ArPipelineHandle;

typedef union ArClearColor {
    float                                   float32[4];
    uint32_t                                uint32[4];
    int32_t                                 int32[4];
} ArClearColor;

typedef union ArClearValue {
    ArClearColor                            color;
    float                                   depth;
} ArClearValue;

typedef struct ArBuffer {
    ArBufferHandle                          handle;
    uint64_t                                address;
    uint64_t                                size;
    void*                                   pMapped;
} ArBuffer;

typedef struct ArImage {
    ArImageHandle                           handle;
    uint32_t                                index;
    uint32_t                                width;
    uint32_t                                height;
    uint32_t                                depth;
} ArImage;

typedef struct ArShader {
    ArShaderHandle                          handle;
} ArShader;

typedef struct ArPipeline {
    ArPipelineHandle                        handle;
} ArPipeline;

typedef struct ArApplicationInfo {
    void                                    (*pfnInit)();
    void                                    (*pfnTeardown)();
    void                                    (*pfnResize)();
    void                                    (*pfnRecordCommands)();
    void                                    (*pfnUpdate)();
    ArRequest                               (*pfnUpdateResources)();
    int                                     width, height;
    bool                                    enableVsync;
} ArApplicationInfo;

typedef struct ArAttachment {
    ArImage const*                          pImage;
    ArLoadOp                                loadOp;
    ArStoreOp                               storeOp;
    ArClearValue                            clearValue;
} ArAttachment;

typedef struct ArBarrier {
    ArImage const*                          pImage;
    ArImageLayout                           oldLayout;
    ArImageLayout                           newLayout;
} ArBarrier;

typedef struct ArDrawIndirectCommand {
    uint32_t                                vertexCount;
    uint32_t                                instanceCount;
    uint32_t                                firstVertex;
    uint32_t                                firstInstance;
} ArDrawIndirectCommand;

typedef struct ArDrawIndexedIndirectCommand {
    uint32_t                                indexCount;
    uint32_t                                instanceCount;
    uint32_t                                firstIndex;
    int32_t                                 vertexOffset;
    uint32_t                                firstInstance;
} ArDrawIndexedIndirectCommand;

typedef struct ArBlendAttachment {
    bool                                    blendEnable;
    ArBlendOp                               colorBlendOp;
    ArBlendOp                               alphaBlendOp;
    ArBlendFactor                           srcColorFactor;
    ArBlendFactor                           dstColorFactor;
    ArBlendFactor                           srcAlphaFactor;
    ArBlendFactor                           dstAlphaFactor;
    ArColorComponentFlags                   colorWriteMask;
} ArBlendAttachment;

typedef struct ArDepthState {
    bool                                    depthTestEnable;
    bool                                    depthWriteEnable;
    ArCompareOp                             compareOp;
} ArDepthState;

typedef struct ArImageCreateInfo {
    ArImageUsage                            usage;
    ArFormat                                format;
    ArSampler                               sampler;
    uint32_t                                dstArrayElement;
    uint32_t                                width;
    uint32_t                                height;
    uint32_t                                depth;
} ArImageCreateInfo;

typedef struct ArGraphicsPipelineCreateInfo {
    uint32_t                                blendAttachmentCount;
    ArBlendAttachment const*                pBlendAttachments;
    ArDepthState                            depthState;
    ArShader                                vertShader;
    ArShader                                fragShader;
    ArPolygonMode                           polygonMode;
    ArTopology                              topology;
    ArCullMode                              cullMode;
    ArFrontFace                             frontFace;
} ArGraphicsPipelineCreateInfo;

#ifdef __cplusplus
extern "C" {
#endif

double arGetTime(void);
double arGetDeltaTime(void);
float  arGetTimef(void);
float  arGetDeltaTimef(void);

float arGetWindowAspectRatio(void);
float arGetRenderAspectRatio(void);

int arGetGlobalCursorX(void);
int arGetGlobalCursorY(void);
int arGetCursorX(void);
int arGetCursorY(void);
int arGetCursorDeltaX(void);
int arGetCursorDeltaY(void);
int arGetRelativeCursorX(void);
int arGetRelativeCursorY(void);

uint32_t arGetWindowWidth(void);
uint32_t arGetWindowHeight(void);
uint32_t arGetRenderWidth(void);
uint32_t arGetRenderHeight(void);

void arPollEvents(void);
void arWaitEvents(void);
void arShowCursor(void);
void arHideCursor(void);

bool arIsKeyDown(
    ArKey                                   key);

bool arIsKeyPressed(
    ArKey                                   key);

bool arIsKeyReleased(
    ArKey                                   key);

bool arIsButtonDown(
    ArButton                                button);

bool arIsButtonPressed(
    ArButton                                button);

bool arIsButtonReleased(
    ArButton                                button);

void arSetCursorPosition(
    int                                     x,
    int                                     y);

void arExecute(
    ArApplicationInfo const*                pApplicationInfo);

void arSetWindowTitle(
    char const*                             title);

void arCreateDynamicBuffer(
    ArBuffer*                               pBuffer,
    uint64_t                                capacity);

void arCreateStaticBuffer(
    ArBuffer*                               pBuffer,
    uint64_t                                size,
    void const*                             pData);

void arDestroyBuffer(
    ArBuffer const*                         pBuffer);

void arCreateImage(
    ArImage*                                pImage,
    ArImageCreateInfo const*                pImageCreateInfo);

void arUpdateImage(
    ArImage*                                pImage,
    size_t                                  dataSize,
    void const*                             pData);

void arDestroyImage(
    ArImage const*                          pImage);

void arCreateShaderFromFile(
    ArShader*                               pShader,
    char const*                             filename);

void arCreateShaderFromMemory(
    ArShader*                               pShader,
    uint32_t const*                         pCode,
    size_t                                  codeSize);

void arDestroyShader(
    ArShader const*                         pShader);

void arCreateGraphicsPipeline(
    ArPipeline*                             pPipeline,
    ArGraphicsPipelineCreateInfo const*     pPipelineCreateInfo);

void arDestroyPipeline(
    ArPipeline const*                       pPipeline);

void arCmdBeginRendering(
    uint32_t                                colorAttachmentCount,
    ArAttachment const*                     pColorAttachments,
    ArAttachment const*                     pDepthAttachment);
    
void arCmdEndRendering(void);

void arCmdPushConstants(
    uint32_t                                offset,
    uint32_t                                size,
    void const*                             pValues);

void arCmdBindIndexBuffer(
    ArBuffer const*                         pBuffer,
    uint64_t                                offset,
    ArIndexType                             indexType);

void arCmdBindGraphicsPipeline(
    ArPipeline const*                       pPipeline);

void arCmdPipelineBarrier(
    uint32_t                                barrierCount,
    ArBarrier const*                        pBarriers);

void arCmdDraw(
    uint32_t                                vertexCount,
    uint32_t                                instanceCount,
    uint32_t                                firstVertex,
    uint32_t                                firstInstance);

void arCmdDrawIndirect(
    ArBuffer const*                         pBuffer,
    uint64_t                                offset,
    uint32_t                                drawCount,
    uint32_t                                stride);

void arCmdDrawIndirectCount(
    ArBuffer const*                         pBuffer,
    uint64_t                                offset,
    ArBuffer const*                         pCountBuffer,
    uint64_t                                countBufferOffset,
    uint32_t                                maxDrawCount,
    uint32_t                                stride);

void arCmdDrawIndexed(
    uint32_t                                indexCount,
    uint32_t                                instanceCount,
    uint32_t                                firstIndex,
    int32_t                                 vertexOffset,
    uint32_t                                firstInstance);

void arCmdDrawIndexedIndirect(
    ArBuffer const*                         pBuffer,
    uint64_t                                offset,
    uint32_t                                drawCount,
    uint32_t                                stride);

void arCmdDrawIndexedIndirectCount(
    ArBuffer const*                         pBuffer,
    uint64_t                                offset,
    ArBuffer const*                         pCountBuffer,
    uint64_t                                countBufferOffset,
    uint32_t                                maxDrawCount,
    uint32_t                                stride);

#ifdef __cplusplus
}
#endif
#endif
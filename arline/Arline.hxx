#pragma once
#ifdef _MSC_VER
#   pragma warning(push, 0)
#elif __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunused-variable"
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wnullability-extension"
#   pragma clang diagnostic ignored "-Wnullability-completeness"
#elif __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wunused-variable"
#   pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include "volk.hxx"
#include "vma.hxx"
#include <initializer_list>

#ifdef _MSC_VER
#   pragma warning(pop)
#elif __clang__
#   pragma clang diagnostic pop
#elif __GNUC__
#   pragma GCC diagnostic pop
#endif

namespace ar
{
    namespace types
    {
        using b8  = bool;
        using u8  = unsigned char;
        using s8  = signed char;
        using u16 = unsigned short;
        using s16 = short;
        using u32 = unsigned int;
        using s32 = int;
        using u64 = unsigned long long;
        using s64 = long long;
        using f32 = float;
        using f64 = double;
    }

    using types::b8;
    using types::u8;
    using types::s8;
    using types::u16;
    using types::s16;
    using types::u32;
    using types::s32;
    using types::u64;
    using types::s64;
    using types::f32;
    using types::f64;

    enum class Key : u8
    {
        eUnknown               = 0x00,
        eA                     = 0x01,
        eB                     = 0x02,
        eC                     = 0x03,
        eD                     = 0x04,
        eE                     = 0x05,
        eF                     = 0x06,
        eG                     = 0x07,
        eH                     = 0x08,
        eI                     = 0x09,
        eJ                     = 0x0a,
        eK                     = 0x0b,
        eL                     = 0x0c,
        eM                     = 0x0d,
        eN                     = 0x0e,
        eO                     = 0x0f,
        eP                     = 0x10,
        eQ                     = 0x11,
        eR                     = 0x12,
        eS                     = 0x13,
        eT                     = 0x14,
        eU                     = 0x15,
        eV                     = 0x16,
        eW                     = 0x17,
        eX                     = 0x18,
        eY                     = 0x19,
        eZ                     = 0x1a,
        e0                     = 0x1b,
        e1                     = 0x1c,
        e2                     = 0x1d,
        e3                     = 0x1e,
        e4                     = 0x1f,
        e5                     = 0x20,
        e6                     = 0x21,
        e7                     = 0x22,
        e8                     = 0x23,
        e9                     = 0x24,
        eF1                    = 0x25,
        eF2                    = 0x26,
        eF3                    = 0x27,
        eF4                    = 0x28,
        eF5                    = 0x29,
        eF6                    = 0x2a,
        eF7                    = 0x2b,
        eF8                    = 0x2c,
        eF9                    = 0x2d,
        eF10                   = 0x2e,
        eF11                   = 0x2f,
        eF12                   = 0x30,
        eSpace                 = 0x31,
        eApostrophe            = 0x32,
        eComma                 = 0x33,
        eMinus                 = 0x34,
        ePeriod                = 0x35,
        eSlash                 = 0x36,
        eSemicolon             = 0x37,
        ePlus                  = 0x38,
        eLBracket              = 0x39,
        eRBracket              = 0x3a,
        eBackslash             = 0x3b,
        eGraveAccent           = 0x3c,
        eEscape                = 0x3d,
        eEnter                 = 0x3e,
        eTab                   = 0x3f,
        eBackspace             = 0x40,
        eInsert                = 0x41,
        eDelete                = 0x42,
        eRight                 = 0x43,
        eLeft                  = 0x44,
        eDown                  = 0x45,
        eUp                    = 0x46,
        ePageDown              = 0x47,
        ePageUp                = 0x48,
        eHome                  = 0x49,
        eEnd                   = 0x4a,
        eCapsLock              = 0x4b,
        eScrollLock            = 0x4c,
        ePrintScreen           = 0x4d,
        ePause                 = 0x4e,
        eShift                 = 0x4f,
        eCtrl                  = 0x50,
        eAlt                   = 0x51,
        eSuper                 = 0x52,
        eMenu                  = 0x53
    };

    enum class Button : u8
    {
        eLeft                  = 0x00,
        eRight                 = 0x01,
        eMiddle                = 0x02,
        eSide1                 = 0x03,
        eSide2                 = 0x04
    };

    enum class ShaderStage : u8
    {
        eVertex                = 0x01,
        eFragment              = 0x10,
        eCompute               = 0x20
    };

    enum class Topology : u8
    {
        ePoint                 = 0x00,
        eLineList              = 0x01,
        eLineStrip             = 0x02,
        eTriangleList          = 0x03,
        eTriangleStrip         = 0x04,
        eTriangleFan           = 0x05
    };

    enum class PolygonMode : u8
    {
        eFill                  = 0x00,
        eLine                  = 0x01,
        ePoint                 = 0x02
    };

    enum class CullMode : u8
    {
        eNone                  = 0x00,
        eFront                 = 0x01,
        eBack                  = 0x02
    };

    enum class LoadOp : u8
    {
        eLoad                  = 0x00,
        eClear                 = 0x01,
        eDontCare              = 0x02
    };

    enum class StoreOp : u8
    {
        eStore                 = 0x00,
        eDontCare              = 0x01
    };

    enum class ImageUsage : u8
    {
        eColorAttachment       = 0x00,
        eDepthAttachment       = 0x01
    };

    enum class ImageLayout : u8
    {
        eColorAttachment       = 0x02,
        eDepthAttachment       = 0x03,
        eDepthReadOnly         = 0x04,
        eShaderReadOnly        = 0x05
    };

    enum class CompareOp : u8
    {
        eNever                 = 0x00,
        eLess                  = 0x01,
        eEqual                 = 0x02,
        eLequal                = 0x03,
        eGreater               = 0x04,
        eNotEqual              = 0x05,
        eGequal                = 0x06,
        eAlways                = 0x07
    };

    enum class Sampler : u8
    {
        eNone                  = 0x00,
        eLinearToEdge          = 0x01,
        eLinearRepeat          = 0x02,
        eNearestToEdge         = 0x03,
        eNearestRepeat         = 0x04
    };

    enum class Request : u8
    {
        eNone                  = 0x00,
        eRecordCommands        = 0x01,
        eEnableVsync           = 0x02,
        eDisableVsync          = 0x03
    };

    enum class BlendOp : u8
    {
        eAdd                   = 0x00,
        eSubtract              = 0x01,
        eReverseSubtract       = 0x02,
        eMin                   = 0x03,
        eMax                   = 0x04
    };

    enum class BlendFactor : u8
    {
        eZero                  = 0x00,
        eOne                   = 0x01,
        eSrcColor              = 0x02,
        eOneMinusSrcColor      = 0x03,
        eDstColor              = 0x04,
        eOneMinusDstColor      = 0x05,
        eSrcAlpha              = 0x06,
        eOneMinusSrcAlpha      = 0x07,
        eDstAlpha              = 0x08,
        eOneMinusDstAlpha      = 0x09,
        eConstantColor         = 0x0a,
        eOneMinusConstantColor = 0x0b,
        eConstantAlpha         = 0x0c,
        eOneMinusConstantAlpha = 0x0d,
        eSrcAlphaSaturate      = 0x0e,
        eSrcColor1             = 0x0f,
        eOneMinusSrcColor1     = 0x10,
        eSrcApha1              = 0x11,
        eOneMinusSrcApha1      = 0x12
    };

    enum class ColorComponent : u8
    {
        eR                     = 0x01,
        eG                     = 0x02,
        eRG                    = 0x03,
        eB                     = 0x04,
        eRB                    = 0x05,
        eGB                    = 0x06,
        eRGB                   = 0x07,
        eA                     = 0x08,
        eRA                    = 0x09,
        eGA                    = 0x0a,
        eRGA                   = 0x0b,
        eBA                    = 0x0c,
        eRBA                   = 0x0d,
        eGBA                   = 0x0e,
        eRGBA                  = 0x0f
    };

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmissing-braces"
#endif

    union ClearColor
    {
        f32 float32[4];
        u32 uint32[4];
        s32 int32[4];

        static consteval auto Blank()   -> ClearColor { return {0.0f, 0.0f, 0.0f, 0.0f}; }
        static consteval auto Black()   -> ClearColor { return {0.0f, 0.0f, 0.0f, 1.0f}; }
        static consteval auto White()   -> ClearColor { return {1.0f, 1.0f, 1.0f, 1.0f}; }
        static consteval auto Red()     -> ClearColor { return {1.0f, 0.0f, 0.0f, 1.0f}; }
        static consteval auto Green()   -> ClearColor { return {0.0f, 1.0f, 0.0f, 1.0f}; }
        static consteval auto Blue()    -> ClearColor { return {0.0f, 0.0f, 1.0f, 1.0f}; }
        static consteval auto Cyan()    -> ClearColor { return {0.0f, 1.0f, 1.0f, 1.0f}; }
        static consteval auto Magenta() -> ClearColor { return {1.0f, 0.0f, 1.0f, 1.0f}; }
        static consteval auto Yellow()  -> ClearColor { return {1.0f, 1.0f, 0.0f, 1.0f}; }
    };

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

    struct GraphicsCommands;
    struct AppInfo
    {
        void    (*infoCallback)(char const*);
        void    (*errorCallback)(char const*);
        void    (*onInit)();
        void    (*onDestroy)();
        void    (*onResize)();
        void    (*onCommandsRecord)(GraphicsCommands);
        void    (*onUpdate)();
        Request (*onResourcesUpdate)();
        s32       width, height;
        b8        enableValidationLayers;
        b8        enalbeVsync;
    };

    struct BlendAttachment
    {
        b8 blendEnable;
        BlendOp colorBlendOp;
        BlendOp alphaBlendOp;
        BlendFactor srcColorFactor;
        BlendFactor dstColorFactor;
        BlendFactor srcAlphaFactor;
        BlendFactor dstAlphaFactor;
        ColorComponent colorComponent;
    };

    struct DrawIndirectCommand
    {
        u32 vertexCount;
        u32 instanceCount;
        u32 vertex;
        u32 instance;
    };

    struct DrawIndexedIndirectCommand
    {
        u32 indexCount;
        u32 instanceCount;
        u32 index;
        s32 vertexOffset;
        u32 instance;
    };

    struct DepthStencilState
    {
        b8 depthTestEnable;
        b8 depthWriteEnable;
        CompareOp compareOp;
    };

    struct Image;
    struct ImageBarrier
    {
        Image& image;
        ImageLayout oldLayout;
        ImageLayout newLayout;
    };

    struct ColorAttachment
    {
        Image const& image;
        Image const* pResolve;
        LoadOp loadOp;
        StoreOp storeOp;
        ClearColor clearColor;
    };

    struct DepthAttachment
    {
        Image const* pImage;
        Image const* pResolve;
        LoadOp loadOp;
        StoreOp storeOp;
    };

    struct ImageCreateInfo
    {
        u32 width;
        u32 height;
        ImageLayout layout;
        ImageUsage usage;
        Sampler sampler;
        u32 shaderArrayElement;
        b8 useMsaa;
    };

    struct MapEntry
    {
        u32 id;
        u32 offset;
        size_t size;
    };

    struct Constants
    {
        std::initializer_list<MapEntry> entries;
        void const* pData;
    };

    struct ShaderHandle
    {
        VkSpecializationMapEntry entries[6];
        VkSpecializationInfo spec;
        VkPipelineShaderStageCreateInfo shaderStage;
    };

    struct PipelineHandle
    {
        VkPipeline handle;
    };

    struct BufferHandle
    {
        VkBuffer handle;
        VmaAllocation allocation;
        size_t capacity;
        u64 address;
        u8* pMapped;
    };

    struct ImageHandle
    {
        VkImage handle;
        VkImageView view;
        VmaAllocation allocation;
        Sampler sampler;
        u32 width;
        u32 height;
        u32 index;
    };

    struct GraphicsConfig
    {
        std::initializer_list<ShaderHandle> shaders;
        std::initializer_list<BlendAttachment> attachments;
        DepthStencilState depthStencilState;
        Topology    topology    = Topology::eTriangleList;
        PolygonMode polygonMode = PolygonMode::eFill;
        CullMode    cullMode    = CullMode::eNone;
        b8          useMsaa     = false;
    };

    void execute(AppInfo&& info)                  noexcept;
    void setCursorPosition(s32 x, s32 y)          noexcept;
    void setTitle(char const* title)              noexcept;
    void messageBoxError(char const* error)       noexcept;
    void showCursor()                             noexcept;
    void hideCursor()                             noexcept;
    void pollEvents()                             noexcept;
    void waitEvents()                             noexcept;

    [[nodiscard]] f32 getTimef()                  noexcept;
    [[nodiscard]] f64 getTime()                   noexcept;
    [[nodiscard]] f32 getDeltaTimef()             noexcept;
    [[nodiscard]] f64 getDeltaTime()              noexcept;

    [[nodiscard]] b8  isKeyPressed(Key)           noexcept;
    [[nodiscard]] b8  isKeyReleased(Key)          noexcept;
    [[nodiscard]] b8  isKeyDown(Key)              noexcept;
    [[nodiscard]] b8  isKeyUp(Key)                noexcept;

    [[nodiscard]] b8  isButtonPressed(Button)     noexcept;
    [[nodiscard]] b8  isButtonReleased(Button)    noexcept;
    [[nodiscard]] b8  isButtonDown(Button)        noexcept;
    [[nodiscard]] b8  isButtonUp(Button)          noexcept;

    [[nodiscard]] s32 getGlobalCursorPositionX()  noexcept;
    [[nodiscard]] s32 getGlobalCursorPositionY()  noexcept;
    [[nodiscard]] s32 getCursorPositionX()        noexcept;
    [[nodiscard]] s32 getCursorPositionY()        noexcept;
    [[nodiscard]] s32 getCursorDeltaX()           noexcept;
    [[nodiscard]] s32 getCursorDeltaY()           noexcept;

    [[nodiscard]] s32 getWidth()                  noexcept;
    [[nodiscard]] s32 getHeight()                 noexcept;
    [[nodiscard]] u32 getFramebufferWidth()       noexcept;
    [[nodiscard]] u32 getFramebufferHeight()      noexcept;
    [[nodiscard]] f32 getAspectRatio()            noexcept;
    [[nodiscard]] f32 getFramebufferAspectRatio() noexcept;

    struct Shader : public ShaderHandle
    {
        void create(const char* path, Constants const& constants = {}) noexcept;
        void create(u32 const* pSpirv, size_t size, ShaderStage stage, Constants const& constants = {}) noexcept;
        void destroy() noexcept;
    };

    struct Pipeline : public PipelineHandle
    {
        void create(GraphicsConfig&& config) noexcept;
        void destroy() noexcept;
    };

    struct Buffer : public BufferHandle
    {
        void create(size_t bufferCapacity) noexcept;
        void create(void const* pData, size_t dataSize) noexcept;
        void destroy() noexcept;
        void write(void const* pData, size_t size = {}, size_t offset = {}) noexcept;
    };

    struct Image : public ImageHandle
    {
        void create(ImageCreateInfo const& imageCreateInfo) noexcept;
        void destroy() noexcept;
    };

    struct GraphicsCommands
    {
        void barrier(ImageBarrier barrier) noexcept;
        void beginRendering(std::initializer_list<ColorAttachment>, DepthAttachment = {}) noexcept;
        void endRendering() noexcept;
        void beginPresent() noexcept;
        void endPresent() noexcept;
        void bindPipeline(Pipeline const& pipeline) noexcept;
        void bindIndexBuffer16(Buffer const& buffer) noexcept;
        void bindIndexBuffer32(Buffer const& buffer) noexcept;
        void draw(u32 vertexCount, u32 instanceCount = 1u, u32 vertex = 0u, u32 instance = 0u) noexcept;
        void drawIndexed(u32 indexCount, u32 instanceCount = 1u, u32 index = 0u, s32 vertexOffset = 0, u32 instance = 0u) noexcept;
        void drawIndirect(Buffer const& buffer, u32 drawCount, u32 stride = sizeof(DrawIndirectCommand)) noexcept;
        void drawIndexedIndirect(Buffer const& buffer, u32 drawCount, u32 stride = sizeof(DrawIndexedIndirectCommand)) noexcept;
        void drawIndirectCount(Buffer const& buffer, Buffer const& countBuffer, u32 maxDraws, u32 stride = sizeof(DrawIndirectCommand)) noexcept;
        void drawIndexedIndirectCount(Buffer const& buffer, Buffer const& countBuffer, u32 maxDraws, u32 stride = sizeof(DrawIndexedIndirectCommand)) noexcept;
        void pushConstant(void const* pData, u32 size) noexcept;
    };
}
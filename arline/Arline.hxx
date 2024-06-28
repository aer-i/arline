#pragma once
#include <string_view>
#include <cstdint>

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
        using callback_t = void(*)(std::string_view);
        using u8_t  = std::uint8_t;
        using u16_t = std::uint16_t;
        using u32_t = std::uint32_t;
        using u64_t = std::uint64_t;
        using i8_t  = std::int8_t;
        using i16_t = std::int16_t;
        using i32_t = std::int32_t;
        using i64_t = std::int64_t;
        using f32_t = float;
        using f64_t = double;
        using b8_t  = bool;
    }

    using types::callback_t;
    using types::u8_t;
    using types::u16_t;
    using types::u32_t;
    using types::u64_t;
    using types::i8_t;
    using types::i16_t;
    using types::i32_t;
    using types::i64_t;
    using types::f32_t;
    using types::f64_t;
    using types::b8_t;

    enum class Key : u8_t
    {
        eA,
        eB,
        eC,
        eD,
        eE,
        eF,
        eG,
        eH,
        eI,
        eJ,
        eK,
        eL,
        eM,
        eN,
        eO,
        eP,
        eQ,
        eR,
        eS,
        eT,
        eU,
        eV,
        eW,
        eX,
        eY,
        eZ,
        e0,
        e1,
        e2,
        e3,
        e4,
        e5,
        e6,
        e7,
        e8,
        e9,
        eSpace,
        eEscape,
        eEnter,
        eBackspace,
        eTab,
        eMinus,
        ePlus,
        eLeftShift,
        eLeftCtrl,
        eLeftAlt,
        eRightShift,
        eRightCtrl,
        eRightAlt,
        eLeft,
        eUp,
        eRight,
        eDown,
        eMaxEnum
    };

    enum class Button : u8_t
    {
        eLeft            = 0x00,
        eRight           = 0x01,
        eMiddle          = 0x02,
        eSide1           = 0x03,
        eSide2           = 0x04
    };

    enum class ShaderStage : u8_t
    {
        eVertex          = 0x01,
        eFragment        = 0x10,
        eCompute         = 0x20
    };

    enum class Topology : u8_t
    {
        ePoint           = 0x00,
        eLineList        = 0x01,
        eLineStrip       = 0x02,
        eTriangleList    = 0x03,
        eTriangleStrip   = 0x04,
        eTriangleFan     = 0x05
    };

    enum class PolygonMode : u8_t
    {
        eFill            = 0x00,
        eLine            = 0x01,
        ePoint           = 0x02
    };

    enum class CullMode : u8_t
    {
        eNone            = 0x00,
        eFront           = 0x01,
        eBack            = 0x02
    };

    enum class LoadOp : u8_t
    {
        eLoad            = 0x00,
        eClear           = 0x01,
        eDontCare        = 0x02
    };

    enum class StoreOp : u8_t
    {
        eStore           = 0x00,
        eDontCare        = 0x01
    };

    enum class ImageUsage : u8_t
    {
        eColorAttachment = 0x00,
        eDepthAttachment = 0x01
    };

    enum class ImageLayout : u8_t
    {
        eColorAttachment = 0x02,
        eDepthAttachment = 0x03,
        eShaderReadOnly  = 0x05
    };

    enum CompareOp : u8_t
    {
        eNever           = 0x00,
        eLess            = 0x01,
        eEqual           = 0x02,
        eLequal          = 0x03,
        eGreater         = 0x04,
        eNotEqual        = 0x05,
        eGequal          = 0x06,
        eAlways          = 0x07
    };

    enum Sampler : u8_t
    {
        eNone            = 0x00,
        eLinearToEdge    = 0x01,
        eLinearRepeat    = 0x02,
        eNearestToEdge   = 0x03,
        eNearestRepeat   = 0x04
    };

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmissing-braces"
#endif

    union ClearColor
    {
        f32_t float32[4];
        u32_t uint32[4];
        i32_t int32[4];

        static consteval auto Blank()   -> ClearColor { return { 0.0f, 0.0f, 0.0f, 0.0f}; }
        static consteval auto Black()   -> ClearColor { return { 0.0f, 0.0f, 0.0f, 1.0f}; }
        static consteval auto White()   -> ClearColor { return { 1.0f, 1.0f, 1.0f, 1.0f}; }
        static consteval auto Red()     -> ClearColor { return { 1.0f, 0.0f, 0.0f, 1.0f}; }
        static consteval auto Green()   -> ClearColor { return { 0.0f, 1.0f, 0.0f, 1.0f}; }
        static consteval auto Blue()    -> ClearColor { return { 0.0f, 0.0f, 1.0f, 1.0f}; }
        static consteval auto Cyan()    -> ClearColor { return { 0.0f, 1.0f, 1.0f, 1.0f}; }
        static consteval auto Magenta() -> ClearColor { return { 1.0f, 0.0f, 1.0f, 1.0f}; }
        static consteval auto Yellow()  -> ClearColor { return { 1.0f, 1.0f, 0.0f, 1.0f}; }
    };

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

    struct GraphicsCommands;
    struct AppInfo
    {
        void (*onInit)();
        void (*onDestroy)();
        void (*onCommandsRecord)(GraphicsCommands);
        b8_t (*onResourcesUpdate)();
        void (*onUpdate)();
        void (*onResize)();
        std::string_view title;
        i32_t width, height;
        callback_t infoCallback;
        callback_t errorCallback;
        b8_t enableValidationLayers;
    };

    struct DrawIndirectCommand
    {
        u32_t vertexCount;
        u32_t instanceCount;
        u32_t vertex;
        u32_t instance;
    };

    struct DrawIndexedIndirectCommand
    {
        u32_t indexCount;
        u32_t instanceCount;
        u32_t index;
        i32_t vertexOffset;
        u32_t instance;
    };

    struct DepthStencilState
    {
        b8_t depthTestEnable;
        b8_t depthWriteEnable;
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
        Image& image;
        LoadOp loadOp;
        StoreOp storeOp;
        ClearColor clearColor;
    };

    struct DepthAttachment
    {
        Image* pImage;
        LoadOp loadOp;
        StoreOp storeOp;
    };

    struct ImageCreateInfo
    {
        u32_t width;
        u32_t height;
        ImageUsage usage;
        Sampler sampler;
        u32_t shaderArrayElement;
    };

    struct MapEntry
    {
        u32_t id;
        u32_t offset;
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
        u64_t address;
        u8_t* pMapped;
    };

    struct ImageHandle
    {
        VkImage handle;
        VkImageView view;
        VmaAllocation allocation;
        Sampler sampler;
        u32_t width;
        u32_t height;
        u32_t index;
    };

    struct GraphicsConfig
    {
        std::initializer_list<ShaderHandle> shaders;
        DepthStencilState depthStencilState;
        Topology    topology    = Topology::eTriangleList;
        PolygonMode polygonMode = PolygonMode::eFill;
        CullMode    cullMode    = CullMode::eNone;
    };

    [[nodiscard]] auto execute(AppInfo&& info)     noexcept -> i32_t;

    [[nodiscard]] auto getTimef()                  noexcept -> f32_t;
    [[nodiscard]] auto getTime()                   noexcept -> f64_t;
    [[nodiscard]] auto getDeltaTimef()             noexcept -> f32_t;
    [[nodiscard]] auto getDeltaTime()              noexcept -> f64_t;

    [[nodiscard]] auto isKeyPressed(Key)           noexcept -> b8_t;
    [[nodiscard]] auto isKeyReleased(Key)          noexcept -> b8_t;
    [[nodiscard]] auto isKeyDown(Key)              noexcept -> b8_t;
    [[nodiscard]] auto isKeyUp(Key)                noexcept -> b8_t;

    [[nodiscard]] auto isButtonPressed(Button)     noexcept -> b8_t;
    [[nodiscard]] auto isButtonReleased(Button)    noexcept -> b8_t;
    [[nodiscard]] auto isButtonDown(Button)        noexcept -> b8_t;
    [[nodiscard]] auto isButtonUp(Button)          noexcept -> b8_t;

    [[nodiscard]] auto getGlobalCursorPositionX()  noexcept -> i32_t;
    [[nodiscard]] auto getGlobalCursorPositionY()  noexcept -> i32_t;
    [[nodiscard]] auto getCursorPositionX()        noexcept -> i32_t;
    [[nodiscard]] auto getCursorPositionY()        noexcept -> i32_t;
    [[nodiscard]] auto getCursorDeltaX()           noexcept -> i32_t;
    [[nodiscard]] auto getCursorDeltaY()           noexcept -> i32_t;

    [[nodiscard]] auto getWidth()                  noexcept -> i32_t;
    [[nodiscard]] auto getHeight()                 noexcept -> i32_t;
    [[nodiscard]] auto getFramebufferWidth()       noexcept -> u32_t;
    [[nodiscard]] auto getFramebufferHeight()      noexcept -> u32_t;
    [[nodiscard]] auto getAspectRatio()            noexcept -> f32_t;
    [[nodiscard]] auto getFramebufferAspectRatio() noexcept -> f32_t;

    auto setCursorPosition(i32_t x, i32_t y)       noexcept -> void;
    auto setTitle(std::string_view title)          noexcept -> void;
    auto messageBoxError(std::string_view error)   noexcept -> void;
    auto showCursor()                              noexcept -> void;
    auto hideCursor()                              noexcept -> void;
    auto pollEvents()                              noexcept -> void;
    auto waitEvents()                              noexcept -> void;

    struct Shader : public ShaderHandle
    {
        auto create(std::string_view path, Constants const& constants = {}) noexcept -> void;
        auto create(u32_t const* pSpirv, size_t size, ShaderStage stage, Constants const& constants = {}) noexcept -> void;
        auto destroy() noexcept -> void;
    };

    struct Pipeline : public PipelineHandle
    {
        auto create(GraphicsConfig&& config) noexcept -> void;
        auto destroy() noexcept -> void;
    };

    struct Buffer : public BufferHandle
    {
        auto create(size_t bufferCapacity) noexcept -> void;
        auto create(void const* pData, size_t dataSize) noexcept -> void;
        auto destroy() noexcept -> void;
        auto write(void const* pData, size_t size = {}, size_t offset = {}) noexcept -> void;
    };

    struct Image : public ImageHandle
    {
        auto create(ImageCreateInfo const& imageCreateInfo) noexcept -> void;
        auto destroy() noexcept -> void;
    };

    struct GraphicsCommands
    {
    private:
        using ColorAttachments = std::initializer_list<ColorAttachment>;
        static constexpr u32_t s_id = sizeof(DrawIndirectCommand);
        static constexpr u32_t s_idi = sizeof(DrawIndexedIndirectCommand);

    public:
        auto barrier(ImageBarrier barrier) noexcept -> void;
        auto beginRendering(ColorAttachments, DepthAttachment = {}) noexcept -> void;
        auto endRendering() noexcept -> void;
        auto beginPresent() noexcept -> void;
        auto endPresent() noexcept -> void;
        auto bindPipeline(Pipeline const& pipeline) noexcept -> void;
        auto draw(u32_t vertexCount, u32_t instanceCount = 1u, u32_t vertex = 0u, u32_t instance = 0u) noexcept -> void;
        auto drawIndexed(u32_t indexCount, u32_t instanceCount = 1u, u32_t index = 0u, i32_t vertexOffset = 0, u32_t instance = 0u) noexcept -> void;
        auto drawIndirect(Buffer const& buffer, u32_t drawCount, u32_t stride = s_id) noexcept -> void;
        auto drawIndexedIndirect(Buffer const& buffer, u32_t drawCount, u32_t stride = s_idi) noexcept -> void;
        auto drawIndirectCount(Buffer const& buffer, Buffer const& countBuffer, u32_t maxDraws, u32_t stride = s_id) noexcept -> void;
        auto drawIndexedIndirectCount(Buffer const& buffer, Buffer const& countBuffer, u32_t maxDraws, u32_t stride = s_idi) noexcept -> void;
        auto pushConstant(void const* pData, u32_t size) noexcept -> void;
    };
}
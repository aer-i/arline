#pragma once
#include <string_view>
#include <cstdint>

namespace arline
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

    enum class ShaderStage : u8_t
    {
        eVertex          = 0x01ui8,
        eFragment        = 0x10ui8,
        eCompute         = 0x20ui8
    };

    enum class Topology : u8_t
    {
        ePoint           = 0x00ui8,
        eLineList        = 0x01ui8,
        eLineStrip       = 0x02ui8,
        eTriangleList    = 0x03ui8,
        eTriangleStrip   = 0x04ui8,
        eTriangleFan     = 0x05ui8
    };

    enum class PolygonMode : u8_t
    {
        eFill            = 0x00ui8,
        eLine            = 0x01ui8,
        ePoint           = 0x02ui8
    };

    enum class CullMode : u8_t
    {
        eNone            = 0x00ui8,
        eFront           = 0x01ui8,
        eBack            = 0x02ui8
    };

    enum class LoadOp : u8_t
    {
        eLoad            = 0x00ui8,
        eClear           = 0x01ui8,
        eDontCare        = 0x02ui8
    };

    enum class StoreOp : u8_t
    {
        eStore           = 0x00ui8,
        eDontCare        = 0x01ui8
    };

    enum class ImageUsage : u8_t
    {
        eColorAttachment = 0x00ui8,
        eDepthAttachment = 0x01ui8
    };

    enum class ImageLayout : u8_t
    {
        eColorAttachment = 0x02ui8,
        eDepthAttachment = 0x03ui8,
        eDepthReadOnly   = 0x04ui8,
        eShaderReadOnly  = 0x05ui8
    };

    enum CompareOp : u8_t
    {
        eNever           = 0x00ui8,
        eLess            = 0x01ui8,
        eEqual           = 0x02ui8,
        eLequal          = 0x03ui8,
        eGreater         = 0x04ui8,
        eNotEqual        = 0x05ui8,
        eGequal          = 0x06ui8,
        eAlways          = 0x07ui8
    };

    enum Sampler : u8_t
    {
        eNone            = 0x00ui8,
        eLinearToEdge    = 0x01ui8,
        eLinearRepeat    = 0x02ui8,
        eNearestToEdge   = 0x03ui8,
        eNearestRepeat   = 0x04ui8
    };

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

    struct EngineConfig
    {
        [[nodiscard]] inline static auto Default() -> EngineConfig
        {
            return {
                .title = "",
                .width = 1280,
                .height = 720,
                .infoCallback = [](std::string_view){},
                .errorCallback = [](std::string_view){},
            };
        }

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

    class Image;
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
    };

    [[nodiscard]] auto getTimef()         noexcept -> f32_t;
    [[nodiscard]] auto getTime()          noexcept -> f64_t;
    [[nodiscard]] auto getDeltaTimef()    noexcept -> f32_t;
    [[nodiscard]] auto getDeltaTime()     noexcept -> f64_t;

    [[nodiscard]] auto isKeyPressed(Key)  noexcept -> b8_t;
    [[nodiscard]] auto isKeyReleased(Key) noexcept -> b8_t;
    [[nodiscard]] auto isKeyDown(Key)     noexcept -> b8_t;
    [[nodiscard]] auto isKeyUp(Key)       noexcept -> b8_t;
    
    namespace Window
    {
        auto PollEvents() noexcept -> void;
        auto WaitEvents() noexcept -> void;
        auto SetTitle(std::string_view title) noexcept -> void;

        [[nodiscard]] auto GetWidth()             noexcept -> i32_t;
        [[nodiscard]] auto GetHeight()            noexcept -> i32_t;
        [[nodiscard]] auto GetFramebufferWidth()  noexcept -> u32_t;
        [[nodiscard]] auto GetFramebufferHeight() noexcept -> u32_t;
        [[nodiscard]] auto GetAspect()            noexcept -> f32_t;
        [[nodiscard]] auto GetFramebufferAspect() noexcept -> f32_t;
    };

    class Shader
    {
    public:
        inline Shader() noexcept : m{} {}
        Shader(std::string_view pathToSpirv) noexcept;
        Shader(u32_t const* pSpirvBinary, size_t dataSize, ShaderStage stage) noexcept;
        ~Shader() noexcept;
        Shader(Shader const&) = delete;
        Shader(Shader&& other) noexcept;
        auto operator=(Shader const&) -> Shader& = delete;
        auto operator=(Shader&& other) noexcept -> Shader&;

    private:
        struct Members
        {
            void* pData[6];
        } m;
    };

    struct GraphicsConfig
    {
        std::initializer_list<Shader> shaders;
        DepthStencilState depthStencilState;
        Topology    topology    = Topology::eTriangleList;
        PolygonMode polygonMode = PolygonMode::eFill;
        CullMode    cullMode    = CullMode::eNone;
    };

    class Pipeline
    {
    public:
        inline Pipeline() noexcept : m{} {}
        Pipeline(GraphicsConfig&& config) noexcept;
        ~Pipeline() noexcept;
        Pipeline(Pipeline const&) = delete;
        Pipeline(Pipeline&& other) noexcept;
        auto operator=(Pipeline const&) -> Pipeline& = delete;
        auto operator=(Pipeline&& other) noexcept -> Pipeline&;

    private:
        struct Members
        {
            void* pHandle;
        } m;
    };

    class Buffer
    {
    public:
        inline Buffer() noexcept : m{} {}
        ~Buffer() noexcept;
        Buffer(Buffer const&) = delete;
        Buffer(Buffer&& other) noexcept;
        auto operator=(Buffer const&) -> Buffer& = delete;
        auto operator=(Buffer&& other) noexcept -> Buffer&;

    public:
        [[nodiscard]] auto getCapacity() const noexcept { return m.capacity; }
        [[nodiscard]] auto getAddress() const noexcept -> u64_t;

    protected:
        struct Members
        {
            void* pHandle;
            void* pAllocation;
            size_t capacity;
        } m;
    };

    class DynamicBuffer : public Buffer
    {
    public:
        using Buffer::Buffer;
        DynamicBuffer(size_t capacity) noexcept;

    public:
        auto write(void const* pData, size_t size = 0ull, size_t offset = 0ull) noexcept -> void;

    private:
        u8_t* m_pMapped;
    };

    class StaticBuffer : public Buffer
    {
    public:
        using Buffer::Buffer;
        StaticBuffer(void const* pData, size_t dataSize);
    };

    class Image
    {
    public:
        inline Image() : m{} {}
        ~Image() noexcept;
        Image(Image const&) = delete;
        Image(Image&& other) noexcept;
        auto operator=(Image const&) -> Image& = delete;
        auto operator=(Image&& other) noexcept -> Image&;

    public:
        Image(ImageCreateInfo const& imageCreateInfo) noexcept;

    private:
        auto makeResident(Sampler, u32_t) noexcept -> void;

    public:
        [[nodiscard]] auto getWidth()  const noexcept { return m.width;  }
        [[nodiscard]] auto getHeight() const noexcept { return m.height; }
        [[nodiscard]] auto getIndex()  const noexcept { return m.index;  }

    private:
        struct Members
        {
            void* pHandle;
            void* pView;
            void* pAllocation;
            Sampler sampler;
            u32_t width;
            u32_t height;
            u32_t index;
        } m;
    };

    class GraphicsCommands
    {
    public:
        GraphicsCommands() noexcept;
        ~GraphicsCommands() noexcept;
        GraphicsCommands(GraphicsCommands const&) = delete;
        GraphicsCommands(GraphicsCommands&&) = delete;
        auto operator=(GraphicsCommands const&) -> GraphicsCommands& = delete;
        auto operator=(GraphicsCommands&&) -> GraphicsCommands& = delete;

    public:
        operator b8_t() noexcept;

    public:
        using ColorAttachments = std::initializer_list<ColorAttachment>;
        auto barrier(ImageBarrier barrier) const noexcept -> void;
        auto beginRendering(ColorAttachments, DepthAttachment = {}) const noexcept -> void;
        auto endRendering() const noexcept -> void;
        auto beginPresent() const noexcept -> void;
        auto endPresent() const noexcept -> void;
        auto bindPipeline(Pipeline const& pipeline) const noexcept -> void;
        auto draw(u32_t vertexCount, u32_t instanceCount = 1u, u32_t vertex = 0u, u32_t instance = 0u) const noexcept -> void;
        auto drawIndexed(u32_t indexCount, u32_t instanceCount = 1u, u32_t index = 0u, i32_t vertexOffset = 0, u32_t instance = 0u) const noexcept -> void;
        auto drawIndirect(Buffer const& buffer, u32_t drawCount, u32_t stride = s_id) const noexcept -> void;
        auto drawIndexedIndirect(Buffer const& buffer, u32_t drawCount, u32_t stride = s_idi) const noexcept -> void;
        auto drawIndirectCount(Buffer const& buffer, Buffer const& countBuffer, u32_t maxDraws, u32_t stride = s_id) const noexcept -> void;
        auto drawIndexedIndirectCount(Buffer const& buffer, Buffer const& countBuffer, u32_t maxDraws, u32_t stride = s_idi) const noexcept -> void;
        auto pushConstant(void const* pData, u32_t size) const noexcept -> void;

    private:
        static constexpr u32_t s_id = sizeof(DrawIndirectCommand);
        static constexpr u32_t s_idi = sizeof(DrawIndexedIndirectCommand);
        struct Members
        {
            u32_t id;
        } m;
    };

    class Engine
    {
    public:
        Engine(EngineConfig const& config = EngineConfig::Default()) noexcept;
        ~Engine() noexcept;
        Engine(Engine const&) = delete;
        Engine(Engine&&) = delete;
        auto operator=(Engine const&) -> Engine& = delete;
        auto operator=(Engine&&) -> Engine& = delete;

    protected:
        virtual auto onResize() -> void { }
        virtual auto onUpdate() -> void { Window::PollEvents(); }
        virtual auto onResourcesUpdate() -> b8_t { return false; }
        virtual auto onCommandsRecord(GraphicsCommands const&) -> void = 0;

    public:
        auto execute() noexcept -> int;
    };
}

namespace ar = arline;
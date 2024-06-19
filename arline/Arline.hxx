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
        eVertex        = 0x01ui8,
        eFragment      = 0x10ui8,
        eCompute       = 0x20ui8
    };

    enum class Topology : u8_t
    {
        ePoint         = 0x00ui8,
        eLineList      = 0x01ui8,
        eLineStrip     = 0x02ui8,
        eTriangleList  = 0x03ui8,
        eTriangleStrip = 0x04ui8,
        eTriangleFan   = 0x05ui8
    };

    enum class PolygonMode : u8_t
    {
        eFill          = 0x00ui8,
        eLine          = 0x01ui8,
        ePoint         = 0x02ui8
    };

    enum class CullMode : u8_t
    {
        eNone          = 0x00ui8,
        eFront         = 0x01ui8,
        eBack          = 0x02ui8
    };

    enum class LoadOp : u8_t
    {
        eLoad          = 0x00ui8,
        eClear         = 0x01ui8,
        eDontCare      = 0x02ui8
    };

    enum class StoreOp : u8_t
    {
        eStore         = 0x00ui8,
        eDontCare      = 0x01ui8
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

    struct RenderPass
    {
        LoadOp loadOp;
        StoreOp storeOp;
        ClearColor clearColor;
    };

    struct EngineConfig
    {
        [[nodiscard]] static auto Default() -> EngineConfig
        {
            return {
                .title = "Arline Application",
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

    [[nodiscard]] auto getTime()          noexcept -> f64_t;
    [[nodiscard]] auto getDeltaTime()     noexcept -> f32_t;

    [[nodiscard]] auto isKeyPressed(Key)  noexcept -> b8_t;
    [[nodiscard]] auto isKeyReleased(Key) noexcept -> b8_t;
    [[nodiscard]] auto isKeyDown(Key)     noexcept -> b8_t;
    [[nodiscard]] auto isKeyUp(Key)       noexcept -> b8_t;
    
    namespace Window
    {
        auto PollEvents() noexcept -> void;
        auto WaitEvents() noexcept -> void;
        auto SetTitle(std::string_view title) noexcept -> void;

        [[nodiscard]] auto GetWidth()  noexcept -> i32_t;
        [[nodiscard]] auto GetHeight() noexcept -> i32_t;
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
        Buffer(size_t capacity) noexcept;
        ~Buffer() noexcept;
        Buffer(Buffer const&) = delete;
        Buffer(Buffer&& other) noexcept;
        auto operator=(Buffer const&) -> Buffer& = delete;
        auto operator=(Buffer&& other) noexcept -> Buffer&;

    public:
        auto getAddress() noexcept -> u64_t;
        auto write(void const* pData) noexcept -> void;
        auto write(void const* pData, size_t size, size_t offset = 0ull) noexcept -> void;

    private:
        struct Members
        {
            void* pHandle;
            void* pAllocation;
            u8_t* pMapped;
            size_t capacity;
        } m;
    };

    class StaticBuffer
    {
    public:
        inline StaticBuffer() noexcept : m{} {}
        StaticBuffer(size_t capacity) noexcept;
        ~StaticBuffer() noexcept;
        StaticBuffer(StaticBuffer const&) = delete;
        StaticBuffer(StaticBuffer&& other) noexcept;
        auto operator=(StaticBuffer const&) -> StaticBuffer& = delete;
        auto operator=(StaticBuffer&& other) noexcept -> StaticBuffer&;

    private:
        struct Members
        {
            void* pHandle;
            void* pAllocation;
            size_t capacity;
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
        auto beginPresent(RenderPass const& renderPass) const noexcept -> void;
        auto endPresent() const noexcept -> void;
        auto bindPipeline(Pipeline const& pipeline) const noexcept -> void;
        auto draw(u32_t vertexCount) const noexcept -> void;
        auto pushConstant(void const* pData, u32_t size) const noexcept -> void;

    private:
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
        virtual auto onUpdate() -> void = 0;
        virtual auto onResourcesUpdate() -> void = 0;
        virtual auto onCommandsRecord(GraphicsCommands const&) -> void = 0;

    public:
        auto execute() noexcept -> int;
    };
}

namespace ar = arline;
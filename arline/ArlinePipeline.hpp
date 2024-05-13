#pragma once
#include "ArlineVkContext.hpp"
#include "ArlineShader.hpp"

namespace arline
{
    using GraphicsFlags = u8;
    namespace GraphicsFlagBits
    {
        enum : u8
        {
            eDepthWrite     = 0x00000000,
            eDepthTest      = 0x00000001,
            eColorBlending  = 0x00000002
        };
    }

    enum class Topology : u8
    {
        ePoint         = 0x00000000,
        eLineList      = 0x00000001,
        eLineStrip     = 0x00000002,
        eTriangleList  = 0x00000003,
        eTriangleStrip = 0x00000004,
        eTriangleFan   = 0x00000005
    };

    enum class PolygonMode : u8
    {
        eFill  = 0x00000000,
        eLine  = 0x00000001,
        ePoint = 0x00000002
    };

    enum class CullMode : u8
    {
        eNone  = 0x00000000,
        eFront = 0x00000001,
        eBack  = 0x00000002
    };

    struct GraphicsConfig
    {
        using Shaders = std::initializer_list<Shader>;

        Shaders       shaders     = Shaders{ };
        GraphicsFlags flags       = GraphicsFlags{ };
        Topology      topology    = Topology::eTriangleList;
        PolygonMode   polygonMode = PolygonMode::eFill;
        CullMode      cullMode    = CullMode::eNone;
    };

    class Pipeline
    {
    public:
        Pipeline() noexcept;
        explicit Pipeline(Shader const& computeShader) noexcept;
        Pipeline(GraphicsConfig&& config) noexcept;
        ~Pipeline() noexcept;
        Pipeline(Pipeline const&) = delete;
        Pipeline(Pipeline&& other) noexcept;
        auto operator=(Pipeline const&) -> Pipeline& = delete;
        auto operator=(Pipeline&& other) noexcept -> Pipeline&;

    public:
        inline operator VkPipeline() const noexcept { return m.handle; }

    protected:
        struct Members
        {
            VkPipeline handle;
        } m;
    };
}
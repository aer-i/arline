#pragma once
#include "ArlineVkContext.hpp"
#include "ArlineShader.hpp"
#include <functional>

namespace arline
{
    class Pipeline
    {
    public:
        Pipeline() noexcept;
        ~Pipeline() noexcept;
        Pipeline(Pipeline const&) = delete;
        Pipeline(Pipeline&& other) noexcept;
        auto operator=(Pipeline const&) -> Pipeline& = delete;
        auto operator=(Pipeline&& other) noexcept -> Pipeline&;

    public:
        inline operator VkPipeline() const noexcept { return p.handle; }

    protected:
        struct Protected
        {
            VkPipeline handle;
        } p;
    };

    class GraphicsPipeline : public Pipeline
    {
    public:
        struct Config;
        using Pipeline::Pipeline;
        GraphicsPipeline(Config&& config) noexcept;

    public:
        using Flags = u8;
        struct FlagBits
        {
            FlagBits() = delete;

            enum : u8
            {
                eDepthWrite     = 0x00000000,
                eDepthTest      = 0x00000001,
                eColorBlending  = 0x00000002,
            };
        };

        enum class Topology : u32
        {
            ePoint         = 0x00000000,
            eLineList      = 0x00000001,
            eLineStrip     = 0x00000002,
            eTriangleList  = 0x00000003,
            eTriangleStrip = 0x00000004,
            eTriangleFan   = 0x00000005
        };

        enum class PolygonMode : u32
        {
            eFill  = 0x00000000,
            eLine  = 0x00000001,
            ePoint = 0x00000002
        };

        enum class CullMode : u32
        {
            eNone  = 0x00000000,
            eFront = 0x00000001,
            eBack  = 0x00000002
        };

        struct Config
        {
            using Shaders = std::initializer_list<std::reference_wrapper<Shader const>>;

            Shaders     shaders     = Shaders{ };
            Topology    topology    = Topology::eTriangleList;
            PolygonMode polygonMode = PolygonMode::eFill;
            CullMode    cullMode    = CullMode::eNone;
            Flags       flags       = Flags{ };
        };
    };
}
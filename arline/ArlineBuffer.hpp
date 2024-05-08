#pragma once
#include "ArlineTypes.hpp"
#include "ArlineVkContext.hpp"

namespace arline
{
    struct BufferHandle
    {
        VkBuffer handle;
        VmaAllocation allocation;
        u64 address;
    };

    class StaticBuffer
    {
    public:
        inline StaticBuffer() noexcept : m{} {}
        StaticBuffer(v0 const* pData, u32 size) noexcept;
        ~StaticBuffer() noexcept;
        StaticBuffer(StaticBuffer const&) = delete;
        StaticBuffer(StaticBuffer&& other) noexcept;
        auto operator=(StaticBuffer const&) -> StaticBuffer& = delete;
        auto operator=(StaticBuffer&& other) noexcept -> StaticBuffer&;

    public:
        inline auto getHandle() const noexcept ->  BufferHandle const& { return m.buffer; }
        inline auto getAddress() const noexcept -> u64 const* { return &m.buffer.address; }
        inline auto getSize() const noexcept -> u32 { return m.size; }

    private:
        struct Members
        {
            BufferHandle buffer;
            u32 size;
        } m;
    };

    class DynamicBuffer
    {
    public:
        inline DynamicBuffer() noexcept : m{} {}
        explicit DynamicBuffer(u32 size) noexcept;
        ~DynamicBuffer() noexcept;
        DynamicBuffer(StaticBuffer const&) = delete;
        DynamicBuffer(DynamicBuffer&& other) noexcept;
        auto operator=(DynamicBuffer const&) -> DynamicBuffer& = delete;
        auto operator=(DynamicBuffer&& other) noexcept -> DynamicBuffer&;

    public:
        auto write(v0 const* pData) noexcept -> v0;
        auto write(v0 const* pData, u32 size) noexcept -> v0;
        auto write(v0 const* pData, u32 size, u32 offset) noexcept -> v0;

    public:
        inline auto getHandle() const noexcept ->  BufferHandle const& { return m.buffers[VkContext::Get()->bufferIndex]; }
        inline auto getAddress() const noexcept -> u64 const* { return &m.buffers[VkContext::Get()->bufferIndex].address; }
        inline auto getSize() const noexcept -> u32 { return m.size; }

    private:
        struct Members
        {
            BufferHandle buffers[2];
            u8* mappedData[2];
            u32 size;
        } m;
    };
}
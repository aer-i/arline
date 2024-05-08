#pragma once
#include "ArlineTypes.hpp"
#include "ArlineVkContext.hpp"
#include <concepts>

namespace arline
{
    struct BufferHandle
    {
        VkBuffer handle;
        VmaAllocation allocation;
        u64 address;
    };

    template<class T>
    concept Buffer = requires(T t)
    {
        { t.getHandle() } -> std::same_as<BufferHandle const&>;
        { t.getAddress() } -> std::same_as<u64 const*>;
        { t.getSize() } -> std::same_as<u32>;
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

    private:
        struct Members
        {
            BufferHandle buffers[2];
            u8* mappedMemories[2];
            u32 size;
        } m;
    };
}
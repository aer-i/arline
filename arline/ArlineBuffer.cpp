#include "ArlineBuffer.hpp"
#include <cstring>

arline::StaticBuffer::StaticBuffer(v0 const* pData, u32 size) noexcept
{
    auto [buffer, allocation] = VkContext::CreateStaticBuffer(size);

    m = {
        .buffer = BufferHandle{
            .handle = buffer,
            .allocation = allocation,
        },
        .size = size
    };

    auto const bufferAddressInfo{ VkBufferDeviceAddressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = m.buffer.handle
    }};

    m.buffer.address = vkGetBufferDeviceAddress(VkContext::Get()->device, &bufferAddressInfo);

    VkMemoryPropertyFlags memoryType;
    vmaGetAllocationMemoryProperties(VkContext::Get()->allocator, m.buffer.allocation, &memoryType);

    if (memoryType & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        vmaCopyMemoryToAllocation(VkContext::Get()->allocator, pData, m.buffer.allocation, 0, static_cast<u64>(size));
    }
    else
    {
        auto [stagingBuffer, stagingAllocation] = VkContext::CreateStagingBuffer(pData, size);

        VkContext::TransferSubmit([&](VkCommandBuffer cmd) -> v0
        {
            auto const copy{ VkBufferCopy{
                .size = size
            }};

            vkCmdCopyBuffer(cmd, stagingBuffer, m.buffer.handle, 1, &copy);
        });

        vmaDestroyBuffer(VkContext::Get()->allocator, stagingBuffer, stagingAllocation);
    }
}

arline::StaticBuffer::~StaticBuffer() noexcept
{
    if (m.buffer.handle)
    {
        vmaDestroyBuffer(VkContext::Get()->allocator, m.buffer.handle, m.buffer.allocation);
    }
}

arline::StaticBuffer::StaticBuffer(StaticBuffer&& other) noexcept
    : m{ other.m }
{
    other.m = {};
}

auto arline::StaticBuffer::operator=(StaticBuffer&& other) noexcept -> StaticBuffer&
{
    this->~StaticBuffer();
    this->m = other.m;
    other.m = {};

    return *this;
}

arline::DynamicBuffer::DynamicBuffer(u32 size) noexcept
    : m{
        .size = size
    }
{
    for (auto i{ u32{} }; i < 2; ++i)
    {
        auto [buffer, allocation, mappedData] = VkContext::CreateDynamicBuffer(size);

        auto const bufferAddressInfo{ VkBufferDeviceAddressInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer
        }};

        m.buffers[i] = BufferHandle{
            .handle = buffer,
            .allocation = allocation,
            .address = vkGetBufferDeviceAddress(VkContext::Get()->device, &bufferAddressInfo)
        };

        m.mappedData[i] = static_cast<u8*>(mappedData);
    }
}

arline::DynamicBuffer::~DynamicBuffer() noexcept
{
    for (auto& buffer : m.buffers)
    {
        if (buffer.handle)
        {
            vmaDestroyBuffer(VkContext::Get()->allocator, buffer.handle, buffer.allocation);
        }
    }
}

arline::DynamicBuffer::DynamicBuffer(DynamicBuffer&& other) noexcept
    : m{ other.m }
{
    other.m = {};
}

auto arline::DynamicBuffer::operator=(DynamicBuffer&& other) noexcept -> DynamicBuffer&
{
    this->~DynamicBuffer();
    this->m = other.m;
    other.m = {};

    return *this;
}

auto arline::DynamicBuffer::write(v0 const* pData) noexcept -> v0
{
    std::memcpy(m.mappedData[VkContext::Get()->bufferIndex], pData, static_cast<u64>(m.size));
    vmaFlushAllocation(
        VkContext::Get()->allocator,
        m.buffers[VkContext::Get()->bufferIndex].allocation,
        0,
        static_cast<u64>(m.size)
    );
}

auto arline::DynamicBuffer::write(v0 const* pData, u32 size) noexcept -> v0
{
    std::memcpy(m.mappedData[VkContext::Get()->bufferIndex], pData, static_cast<u64>(size));
    vmaFlushAllocation(
        VkContext::Get()->allocator,
        m.buffers[VkContext::Get()->bufferIndex].allocation,
        0,
        static_cast<u64>(size)
    );
}

auto arline::DynamicBuffer::write(v0 const* pData, u32 size, u32 offset) noexcept -> v0
{
    std::memcpy(m.mappedData[VkContext::Get()->bufferIndex] + offset, pData, static_cast<u64>(size));
    vmaFlushAllocation(
        VkContext::Get()->allocator,
        m.buffers[VkContext::Get()->bufferIndex].allocation,
        static_cast<u64>(offset),
        static_cast<u64>(size)
    );
}

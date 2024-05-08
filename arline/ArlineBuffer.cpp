#include "ArlineBuffer.hpp"

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
    vmaDestroyBuffer(VkContext::Get()->allocator, m.buffer.handle, m.buffer.allocation);
}

arline::StaticBuffer::StaticBuffer(StaticBuffer&& other) noexcept
    : m{ other.m }
{
    other.m = {};
}

auto arline::StaticBuffer::operator=(StaticBuffer&& other) noexcept -> StaticBuffer&
{
    vmaDestroyBuffer(VkContext::Get()->allocator, m.buffer.handle, m.buffer.allocation);

    this->m = other.m;
    other.m = {};

    return *this;
}

arline::DynamicBuffer::DynamicBuffer(u32 size) noexcept
    : m{
        .size = size
    }
{

}

arline::DynamicBuffer::~DynamicBuffer() noexcept
{

}

arline::DynamicBuffer::DynamicBuffer(DynamicBuffer&& other) noexcept
{

}

auto arline::DynamicBuffer::operator=(DynamicBuffer&& other) noexcept -> DynamicBuffer&
{
    return *this;
}

#define STB_IMAGE_IMPLEMENTATION
#include "ArlineImage.hpp"
#include <stb_image.h>

arline::Image::~Image() noexcept
{
    if (p.handle)
    {
        vkDestroyImageView(VkContext::Get()->device, p.view, nullptr);
        vmaDestroyImage(VkContext::Get()->allocator, p.handle, p.allocation);
    }
}

arline::Image::Image(Image&& other) noexcept
    : p{ other.p }
{
    other.p = {};
}

auto arline::Image::operator=(Image&& other) noexcept -> Image&
{
    this->~Image();
    this->p = other.p;
    other.p = {};
    
    return *this;
}

auto arline::Image::write(v0 const* pData, u32 size, VkExtent2D extent) noexcept -> v0
{
    auto [stagingBuffer, stagingAllocation] = VkContext::CreateStagingBuffer(pData, size);

    auto const copy{ VkBufferImageCopy{
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1
        },
        .imageExtent = {
            .width = extent.width,
            .height = extent.height,
            .depth = 1
        }
    }};

    auto barrier{ VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcAccessMask = VK_QUEUE_FAMILY_IGNORED,
        .dstAccessMask = VK_QUEUE_FAMILY_IGNORED,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .image = p.handle,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = p.mipLevels,
            .layerCount = 1
        }
    }};

    auto const dependency{ VkDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    }};

    VkContext::TransferSubmit([&](VkCommandBuffer cmd)
    {
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_NONE;
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier2(cmd, &dependency);

        vkCmdCopyBufferToImage(cmd, stagingBuffer, p.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

        this->generateMipMaps(cmd, extent);
    });

    vmaDestroyBuffer(VkContext::Get()->allocator, stagingBuffer, stagingAllocation);
}

auto arline::Image::generateMipMaps(VkCommandBuffer commandBuffer, VkExtent2D extent) noexcept -> v0
{
    auto barrier{ VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcAccessMask = VK_QUEUE_FAMILY_IGNORED,
        .dstAccessMask = VK_QUEUE_FAMILY_IGNORED,
        .image = p.handle,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    }};

    auto const dependency{ VkDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    }};

    auto mipWidth{ static_cast<i32>(extent.width) };
    auto mipHeight{ static_cast<i32>(extent.height) };

    for (auto i{ u32{1} }; i < p.mipLevels; ++i)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier2(commandBuffer, &dependency);

        auto const blit{ VkImageBlit{
            .srcSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i - 1,
                .layerCount = 1
            },
            .srcOffsets = {
                { 0, 0, 0 },
                { mipWidth, mipHeight, 1 }
            },
            .dstSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i,
                .layerCount = 1
            },
            .dstOffsets = {
                { 0, 0, 0 },
                { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 }
            }
        }};

        vkCmdBlitImage(
            commandBuffer,
            p.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            p.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR
        );

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

        vkCmdPipelineBarrier2(commandBuffer, &dependency);

        if (mipWidth > 1)
        {
            mipWidth /= 2;
        }

        if (mipHeight > 1)
        {
            mipHeight /= 2;
        }
    }

    barrier.subresourceRange.baseMipLevel = p.mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
    vkCmdPipelineBarrier2(commandBuffer, &dependency);
}

auto arline::Image::makeResident() noexcept -> v0
{
    p.index = ++VkContext::Get()->currentTextureIndex;

    auto const imageInfo{ VkDescriptorImageInfo{
        .imageView = p.view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    }};

    auto const write{ VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = VkContext::Get()->descriptorSet,
        .dstBinding = 1,
        .dstArrayElement = p.index,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .pImageInfo = &imageInfo
    }};

    vkUpdateDescriptorSets(VkContext::Get()->device, 1, &write, 0, nullptr);
}

arline::Texture2D::Texture2D(std::filesystem::path const& path) noexcept
{
    auto width{ i32{} }, height{ i32{} }, channels{ i32{} };
    auto data{ stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha) };

    if (!data)
    {
        VkContext::Get()->errorCallback("Failed to load texture: " + std::string{path});
        return;
    }

    auto [image, allocation, view, mipLevels] = VkContext::CreateTexture2D(
        static_cast<u32>(width), static_cast<u32>(height)
    );

    p = Protected{
        .handle = image,
        .view = view,
        .allocation = allocation,
        .mipLevels = mipLevels
    };

    this->write(data, width * height * 4, { static_cast<u32>(width), static_cast<u32>(height)});
    this->makeResident();

    stbi_image_free(data);
}

arline::Texture2D::Texture2D(v0 const* pData, u32 width, u32 height) noexcept
{
    auto [image, allocation, view, mipLevels] = VkContext::CreateTexture2D(
        static_cast<u32>(width), static_cast<u32>(height)
    );

    p = Protected{
        .handle = image,
        .view = view,
        .allocation = allocation,
        .mipLevels = mipLevels
    };

    this->write(pData, width * height * 4, { static_cast<u32>(width), static_cast<u32>(height)});
    this->makeResident();
}

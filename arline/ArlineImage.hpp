#pragma once
#include "ArlineVkContext.hpp"
#include <filesystem>

namespace arline
{
    class Image
    {
    public:
        inline Image() noexcept : p{} {}
        ~Image() noexcept;
        Image(Image const&) = delete;
        Image(Image&& other) noexcept;
        auto operator=(Image const&) -> Image& = delete;
        auto operator=(Image&& other) noexcept -> Image&;

    public:
        inline auto getIndex() const noexcept -> u32 { return p.index; }
        inline auto getMipLevelCount() const noexcept -> u32 { return p.mipLevels; }

    protected:
        auto write(v0 const* pData, u32 size, VkExtent2D extent) noexcept -> v0;
        auto generateMipMaps(VkCommandBuffer commandBuffer, VkExtent2D extent) noexcept -> v0;
        auto makeResident() noexcept -> v0;

    protected:
        struct Protected
        {
            VkImage handle;
            VkImageView view;
            VmaAllocation allocation;
            u32 index;
            u32 mipLevels;
        } p;
    };

    class Texture2D : public Image
    {
    public:
        using Image::Image;
        Texture2D(std::filesystem::path const& path) noexcept;
        Texture2D(v0 const* pData, u32 width, u32 height) noexcept;
    };
}
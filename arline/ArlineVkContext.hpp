#pragma once
#include "ArlineTypes.hpp"
#include <volk.h>
#include <vk_mem_alloc.h>
#include <string_view>
#include <vector>
#include <array>
#include <tuple>
#include <functional>

namespace arline
{
    struct ContextInfo
    {
        v0 (*infoCallback)(std::string_view){ [](std::string_view){} };
        v0 (*errorCallback)(std::string_view){ [](std::string_view){} };
        b8 enableValidationLayers{};
    };

    class VkContext
    {
    public:
        VkContext() = delete;
        ~VkContext() = delete;
        VkContext(VkContext const&) = delete;
        VkContext(VkContext&&) = delete;
        auto operator=(VkContext const&) -> VkContext& = delete;
        auto operator=(VkContext&&) -> VkContext& = delete;

    public:
        static auto Create(ContextInfo const& info) noexcept -> v0;
        static auto Teardown() noexcept -> v0;
        static auto WaitForDevice() noexcept -> v0;
        static auto RecreateSwapchain() noexcept -> v0;
        static auto AcquireNextImage() noexcept -> v0;
        static auto PresentFrame() noexcept -> v0;
        static auto TransferSubmit(std::function<v0(VkCommandBuffer)>&& function) noexcept -> v0;
        static auto CreateShaderModule(v0 const* pData, u64 size) noexcept -> VkShaderModule;
        static auto CreatePipeline(VkComputePipelineCreateInfo const* pInfo) noexcept -> VkPipeline;
        static auto CreatePipeline(VkGraphicsPipelineCreateInfo const* pInfo) noexcept -> VkPipeline;
        static auto CreateStagingBuffer(v0 const* pData, u32 size) noexcept -> std::tuple<VkBuffer, VmaAllocation>;
        static auto CreateStaticBuffer(u32 size) noexcept -> std::tuple<VkBuffer, VmaAllocation>;
        static auto CreateDynamicBuffer(u32 size) noexcept -> std::tuple<VkBuffer, VmaAllocation, v0*>;
        static auto CreateTexture2D(u32 width, u32 height) noexcept -> std::tuple<VkImage, VmaAllocation, VkImageView, u32>;

    public:
        static inline auto Get() { return &m; }

    private:
        static auto CreateInstance() noexcept -> v0;
        static auto CreateSurface() noexcept -> v0;
        static auto CreatePhysicalDevice() noexcept -> v0;
        static auto CreateDevice() noexcept -> v0;
        static auto CreateAllocator() noexcept -> v0;
        static auto CreateSwapchain() noexcept -> v0;
        static auto CreateCommandBuffers() noexcept -> v0;
        static auto CreateSyncObjects() noexcept -> v0;
        static auto CreateSamplers() noexcept -> v0;
        static auto CreateDescriptor() noexcept -> v0;
        static auto CreatePipelineLayout() noexcept -> v0;
        static auto CreateTransferResources() noexcept -> v0;

    private:
        friend class ComputeCommands;
        friend class Commands;

        static inline struct Members
        {
            static constexpr u32 framesInFlight = 2;

            struct SwapchainImage
            {
                VkImage handle;
                VkImageView view;
                VkImageLayout layout;
            };

            struct Frame
            {
                VkCommandPool computeCommandPool;
                VkCommandBuffer computeCommandBuffer;
                VkCommandPool commandPool;
                VkCommandBuffer commandBuffer;
                VkSemaphore acquireSemaphore;
                VkSemaphore renderSemaphore;
                VkFence renderFence;
            };

            b8 validation;
            v0 (*infoCallback)(std::string_view);
            v0 (*errorCallback)(std::string_view);
            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkSurfaceKHR surface;
            VkPhysicalDevice physicalDevice;
            VkDescriptorPool descriptorPool;
            VkDescriptorSetLayout setLayout;
            u32 graphicsFamily;
            u32 presentFamily;
            VkFormat colorFormat;
            VmaAllocator allocator;
            VkDevice device;
            VkQueue graphicsQueue;
            VkQueue presentQueue;
            VkExtent2D swapchainExtent;
            VkSwapchainKHR swapchain;
            VkPipelineLayout pipelineLayout;
            VkPipelineLayout computePipelineLayout;
            VkDescriptorSet descriptorSet;
            VkSampler sampler;
            VkFence transferFence;
            VkCommandPool transferCommandPool;
            VkCommandBuffer transferCommandBuffer;
            u32 currentTextureIndex;
            u32 bufferIndex;
            u32 frameIndex, imageIndex;
            std::array<Frame, framesInFlight> frames;
            std::array<SwapchainImage, 5> swapchainImages;
        } m;
    };
}
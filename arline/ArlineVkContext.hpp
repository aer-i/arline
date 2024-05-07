#pragma once
#include "ArlineTypes.hpp"
#include <volk.h>
#include <vk_mem_alloc.h>
#include <string_view>
#include <vector>
#include <array>

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

    private:
        friend class Context;
        static auto Create(ContextInfo const& info) noexcept -> v0;
        static auto Teardown() noexcept -> v0;
        static auto WaitForDevice() noexcept -> v0;
        static auto RecreateSwapchain() noexcept -> v0;

    public:
        static auto AcquireNextImage() noexcept -> v0;
        static auto PresentFrame() noexcept -> v0;
        static auto CreateShaderModule(std::vector<c8> const& code) noexcept -> VkShaderModule;
        static auto CreatePipeline(VkGraphicsPipelineCreateInfo const* pInfo) noexcept -> VkPipeline;

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
                VkCommandPool commandPool;
                VkCommandBuffer commandBuffer;
                VkSemaphore acquireSemaphore;
                VkSemaphore renderSemaphore;
                VkFence renderFence;
            };

            v0 (*infoCallback)(std::string_view);
            v0 (*errorCallback)(std::string_view);
            std::vector<SwapchainImage> swapchainImages;
            std::array<Frame, framesInFlight> frames;
            Frame* currentFrame;
            u32 frameIndex, imageIndex;
            VmaAllocator allocator;
            VkInstance instance;
            VkDebugUtilsMessengerEXT debugMessenger;
            VkSurfaceKHR surface;
            VkPhysicalDevice physicalDevice;
            VkDevice device;
            VkQueue graphicsQueue;
            VkQueue presentQueue;
            VkExtent2D swapchainExtent;
            VkSwapchainKHR swapchain;
            VkPipelineLayout pipelineLayout;
            VkDescriptorPool descriptorPool;
            VkDescriptorSetLayout setLayout;
            VkDescriptorSet descriptorSet;
            VkSampler sampler;
            VkFence transferFence;
            VkCommandPool transferCommandPool;
            VkCommandBuffer transferCommandBuffer;
            VkFormat colorFormat;
            u32 graphicsFamily;
            u32 presentFamily;
            b8 validation;
        } m;
    };
}
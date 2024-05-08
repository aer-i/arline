#define GLFW_INCLUDE_NONE
#define VMA_IMPLEMENTATION
#include "ArlineVkContext.hpp"
#include "ArlineWindow.hpp"
#include <GLFW/glfw3.h>
#include <vector>
#include <format>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
    [[maybe_unused]] arline::v0* pUserData
)
{
    arline::VkContext::Get()->infoCallback(pCallbackData->pMessage);

    return VK_FALSE;
}

template<arline::u32 N>
struct ConstexprString
{
    constexpr ConstexprString(arline::c8 const (&str)[N])
    {
        std::copy_n(str, N, data);
    }

    arline::c8 data[N];
};

template<ConstexprString message>
static inline auto vkErrorCheck(VkResult result) noexcept -> arline::v0;

namespace surface
{
    static auto getFormat() noexcept -> VkFormat;
    static auto presentModeSupport(VkPresentModeKHR mode) noexcept -> arline::b8;
    static auto getExtent() noexcept -> VkExtent2D;
    static auto getClampedImageCount(arline::u32 imageCount) noexcept -> arline::u32;
}

auto arline::VkContext::Create(ContextInfo const& info) noexcept -> v0
{
    m = {
        .infoCallback = info.infoCallback,
        .errorCallback = info.errorCallback,
        .validation = info.enableValidationLayers
    };

    CreateInstance();
    CreateSurface();
    CreatePhysicalDevice();
    CreateDevice();
    CreateAllocator();
    CreateSwapchain();
    CreateCommandBuffers();
    CreateSyncObjects();
    CreateSamplers();
    CreateDescriptor();
    CreatePipelineLayout();
    CreateTransferResources();
}

auto arline::VkContext::Teardown() noexcept -> v0
{
    for (auto& frame : m.frames)
    {
        vkDestroyFence(m.device, frame.renderFence, nullptr);
        vkDestroySemaphore(m.device, frame.renderSemaphore, nullptr);
        vkDestroySemaphore(m.device, frame.acquireSemaphore, nullptr);
        vkDestroyCommandPool(m.device, frame.commandPool, nullptr);
    }

    for (auto& image : m.swapchainImages)
    {
        vkDestroyImageView(m.device, image.view, nullptr);
    }

    vkDestroyCommandPool(m.device, m.transferCommandPool, nullptr);
    vkDestroyFence(m.device, m.transferFence, nullptr);
    vkDestroyPipelineLayout(m.device, m.pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m.device, m.setLayout, nullptr);
    vkDestroyDescriptorPool(m.device, m.descriptorPool, nullptr);
    vkDestroySampler(m.device, m.sampler, nullptr);
    vkDestroySwapchainKHR(m.device, m.swapchain, nullptr);
    vmaDestroyAllocator(m.allocator);
    vkDestroyDevice(m.device, nullptr);
    vkDestroySurfaceKHR(m.instance, m.surface, nullptr);
    if (m.validation)
        vkDestroyDebugUtilsMessengerEXT(m.instance, m.debugMessenger, nullptr);
    vkDestroyInstance(m.instance, nullptr);

    m.infoCallback("Destroyed Vulkan context");
}

auto arline::VkContext::WaitForDevice() noexcept -> v0
{
    vkDeviceWaitIdle(m.device);
}

auto arline::VkContext::RecreateSwapchain() noexcept -> v0
{
    vkDeviceWaitIdle(m.device);
    CreateSwapchain();
}

auto arline::VkContext::AcquireNextImage() noexcept -> v0
{
    m.currentFrame = &m.frames[m.frameIndex];

    vkWaitForFences(m.device, 1, &m.currentFrame->renderFence, 0u, ~0ull);
    vkResetFences(m.device, 1, &m.currentFrame->renderFence);

    auto result{ vkAcquireNextImageKHR(m.device, m.swapchain, ~0ull, m.currentFrame->acquireSemaphore, nullptr, &m.imageIndex) };
    switch (result)
    {
    case VK_SUCCESS:
    case VK_SUBOPTIMAL_KHR:
        break;
    [[unlikely]] case VK_ERROR_OUT_OF_DATE_KHR:
        RecreateSwapchain();
        break;
    [[unlikely]] default:
        vkErrorCheck<"Failed to acquire swapchain image">(result);
        break;
    }
}

auto arline::VkContext::PresentFrame() noexcept -> v0
{
    auto const waitStage{ VkPipelineStageFlags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT} };
    auto const submitInfo{ VkSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m.currentFrame->acquireSemaphore,
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &m.currentFrame->commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m.currentFrame->renderSemaphore
    }};

    vkErrorCheck<"Failed to submit commands">(
        vkQueueSubmit(m.graphicsQueue, 1, &submitInfo, m.currentFrame->renderFence)
    );

    auto const presentInfo{ VkPresentInfoKHR{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m.currentFrame->renderSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &m.swapchain,
        .pImageIndices = &m.imageIndex
    }};

    auto result{ vkQueuePresentKHR(m.presentQueue, &presentInfo) };
    switch (result)
    {
    [[likely]]   case VK_SUCCESS: break;
    [[unlikely]] case VK_SUBOPTIMAL_KHR:
    [[unlikely]] case VK_ERROR_OUT_OF_DATE_KHR:
        RecreateSwapchain();
        break;
    [[unlikely]] default:
        vkErrorCheck<"Failed to present frame">(result);
        break;
    }

    ++m.frameIndex %= Members::framesInFlight;
}

auto arline::VkContext::TransferSubmit(std::function<v0(VkCommandBuffer)>&& function) noexcept -> v0
{
    auto const beginInfo{ VkCommandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    }};

    auto const submitInfo{ VkSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &m.transferCommandBuffer
    }};

    vkResetFences(m.device, 1, &m.transferFence);

    vkErrorCheck<"Failed to reset VkCommandPool">(
        vkResetCommandPool(m.device, m.transferCommandPool, 0)
    );

    vkErrorCheck<"Failed to begin VkCommandBuffer">(
        vkBeginCommandBuffer(m.transferCommandBuffer, &beginInfo)
    );
    function(m.transferCommandBuffer);
    vkErrorCheck<"Failed to end VkCommandBuffer">(
        vkEndCommandBuffer(m.transferCommandBuffer)
    );

    vkErrorCheck<"Failed to submit commands">(
        vkQueueSubmit(m.graphicsQueue, 1, &submitInfo, m.transferFence)
    );

    vkWaitForFences(m.device, 1, &m.transferFence, 0u, ~0ull);
}

auto arline::VkContext::CreateShaderModule(std::vector<c8> const& code) noexcept -> VkShaderModule
{
    auto const shaderModuleCreateInfo{ VkShaderModuleCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<u32 const*>(code.data())
    }};

    VkShaderModule shaderModule;

    vkErrorCheck<"Failed to create VkShaderModule">(
        vkCreateShaderModule(m.device, &shaderModuleCreateInfo, nullptr, &shaderModule)
    );

    return shaderModule;
}

auto arline::VkContext::CreatePipeline(VkGraphicsPipelineCreateInfo const* pInfo) noexcept -> VkPipeline
{
    VkPipeline pipeline;

    vkErrorCheck<"Failed to create VkPipeline">(
        vkCreateGraphicsPipelines(m.device, nullptr, 1, pInfo, nullptr, &pipeline)
    );

    return pipeline;
}

auto arline::VkContext::CreateStagingBuffer(v0 const* pData, u32 size) noexcept -> std::tuple<VkBuffer, VmaAllocation>
{
    auto const bufferCreateInfo{ VkBufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    }};

    auto const allocationCreateInfo{ VmaAllocationCreateInfo{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    }};

    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;

    vkErrorCheck<"Failed to allocate VkBuffer">(
        vmaCreateBuffer(m.allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo)
    );

    memcpy(allocationInfo.pMappedData, pData, size);
    vmaFlushAllocation(m.allocator, allocation, 0, static_cast<u64>(size));

    return { buffer, allocation };
}

auto arline::VkContext::CreateStaticBuffer(u32 size) noexcept -> std::tuple<VkBuffer, VmaAllocation>
{
    auto static constexpr bufferUsage{ VkBufferUsageFlags{
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
    }};

    auto const bufferCreateInfo{ VkBufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = bufferUsage
    }};

    auto const allocationCreateInfo{ VmaAllocationCreateInfo{
        .usage = VMA_MEMORY_USAGE_AUTO,
        .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    }};

    VkBuffer buffer;
    VmaAllocation allocation;

    vkErrorCheck<"Failed to allocate VkBuffer">(
        vmaCreateBuffer(m.allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr)
    );

    return { buffer, allocation };
}

auto arline::VkContext::CreateInstance() noexcept -> v0
{
    if (volkInitialize())
    {
        m.errorCallback("Failed to initialize Volk");
    }

    auto apiVersion{ u32{} };
    auto extensionCount{ u32{} };
    auto extensionNames{ glfwGetRequiredInstanceExtensions(&extensionCount) };

    auto extensions{ std::vector<c8 const*>{extensionNames, extensionNames + extensionCount} };
    auto layers    { std::vector<c8 const*>{} };

    if (extensions.empty())
    {
        m.errorCallback("Surface extensions not supported");
    }

    vkErrorCheck<"Failed to enumerate instance version">(
        vkEnumerateInstanceVersion(&apiVersion)
    );

    if (apiVersion < VK_API_VERSION_1_3)
    {
        m.errorCallback("Instance version must be Vulkan 1.3 or higher");
    }

    if (m.validation)
    {
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    auto const syncValidationFeature{ VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT };

    auto const validationFeatures{ VkValidationFeaturesEXT{
        .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .enabledValidationFeatureCount = 1,
        .pEnabledValidationFeatures = &syncValidationFeature
    }};

    auto const applicationInfo{ VkApplicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Light Frame Application",
        .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .pEngineName = "Light Frame",
        .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
        .apiVersion = apiVersion
    }};

    auto const instanceCreateInfo{ VkInstanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = m.validation ? &validationFeatures : nullptr,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<u32>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<u32>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    }};

    vkErrorCheck<"Failed to create VkInstance">(
        vkCreateInstance(&instanceCreateInfo, nullptr, &m.instance)
    );

    volkLoadInstanceOnly(m.instance);

    if (m.validation)
    {
        auto const debugMessengerCreateInfo{ VkDebugUtilsMessengerCreateInfoEXT{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = &debugCallback
        }};

        vkErrorCheck<"Failed to create VkDebugUtilsMessengerEXT">(
            vkCreateDebugUtilsMessengerEXT(m.instance, &debugMessengerCreateInfo, nullptr, &m.debugMessenger)
        );
    }

    m.infoCallback(std::format(
        "Instance API version [ {}.{}.{} ]",
        VK_VERSION_MAJOR(apiVersion),
        VK_VERSION_MINOR(apiVersion),
        VK_VERSION_PATCH(apiVersion)
    ));
    m.infoCallback(std::format("Enabled instance extensions [ {} ]:", extensions.size()));

    for (auto const& extension : extensions)
    {
        m.infoCallback(std::format("   {}", extension));
    }

    m.infoCallback(std::format("Enabled instance layers [ {} ]:", layers.size()));

    for (auto const& layer : layers)
    {
        m.infoCallback(std::format("   {}", layer));
    }
}

auto arline::VkContext::CreateSurface() noexcept -> v0
{
    vkErrorCheck<"Failed to create VkSurfaceKHR">(
        glfwCreateWindowSurface(m.instance, Window::GetHandle(), nullptr, &m.surface)
    );
}

auto arline::VkContext::CreatePhysicalDevice() noexcept -> v0
{
    auto vulkan11Features{ VkPhysicalDeviceVulkan11Features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES} };
    auto vulkan12Features{ VkPhysicalDeviceVulkan12Features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES} };
    auto vulkan13Features{ VkPhysicalDeviceVulkan13Features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES} };
    auto properties      { VkPhysicalDeviceProperties2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2} };
    auto features        { VkPhysicalDeviceFeatures2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2} };

    vulkan12Features.pNext = &vulkan11Features;
    vulkan13Features.pNext = &vulkan12Features;
    features.pNext = &vulkan13Features;

    auto deviceCount{ u32{} };
    vkEnumeratePhysicalDevices(m.instance, &deviceCount, nullptr);
    auto physicalDevices{ std::vector<VkPhysicalDevice>{deviceCount} };
    vkEnumeratePhysicalDevices(m.instance, &deviceCount, physicalDevices.data());

    if (physicalDevices.empty())
    {
        m.errorCallback("No graphics cards available");
    }

    m.infoCallback("Enumerating GPUs:");
    auto missingFeature{ std::string{} };

    for (auto const currentPhysicalDevice : physicalDevices)
    {
        vkGetPhysicalDeviceProperties2(currentPhysicalDevice, &properties);
        vkGetPhysicalDeviceFeatures2(currentPhysicalDevice, &features);

        m.infoCallback(std::format("    Current GPU: {}", properties.properties.deviceName));

        if (properties.properties.apiVersion < VK_API_VERSION_1_3) {
            missingFeature = std::format("GPU {} doesn't support Vulkan 1.3", properties.properties.deviceName);
            continue;
        } if (!vulkan13Features.synchronization2) {
            missingFeature = std::format("GPU {} doesn't support synchronization2 feature", properties.properties.deviceName);
            continue;
        } if (!vulkan13Features.dynamicRendering) {
            missingFeature = std::format("GPU {} doesn't support dynamicRendering feature", properties.properties.deviceName);
            continue;
        } if (!vulkan12Features.drawIndirectCount) {
            missingFeature = std::format("GPU {} doesn't support drawIndirectCount feature", properties.properties.deviceName);
            continue;
        } if (!vulkan12Features.descriptorBindingPartiallyBound) {
            missingFeature = std::format("GPU {} doesn't support descriptorBindingPartiallyBound feature", properties.properties.deviceName);
            continue;
        } if (!vulkan12Features.bufferDeviceAddress) {
            missingFeature = std::format("GPU {} doesn't support bufferDeviceAddress feature", properties.properties.deviceName);
            continue;
        } if (!features.features.multiDrawIndirect) {
            missingFeature = std::format("GPU {} doesn't support multiDrawIndirect feature", properties.properties.deviceName);
            continue;
        } if (!features.features.fillModeNonSolid) {
            missingFeature = std::format("GPU {} doesn't support fillModeNonSolid feature", properties.properties.deviceName);
            continue;
        } if (!features.features.samplerAnisotropy) {
            missingFeature = std::format("GPU {} doesn't support samplerAnisotropy feature", properties.properties.deviceName);
            continue;
        }

        m.physicalDevice = currentPhysicalDevice;
    }

    if (!m.physicalDevice)
    {
        m.errorCallback(missingFeature + ", make sure you have installed the latest drivers");
    }

    m.infoCallback(std::format("Selected graphics card [ {} ]", properties.properties.deviceName));
    m.infoCallback(std::format(
        "Graphics card Vulkan driver version [ {}.{}.{} ]",
        VK_VERSION_MAJOR(properties.properties.driverVersion),
        VK_VERSION_MINOR(properties.properties.driverVersion),
        VK_VERSION_PATCH(properties.properties.driverVersion)
    ));
    m.infoCallback(std::format(
        "Graphics card Vulkan version [ {}.{}.{} ]",
        VK_VERSION_MAJOR(properties.properties.apiVersion),
        VK_VERSION_MINOR(properties.properties.apiVersion),
        VK_VERSION_PATCH(properties.properties.apiVersion)
    ));
}

auto arline::VkContext::CreateDevice() noexcept -> v0
{
    auto propertyCount{ u32{} };
    vkGetPhysicalDeviceQueueFamilyProperties(m.physicalDevice, &propertyCount, nullptr);
    auto properties{ std::vector<VkQueueFamilyProperties>{propertyCount} };
    vkGetPhysicalDeviceQueueFamilyProperties(m.physicalDevice, &propertyCount, properties.data());

    m.graphicsFamily = ~0u;
    m.presentFamily = ~0u;

    for (auto i{ u32{} }; i < propertyCount; ++i)
    {
        auto presentSupport{ VkBool32{} };
        vkGetPhysicalDeviceSurfaceSupportKHR(m.physicalDevice, i, m.surface, &presentSupport);

        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            m.graphicsFamily = i;
        }

        if (presentSupport)
        {
            m.presentFamily = i;
        }

        if (m.graphicsFamily != ~0u && m.presentFamily != ~0u)
        {
            break;
        }
    }

    auto const queuePriority{ f32{1.f} };

    auto const queueCreateInfos = [&]
    {
        auto result{ std::vector<VkDeviceQueueCreateInfo>{} };

        if (m.graphicsFamily == m.presentFamily)
        {
            result.emplace_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = m.graphicsFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            });
        }
        else
        {
            result.emplace_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = m.graphicsFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            });

            result.emplace_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = m.presentFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            });
        }

        return result;
    }();

    auto vulkan11Features{ VkPhysicalDeviceVulkan11Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES
    }};

    auto vulkan12Features{ VkPhysicalDeviceVulkan12Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &vulkan11Features,
        .drawIndirectCount = true,
        .descriptorBindingPartiallyBound = true,
        .bufferDeviceAddress = true
    }};

    auto vulkan13Features{ VkPhysicalDeviceVulkan13Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &vulkan12Features,
        .synchronization2 = true,
        .dynamicRendering = true
    }};

    auto const enabledFeatures{ VkPhysicalDeviceFeatures2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vulkan13Features,
        .features = {
            .multiDrawIndirect = true,
            .fillModeNonSolid = true,
            .samplerAnisotropy = true
        }
    }};

    auto deviceExtensions{ std::vector{VK_KHR_SWAPCHAIN_EXTENSION_NAME} };

    auto const deviceCreateInfo{ VkDeviceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &enabledFeatures,
        .queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<u32>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data()
    }};

    vkErrorCheck<"Failed to create VkDevice">(
        vkCreateDevice(m.physicalDevice, &deviceCreateInfo, nullptr, &m.device)
    );

    volkLoadDevice(m.device);

    vkGetDeviceQueue(m.device, m.graphicsFamily, 0, &m.graphicsQueue);
    vkGetDeviceQueue(m.device, m.presentFamily, 0, &m.presentQueue);
}

auto arline::VkContext::CreateAllocator() noexcept -> v0
{
    auto const functions{ VmaVulkanFunctions{
        .vkGetInstanceProcAddr                   = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr                     = vkGetDeviceProcAddr,
        .vkGetPhysicalDeviceProperties           = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties     = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory                        = vkAllocateMemory,
        .vkFreeMemory                            = vkFreeMemory,
        .vkMapMemory                             = vkMapMemory,
        .vkUnmapMemory                           = vkUnmapMemory,
        .vkFlushMappedMemoryRanges               = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges          = vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory                      = vkBindBufferMemory,
        .vkBindImageMemory                       = vkBindImageMemory,
        .vkGetBufferMemoryRequirements           = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements            = vkGetImageMemoryRequirements,
        .vkCreateBuffer                          = vkCreateBuffer,
        .vkDestroyBuffer                         = vkDestroyBuffer,
        .vkCreateImage                           = vkCreateImage,
        .vkDestroyImage                          = vkDestroyImage,
        .vkCmdCopyBuffer                         = vkCmdCopyBuffer,
        .vkGetBufferMemoryRequirements2KHR       = vkGetBufferMemoryRequirements2,
        .vkGetImageMemoryRequirements2KHR        = vkGetImageMemoryRequirements2,
        .vkBindBufferMemory2KHR                  = vkBindBufferMemory2,
        .vkBindImageMemory2KHR                   = vkBindImageMemory2,
        .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2,
        .vkGetDeviceBufferMemoryRequirements     = vkGetDeviceBufferMemoryRequirements,
        .vkGetDeviceImageMemoryRequirements      = vkGetDeviceImageMemoryRequirements
    }};

    auto const allocatorCreateInfo{ VmaAllocatorCreateInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = m.physicalDevice,
        .device = m.device,
        .pVulkanFunctions = &functions,
        .instance = m.instance,
        .vulkanApiVersion = VK_API_VERSION_1_3
    }};

    vkErrorCheck<"Failed to create vulkan memory allocator">(
        vmaCreateAllocator(&allocatorCreateInfo, &m.allocator)
    );
}

auto arline::VkContext::CreateSwapchain() noexcept -> v0
{
    if (m.swapchain)
    {
        vkDestroySwapchainKHR(m.device, m.swapchain, nullptr);
        m.swapchain = nullptr;

        for (auto const& image : m.swapchainImages)
        {
            vkDestroyImageView(m.device, image.view, nullptr);
        }
    }

    m.swapchainImages.clear();

    auto format{ surface::getFormat() };
    auto extent{ surface::getExtent() };
    auto minImageCount{ surface::getClampedImageCount(3u) };
    auto presentMode{ VK_PRESENT_MODE_FIFO_KHR };

    m.colorFormat = format;
    m.swapchainExtent = extent;

    if (surface::presentModeSupport(VK_PRESENT_MODE_IMMEDIATE_KHR))
    {
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }

    if (surface::presentModeSupport(VK_PRESENT_MODE_MAILBOX_KHR))
    {
        presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }

    auto const swapchainCreateInfo{ VkSwapchainCreateInfoKHR{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m.surface,
        .minImageCount = minImageCount,
        .imageFormat = format,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = m.swapchainExtent,
        .imageArrayLayers = 1u,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = 1u
    }};

    vkErrorCheck<"Failed to create VkSwapchain">(
        vkCreateSwapchainKHR(m.device, &swapchainCreateInfo, nullptr, &m.swapchain)
    );

    auto imageCount{ u32{} };
    vkGetSwapchainImagesKHR(m.device, m.swapchain, &imageCount, nullptr);
    auto images{ std::vector<VkImage>(imageCount) };
    vkGetSwapchainImagesKHR(m.device, m.swapchain, &imageCount, images.data());

    m.swapchainImages.reserve(images.size());
    for (auto& image : images)
    {
        auto const imageViewCreateInfo{ VkImageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1
            }
        }};

        auto view{ VkImageView{} };

        vkErrorCheck<"Failed to create VkImageView">(
            vkCreateImageView(m.device, &imageViewCreateInfo, nullptr, &view)
        );

        m.swapchainImages.emplace_back(image, view, VK_IMAGE_LAYOUT_UNDEFINED);
    }
}

auto arline::VkContext::CreateCommandBuffers() noexcept -> v0
{
    auto const commandPoolCreateInfo{ VkCommandPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = m.graphicsFamily
    }};

    for (auto& frame : m.frames)
    {
        vkErrorCheck<"Failed to create VkCommandPool">(
            vkCreateCommandPool(m.device, &commandPoolCreateInfo, nullptr, &frame.commandPool)
        );

        auto const commandBufferAllocateInfo{ VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = frame.commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        }};

        vkErrorCheck<"Failed to allocate VkCommandBuffer">(
            vkAllocateCommandBuffers(m.device, &commandBufferAllocateInfo, &frame.commandBuffer)
        );
    }
}

auto arline::VkContext::CreateSyncObjects() noexcept -> v0
{
    auto const semaphoreCreateInfo{ VkSemaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    }};

    auto const fenceCreateInfo{ VkFenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    }};

    for (auto& frame : m.frames)
    {
        vkErrorCheck<"Failed to create VkSemaphore">(
            vkCreateSemaphore(m.device, &semaphoreCreateInfo, nullptr, &frame.acquireSemaphore)
        );

        vkErrorCheck<"Failed to create VkSemaphore">(
            vkCreateSemaphore(m.device, &semaphoreCreateInfo, nullptr, &frame.renderSemaphore)
        );

        vkErrorCheck<"Failed to create VkFence">(
            vkCreateFence(m.device, &fenceCreateInfo, nullptr, &frame.renderFence)
        );
    }
}

auto arline::VkContext::CreateSamplers() noexcept -> v0
{
    auto const samplerCreateInfo{ VkSamplerCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .minLod = 0.f,
        .maxLod = 1000.f,
    }};

    vkErrorCheck<"Failed to create VkSampler">(
        vkCreateSampler(m.device, &samplerCreateInfo, nullptr, &m.sampler)
    );
}

auto arline::VkContext::CreateDescriptor() noexcept -> v0
{
    auto const poolSizes{ std::array{
        VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1 },
        VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1024 },
    }};

    auto const descriptorPoolCreateInfo{ VkDescriptorPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 1,
        .poolSizeCount = static_cast<u32>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    }};

    vkErrorCheck<"Failed to create VkDescriptorPool">(
        vkCreateDescriptorPool(m.device, &descriptorPoolCreateInfo, nullptr, &m.descriptorPool)
    );

    auto const layoutBindings{ std::array{
        VkDescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = &m.sampler
        },
        VkDescriptorSetLayoutBinding{
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 1024,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        }
    }};

    auto const descriptorBindingFlags{ std::array{
        VkDescriptorBindingFlags{},
        VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT }
    }};

    auto const setLayoutBindingFlags{ VkDescriptorSetLayoutBindingFlagsCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = static_cast<u32>(descriptorBindingFlags.size()),
        .pBindingFlags = descriptorBindingFlags.data()
    }};

    auto const setLayoutCreateInfo{ VkDescriptorSetLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &setLayoutBindingFlags,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = static_cast<u32>(layoutBindings.size()),
        .pBindings = layoutBindings.data()
    }};

    vkErrorCheck<"Failed to create VkDescriptorSetLayout">(
        vkCreateDescriptorSetLayout(m.device, &setLayoutCreateInfo, nullptr, &m.setLayout)
    );

    auto const descriptorSetAllocateInfo{ VkDescriptorSetAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m.descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m.setLayout
    }};

    vkErrorCheck<"Failed to allocate VkDescriptorSet">(
        vkAllocateDescriptorSets(m.device, &descriptorSetAllocateInfo, &m.descriptorSet)
    );
}

auto arline::VkContext::CreatePipelineLayout() noexcept -> v0
{
    auto const pushConstantRange{ VkPushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .size = 128
    }};

    auto const pipelineLayoutCreateInfo{ VkPipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &m.setLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange,
    }};

    vkErrorCheck<"Failed to create VkPipelineLayout">(
        vkCreatePipelineLayout(m.device, &pipelineLayoutCreateInfo, nullptr, &m.pipelineLayout)
    );
}

auto arline::VkContext::CreateTransferResources() noexcept -> v0
{
    auto const fenceCreateInfo{ VkFenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    }};

    vkErrorCheck<"Failed to create VkFence">(
        vkCreateFence(m.device, &fenceCreateInfo, nullptr, &m.transferFence)
    );

    auto const commandPoolCreateInfo{ VkCommandPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = m.graphicsFamily
    }};

    vkErrorCheck<"Failed to create VkCommandPool">(
        vkCreateCommandPool(m.device, &commandPoolCreateInfo, nullptr, &m.transferCommandPool)
    );

    auto const commandBufferAllocateInfo{ VkCommandBufferAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m.transferCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    }};

    vkErrorCheck<"Failed to allocate VkCommandBuffer">(
        vkAllocateCommandBuffers(m.device, &commandBufferAllocateInfo, &m.transferCommandBuffer)
    );
}

namespace surface
{
    static auto getFormat() noexcept -> VkFormat
    {
        using namespace arline;

        auto surfaceFormatCount{ u32{} };
        vkGetPhysicalDeviceSurfaceFormatsKHR(VkContext::Get()->physicalDevice, VkContext::Get()->surface, &surfaceFormatCount, nullptr);
        auto availableFormats{ std::vector<VkSurfaceFormatKHR>{surfaceFormatCount} };
        vkGetPhysicalDeviceSurfaceFormatsKHR(VkContext::Get()->physicalDevice, VkContext::Get()->surface, &surfaceFormatCount, availableFormats.data());

        if (availableFormats.empty())
        {
            VkContext::Get()->errorCallback("No available surface formats");
        }

        if (availableFormats.size() == 1 && availableFormats.front().format == VK_FORMAT_UNDEFINED)
        {
            return VK_FORMAT_R8G8B8A8_UNORM;
        }

        for (auto const& currentFormat : availableFormats)
        {
            if (currentFormat.format == VK_FORMAT_R8G8B8A8_UNORM && currentFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return VK_FORMAT_R8G8B8A8_UNORM;
            }
        }

        for (auto const& currentFormat : availableFormats)
        {
            if (currentFormat.format == VK_FORMAT_B8G8R8A8_UNORM && currentFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return VK_FORMAT_B8G8R8A8_UNORM;
            }
        }

        return availableFormats.front().format;
    }

    static auto presentModeSupport(VkPresentModeKHR mode) noexcept -> arline::b8
    {
        using namespace arline;

        auto modeCount{ u32{} };
        vkGetPhysicalDeviceSurfacePresentModesKHR(VkContext::Get()->physicalDevice, VkContext::Get()->surface, &modeCount, nullptr);
        auto presentModes{ std::vector<VkPresentModeKHR>{modeCount} };
        vkGetPhysicalDeviceSurfacePresentModesKHR(VkContext::Get()->physicalDevice, VkContext::Get()->surface, &modeCount, presentModes.data());

        for (auto presentMode : presentModes)
        {
            if (presentMode == mode)
            {
                return true;
            }
        }

        return false;
    }

    static auto getExtent() noexcept -> VkExtent2D
    {
        using namespace arline;

        auto capabilities{ VkSurfaceCapabilitiesKHR{} };
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkContext::Get()->physicalDevice, VkContext::Get()->surface, &capabilities);

        if (capabilities.currentExtent.width != 0xFFFFFFFF)
        {
            return capabilities.currentExtent;
        }

        auto width{ u32{} }, height{ u32{} };
        glfwGetFramebufferSize(Window::GetHandle(), reinterpret_cast<i32*>(&width), reinterpret_cast<i32*>(&height));

        auto min{ capabilities.minImageExtent };
        auto max{ capabilities.maxImageExtent };

        return VkExtent2D{
            .width  = (width  > max.width)  ? max.width  : (width  < min.width)  ? min.width  : width,
            .height = (height > max.height) ? max.height : (height < min.height) ? min.height : height
        };
    }

    static auto getClampedImageCount(arline::u32 imageCount) noexcept -> arline::u32
    {
        using namespace arline;
        
        auto capabilities{ VkSurfaceCapabilitiesKHR{} };
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkContext::Get()->physicalDevice, VkContext::Get()->surface, &capabilities);

        if (imageCount < capabilities.minImageCount)
        {
            imageCount = capabilities.minImageCount;
        }

        if (capabilities.maxImageCount && imageCount > capabilities.maxImageCount)
        {
            imageCount = capabilities.maxImageCount;
        }

        return imageCount;
    }
}

static inline auto vkResultToString(VkResult input) -> arline::c8 const*
{
    switch (input)
    {
    case VK_SUCCESS:
        return "VK_SUCCESS";
    case VK_NOT_READY:
        return "VK_NOT_READY";
    case VK_TIMEOUT:
        return "VK_TIMEOUT";
    case VK_EVENT_SET:
        return "VK_EVENT_SET";
    case VK_EVENT_RESET:
        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
        return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
        return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:
        return "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:
        return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_PIPELINE_COMPILE_REQUIRED:
        return "VK_PIPELINE_COMPILE_REQUIRED";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:
        return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV:
        return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
        return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
        return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
        return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
        return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
        return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
    case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
        return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_NOT_PERMITTED_KHR:
        return "VK_ERROR_NOT_PERMITTED_KHR";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR:
        return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:
        return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:
        return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:
        return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
        return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
    case VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT:
        return "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT";
    default:
        return "Unhandled VkResult";
    }
}

template<ConstexprString message>
static inline auto vkErrorCheck(VkResult result) noexcept -> arline::v0
{
    if (result) [[unlikely]]
    {
        static constexpr auto errorMessage{ message.data };
        arline::VkContext::Get()->errorCallback(std::format(
            "{}: {}",
            errorMessage,
            vkResultToString(result)
        ));
    }
}
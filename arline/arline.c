#include "arline.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <vulkan/vulkan.h>
#include <dwmapi.h>

typedef struct SwapchainImage {
    VkCommandBuffer cmd;
    VkImage handle;
    VkImageView view;
} SwapchainImage;

struct {
    HINSTANCE hinstance;
    HWND hwnd;
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkSurfaceKHR surface;
    VkCommandPool commandPool;
    VkDevice device;
    VkQueue queue;
    VkSwapchainKHR swapchain;
    VkSemaphore semaphoreAcquire;
    VkSemaphore semaphoreRender;
    VkFence fence;
    uint32_t imageIndex;
    uint32_t imageCount;
    SwapchainImage images[6];
    VkExtent2D extent;
    int32_t width, height;
} static g;

static LRESULT arWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcA(hwnd, msg, wp, lp);
    }

    return 0;
}

static void arWindowCreate(int width, int height)
{
    g.hinstance = GetModuleHandleA(NULL);

    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = arWndProc;
    wc.hInstance = g.hinstance;
    wc.hCursor = LoadCursorA(g.hinstance, IDC_ARROW);
    wc.lpszClassName = "arline";

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    RegisterClassExA(&wc);

    g.hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME, "arline", "", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, g.hinstance, NULL
    );

    BOOL useDarkMode = true;
    DwmSetWindowAttribute(g.hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
    ShowWindow(g.hwnd, SW_SHOW);
    UpdateWindow(g.hwnd);
}

static void arWindowTeardown()
{
    DestroyWindow(g.hwnd);
    UnregisterClassA("arline", g.hinstance);
}

static void arSwapchainCreate()
{
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g.gpu, g.surface, &caps);

    g.extent.width  = caps.currentExtent.width;
    g.extent.height = caps.currentExtent.height;

    if (g.extent.width == 0xffffffff)
    {
        g.extent.width  = caps.minImageExtent.width;
        g.extent.height = caps.minImageExtent.height;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = NULL;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = g.surface;
    swapchainCreateInfo.minImageCount = caps.minImageCount > 3 ? caps.minImageCount : 3;
    swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfo.imageExtent = g.extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.clipped = true;
    swapchainCreateInfo.oldSwapchain = NULL;
    vkCreateSwapchainKHR(g.device, &swapchainCreateInfo, NULL, &g.swapchain);

    VkCommandBuffer commandBuffers[6];
    VkImage swapchainImages[6];
    vkGetSwapchainImagesKHR(g.device, g.swapchain, &g.imageCount, NULL);
    vkGetSwapchainImagesKHR(g.device, g.swapchain, &g.imageCount, swapchainImages);

    VkCommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = NULL;
    commandPoolCreateInfo.flags = 0;
    commandPoolCreateInfo.queueFamilyIndex = 0;
    vkCreateCommandPool(g.device, &commandPoolCreateInfo, NULL, &g.commandPool);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = NULL;
    commandBufferAllocateInfo.commandPool = g.commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = g.imageCount;
    vkAllocateCommandBuffers(g.device, &commandBufferAllocateInfo, commandBuffers);

    for (uint32_t i = g.imageCount; i--; )
    {
        g.images[i].cmd = commandBuffers[i];
        g.images[i].handle = swapchainImages[i];

        VkImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = NULL;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
        imageViewCreateInfo.image = swapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        vkCreateImageView(g.device, &imageViewCreateInfo, NULL, &g.images[i].view);
    }
}

static void arSwapchainTeardown()
{
    for (uint32_t i = g.imageCount; i--; )
    {
        vkDestroyImageView(g.device, g.images[i].view, NULL);
    }

    vkDestroyCommandPool(g.device, g.commandPool, NULL);
    vkDestroySwapchainKHR(g.device, g.swapchain, NULL);
}

static void arContextCreate()
{
    {
        char const* instanceExtensions[2];
        instanceExtensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
        instanceExtensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;

        VkApplicationInfo applicationInfo;
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext = NULL;
        applicationInfo.applicationVersion = 0;
        applicationInfo.engineVersion = 0;
        applicationInfo.pEngineName = "arline";
        applicationInfo.pApplicationName = "arline";
        applicationInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = NULL;
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledExtensionCount = 2;
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;
        instanceCreateInfo.enabledLayerCount = 0;
        vkCreateInstance(&instanceCreateInfo, NULL, &g.instance);
    }
    {
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.pNext = NULL;
        surfaceCreateInfo.flags = 0;
        surfaceCreateInfo.hinstance = g.hinstance;
        surfaceCreateInfo.hwnd = g.hwnd;
        vkCreateWin32SurfaceKHR(g.instance, &surfaceCreateInfo, NULL, &g.surface);
    }
    {
        VkPhysicalDevice gpus[64];
        uint32_t gpuCount;
        vkEnumeratePhysicalDevices(g.instance, &gpuCount, NULL);
        vkEnumeratePhysicalDevices(g.instance, &gpuCount, gpus);

        g.gpu = gpus[0];
    }
    {
        char const* deviceExtensions[1];
        deviceExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

        float priorities[1];
        priorities[0] = 1.0f;

        VkDeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = NULL;
        queueCreateInfo.flags = 0;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.queueFamilyIndex = 0;
        queueCreateInfo.pQueuePriorities = priorities;

        VkPhysicalDeviceVulkan13Features vulkan13Features;
        vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vulkan13Features.pNext = NULL;
        vulkan13Features.robustImageAccess = false;
        vulkan13Features.inlineUniformBlock = false;
        vulkan13Features.descriptorBindingInlineUniformBlockUpdateAfterBind = false;
        vulkan13Features.pipelineCreationCacheControl = false;
        vulkan13Features.privateData = false;
        vulkan13Features.shaderDemoteToHelperInvocation = false;
        vulkan13Features.shaderTerminateInvocation = false;
        vulkan13Features.subgroupSizeControl = false;
        vulkan13Features.computeFullSubgroups = false;
        vulkan13Features.synchronization2 = true;
        vulkan13Features.textureCompressionASTC_HDR = false;
        vulkan13Features.shaderZeroInitializeWorkgroupMemory = false;
        vulkan13Features.dynamicRendering = true;
        vulkan13Features.shaderIntegerDotProduct = false;
        vulkan13Features.maintenance4 = false;

        VkDeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = &vulkan13Features;
        deviceCreateInfo.flags = 0;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.enabledLayerCount = 0;
        deviceCreateInfo.enabledExtensionCount = 1;
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
        deviceCreateInfo.pEnabledFeatures = NULL;
        vkCreateDevice(g.gpu, &deviceCreateInfo, NULL, &g.device);
        vkGetDeviceQueue(g.device, 0, 0, &g.queue);
    }
    {
        arSwapchainCreate();
    }
    {
        for (uint32_t i = g.imageCount; i--; )
        {
            VkCommandBufferBeginInfo commandBufferBeginInfo;
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.pNext = NULL;
            commandBufferBeginInfo.flags = 0;
            commandBufferBeginInfo.pInheritanceInfo = NULL;

            ArCommandBuffer cmd;
            cmd.pHandle = &g.images[i];

            vkBeginCommandBuffer(g.images[i].cmd, &commandBufferBeginInfo);

            arCmdBeginPresent(cmd);
            arCmdEndPresent(cmd);

            vkEndCommandBuffer(g.images[i].cmd);
        }
    }
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo;
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = NULL;
        semaphoreCreateInfo.flags = 0;
        vkCreateSemaphore(g.device, &semaphoreCreateInfo, NULL, &g.semaphoreAcquire);
        vkCreateSemaphore(g.device, &semaphoreCreateInfo, NULL, &g.semaphoreRender);

        VkFenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = NULL;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(g.device, &fenceCreateInfo, NULL, &g.fence);
    }
}

static void arContextTeardown()
{
    vkDeviceWaitIdle(g.device);

    arSwapchainTeardown();
    vkDestroyFence(g.device, g.fence, NULL);
    vkDestroySemaphore(g.device, g.semaphoreRender, NULL);
    vkDestroySemaphore(g.device, g.semaphoreAcquire, NULL);
    vkDestroyDevice(g.device, NULL);
    vkDestroySurfaceKHR(g.instance, g.surface, NULL);
    vkDestroyInstance(g.instance, NULL);
}

void arExecute(ArApplicationInfo const* pApplicationInfo)
{
    arWindowCreate(pApplicationInfo->width, pApplicationInfo->height);
    arContextCreate();

    MSG msg;
    for (;;)
    {
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) goto teardown;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        vkWaitForFences(g.device, 1, &g.fence, 0, UINT64_MAX);
        vkResetFences(g.device, 1, &g.fence);

        vkAcquireNextImageKHR(g.device, g.swapchain, UINT64_MAX, g.semaphoreAcquire, NULL, &g.imageIndex);

        VkPipelineStageFlags waitDstStageMask;
        waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = NULL;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitDstStageMask = &waitDstStageMask;
        submitInfo.pWaitSemaphores = &g.semaphoreAcquire;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &g.images[g.imageIndex].cmd;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &g.semaphoreRender;
        vkQueueSubmit(g.queue, 1, &submitInfo, g.fence);

        VkPresentInfoKHR presentInfo;
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = NULL;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &g.semaphoreRender;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &g.swapchain;
        presentInfo.pImageIndices = &g.imageIndex;
        presentInfo.pResults = NULL;
        vkQueuePresentKHR(g.queue, &presentInfo);
    }

    teardown:
    arContextTeardown();
    arWindowTeardown();
}

void arCreateGraphicsPipeline(ArPipeline* pPipeline, ArGraphicsPipelineCreateInfo const* pPipelineCreateInfo)
{
    VkFormat colorFormats[1];
    colorFormats[0] = VK_FORMAT_B8G8R8A8_UNORM;

    VkPipelineRenderingCreateInfo renderingCreateInfo;
    renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.pNext = NULL;
    renderingCreateInfo.viewMask = 0;
    renderingCreateInfo.colorAttachmentCount = 1;
    renderingCreateInfo.pColorAttachmentFormats = colorFormats;
    renderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    VkPipelineVertexInputStateCreateInfo vertexInputState;
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.pNext = NULL;
    vertexInputState.flags = 0;
    vertexInputState.vertexBindingDescriptionCount = 0;
    vertexInputState.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.pNext = NULL;
    inputAssemblyState.flags = 0;
    inputAssemblyState.primitiveRestartEnable = false;
    inputAssemblyState.topology = pPipelineCreateInfo->topology;

    VkPipelineViewportStateCreateInfo viewportState;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = NULL;
    viewportState.flags = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports = NULL;
    viewportState.scissorCount = 1;
    viewportState.pScissors = NULL;

    VkPipelineRasterizationStateCreateInfo rasterizationState;
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.pNext = NULL;
    rasterizationState.flags = 0;
    rasterizationState.depthClampEnable = false;
    rasterizationState.rasterizerDiscardEnable = false;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = false;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState;
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.pNext = NULL;
    multisampleState.flags = 0;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.alphaToCoverageEnable = false;
    multisampleState.alphaToOneEnable = false;
    multisampleState.sampleShadingEnable = false;

    VkPipelineDepthStencilStateCreateInfo depthStencilState;
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.pNext = NULL;
    depthStencilState.flags = 0;
    depthStencilState.stencilTestEnable = false;
    depthStencilState.depthBoundsTestEnable = false;
    depthStencilState.depthWriteEnable = false;
    depthStencilState.depthTestEnable = false;

    VkPipelineColorBlendStateCreateInfo colorBlendState;
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.pNext = NULL;
    colorBlendState.flags = 0;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = NULL;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = &renderingCreateInfo;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = NULL;
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;

    vkCreateGraphicsPipelines(g.device, NULL, 1, &pipelineCreateInfo, NULL, (VkPipeline*)&pPipeline->pHandle);
}

void arDestroyPipeline(ArPipeline pipeline)
{
    vkDestroyPipeline(g.device, pipeline.pHandle, NULL);
}

void arCmdBeginPresent(ArCommandBuffer cmd)
{
    VkImageMemoryBarrier2 imageMemoryBarrier;
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageMemoryBarrier.pNext = NULL;
    imageMemoryBarrier.image = ((SwapchainImage*)cmd.pHandle)->handle;
    imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_NONE;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.subresourceRange.levelCount = 1;

    VkDependencyInfo dependencyInfo;
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = NULL;
    dependencyInfo.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.memoryBarrierCount = 0;
    dependencyInfo.bufferMemoryBarrierCount = 0;
    dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;
    vkCmdPipelineBarrier2(((SwapchainImage*)cmd.pHandle)->cmd, &dependencyInfo);

    VkRenderingAttachmentInfo colorAttachmentInfo;
    colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachmentInfo.pNext = NULL;
    colorAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
    colorAttachmentInfo.resolveImageView = NULL;
    colorAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentInfo.imageView = ((SwapchainImage*)cmd.pHandle)->view;
    colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkRenderingInfo renderingInfo;
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.pNext = NULL;
    renderingInfo.flags = 0;
    renderingInfo.renderArea.offset.x = 0;
    renderingInfo.renderArea.offset.y = 0;
    renderingInfo.renderArea.extent.width  = g.extent.width;
    renderingInfo.renderArea.extent.height = g.extent.height;
    renderingInfo.layerCount = 1;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachmentInfo;
    renderingInfo.pDepthAttachment = NULL;
    renderingInfo.pStencilAttachment = NULL;
    vkCmdBeginRendering(((SwapchainImage*)cmd.pHandle)->cmd, &renderingInfo);
}

void arCmdEndPresent(ArCommandBuffer cmd)
{
    vkCmdEndRendering(((SwapchainImage*)cmd.pHandle)->cmd);

    VkImageMemoryBarrier2 imageMemoryBarrier;
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageMemoryBarrier.pNext = NULL;
    imageMemoryBarrier.image = ((SwapchainImage*)cmd.pHandle)->handle;
    imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_NONE;
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.subresourceRange.levelCount = 1;

    VkDependencyInfo dependencyInfo;
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = NULL;
    dependencyInfo.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.memoryBarrierCount = 0;
    dependencyInfo.bufferMemoryBarrierCount = 0;
    dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;
    vkCmdPipelineBarrier2(((SwapchainImage*)cmd.pHandle)->cmd, &dependencyInfo);
}
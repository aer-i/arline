#include "arline.h"

#define internal static
#define global static
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <vulkan/vulkan.h>
#include <dwmapi.h>

typedef struct
{
    VkCommandBuffer cmd;
    VkImage image;
    VkImageView view;
}
ArFrame;

struct
{
    void (*pfnUpdate)();
    void (*pfnResize)();
    void (*pfnRecordCommands)(ArCommandBuffer);
    HINSTANCE hinstance;
    HWND hwnd;
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkSurfaceKHR surface;
    VkCommandPool commandPool;
    VkPipelineLayout pipelineLayout;
    uint32_t imageIndex;
    uint32_t imageCount;
    VkDevice device;
    VkQueue queue;
    VkSwapchainKHR swapchain;
    VkFence fence;
    VkSemaphoreSubmitInfo waitSemaphoreInfo;
    VkCommandBufferSubmitInfo commandBufferInfo;
    VkSemaphoreSubmitInfo signalSemaphoreInfo;
    VkSubmitInfo2 submitInfo;
    VkPresentInfoKHR presentInfo;
    ArFrame frames[6];
    VkExtent2D extent;
    int width, height;
    bool windowShouldClose;
}
global g;

internal void arWindowCreate(int width, int height);
internal void arWindowTeardown(void);
internal void arSwapchainCreate(void);
internal void arSwapchainTeardown(void);
internal void arSwapchainRecreate(void);
internal void arContextCreate(void);
internal void arContextTeardown(void);
internal void arRecordCommands(void);

internal LRESULT
arWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_SIZE:
        g.width = LOWORD(lp);
        g.height = HIWORD(lp);

        while (!g.width || !g.height)
        {
            g.pfnUpdate();

            if (g.windowShouldClose)
            {
                return(0);
            }
        }

        arSwapchainRecreate();
        break;
    case WM_GETMINMAXINFO:
        ((PMINMAXINFO)lp)->ptMinTrackSize.x = 150;
        ((PMINMAXINFO)lp)->ptMinTrackSize.y = 150;
        break;
    case WM_CLOSE:
        g.windowShouldClose = true;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return(DefWindowProcA(hwnd, msg, wp, lp));
    }

    return(0);
}

internal void
arWindowCreate(int width, int height)
{
    g.hinstance = GetModuleHandleA(NULL);

    WNDCLASSEXA wc;
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = arWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g.hinstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursorA(NULL, MAKEINTRESOURCEA(32512));
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "arline";
    wc.hIconSm = NULL;

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    RegisterClassExA(&wc);

    g.hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME, "arline", NULL, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, g.hinstance, NULL
    );

    BOOL useDarkMode = true;
    DwmSetWindowAttribute(g.hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
    ShowWindow(g.hwnd, SW_SHOW);
}

internal void
arWindowTeardown(void)
{
    DestroyWindow(g.hwnd);
    UnregisterClassA("arline", g.hinstance);
}

internal void
arSwapchainCreate(void)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g.gpu, g.surface, &surfaceCapabilities);

    g.extent.width  = surfaceCapabilities.currentExtent.width;
    g.extent.height = surfaceCapabilities.currentExtent.height;

    if (g.extent.width == 0xffffffff)
    {
        g.extent.width  = surfaceCapabilities.minImageExtent.width;
        g.extent.height = surfaceCapabilities.minImageExtent.height;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = NULL;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = g.surface;
    swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount > 3 ? surfaceCapabilities.minImageCount : 3;
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
        g.frames[i].cmd = commandBuffers[i];
        g.frames[i].image = swapchainImages[i];

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
        vkCreateImageView(g.device, &imageViewCreateInfo, NULL, &g.frames[i].view);
    }

    g.submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    g.submitInfo.pNext = NULL;
    g.submitInfo.flags = 0;
    g.submitInfo.waitSemaphoreInfoCount = 1;
    g.submitInfo.pWaitSemaphoreInfos = &g.waitSemaphoreInfo;
    g.submitInfo.commandBufferInfoCount = 1;
    g.submitInfo.pCommandBufferInfos = &g.commandBufferInfo;
    g.submitInfo.signalSemaphoreInfoCount = 1;
    g.submitInfo.pSignalSemaphoreInfos = &g.signalSemaphoreInfo;

    g.presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    g.presentInfo.pNext = NULL;
    g.presentInfo.waitSemaphoreCount = 1;
    g.presentInfo.pWaitSemaphores = &g.signalSemaphoreInfo.semaphore;
    g.presentInfo.swapchainCount = 1;
    g.presentInfo.pSwapchains = &g.swapchain;
    g.presentInfo.pImageIndices = &g.imageIndex;
    g.presentInfo.pResults = NULL;
}

internal void
arSwapchainTeardown(void)
{
    for (uint32_t i = g.imageCount; i--; )
    {
        vkDestroyImageView(g.device, g.frames[i].view, NULL);
    }

    vkDestroyCommandPool(g.device, g.commandPool, NULL);
    vkDestroySwapchainKHR(g.device, g.swapchain, NULL);
}

internal void
arSwapchainRecreate(void)
{
    if (!g.device)
    {
        return;
    }

    vkDeviceWaitIdle(g.device);
    arSwapchainTeardown();
    arSwapchainCreate();
    g.pfnResize();
    arRecordCommands();
}

internal void
arContextCreate(void)
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
        VkSemaphoreCreateInfo semaphoreCreateInfo;
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = NULL;
        semaphoreCreateInfo.flags = 0;
        vkCreateSemaphore(g.device, &semaphoreCreateInfo, NULL, &g.waitSemaphoreInfo.semaphore);
        vkCreateSemaphore(g.device, &semaphoreCreateInfo, NULL, &g.signalSemaphoreInfo.semaphore);

        VkFenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = NULL;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(g.device, &fenceCreateInfo, NULL, &g.fence);
    }
    {
        g.waitSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        g.waitSemaphoreInfo.pNext = NULL;
        g.waitSemaphoreInfo.value = 0;
        g.waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        g.waitSemaphoreInfo.deviceIndex = 0;

        g.commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        g.commandBufferInfo.pNext = NULL;
        g.commandBufferInfo.deviceMask = 0;

        g.signalSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        g.signalSemaphoreInfo.pNext = NULL;
        g.signalSemaphoreInfo.value = 0;
        g.signalSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        g.signalSemaphoreInfo.deviceIndex = 0;
    }
    {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pNext = NULL;
        pipelineLayoutCreateInfo.flags = 0;
        pipelineLayoutCreateInfo.setLayoutCount = 0;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        vkCreatePipelineLayout(g.device, &pipelineLayoutCreateInfo, NULL, &g.pipelineLayout);
    }
    {
        arSwapchainCreate();
    }
}

internal void
arContextTeardown(void)
{
    arSwapchainTeardown();
    vkDestroyPipelineLayout(g.device, g.pipelineLayout, NULL);
    vkDestroyFence(g.device, g.fence, NULL);
    vkDestroySemaphore(g.device, g.signalSemaphoreInfo.semaphore, NULL);
    vkDestroySemaphore(g.device, g.waitSemaphoreInfo.semaphore, NULL);
    vkDestroyDevice(g.device, NULL);
    vkDestroySurfaceKHR(g.instance, g.surface, NULL);
    vkDestroyInstance(g.instance, NULL);
}

void
arExecute(ArApplicationInfo const* pApplicationInfo)
{
    g.pfnUpdate = pApplicationInfo->pfnUpdate;
    g.pfnResize = pApplicationInfo->pfnResize;
    g.pfnRecordCommands = pApplicationInfo->pfnRecordCommands;
    arWindowCreate(pApplicationInfo->width, pApplicationInfo->height);
    arContextCreate();

    pApplicationInfo->pfnInit();
    arRecordCommands(); 

    for ( ; ; )
    {
        pApplicationInfo->pfnUpdate();

        if (g.windowShouldClose)
        {
            break;
        }

        vkWaitForFences(g.device, 1, &g.fence, 0, UINT64_MAX);
        vkResetFences(g.device, 1, &g.fence);

        vkAcquireNextImageKHR(g.device, g.swapchain, UINT64_MAX, g.waitSemaphoreInfo.semaphore, NULL, &g.imageIndex);

        g.commandBufferInfo.commandBuffer = g.frames[g.imageIndex].cmd;
        vkQueueSubmit2(g.queue, 1, &g.submitInfo, g.fence);
        vkQueuePresentKHR(g.queue, &g.presentInfo);
    }

    vkDeviceWaitIdle(g.device);
    pApplicationInfo->pfnTeardown();
    arContextTeardown();
    arWindowTeardown();
}

internal void
arRecordCommands(void)
{
    for (uint32_t i = g.imageCount; i--; )
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo;
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = NULL;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = NULL;

        ArCommandBuffer cmd;
        cmd._data = &g.frames[i];

        vkBeginCommandBuffer(((ArFrame*)cmd._data)->cmd, &commandBufferBeginInfo);
        g.pfnRecordCommands(cmd);
        vkEndCommandBuffer(((ArFrame*)cmd._data)->cmd);
    }
}

void
arPollEvents(void)
{
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

void
arWaitEvents(void)
{
    WaitMessage();
    arPollEvents();
}

void
arCreateShaderFromFile(ArShader* pShader, char const* filename)
{
    LARGE_INTEGER fileSize;
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    GetFileSizeEx(file, &fileSize);

    uint32_t* pBuffer = HeapAlloc(GetProcessHeap(), 0, fileSize.QuadPart);
    DWORD bytesRead;

    ReadFile(file, pBuffer, fileSize.LowPart, &bytesRead, NULL);
    CloseHandle(file);
    arCreateShaderFromMemory(pShader, pBuffer, fileSize.QuadPart);
    HeapFree(GetProcessHeap(), 0, pBuffer);
}

void
arCreateShaderFromMemory(ArShader* pShader, uint32_t const* pCode, size_t codeSize)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo;
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = NULL;
    shaderModuleCreateInfo.flags = 0;
    shaderModuleCreateInfo.codeSize = codeSize;
    shaderModuleCreateInfo.pCode = pCode;
    vkCreateShaderModule(g.device, &shaderModuleCreateInfo, NULL, (VkShaderModule*)pShader);
}

void
arDestroyShader(ArShader shader)
{
    vkDestroyShaderModule(g.device, shader._data, NULL);
}

void
arCreateGraphicsPipeline(ArPipeline* pPipeline, ArGraphicsPipelineCreateInfo const* pPipelineCreateInfo)
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

    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].pNext = NULL;
    shaderStages[0].flags = 0;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = pPipelineCreateInfo->vertShader._data;
    shaderStages[0].pName = "main";
    shaderStages[0].pSpecializationInfo = NULL;

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].pNext = NULL;
    shaderStages[1].flags = 0;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = pPipelineCreateInfo->fragShader._data;
    shaderStages[1].pName = "main";
    shaderStages[1].pSpecializationInfo = NULL;

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
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

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
    rasterizationState.depthBiasClamp = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState;
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.pNext = NULL;
    multisampleState.flags = 0;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.sampleShadingEnable = false;
    multisampleState.pSampleMask = NULL;
    multisampleState.alphaToCoverageEnable = false;
    multisampleState.alphaToOneEnable = false;

    VkPipelineDepthStencilStateCreateInfo depthStencilState;
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.pNext = NULL;
    depthStencilState.flags = 0;
    depthStencilState.stencilTestEnable = false;
    depthStencilState.depthBoundsTestEnable = false;
    depthStencilState.depthWriteEnable = false;
    depthStencilState.depthTestEnable = false;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
    colorBlendAttachmentState.blendEnable = false;
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendState;
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.pNext = NULL;
    colorBlendState.flags = 0;
    colorBlendState.logicOpEnable = false;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachmentState;

    VkDynamicState dynamicStates[2];
    dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDynamicStateCreateInfo dynamicState;
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = NULL;
    dynamicState.flags = 0;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = &renderingCreateInfo;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pTessellationState = NULL;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.layout = g.pipelineLayout;
    pipelineCreateInfo.renderPass = NULL;
    vkCreateGraphicsPipelines(g.device, NULL, 1, &pipelineCreateInfo, NULL, (VkPipeline*)pPipeline);
}

void
arDestroyPipeline(ArPipeline pipeline)
{
    vkDestroyPipeline(g.device, pipeline._data, NULL);
}

void
arCmdBeginPresent(ArCommandBuffer cmd)
{
    VkImageMemoryBarrier2 imageMemoryBarrier;
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageMemoryBarrier.pNext = NULL;
    imageMemoryBarrier.image = ((ArFrame*)cmd._data)->image;
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
    vkCmdPipelineBarrier2(((ArFrame*)cmd._data)->cmd, &dependencyInfo);

    VkRenderingAttachmentInfo colorAttachmentInfo;
    colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachmentInfo.pNext = NULL;
    colorAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
    colorAttachmentInfo.resolveImageView = NULL;
    colorAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentInfo.imageView = ((ArFrame*)cmd._data)->view;
    colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentInfo.clearValue.color.float32[0] = 0.1f;
    colorAttachmentInfo.clearValue.color.float32[1] = 0.1f;
    colorAttachmentInfo.clearValue.color.float32[2] = 0.1f;
    colorAttachmentInfo.clearValue.color.float32[3] = 1.0f;

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
    vkCmdBeginRendering(((ArFrame*)cmd._data)->cmd, &renderingInfo);

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width  = g.extent.width;
    scissor.extent.height = g.extent.height;
    vkCmdSetScissor(((ArFrame*)cmd._data)->cmd, 0, 1, &scissor);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width  = (float)g.extent.width;
    viewport.height = (float)g.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(((ArFrame*)cmd._data)->cmd, 0, 1, &viewport);
}

void
arCmdEndPresent(ArCommandBuffer cmd)
{
    vkCmdEndRendering(((ArFrame*)cmd._data)->cmd);

    VkImageMemoryBarrier2 imageMemoryBarrier;
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageMemoryBarrier.pNext = NULL;
    imageMemoryBarrier.image = ((ArFrame*)cmd._data)->image;
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
    vkCmdPipelineBarrier2(((ArFrame*)cmd._data)->cmd, &dependencyInfo);
}

void
arCmdBindGraphicsPipeline(ArCommandBuffer cmd, ArPipeline pipeline)
{
    vkCmdBindPipeline(((ArFrame*)cmd._data)->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._data);
}

void
arCmdDraw(ArCommandBuffer cmd, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertex, uint32_t instance)
{
    vkCmdDraw(((ArFrame*)cmd._data)->cmd, vertexCount, instanceCount, vertex, instance);
}
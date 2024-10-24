#include "arline.h"

#define internal static
#define global static
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#define WIN32_LEAN_AND_MEAN
#define VKAPI_ATTR
#define VKAPI_CALL __stdcall
#define VKAPI_PTR VKAPI_CALL
#include <stdbool.h>
#include <windows.h>
#include <dwmapi.h>
#include <hidusage.h>
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_win32.h"

typedef struct
{
    VkCommandBuffer cmd;
    VkCommandBuffer presentCmd;
    VkImage image;
    VkImageView view;
}
ArFrame;

typedef struct
{
    bool isDown     : 1;
    bool isPressed  : 1;
    bool isReleased : 1;
}
ArKeyInternal;

struct
{
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
    PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
    PFN_vkDestroyDevice vkDestroyDevice;
    PFN_vkDestroyFence vkDestroyFence;
    PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
    PFN_vkDestroySampler vkDestroySampler;
    PFN_vkDestroySemaphore vkDestroySemaphore;

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
    PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
    PFN_vkCreateInstance vkCreateInstance;
    PFN_vkCreateDevice vkCreateDevice;
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
    PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
    PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
    PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
    PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
    PFN_vkCreateFence vkCreateFence;
    PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
    PFN_vkCreateSampler vkCreateSampler;
    PFN_vkCreateSemaphore vkCreateSemaphore;
    PFN_vkGetDeviceQueue vkGetDeviceQueue;

    PFN_vkDestroyImage vkDestroyImage;
    PFN_vkDestroyCommandPool vkDestroyCommandPool;
    PFN_vkDestroyImageView vkDestroyImageView;
    PFN_vkDestroyPipeline vkDestroyPipeline;
    PFN_vkDestroyShaderModule vkDestroyShaderModule;
    PFN_vkFreeMemory vkFreeMemory;
    PFN_vkUnmapMemory vkUnmapMemory;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
    PFN_vkDestroyBuffer vkDestroyBuffer;

    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
    PFN_vkAllocateMemory vkAllocateMemory;
    PFN_vkBindImageMemory vkBindImageMemory;
    PFN_vkBindBufferMemory vkBindBufferMemory;
    PFN_vkCreateCommandPool vkCreateCommandPool;
    PFN_vkCreateBuffer vkCreateBuffer;
    PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
    PFN_vkCreateImage vkCreateImage;
    PFN_vkCreateImageView vkCreateImageView;
    PFN_vkCreateShaderModule vkCreateShaderModule;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
    PFN_vkMapMemory vkMapMemory;
    PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
    PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
    PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
    PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;

    PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
    PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
    PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
    PFN_vkCmdBindPipeline vkCmdBindPipeline;
    PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
    PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
    PFN_vkCmdDraw vkCmdDraw;
    PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
    PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
    PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
    PFN_vkCmdPushConstants vkCmdPushConstants;
    PFN_vkCmdSetScissor vkCmdSetScissor;
    PFN_vkCmdSetViewport vkCmdSetViewport;
    PFN_vkEndCommandBuffer vkEndCommandBuffer;
    PFN_vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCount;
    PFN_vkCmdDrawIndirectCount vkCmdDrawIndirectCount;
    PFN_vkCmdBeginRendering vkCmdBeginRendering;
    PFN_vkCmdEndRendering vkCmdEndRendering;
    PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2;

    PFN_vkResetCommandPool vkResetCommandPool;
    PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
    PFN_vkQueueWaitIdle vkQueueWaitIdle;
    
    PFN_vkWaitForFences vkWaitForFences;
    PFN_vkResetFences vkResetFences;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
    PFN_vkQueueSubmit2 vkQueueSubmit2;
    PFN_vkQueuePresentKHR vkQueuePresentKHR;

    void (*pfnUpdate)();
    void (*pfnResize)();
    void (*pfnRecordCommands)();
    LARGE_INTEGER timeOffset;
    LARGE_INTEGER timeFrequency;
    double previousTime;
    double deltaTime;
    HINSTANCE hinstance;
    HWND hwnd;
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkSurfaceKHR surface;
    uint32_t graphicsQueueFamily;
    uint32_t presentQueueFamily;
    VkCommandPool graphicsCommandPool;
    VkCommandPool presentCommandPool;
    VkCommandPool transferCommandPool;
    VkCommandBuffer transferCommandBuffer;
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    VkSampler samplerLinearToEdge;
    VkSampler samplerLinearRepeat;
    VkSampler samplerNearestToEdge;
    VkSampler samplerNearestRepeat;
    ArFrame* pFrame;
    uint32_t imageIndex;
    uint32_t imageCount;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapchain;
    VkFence fence;
    VkCommandBufferSubmitInfo graphicsCommandBufferInfo;
    VkCommandBufferSubmitInfo presentCommandBufferInfo;
    VkSemaphoreSubmitInfo acqSemaphore;
    VkSemaphoreSubmitInfo renSemaphore;
    VkSemaphoreSubmitInfo preSemaphore;
    VkSubmitInfo2 submitInfo;
    VkSubmitInfo2 presentSubmitInfo;
    VkPresentInfoKHR presentInfo;
    ArFrame frames[6];
    VkExtent2D extent;
    int width, height;
    bool unifiedQueue;
    bool vsyncEnabled;
    bool windowShouldClose;
    int globalCursorX;
    int globalCursorY;
    int cursorX;
    int cursorY;
    int cursorDeltaX;
    int cursorDeltaY;
    int cursorRelX;
    int cursorRelY;
    ArKeyInternal keys[255];
    ArKeyInternal buttons[5];
    BYTE lpb[sizeof(RAWINPUT)];
}
global g;

internal uint32_t arFindMemoryType(uint32_t typeBitsRequirement, VkMemoryPropertyFlags properties);
internal void* arLoadInstanceFunction(char const* name);
internal void* arLoadDeviceFunction(char const* name);
internal void arError(char const* message);
internal void arLoadInstanceFunctions(void);
internal void arLoadDeviceFunctions(void);
internal void arAllocBuffer(ArBuffer* pBuffer, bool isDynamic);
internal void arWindowCreate(int width, int height);
internal void arWindowTeardown(void);
internal void arSwapchainCreate(bool vsync);
internal void arSwapchainTeardown(void);
internal void arSwapchainRecreate(bool vsync);
internal void arContextCreate(void);
internal void arContextTeardown(void);
internal void arRecordCommands(void);
internal void arBeginTransfer();
internal void arEndTransfer();

internal void
arError(
    char const* message)
{
    MessageBoxA(NULL, message, NULL, MB_ICONERROR);
    ExitProcess(1);
}

internal void
arVkCheck(VkResult result)
{
    switch (result)
    {
    case VK_SUCCESS:
    case VK_INCOMPLETE:
        break;
    case VK_ERROR_DEVICE_LOST:
        arError("Device Lost");
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        arError("Out of device memory");
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        arError("Out of host memory");
    case VK_ERROR_SURFACE_LOST_KHR:
        arError("Surface lost");
    case VK_ERROR_INITIALIZATION_FAILED:
        arError("Initialization failed");
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        arError("Incompatible driver");
    case VK_ERROR_MEMORY_MAP_FAILED:
        arError("Memory map failed");
    default:
        arError("Unknown Vulkan Error");
    }
}

internal void*
arLoadInstanceFunction(
    char const* name)
{
    return((void*)g.vkGetInstanceProcAddr(g.instance, name));
}

internal void*
arLoadDeviceFunction(
    char const* name)
{
    return((void*)g.vkGetDeviceProcAddr(g.device, name));
}

internal void
arLoadInstanceFunctions(void)
{
    g.vkCreateDevice = (PFN_vkCreateDevice)arLoadInstanceFunction("vkCreateDevice");
    g.vkDestroyInstance = (PFN_vkDestroyInstance)arLoadInstanceFunction("vkDestroyInstance");
    g.vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)arLoadInstanceFunction("vkEnumerateDeviceExtensionProperties");
    g.vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)arLoadInstanceFunction("vkEnumeratePhysicalDevices");
    g.vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)arLoadInstanceFunction("vkGetDeviceProcAddr");
    g.vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)arLoadInstanceFunction("vkGetPhysicalDeviceFeatures");
    g.vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties)arLoadInstanceFunction("vkGetPhysicalDeviceFormatProperties");
    g.vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties)arLoadInstanceFunction("vkGetPhysicalDeviceImageFormatProperties");
    g.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)arLoadInstanceFunction("vkGetPhysicalDeviceMemoryProperties");
    g.vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)arLoadInstanceFunction("vkGetPhysicalDeviceProperties");
    g.vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)arLoadInstanceFunction("vkGetPhysicalDeviceQueueFamilyProperties");
    g.vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)arLoadInstanceFunction("vkCreateWin32SurfaceKHR");
    g.vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)arLoadInstanceFunction("vkDestroySurfaceKHR");
    g.vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)arLoadInstanceFunction("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    g.vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)arLoadInstanceFunction("vkGetPhysicalDeviceSurfaceFormatsKHR");
    g.vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)arLoadInstanceFunction("vkGetPhysicalDeviceSurfacePresentModesKHR");
    g.vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)arLoadInstanceFunction("vkGetPhysicalDeviceSurfaceSupportKHR");
}

internal void
arLoadDeviceFunctions()
{
    g.vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)arLoadDeviceFunction("vkAllocateCommandBuffers");
    g.vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)arLoadDeviceFunction("vkAllocateDescriptorSets");
    g.vkAllocateMemory = (PFN_vkAllocateMemory)arLoadDeviceFunction("vkAllocateMemory");
    g.vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)arLoadDeviceFunction("vkBeginCommandBuffer");
    g.vkBindBufferMemory = (PFN_vkBindBufferMemory)arLoadDeviceFunction("vkBindBufferMemory");
    g.vkBindImageMemory = (PFN_vkBindImageMemory)arLoadDeviceFunction("vkBindImageMemory");
    g.vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)arLoadDeviceFunction("vkCmdBindDescriptorSets");
    g.vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)arLoadDeviceFunction("vkCmdBindIndexBuffer");
    g.vkCmdBindPipeline = (PFN_vkCmdBindPipeline)arLoadDeviceFunction("vkCmdBindPipeline");
    g.vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)arLoadDeviceFunction("vkCmdCopyBuffer");
    g.vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)arLoadDeviceFunction("vkCmdCopyBufferToImage");
    g.vkCmdDraw = (PFN_vkCmdDraw)arLoadDeviceFunction("vkCmdDraw");
    g.vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)arLoadDeviceFunction("vkCmdDrawIndexed");
    g.vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)arLoadDeviceFunction("vkCmdDrawIndexedIndirect");
    g.vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect)arLoadDeviceFunction("vkCmdDrawIndirect");
    g.vkCmdPushConstants = (PFN_vkCmdPushConstants)arLoadDeviceFunction("vkCmdPushConstants");
    g.vkCmdSetScissor = (PFN_vkCmdSetScissor)arLoadDeviceFunction("vkCmdSetScissor");
    g.vkCmdSetViewport = (PFN_vkCmdSetViewport)arLoadDeviceFunction("vkCmdSetViewport");
    g.vkCreateBuffer = (PFN_vkCreateBuffer)arLoadDeviceFunction("vkCreateBuffer");
    g.vkCreateCommandPool = (PFN_vkCreateCommandPool)arLoadDeviceFunction("vkCreateCommandPool");
    g.vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)arLoadDeviceFunction("vkCreateDescriptorPool");
    g.vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)arLoadDeviceFunction("vkCreateDescriptorSetLayout");
    g.vkCreateFence = (PFN_vkCreateFence)arLoadDeviceFunction("vkCreateFence");
    g.vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)arLoadDeviceFunction("vkCreateGraphicsPipelines");
    g.vkCreateImage = (PFN_vkCreateImage)arLoadDeviceFunction("vkCreateImage");
    g.vkCreateImageView = (PFN_vkCreateImageView)arLoadDeviceFunction("vkCreateImageView");
    g.vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)arLoadDeviceFunction("vkCreatePipelineLayout");
    g.vkCreateSampler = (PFN_vkCreateSampler)arLoadDeviceFunction("vkCreateSampler");
    g.vkCreateSemaphore = (PFN_vkCreateSemaphore)arLoadDeviceFunction("vkCreateSemaphore");
    g.vkCreateShaderModule = (PFN_vkCreateShaderModule)arLoadDeviceFunction("vkCreateShaderModule");
    g.vkDestroyBuffer = (PFN_vkDestroyBuffer)arLoadDeviceFunction("vkDestroyBuffer");
    g.vkDestroyCommandPool = (PFN_vkDestroyCommandPool)arLoadDeviceFunction("vkDestroyCommandPool");
    g.vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)arLoadDeviceFunction("vkDestroyDescriptorPool");
    g.vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)arLoadDeviceFunction("vkDestroyDescriptorSetLayout");
    g.vkDestroyDevice = (PFN_vkDestroyDevice)arLoadDeviceFunction("vkDestroyDevice");
    g.vkDestroyFence = (PFN_vkDestroyFence)arLoadDeviceFunction("vkDestroyFence");
    g.vkDestroyImage = (PFN_vkDestroyImage)arLoadDeviceFunction("vkDestroyImage");
    g.vkDestroyImageView = (PFN_vkDestroyImageView)arLoadDeviceFunction("vkDestroyImageView");
    g.vkDestroyPipeline = (PFN_vkDestroyPipeline)arLoadDeviceFunction("vkDestroyPipeline");
    g.vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)arLoadDeviceFunction("vkDestroyPipelineLayout");
    g.vkDestroySampler = (PFN_vkDestroySampler)arLoadDeviceFunction("vkDestroySampler");
    g.vkDestroySemaphore = (PFN_vkDestroySemaphore)arLoadDeviceFunction("vkDestroySemaphore");
    g.vkDestroyShaderModule = (PFN_vkDestroyShaderModule)arLoadDeviceFunction("vkDestroyShaderModule");
    g.vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)arLoadDeviceFunction("vkDeviceWaitIdle");
    g.vkEndCommandBuffer = (PFN_vkEndCommandBuffer)arLoadDeviceFunction("vkEndCommandBuffer");
    g.vkFreeMemory = (PFN_vkFreeMemory)arLoadDeviceFunction("vkFreeMemory");
    g.vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)arLoadDeviceFunction("vkGetBufferMemoryRequirements");
    g.vkGetDeviceQueue = (PFN_vkGetDeviceQueue)arLoadDeviceFunction("vkGetDeviceQueue");
    g.vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)arLoadDeviceFunction("vkGetImageMemoryRequirements");
    g.vkMapMemory = (PFN_vkMapMemory)arLoadDeviceFunction("vkMapMemory");
    g.vkQueueWaitIdle = (PFN_vkQueueWaitIdle)arLoadDeviceFunction("vkQueueWaitIdle");
    g.vkResetCommandPool = (PFN_vkResetCommandPool)arLoadDeviceFunction("vkResetCommandPool");
    g.vkResetFences = (PFN_vkResetFences)arLoadDeviceFunction("vkResetFences");
    g.vkUnmapMemory = (PFN_vkUnmapMemory)arLoadDeviceFunction("vkUnmapMemory");
    g.vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)arLoadDeviceFunction("vkUpdateDescriptorSets");
    g.vkWaitForFences = (PFN_vkWaitForFences)arLoadDeviceFunction("vkWaitForFences");
    g.vkCmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)arLoadDeviceFunction("vkCmdDrawIndexedIndirectCount");
    g.vkCmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)arLoadDeviceFunction("vkCmdDrawIndirectCount");
    g.vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)arLoadDeviceFunction("vkGetBufferDeviceAddress");
    g.vkCmdBeginRendering = (PFN_vkCmdBeginRendering)arLoadDeviceFunction("vkCmdBeginRendering");
    g.vkCmdEndRendering = (PFN_vkCmdEndRendering)arLoadDeviceFunction("vkCmdEndRendering");
    g.vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)arLoadDeviceFunction("vkCmdPipelineBarrier2");
    g.vkQueueSubmit2 = (PFN_vkQueueSubmit2)arLoadDeviceFunction("vkQueueSubmit2");
    g.vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)arLoadDeviceFunction("vkAcquireNextImageKHR");
    g.vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)arLoadDeviceFunction("vkCreateSwapchainKHR");
    g.vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)arLoadDeviceFunction("vkDestroySwapchainKHR");
    g.vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)arLoadDeviceFunction("vkGetSwapchainImagesKHR");
    g.vkQueuePresentKHR = (PFN_vkQueuePresentKHR)arLoadDeviceFunction("vkQueuePresentKHR");
}

internal LRESULT
arWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wp,
    LPARAM lp)
{
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        g.buttons[AR_BUTTON_LEFT].isDown    = true;
        g.buttons[AR_BUTTON_LEFT].isPressed = true;
        break;
    case WM_LBUTTONUP:
        g.buttons[AR_BUTTON_LEFT].isDown     = false;
        g.buttons[AR_BUTTON_LEFT].isReleased = true;
        break;
    case WM_RBUTTONDOWN:
        g.buttons[AR_BUTTON_RIGHT].isDown    = true;
        g.buttons[AR_BUTTON_RIGHT].isPressed = true;
        break;
    case WM_RBUTTONUP:
        g.buttons[AR_BUTTON_RIGHT].isDown     = false;
        g.buttons[AR_BUTTON_RIGHT].isReleased = true;
        break;
    case WM_MBUTTONDOWN:
        g.buttons[AR_BUTTON_MIDDLE].isDown    = true;
        g.buttons[AR_BUTTON_MIDDLE].isPressed = true;
        break;
    case WM_MBUTTONUP:
        g.buttons[AR_BUTTON_MIDDLE].isDown     = false;
        g.buttons[AR_BUTTON_MIDDLE].isReleased = true;
        break;
    case WM_XBUTTONDOWN:
        g.buttons[AR_BUTTON_MIDDLE + HIWORD(wp)].isDown    = true;
        g.buttons[AR_BUTTON_MIDDLE + HIWORD(wp)].isPressed = true;
        break;   
    case WM_XBUTTONUP:
        g.buttons[AR_BUTTON_MIDDLE + HIWORD(wp)].isDown     = false;
        g.buttons[AR_BUTTON_MIDDLE + HIWORD(wp)].isReleased = true;
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        g.keys[wp].isDown = true;
        g.keys[wp].isPressed = !(lp & (1 << 30));
        return(DefWindowProcA(hwnd, msg, wp, lp));
    case WM_KEYUP:
    case WM_SYSKEYUP:
        g.keys[wp].isDown = false;
        g.keys[wp].isReleased = true;
        return(DefWindowProcA(hwnd, msg, wp, lp));
    case WM_INPUT:
    {
        UINT dwSize = sizeof(RAWINPUT);

        GetRawInputData((HRAWINPUT)lp, RID_INPUT, g.lpb, &dwSize, sizeof(RAWINPUTHEADER));

        RAWINPUT* raw = (RAWINPUT*)g.lpb;

        if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            g.cursorRelX = raw->data.mouse.lLastX;
            g.cursorRelY = raw->data.mouse.lLastY;
        } 
    } break;
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

        if (g.device)
        {
            g.vkDeviceWaitIdle(g.device);
            arSwapchainRecreate(g.vsyncEnabled);
        }
        break;
    case WM_GETMINMAXINFO:
        ((PMINMAXINFO)lp)->ptMinTrackSize.x = 150;
        ((PMINMAXINFO)lp)->ptMinTrackSize.y = 150;
        break;
    case WM_CLOSE:
        g.windowShouldClose = true;
        break;
    case WM_MOVE:
    case WM_ACTIVATE:
        memset(g.keys, 0, sizeof(g.keys));
        memset(g.buttons, 0, sizeof(g.buttons));
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SYSCOMMAND:
        if ((wp & 0xfff0) == SC_KEYMENU) break;
    default:
        return(DefWindowProcA(hwnd, msg, wp, lp));
    }

    return(0);
}

internal void
arWindowCreate(
    int width,
    int height)
{
    QueryPerformanceFrequency(&g.timeFrequency);
    QueryPerformanceCounter(&g.timeOffset);
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
    if (!RegisterClassExA(&wc))
    {
        arError("Failed to register window class");
    }

    g.hwnd = CreateWindowExA(
        WS_EX_DLGMODALFRAME,
        "arline",
        NULL,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        NULL,
        NULL,
        g.hinstance,
        NULL);

    if (!g.hwnd)
    {
        arError("Failed to create window");
    }

    BOOL useDarkMode = true;
    DwmSetWindowAttribute(g.hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
    ShowWindow(g.hwnd, SW_SHOW);

    RAWINPUTDEVICE rid[1];
    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
    rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
    rid[0].dwFlags = RIDEV_INPUTSINK;   
    rid[0].hwndTarget = g.hwnd;
    RegisterRawInputDevices(rid, 1, sizeof(rid[0]));
}

internal void
arWindowTeardown(void)
{
    DestroyWindow(g.hwnd);
    UnregisterClassA("arline", g.hinstance);
}

internal void
arSwapchainCreate(
    bool vsync)
{
    g.vsyncEnabled = vsync;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    arVkCheck(g.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g.gpu, g.surface, &surfaceCapabilities));

    g.extent.width  = surfaceCapabilities.currentExtent.width;
    g.extent.height = surfaceCapabilities.currentExtent.height;

    if (g.extent.width == 0xffffffff)
    {
        g.extent.width  = surfaceCapabilities.minImageExtent.width;
        g.extent.height = surfaceCapabilities.minImageExtent.height;
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    if (!vsync)
    {
        VkPresentModeKHR presentModes[6];
        uint32_t presentModeCount;
        arVkCheck(g.vkGetPhysicalDeviceSurfacePresentModesKHR(g.gpu, g.surface, &presentModeCount, NULL));
        presentModeCount = min(presentModeCount, 6);
        arVkCheck(g.vkGetPhysicalDeviceSurfacePresentModesKHR(g.gpu, g.surface, &presentModeCount, presentModes));

        for ( ; presentModeCount--; )
        {
            if (presentModes[presentModeCount] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }

            if (presentModes[presentModeCount] == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
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
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = true;
    swapchainCreateInfo.oldSwapchain = NULL;
    arVkCheck(g.vkCreateSwapchainKHR(g.device, &swapchainCreateInfo, NULL, &g.swapchain));

    VkCommandBuffer commandBuffers[6];
    VkCommandBuffer presentCommandBuffers[6];
    VkImage swapchainImages[6];
    arVkCheck(g.vkGetSwapchainImagesKHR(g.device, g.swapchain, &g.imageCount, NULL));
    g.imageCount = min(g.imageCount, 6);
    arVkCheck(g.vkGetSwapchainImagesKHR(g.device, g.swapchain, &g.imageCount, swapchainImages));

    VkCommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = NULL;
    commandPoolCreateInfo.flags = 0;
    commandPoolCreateInfo.queueFamilyIndex = g.graphicsQueueFamily;
    arVkCheck(g.vkCreateCommandPool(g.device, &commandPoolCreateInfo, NULL, &g.graphicsCommandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = NULL;
    commandBufferAllocateInfo.commandPool = g.graphicsCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = g.imageCount;
    arVkCheck(g.vkAllocateCommandBuffers(g.device, &commandBufferAllocateInfo, commandBuffers));

    if (!g.unifiedQueue)
    {
        commandPoolCreateInfo.queueFamilyIndex = g.presentQueueFamily;
        arVkCheck(g.vkCreateCommandPool(g.device, &commandPoolCreateInfo, NULL, &g.presentCommandPool));
        
        commandBufferAllocateInfo.commandPool = g.presentCommandPool;
        arVkCheck(g.vkAllocateCommandBuffers(g.device, &commandBufferAllocateInfo, presentCommandBuffers));
    }

    for (uint32_t i = g.imageCount; i--; )
    {
        g.frames[i].cmd = commandBuffers[i];
        g.frames[i].presentCmd = presentCommandBuffers[i];
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
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        arVkCheck(g.vkCreateImageView(g.device, &imageViewCreateInfo, NULL, &g.frames[i].view));

        if (!g.unifiedQueue)
        {
            VkCommandBufferBeginInfo commandBufferBeginInfo;
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.pNext = NULL;
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            commandBufferBeginInfo.pInheritanceInfo = NULL;

            VkImageMemoryBarrier2 imageMemoryBarrier;
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            imageMemoryBarrier.pNext = NULL;
            imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_NONE;
            imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_NONE;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            imageMemoryBarrier.srcQueueFamilyIndex = g.graphicsQueueFamily;
            imageMemoryBarrier.dstQueueFamilyIndex = g.presentQueueFamily;
            imageMemoryBarrier.image = swapchainImages[i];
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = 1;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = 1;

            VkDependencyInfo dependencyInfo;
            dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependencyInfo.pNext = NULL;
            dependencyInfo.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            dependencyInfo.memoryBarrierCount = 0;
            dependencyInfo.bufferMemoryBarrierCount = 0;
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;

            arVkCheck(g.vkBeginCommandBuffer(presentCommandBuffers[i], &commandBufferBeginInfo));
            g.vkCmdPipelineBarrier2(presentCommandBuffers[i], &dependencyInfo);
            arVkCheck(g.vkEndCommandBuffer(presentCommandBuffers[i]));
        }
    }

    g.submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    g.submitInfo.pNext = NULL;
    g.submitInfo.flags = 0;
    g.submitInfo.waitSemaphoreInfoCount = 1;
    g.submitInfo.pWaitSemaphoreInfos = &g.acqSemaphore;
    g.submitInfo.commandBufferInfoCount = 1;
    g.submitInfo.pCommandBufferInfos = &g.graphicsCommandBufferInfo;
    g.submitInfo.signalSemaphoreInfoCount = 1;
    g.submitInfo.pSignalSemaphoreInfos = &g.renSemaphore;

    g.presentSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    g.presentSubmitInfo.pNext = NULL;
    g.presentSubmitInfo.flags = 0;
    g.presentSubmitInfo.waitSemaphoreInfoCount = 1;
    g.presentSubmitInfo.pWaitSemaphoreInfos = &g.renSemaphore;
    g.presentSubmitInfo.commandBufferInfoCount = 1;
    g.presentSubmitInfo.pCommandBufferInfos = &g.presentCommandBufferInfo;
    g.presentSubmitInfo.signalSemaphoreInfoCount = 1;
    g.presentSubmitInfo.pSignalSemaphoreInfos = &g.preSemaphore;

    g.presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    g.presentInfo.pNext = NULL;
    g.presentInfo.waitSemaphoreCount = 1;
    g.presentInfo.pWaitSemaphores = g.unifiedQueue ? &g.renSemaphore.semaphore : &g.preSemaphore.semaphore;
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
        g.vkDestroyImageView(g.device, g.frames[i].view, NULL);
    }

    if (!g.unifiedQueue)
    {
        g.vkDestroyCommandPool(g.device, g.presentCommandPool, NULL);
    }

    g.vkDestroyCommandPool(g.device, g.graphicsCommandPool, NULL);
    g.vkDestroySwapchainKHR(g.device, g.swapchain, NULL);
}

internal void
arSwapchainRecreate(bool vsync)
{
    uint32_t prevWidth  = g.extent.width;
    uint32_t prevHeihgt = g.extent.height;

    arSwapchainTeardown();
    arSwapchainCreate(vsync);

    if (prevWidth != g.extent.width || prevHeihgt != g.extent.height)
    {
        g.pfnResize();
    }

    arRecordCommands();
}

internal void
arContextCreate(void)
{
    {
        HMODULE vulkanDll = LoadLibraryA("vulkan-1.dll");

        if (!vulkanDll)
        {
            arError("Failed to load vulkan-1.dll");
        }

        g.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(vulkanDll, "vkGetInstanceProcAddr");
        g.vkCreateInstance = (PFN_vkCreateInstance)g.vkGetInstanceProcAddr(NULL, "vkCreateInstance");
        g.vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)g.vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");
    }
    {
        uint32_t apiVersion;
        arVkCheck(g.vkEnumerateInstanceVersion(&apiVersion));

        if (apiVersion < VK_API_VERSION_1_3)
        {
            arError("Vulkan 1.3 required");
        }

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
        applicationInfo.apiVersion = apiVersion;

        VkInstanceCreateInfo instanceCreateInfo;
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = NULL;
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledExtensionCount = 2;
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;
        instanceCreateInfo.enabledLayerCount = 0;
        arVkCheck(g.vkCreateInstance(&instanceCreateInfo, NULL, &g.instance));
        arLoadInstanceFunctions();
    }
    {
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.pNext = NULL;
        surfaceCreateInfo.flags = 0;
        surfaceCreateInfo.hinstance = g.hinstance;
        surfaceCreateInfo.hwnd = g.hwnd;
        arVkCheck(g.vkCreateWin32SurfaceKHR(g.instance, &surfaceCreateInfo, NULL, &g.surface));
    }
    {
        VkPhysicalDevice gpus[64];
        uint32_t gpuCount;
        arVkCheck(g.vkEnumeratePhysicalDevices(g.instance, &gpuCount, NULL));
        gpuCount = min(gpuCount, 64);
        arVkCheck(g.vkEnumeratePhysicalDevices(g.instance, &gpuCount, gpus));

        for ( ; gpuCount--; )
        {
            VkPhysicalDeviceProperties properties;
            g.vkGetPhysicalDeviceProperties(gpus[gpuCount], &properties);

            if (properties.apiVersion < VK_API_VERSION_1_3)
            {
                continue;
            }

            g.gpu = gpus[gpuCount];
        }

        if (!g.gpu)
        {
            arError("No suitable gpu. Try updating drivers");
        }

        VkQueueFamilyProperties queueProperties[32];
        uint32_t queuePropertyCount;
        g.vkGetPhysicalDeviceQueueFamilyProperties(g.gpu, &queuePropertyCount, NULL);
        queuePropertyCount = min(queuePropertyCount, 32);
        g.vkGetPhysicalDeviceQueueFamilyProperties(g.gpu, &queuePropertyCount, queueProperties);

        g.graphicsQueueFamily = ~0u;
        g.presentQueueFamily  = ~0u;

        for ( ; queuePropertyCount--; )
        {
            VkBool32 presentSupported;
            arVkCheck(g.vkGetPhysicalDeviceSurfaceSupportKHR(
                g.gpu,
                queuePropertyCount,
                g.surface,
                &presentSupported));

            if ((g.graphicsQueueFamily & ~0u) & (queueProperties[queuePropertyCount].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                g.graphicsQueueFamily = queuePropertyCount;
            }
            
            if (presentSupported & (queuePropertyCount != g.graphicsQueueFamily))
            {
                g.presentQueueFamily = queuePropertyCount;
            }
        }

        if (g.presentQueueFamily & ~0u)
        {
            g.presentQueueFamily = g.graphicsQueueFamily;
            g.unifiedQueue = true;
        }
    }
    {
        char const* deviceExtensions[1];
        deviceExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

        float priorities[1];
        priorities[0] = 0.0f;

        VkDeviceQueueCreateInfo queueCreateInfos[2];
        queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[0].pNext = NULL;
        queueCreateInfos[0].flags = 0;
        queueCreateInfos[0].queueCount = 1;
        queueCreateInfos[0].queueFamilyIndex = g.graphicsQueueFamily;
        queueCreateInfos[0].pQueuePriorities = priorities;

        queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[1].pNext = NULL;
        queueCreateInfos[1].flags = 0;
        queueCreateInfos[1].queueCount = 1;
        queueCreateInfos[1].queueFamilyIndex = g.presentQueueFamily;
        queueCreateInfos[1].pQueuePriorities = priorities;

        VkPhysicalDeviceVulkan12Features vulkan12Features;
        vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        vulkan12Features.pNext = NULL;
        vulkan12Features.samplerMirrorClampToEdge = false;
        vulkan12Features.drawIndirectCount = true;
        vulkan12Features.storageBuffer8BitAccess = false;
        vulkan12Features.uniformAndStorageBuffer8BitAccess = false;
        vulkan12Features.storagePushConstant8 = false;
        vulkan12Features.shaderBufferInt64Atomics = false;
        vulkan12Features.shaderSharedInt64Atomics = false;
        vulkan12Features.shaderFloat16 = false;
        vulkan12Features.shaderInt8 = false;
        vulkan12Features.descriptorIndexing = true;
        vulkan12Features.shaderInputAttachmentArrayDynamicIndexing = false;
        vulkan12Features.shaderUniformTexelBufferArrayDynamicIndexing = false;
        vulkan12Features.shaderStorageTexelBufferArrayDynamicIndexing = false;
        vulkan12Features.shaderUniformBufferArrayNonUniformIndexing = false;
        vulkan12Features.shaderSampledImageArrayNonUniformIndexing = true;
        vulkan12Features.shaderStorageBufferArrayNonUniformIndexing = false;
        vulkan12Features.shaderStorageImageArrayNonUniformIndexing = false;
        vulkan12Features.shaderInputAttachmentArrayNonUniformIndexing = false;
        vulkan12Features.shaderUniformTexelBufferArrayNonUniformIndexing = false;
        vulkan12Features.shaderStorageTexelBufferArrayNonUniformIndexing = false;
        vulkan12Features.descriptorBindingUniformBufferUpdateAfterBind = false;
        vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = true;
        vulkan12Features.descriptorBindingStorageImageUpdateAfterBind = false;
        vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = false;
        vulkan12Features.descriptorBindingUniformTexelBufferUpdateAfterBind = false;
        vulkan12Features.descriptorBindingStorageTexelBufferUpdateAfterBind = false;
        vulkan12Features.descriptorBindingUpdateUnusedWhilePending = false;
        vulkan12Features.descriptorBindingPartiallyBound = true;
        vulkan12Features.descriptorBindingVariableDescriptorCount = false;
        vulkan12Features.runtimeDescriptorArray = true;
        vulkan12Features.samplerFilterMinmax = false;
        vulkan12Features.scalarBlockLayout = false;
        vulkan12Features.imagelessFramebuffer = false;
        vulkan12Features.uniformBufferStandardLayout = false;
        vulkan12Features.shaderSubgroupExtendedTypes = false;
        vulkan12Features.separateDepthStencilLayouts = false;
        vulkan12Features.hostQueryReset = false;
        vulkan12Features.timelineSemaphore = false;
        vulkan12Features.bufferDeviceAddress = true;
        vulkan12Features.bufferDeviceAddressCaptureReplay = false;
        vulkan12Features.bufferDeviceAddressMultiDevice = false;
        vulkan12Features.vulkanMemoryModel = false;
        vulkan12Features.vulkanMemoryModelDeviceScope = false;
        vulkan12Features.vulkanMemoryModelAvailabilityVisibilityChains = false;
        vulkan12Features.shaderOutputViewportIndex = false;
        vulkan12Features.shaderOutputLayer = false;
        vulkan12Features.subgroupBroadcastDynamicId = false;

        VkPhysicalDeviceVulkan13Features vulkan13Features;
        vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vulkan13Features.pNext = &vulkan12Features;
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

        VkPhysicalDeviceFeatures2 features;
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = &vulkan13Features;
        features.features.robustBufferAccess = false;
        features.features.fullDrawIndexUint32 = true;
        features.features.imageCubeArray = false;
        features.features.independentBlend = false;
        features.features.geometryShader = false;
        features.features.tessellationShader = false;
        features.features.sampleRateShading = false;
        features.features.dualSrcBlend = false;
        features.features.logicOp = false;
        features.features.multiDrawIndirect = true;
        features.features.drawIndirectFirstInstance = true;
        features.features.depthClamp = false;
        features.features.depthBiasClamp = false;
        features.features.fillModeNonSolid = true;
        features.features.depthBounds = false;
        features.features.wideLines = false;
        features.features.largePoints = false;
        features.features.alphaToOne = false;
        features.features.multiViewport = false;
        features.features.samplerAnisotropy = false;
        features.features.textureCompressionETC2 = false;
        features.features.textureCompressionASTC_LDR = false;
        features.features.textureCompressionBC = false;
        features.features.occlusionQueryPrecise = false;
        features.features.pipelineStatisticsQuery = false;
        features.features.vertexPipelineStoresAndAtomics = false;
        features.features.fragmentStoresAndAtomics = false;
        features.features.shaderTessellationAndGeometryPointSize = false;
        features.features.shaderImageGatherExtended = false;
        features.features.shaderStorageImageExtendedFormats = false;
        features.features.shaderStorageImageMultisample = false;
        features.features.shaderStorageImageReadWithoutFormat = false;
        features.features.shaderStorageImageWriteWithoutFormat = false;
        features.features.shaderUniformBufferArrayDynamicIndexing = false;
        features.features.shaderSampledImageArrayDynamicIndexing = false;
        features.features.shaderStorageBufferArrayDynamicIndexing = false;
        features.features.shaderStorageImageArrayDynamicIndexing = false;
        features.features.shaderClipDistance = false;
        features.features.shaderCullDistance = false;
        features.features.shaderFloat64 = false;
        features.features.shaderInt64 = false;
        features.features.shaderInt16 = false;
        features.features.shaderResourceResidency = false;
        features.features.shaderResourceMinLod = false;
        features.features.sparseBinding = false;
        features.features.sparseResidencyBuffer = false;
        features.features.sparseResidencyImage2D = false;
        features.features.sparseResidencyImage3D = false;
        features.features.sparseResidency2Samples = false;
        features.features.sparseResidency4Samples = false;
        features.features.sparseResidency8Samples = false;
        features.features.sparseResidency16Samples = false;
        features.features.sparseResidencyAliased = false;
        features.features.variableMultisampleRate = false;
        features.features.inheritedQueries = false;

        VkDeviceCreateInfo deviceCreateInfo;
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = &features;
        deviceCreateInfo.flags = 0;
        deviceCreateInfo.queueCreateInfoCount = 2 - g.unifiedQueue;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
        deviceCreateInfo.enabledLayerCount = 0;
        deviceCreateInfo.enabledExtensionCount = 1;
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
        deviceCreateInfo.pEnabledFeatures = NULL;
        arVkCheck(g.vkCreateDevice(g.gpu, &deviceCreateInfo, NULL, &g.device));
        arLoadDeviceFunctions();

        g.vkGetDeviceQueue(g.device, g.graphicsQueueFamily, 0, &g.graphicsQueue);
        g.vkGetDeviceQueue(g.device, g.presentQueueFamily,  0, &g.presentQueue);
    }
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.pNext = NULL;
        commandPoolCreateInfo.flags = 0;
        commandPoolCreateInfo.queueFamilyIndex = g.graphicsQueueFamily;
        arVkCheck(g.vkCreateCommandPool(g.device, &commandPoolCreateInfo, NULL, &g.transferCommandPool));

        VkCommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.pNext = NULL;
        commandBufferAllocateInfo.commandPool = g.transferCommandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;
        arVkCheck(g.vkAllocateCommandBuffers(g.device, &commandBufferAllocateInfo, &g.transferCommandBuffer));
    }
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo;
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = NULL;
        semaphoreCreateInfo.flags = 0;

        if (!g.unifiedQueue)
        arVkCheck(g.vkCreateSemaphore(g.device, &semaphoreCreateInfo, NULL, &g.preSemaphore.semaphore));
        arVkCheck(g.vkCreateSemaphore(g.device, &semaphoreCreateInfo, NULL, &g.acqSemaphore.semaphore));
        arVkCheck(g.vkCreateSemaphore(g.device, &semaphoreCreateInfo, NULL, &g.renSemaphore.semaphore));

        VkFenceCreateInfo fenceCreateInfo;
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = NULL;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        arVkCheck(g.vkCreateFence(g.device, &fenceCreateInfo, NULL, &g.fence));
    }
    {
        VkDescriptorPoolSize poolSizes[1];
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[0].descriptorCount = 500000;

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.pNext = NULL;
        descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.poolSizeCount = 1;
        descriptorPoolCreateInfo.pPoolSizes = poolSizes;
        arVkCheck(g.vkCreateDescriptorPool(g.device, &descriptorPoolCreateInfo, NULL, &g.descriptorPool));

        VkDescriptorBindingFlags bindingFlags[1];
        bindingFlags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

        VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorSetLayoutBindingFlags;
        descriptorSetLayoutBindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        descriptorSetLayoutBindingFlags.pNext = NULL;
        descriptorSetLayoutBindingFlags.bindingCount = 1;
        descriptorSetLayoutBindingFlags.pBindingFlags = bindingFlags;

        VkDescriptorSetLayoutBinding bindings[1];
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount = 500000;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[0].pImmutableSamplers = NULL;

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlags;
        descriptorSetLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        descriptorSetLayoutCreateInfo.bindingCount = 1;
        descriptorSetLayoutCreateInfo.pBindings = bindings;
        arVkCheck(g.vkCreateDescriptorSetLayout(g.device, &descriptorSetLayoutCreateInfo, NULL, &g.descriptorSetLayout));

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.pNext = NULL;
        descriptorSetAllocateInfo.descriptorPool = g.descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = &g.descriptorSetLayout;
        arVkCheck(g.vkAllocateDescriptorSets(g.device, &descriptorSetAllocateInfo, &g.descriptorSet));
    }
    {
        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = 128;

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pNext = NULL;
        pipelineLayoutCreateInfo.flags = 0;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &g.descriptorSetLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
        arVkCheck(g.vkCreatePipelineLayout(g.device, &pipelineLayoutCreateInfo, NULL, &g.pipelineLayout));
    }
    {
        VkSamplerCreateInfo samplerCreateInfo;
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.pNext = NULL;
        samplerCreateInfo.flags = 0;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.anisotropyEnable = false;
        samplerCreateInfo.maxAnisotropy = 0.0f;
        samplerCreateInfo.compareEnable = false;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
        samplerCreateInfo.unnormalizedCoordinates = false;
        arVkCheck(g.vkCreateSampler(g.device, &samplerCreateInfo, NULL, &g.samplerLinearToEdge));

        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        arVkCheck(g.vkCreateSampler(g.device, &samplerCreateInfo, NULL, &g.samplerLinearRepeat));

        samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        arVkCheck(g.vkCreateSampler(g.device, &samplerCreateInfo, NULL, &g.samplerNearestRepeat));

        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        arVkCheck(g.vkCreateSampler(g.device, &samplerCreateInfo, NULL, &g.samplerNearestToEdge));
    }
    {
        arSwapchainCreate(g.vsyncEnabled);
    }
    {
        g.graphicsCommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        g.graphicsCommandBufferInfo.pNext = NULL;
        g.graphicsCommandBufferInfo.deviceMask = 0;

        g.presentCommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        g.presentCommandBufferInfo.pNext = NULL;
        g.presentCommandBufferInfo.deviceMask = 0;

        g.acqSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        g.acqSemaphore.pNext = NULL;
        g.acqSemaphore.value = 0;
        g.acqSemaphore.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        g.acqSemaphore.deviceIndex = 0;

        g.renSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        g.renSemaphore.pNext = NULL;
        g.renSemaphore.value = 0;
        g.renSemaphore.stageMask = g.unifiedQueue ? VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT : VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        g.renSemaphore.deviceIndex = 0;

        g.preSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        g.preSemaphore.pNext = NULL;
        g.preSemaphore.value = 0;
        g.preSemaphore.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        g.preSemaphore.deviceIndex = 0;
    }
}

internal void
arContextTeardown(void)
{
    arSwapchainTeardown();

    if (!g.unifiedQueue)
    {
        g.vkDestroySemaphore(g.device, g.preSemaphore.semaphore, NULL);
    }
    
    g.vkDestroyPipelineLayout(g.device, g.pipelineLayout, NULL);
    g.vkDestroySampler(g.device, g.samplerNearestRepeat, NULL);
    g.vkDestroySampler(g.device, g.samplerNearestToEdge, NULL);
    g.vkDestroySampler(g.device, g.samplerLinearRepeat, NULL);
    g.vkDestroySampler(g.device, g.samplerLinearToEdge, NULL);
    g.vkDestroyDescriptorSetLayout(g.device, g.descriptorSetLayout, NULL);
    g.vkDestroyDescriptorPool(g.device, g.descriptorPool, NULL);
    g.vkDestroyFence(g.device, g.fence, NULL);
    g.vkDestroySemaphore(g.device, g.renSemaphore.semaphore, NULL);
    g.vkDestroySemaphore(g.device, g.acqSemaphore.semaphore, NULL);
    g.vkDestroyCommandPool(g.device, g.transferCommandPool, NULL);
    g.vkDestroyDevice(g.device, NULL);
    g.vkDestroySurfaceKHR(g.instance, g.surface, NULL);
    g.vkDestroyInstance(g.instance, NULL);
}

internal void
arRecordCommands(void)
{
    arVkCheck(g.vkResetCommandPool(g.device, g.graphicsCommandPool, 0));

    for (uint32_t i = g.imageCount; i--; )
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo;
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = NULL;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = NULL;

        g.pFrame = &g.frames[i];

        arVkCheck(g.vkBeginCommandBuffer(g.pFrame->cmd, &commandBufferBeginInfo));
        g.vkCmdBindDescriptorSets(
            g.pFrame->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
            g.pipelineLayout, 0, 1, &g.descriptorSet, 0, NULL);
        g.pfnRecordCommands();
        arVkCheck(g.vkEndCommandBuffer(g.pFrame->cmd));
    }
}

internal void
arBeginTransfer(void)
{
    arVkCheck(g.vkResetCommandPool(g.device, g.transferCommandPool, 0));

    VkCommandBufferBeginInfo commandBufferBeginInfo;
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = NULL;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    commandBufferBeginInfo.pInheritanceInfo = NULL;
    arVkCheck(g.vkBeginCommandBuffer(g.transferCommandBuffer, &commandBufferBeginInfo));
}

internal void
arEndTransfer(void)
{
    arVkCheck(g.vkEndCommandBuffer(g.transferCommandBuffer));

    VkCommandBufferSubmitInfo commandBufferInfo;
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferInfo.pNext = NULL;
    commandBufferInfo.deviceMask = 0;
    commandBufferInfo.commandBuffer = g.transferCommandBuffer;

    VkSubmitInfo2 submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.pNext = NULL;
    submitInfo.flags = 0;
    submitInfo.waitSemaphoreInfoCount = 0;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferInfo;
    submitInfo.signalSemaphoreInfoCount = 0;
    arVkCheck(g.vkQueueSubmit2(g.graphicsQueue, 1, &submitInfo, NULL));
    arVkCheck(g.vkQueueWaitIdle(g.graphicsQueue));
}

internal uint32_t
arFindMemoryType(
    uint32_t typeBitsRequirement,
    VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    g.vkGetPhysicalDeviceMemoryProperties(g.gpu, &memoryProperties);

    for (uint32_t memoryIndex = 0;
         memoryIndex < memoryProperties.memoryTypeCount;
         ++memoryIndex)
    {
        if ((typeBitsRequirement & (1 << memoryIndex)) &&
            (memoryProperties.memoryTypes[memoryIndex].propertyFlags
            & properties) == properties)
        {
            return(memoryIndex);
        }
    }

    return(UINT32_MAX);
}

internal void
arAllocBuffer(
    ArBuffer* pBuffer,
    bool isDynamic)
{
    const VkBufferUsageFlags bufferUsage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    VkBufferCreateInfo bufferCreateInfo;
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = NULL;
    bufferCreateInfo.flags = 0;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.queueFamilyIndexCount = 0;
    bufferCreateInfo.size = pBuffer->size;
    bufferCreateInfo.usage = bufferUsage;
    arVkCheck(g.vkCreateBuffer(g.device, &bufferCreateInfo, NULL, (VkBuffer*)&pBuffer->handle.data[0]));

    VkMemoryRequirements memoryRequirements;
    g.vkGetBufferMemoryRequirements(g.device, pBuffer->handle.data[0], &memoryRequirements);

    VkMemoryPropertyFlags preferedFlags[2];
    preferedFlags[0] = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    preferedFlags[1] = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkMemoryPropertyFlags fallbackFlags[2];
    fallbackFlags[0] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    fallbackFlags[1] = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    uint32_t typeIndex = arFindMemoryType(
        memoryRequirements.memoryTypeBits,
        preferedFlags[isDynamic]);

    if (typeIndex == UINT32_MAX)
    {
        typeIndex = arFindMemoryType(
            memoryRequirements.memoryTypeBits,
            fallbackFlags[isDynamic]);
        
        if (typeIndex == UINT32_MAX)
        {
            arVkCheck(VK_ERROR_INITIALIZATION_FAILED);
        }
    }

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo;
    memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.pNext = NULL;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    memoryAllocateFlagsInfo.deviceMask = 0;

    VkMemoryAllocateInfo memoryAllocateInfo;
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.memoryTypeIndex = typeIndex;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    arVkCheck(g.vkAllocateMemory(g.device, &memoryAllocateInfo, NULL, (VkDeviceMemory*)&pBuffer->handle.data[1]));
    arVkCheck(g.vkBindBufferMemory(g.device, pBuffer->handle.data[0], pBuffer->handle.data[1], 0));

    VkBufferDeviceAddressInfo addressInfo;
    addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addressInfo.pNext = NULL;
    addressInfo.buffer = pBuffer->handle.data[0];
    pBuffer->address = g.vkGetBufferDeviceAddress(g.device, &addressInfo);
}

void
arCreateDynamicBuffer(
    ArBuffer* pBuffer,
    uint64_t capacity)
{
    pBuffer->size = capacity;
    arAllocBuffer(pBuffer, true);
    arVkCheck(g.vkMapMemory(g.device, pBuffer->handle.data[1], 0, capacity, 0, &pBuffer->pMapped));
}

void
arCreateStaticBuffer(
    ArBuffer* pBuffer,
    uint64_t size,
    void const* pData)
{
    pBuffer->size = size;
    arAllocBuffer(pBuffer, false);

    ArBuffer stagingBuffer;
    arCreateDynamicBuffer(&stagingBuffer, size);
    memcpy(stagingBuffer.pMapped, pData, size);

    arBeginTransfer();
    {
        VkBufferCopy region;
        region.srcOffset = 0;
        region.dstOffset = 0;
        region.size = size;

        g.vkCmdCopyBuffer(
            g.transferCommandBuffer,
            stagingBuffer.handle.data[0],
            pBuffer->handle.data[0],
            1,
            &region);
    }
    arEndTransfer();

    arDestroyBuffer(&stagingBuffer);
}

void
arDestroyBuffer(
    ArBuffer const* pBuffer)
{
    if (pBuffer->pMapped)
    {
        g.vkUnmapMemory(g.device, pBuffer->handle.data[1]);
    }

    g.vkFreeMemory(g.device, pBuffer->handle.data[1], NULL);
    g.vkDestroyBuffer(g.device, pBuffer->handle.data[0], NULL);
}

void
arCreateImage(
    ArImage* pImage,
    ArImageCreateInfo const* pImageCreateInfo)
{
    VkImageAspectFlags aspect = 0;
    VkImageUsageFlags usage = 0;
    VkFormat format = 0;

    switch (pImageCreateInfo->usage)
    {
    case AR_IMAGE_USAGE_COLOR_ATTACHMENT:
        usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        format = VK_FORMAT_B8G8R8A8_UNORM;
        break;
    case AR_IMAGE_USAGE_DEPTH_ATTACHMENT:
        usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
        format = VK_FORMAT_D32_SFLOAT;
        break;
    case AR_IMAGE_USAGE_TEXTURE:
        usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        format = (VkFormat)pImageCreateInfo->format;
        break;
    }

    if (pImageCreateInfo->sampler)
    {
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (!pImageCreateInfo->depth)
    {
        pImage->depth = 1;
    }
    else
    {
        pImage->depth = pImageCreateInfo->depth;
    }

    if (pImageCreateInfo->width && pImageCreateInfo->height)
    {
        pImage->width  = pImageCreateInfo->width;
        pImage->height = pImageCreateInfo->height;
    }
    else
    {
        pImage->width  = g.extent.width;
        pImage->height = g.extent.height;
    }

    VkImageCreateInfo imageCreateInfo;
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = NULL;
    imageCreateInfo.flags = 0;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent.width = pImage->width;
    imageCreateInfo.extent.height = pImage->height;
    imageCreateInfo.extent.depth = pImage->depth;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = usage;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    arVkCheck(g.vkCreateImage(g.device, &imageCreateInfo, NULL, (VkImage*)&pImage->handle.data[0]));

    VkMemoryRequirements memoryRequirements;
    g.vkGetImageMemoryRequirements(g.device, pImage->handle.data[0], &memoryRequirements);

    const VkMemoryPropertyFlags preferedFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    const VkMemoryPropertyFlags fallbackFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    uint32_t typeIndex = arFindMemoryType(memoryRequirements.memoryTypeBits, preferedFlags);

    if (typeIndex == UINT32_MAX)
    {
        typeIndex = arFindMemoryType(memoryRequirements.memoryTypeBits, fallbackFlags);

        if (typeIndex == UINT32_MAX)
        {
            arVkCheck(VK_ERROR_INITIALIZATION_FAILED);
        }
    }

    VkMemoryAllocateInfo memoryAllocateInfo;
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = NULL;
    memoryAllocateInfo.memoryTypeIndex = typeIndex;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    arVkCheck(g.vkAllocateMemory(g.device, &memoryAllocateInfo, NULL, (VkDeviceMemory*)&pImage->handle.data[1]));
    arVkCheck(g.vkBindImageMemory(g.device, pImage->handle.data[0], pImage->handle.data[1], 0));

    VkImageViewCreateInfo imageViewCreateInfo;
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = NULL;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.image = pImage->handle.data[0];
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask = aspect;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    arVkCheck(g.vkCreateImageView(g.device, &imageViewCreateInfo, NULL, (VkImageView*)&pImage->handle.data[2]));

    if (pImageCreateInfo->sampler)
    {
        VkDescriptorImageInfo descriptorImageInfo;
        descriptorImageInfo.sampler = (&g.samplerLinearToEdge)[pImageCreateInfo->sampler - 1];
        descriptorImageInfo.imageView = pImage->handle.data[2];
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet descriptorWrite;
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.pNext = NULL;
        descriptorWrite.dstSet = g.descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = pImageCreateInfo->dstArrayElement;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.pImageInfo = &descriptorImageInfo;
        descriptorWrite.pBufferInfo = NULL;
        descriptorWrite.pTexelBufferView = NULL;
        g.vkUpdateDescriptorSets(g.device, 1, &descriptorWrite, 0, NULL);
    }
}

void
arUpdateImage(
    ArImage* pImage,
    size_t dataSize,
    void const* pData)
{
    ArBuffer stagingBuffer;
    arCreateDynamicBuffer(&stagingBuffer, dataSize);
    memcpy(stagingBuffer.pMapped, pData, dataSize);

    arBeginTransfer();
    {
        VkImageMemoryBarrier2 imageMemoryBarrier;
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        imageMemoryBarrier.pNext = NULL;
        imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_NONE;
        imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = pImage->handle.data[0];
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;

        VkDependencyInfo dependencyInfo;
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = NULL;
        dependencyInfo.dependencyFlags = 0;
        dependencyInfo.memoryBarrierCount = 0;
        dependencyInfo.bufferMemoryBarrierCount = 0;
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;
        g.vkCmdPipelineBarrier2(g.transferCommandBuffer, &dependencyInfo);

        VkBufferImageCopy region;
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset.x = 0;
        region.imageOffset.y = 0;
        region.imageOffset.z = 0;
        region.imageExtent.width  = pImage->width;
        region.imageExtent.height = pImage->height;
        region.imageExtent.depth  = pImage->depth;
        g.vkCmdCopyBufferToImage(
            g.transferCommandBuffer,
            stagingBuffer.handle.data[0],
            pImage->handle.data[0],
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region);

        imageMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
        imageMemoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        g.vkCmdPipelineBarrier2(g.transferCommandBuffer, &dependencyInfo);
    }
    arEndTransfer();

    arDestroyBuffer(&stagingBuffer);
}

void
arDestroyImage(
    ArImage const* pImage)
{
    g.vkDestroyImageView(g.device, pImage->handle.data[2], NULL);
    g.vkFreeMemory(g.device, pImage->handle.data[1], NULL);
    g.vkDestroyImage(g.device, pImage->handle.data[0], NULL);
}

void
arCreateShaderFromFile(
    ArShader* pShader,
    char const* filename)
{
    LARGE_INTEGER fileSize;
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE ||
        !GetFileSizeEx(file, &fileSize))
    {
        arError("Failed to create file");
    }

    uint32_t* pBuffer = HeapAlloc(GetProcessHeap(), 0, fileSize.QuadPart);
    DWORD bytesRead;

    if (!pBuffer)
    {
        arError("Failed to allocate memory");
    }

    if (!ReadFile(file, pBuffer, fileSize.LowPart, &bytesRead, NULL))
    {
        arError("Failed to read shader file");
    }

    CloseHandle(file);
    arCreateShaderFromMemory(pShader, pBuffer, fileSize.QuadPart);
    HeapFree(GetProcessHeap(), 0, pBuffer);
}

void
arCreateShaderFromMemory(
    ArShader* pShader,
    uint32_t const* pCode,
    size_t codeSize)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo;
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = NULL;
    shaderModuleCreateInfo.flags = 0;
    shaderModuleCreateInfo.codeSize = codeSize;
    shaderModuleCreateInfo.pCode = pCode;
    arVkCheck(g.vkCreateShaderModule(g.device, &shaderModuleCreateInfo, NULL, (VkShaderModule*)pShader));
}

void
arDestroyShader(
    ArShader const* pShader)
{
    g.vkDestroyShaderModule(g.device, pShader->handle.data, NULL);
}

void
arCreateGraphicsPipeline(
    ArPipeline* pPipeline,
    ArGraphicsPipelineCreateInfo const* pPipelineCreateInfo)
{
    VkFormat colorFormats[8];
    colorFormats[0] = VK_FORMAT_B8G8R8A8_UNORM;
    colorFormats[1] = VK_FORMAT_B8G8R8A8_UNORM;
    colorFormats[2] = VK_FORMAT_B8G8R8A8_UNORM;
    colorFormats[3] = VK_FORMAT_B8G8R8A8_UNORM;
    colorFormats[4] = VK_FORMAT_B8G8R8A8_UNORM;
    colorFormats[5] = VK_FORMAT_B8G8R8A8_UNORM;
    colorFormats[6] = VK_FORMAT_B8G8R8A8_UNORM;
    colorFormats[7] = VK_FORMAT_B8G8R8A8_UNORM;

    VkPipelineColorBlendAttachmentState blendAttachments[8];
    for (uint32_t i = pPipelineCreateInfo->blendAttachmentCount; i--; )
    {
        blendAttachments[i].blendEnable         = pPipelineCreateInfo->pBlendAttachments[i].blendEnable;
        blendAttachments[i].srcColorBlendFactor = (VkBlendFactor)pPipelineCreateInfo->pBlendAttachments[i].srcColorFactor;
        blendAttachments[i].dstColorBlendFactor = (VkBlendFactor)pPipelineCreateInfo->pBlendAttachments[i].dstColorFactor;
        blendAttachments[i].colorBlendOp        = (VkBlendOp)pPipelineCreateInfo->pBlendAttachments[i].colorBlendOp;
        blendAttachments[i].srcAlphaBlendFactor = (VkBlendFactor)pPipelineCreateInfo->pBlendAttachments[i].srcAlphaFactor;
        blendAttachments[i].dstAlphaBlendFactor = (VkBlendFactor)pPipelineCreateInfo->pBlendAttachments[i].dstAlphaFactor;
        blendAttachments[i].alphaBlendOp        = (VkBlendOp)pPipelineCreateInfo->pBlendAttachments[i].alphaBlendOp;
        blendAttachments[i].colorWriteMask      = pPipelineCreateInfo->pBlendAttachments[i].colorWriteMask;
    }

    VkPipelineRenderingCreateInfo renderingCreateInfo;
    renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.pNext = NULL;
    renderingCreateInfo.viewMask = 0;
    renderingCreateInfo.colorAttachmentCount = pPipelineCreateInfo->blendAttachmentCount;
    renderingCreateInfo.pColorAttachmentFormats = colorFormats;
    renderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].pNext = NULL;
    shaderStages[0].flags = 0;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = pPipelineCreateInfo->vertShader.handle.data;
    shaderStages[0].pName = "main";
    shaderStages[0].pSpecializationInfo = NULL;

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].pNext = NULL;
    shaderStages[1].flags = 0;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = pPipelineCreateInfo->fragShader.handle.data;
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
    inputAssemblyState.topology = (VkPrimitiveTopology)pPipelineCreateInfo->topology;

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
    rasterizationState.polygonMode = (VkPolygonMode)pPipelineCreateInfo->polygonMode;
    rasterizationState.cullMode = pPipelineCreateInfo->cullMode;
    rasterizationState.frontFace = (VkFrontFace)pPipelineCreateInfo->frontFace;
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
    depthStencilState.depthWriteEnable = pPipelineCreateInfo->depthState.depthWriteEnable;
    depthStencilState.depthTestEnable = pPipelineCreateInfo->depthState.depthTestEnable;
    depthStencilState.depthCompareOp = (VkCompareOp)pPipelineCreateInfo->depthState.compareOp;
    depthStencilState.front.failOp = VK_STENCIL_OP_KEEP;
    depthStencilState.front.passOp = VK_STENCIL_OP_KEEP;
    depthStencilState.front.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencilState.front.compareOp = VK_COMPARE_OP_NEVER;
    depthStencilState.front.compareMask = 0;
    depthStencilState.front.writeMask = 0;
    depthStencilState.front.reference = 0;
    depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.compareOp = VK_COMPARE_OP_NEVER;
    depthStencilState.back.compareMask = 0;
    depthStencilState.back.writeMask = 0;
    depthStencilState.back.reference = 0;

    VkPipelineColorBlendStateCreateInfo colorBlendState;
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.pNext = NULL;
    colorBlendState.flags = 0;
    colorBlendState.logicOpEnable = false;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = pPipelineCreateInfo->blendAttachmentCount;
    colorBlendState.pAttachments = blendAttachments;

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
    arVkCheck(g.vkCreateGraphicsPipelines(g.device, NULL, 1, &pipelineCreateInfo, NULL, (VkPipeline*)pPipeline));
}

void
arDestroyPipeline(
    ArPipeline const* pPipeline)
{
    g.vkDestroyPipeline(g.device, pPipeline->handle.data, NULL);
}

void
arCmdBeginRendering(
    uint32_t colorAttachmentCount,
    ArAttachment const* pColorAttachments,
    ArAttachment const* pDepthAttachment)
{
    VkRenderingAttachmentInfo attachments[8];
    VkRenderingAttachmentInfo depthAttachment;

    if (pDepthAttachment)
    {
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.pNext = NULL;
        depthAttachment.imageView = pDepthAttachment->pImage->handle.data[2];
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
        depthAttachment.resolveImageView = NULL;
        depthAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.loadOp = (VkAttachmentLoadOp)pDepthAttachment->loadOp;
        depthAttachment.storeOp = (VkAttachmentStoreOp)pDepthAttachment->storeOp;
        depthAttachment.clearValue.depthStencil.depth = pDepthAttachment->clearValue.depth;
        depthAttachment.clearValue.depthStencil.stencil = 0;
    }

    for (uint32_t i = 0; i < colorAttachmentCount; ++i)
    {
        if (!pColorAttachments[i].pImage)
        {
            attachments[i].imageView = g.pFrame->view;
        }
        else
        {
            attachments[i].imageView = pColorAttachments[i].pImage->handle.data[2];
        }

        attachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachments[i].pNext = NULL;
        attachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachments[i].resolveMode = VK_RESOLVE_MODE_NONE;
        attachments[i].resolveImageView = NULL;
        attachments[i].resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[i].loadOp = (VkAttachmentLoadOp)pColorAttachments[i].loadOp;
        attachments[i].storeOp = (VkAttachmentStoreOp)pColorAttachments[i].storeOp;
        attachments[i].clearValue.color.int32[0] = pColorAttachments[i].clearValue.color.int32[0];
        attachments[i].clearValue.color.int32[1] = pColorAttachments[i].clearValue.color.int32[1];
        attachments[i].clearValue.color.int32[2] = pColorAttachments[i].clearValue.color.int32[2];
        attachments[i].clearValue.color.int32[3] = pColorAttachments[i].clearValue.color.int32[3];
    }

    VkExtent2D extent;
    if (pColorAttachments->pImage)
    {
        extent.width  = pColorAttachments->pImage->width;
        extent.height = pColorAttachments->pImage->height;
    }
    else
    {
        extent.width  = g.extent.width;
        extent.height = g.extent.height;
    }

    VkRenderingInfo renderingInfo;
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.pNext = NULL;
    renderingInfo.flags = 0;
    renderingInfo.renderArea.offset.x = 0;
    renderingInfo.renderArea.offset.y = 0;
    renderingInfo.renderArea.extent.width  = extent.width;
    renderingInfo.renderArea.extent.height = extent.height;
    renderingInfo.layerCount = 1;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = colorAttachmentCount;
    renderingInfo.pColorAttachments = attachments;
    renderingInfo.pDepthAttachment = pDepthAttachment ? &depthAttachment : NULL;
    renderingInfo.pStencilAttachment = NULL;
    g.vkCmdBeginRendering(g.pFrame->cmd, &renderingInfo);

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width  = extent.width;
    scissor.extent.height = extent.height;
    g.vkCmdSetScissor(g.pFrame->cmd, 0, 1, &scissor);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width  = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    g.vkCmdSetViewport(g.pFrame->cmd, 0, 1, &viewport);
}

void
arCmdEndRendering(void)
{
    g.vkCmdEndRendering(g.pFrame->cmd);
}

void
arCmdPushConstants(
    uint32_t offset,
    uint32_t size,
    void const* pValues)
{
    g.vkCmdPushConstants(
        g.pFrame->cmd,
        g.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        offset,
        size,
        pValues);
}

void
arCmdBindIndexBuffer(
    ArBuffer const* pBuffer,
    uint64_t offset,
    ArIndexType indexType)
{
    g.vkCmdBindIndexBuffer(
        g.pFrame->cmd,
        *pBuffer->handle.data,
        offset,
        (VkIndexType)indexType);
}

void
arCmdBindGraphicsPipeline(
    ArPipeline const* pPipeline)
{
    g.vkCmdBindPipeline(
        g.pFrame->cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pPipeline->handle.data);
}

internal VkPipelineStageFlags2
arImageLayoutToPipelineStage(
    ArImageLayout layout)
{
    switch (layout)
    {
    case AR_IMAGE_LAYOUT_COLOR_ATTACHMENT:
        return(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT); 
    case AR_IMAGE_LAYOUT_DEPTH_ATTACHMENT:
        return(VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
               VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT);
    case AR_IMAGE_LAYOUT_SHADER_READ:
        return(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    case AR_IMAGE_LAYOUT_TRANSFER_SRC:
    case AR_IMAGE_LAYOUT_TRANSFER_DST:
        return(VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    case AR_IMAGE_LAYOUT_PRESENT_SRC:
        return(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
    default:
        return(VK_PIPELINE_STAGE_2_NONE);
    }
}

internal VkAccessFlags2
arImageLayoutToAccess(
    ArImageLayout layout)
{
    switch (layout)
    {
    case AR_IMAGE_LAYOUT_COLOR_ATTACHMENT:
        return(VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT); 
    case AR_IMAGE_LAYOUT_DEPTH_ATTACHMENT:
        return(VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    case AR_IMAGE_LAYOUT_SHADER_READ:
        return(VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    case AR_IMAGE_LAYOUT_TRANSFER_SRC:
        return(VK_ACCESS_2_TRANSFER_READ_BIT);
    case AR_IMAGE_LAYOUT_TRANSFER_DST:
        return(VK_ACCESS_2_TRANSFER_WRITE_BIT);
    default:
        return(VK_ACCESS_2_NONE);
    }
}

void
arCmdPipelineBarrier(
    uint32_t barrierCount,
    ArBarrier const* pBarriers)
{
    VkImageMemoryBarrier2 imageMemoryBarriers[8];

    for (uint32_t i = 0; i < barrierCount; ++i)
    {
        imageMemoryBarriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        imageMemoryBarriers[i].pNext = NULL;
        imageMemoryBarriers[i].srcStageMask = arImageLayoutToPipelineStage(pBarriers[i].oldLayout);
        imageMemoryBarriers[i].srcAccessMask = arImageLayoutToAccess(pBarriers[i].oldLayout);
        imageMemoryBarriers[i].dstStageMask = arImageLayoutToPipelineStage(pBarriers[i].newLayout);
        imageMemoryBarriers[i].dstAccessMask = arImageLayoutToAccess(pBarriers[i].newLayout);
        imageMemoryBarriers[i].oldLayout = (VkImageLayout)pBarriers[i].oldLayout;
        imageMemoryBarriers[i].newLayout = (VkImageLayout)pBarriers[i].newLayout;
        imageMemoryBarriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarriers[i].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarriers[i].subresourceRange.baseArrayLayer = 0;
        imageMemoryBarriers[i].subresourceRange.baseMipLevel = 0;
        imageMemoryBarriers[i].subresourceRange.layerCount = 1;
        imageMemoryBarriers[i].subresourceRange.levelCount = 1;

        if (pBarriers[i].pImage)
        {
            imageMemoryBarriers[i].image = *pBarriers[i].pImage->handle.data;
        }
        else
        {
            imageMemoryBarriers[i].srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            imageMemoryBarriers[i].image = g.pFrame->image;
        }

        if (pBarriers[i].newLayout == AR_IMAGE_LAYOUT_PRESENT_SRC)
        {
            imageMemoryBarriers[i].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            
            if (!g.unifiedQueue)
            {
                imageMemoryBarriers[i].dstStageMask = VK_PIPELINE_STAGE_2_NONE;
                imageMemoryBarriers[i].srcQueueFamilyIndex = g.graphicsQueueFamily;
                imageMemoryBarriers[i].dstQueueFamilyIndex = g.presentQueueFamily;
            }
        }
        else if (pBarriers[i].oldLayout == AR_IMAGE_LAYOUT_DEPTH_ATTACHMENT ||
                 pBarriers[i].newLayout == AR_IMAGE_LAYOUT_DEPTH_ATTACHMENT)
        {
            imageMemoryBarriers[i].subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
    }

    VkDependencyInfo dependencyInfo;
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = NULL;
    dependencyInfo.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencyInfo.memoryBarrierCount = 0;
    dependencyInfo.bufferMemoryBarrierCount = 0;
    dependencyInfo.imageMemoryBarrierCount = barrierCount;
    dependencyInfo.pImageMemoryBarriers = imageMemoryBarriers;
    g.vkCmdPipelineBarrier2(g.pFrame->cmd, &dependencyInfo);
}

void
arCmdDraw(
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance)
{
    g.vkCmdDraw(
        g.pFrame->cmd,
        vertexCount,
        instanceCount,
        firstVertex,
        firstInstance);
}

void
arCmdDrawIndirect(
    ArBuffer const* pBuffer,
    uint64_t offset,
    uint32_t drawCount,
    uint32_t stride)
{
    g.vkCmdDrawIndirect(
        g.pFrame->cmd,
        *pBuffer->handle.data,
        offset,
        drawCount,
        stride);
}

void
arCmdDrawIndirectCount(
    ArBuffer const* pBuffer,
    uint64_t offset,
    ArBuffer const* pCountBuffer,
    uint64_t countBufferOffset,
    uint32_t maxDrawCount,
    uint32_t stride)
{
    g.vkCmdDrawIndirectCount(
        g.pFrame->cmd,
        *pBuffer->handle.data,
        offset,
        *pCountBuffer->handle.data,
        countBufferOffset,
        maxDrawCount,
        stride);
}

void
arCmdDrawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance)
{
    g.vkCmdDrawIndexed(
        g.pFrame->cmd,
        indexCount,
        instanceCount,
        firstIndex,
        vertexOffset,
        firstInstance);
}

void
arCmdDrawIndexedIndirect(
    ArBuffer const* pBuffer,
    uint64_t offset,
    uint32_t drawCount,
    uint32_t stride)
{
    g.vkCmdDrawIndexedIndirect(
        g.pFrame->cmd,
        *pBuffer->handle.data,
        offset,
        drawCount,
        stride);
}

void
arCmdDrawIndexedIndirectCount(
    ArBuffer const* pBuffer,
    uint64_t offset,
    ArBuffer const* pCountBuffer,
    uint64_t countBufferOffset,
    uint32_t maxDrawCount,
    uint32_t stride)
{
    g.vkCmdDrawIndexedIndirectCount(
        g.pFrame->cmd,
        *pBuffer->handle.data,
        offset,
        *pCountBuffer->handle.data,
        countBufferOffset,
        maxDrawCount,
        stride);
}

void
arSetWindowTitle(
    char const* title)
{
    SetWindowTextA(
        g.hwnd,
        title);
}

void
arPollEvents(void)
{
    for (ArKeyInternal* key = g.keys; key != g.keys + 260; key += 1)
    {
        key->isPressed  = false;
        key->isReleased = false;
    }

    g.cursorRelX = 0;
    g.cursorRelY = 0;

    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    double now = arGetTime();
    g.deltaTime = now - g.previousTime;
    g.previousTime = now;

    POINT cursorPos;
    GetPhysicalCursorPos(&cursorPos);

    g.cursorDeltaX = cursorPos.x - g.globalCursorX;
    g.cursorDeltaY = cursorPos.y - g.globalCursorY;

    g.globalCursorX = cursorPos.x;
    g.globalCursorY = cursorPos.y;

    ScreenToClient(g.hwnd, &cursorPos);

    g.cursorX = cursorPos.x;
    g.cursorY = cursorPos.y;
}

void
arWaitEvents(void)
{
    WaitMessage();
    arPollEvents();
}

void
arShowCursor(void)
{
    while (ShowCursor(true) < 0);
}

void
arHideCursor(void)
{
    while (ShowCursor(false) >= 0);
}

void
arSetCursorPosition(
    int x,
    int y)
{
    SetCursorPos(x, y);
}

bool
arIsKeyDown(
    ArKey key)
{
    return(g.keys[key].isDown);
}

bool
arIsKeyPressed(
    ArKey key)
{
    return(g.keys[key].isPressed);
}

bool
arIsKeyReleased(
    ArKey key)
{
    return(g.keys[key].isReleased);
}

bool
arIsButtonDown(
    ArButton button)
{
    return(g.buttons[button].isDown);
}

bool
arIsButtonPressed(
    ArButton button)
{
    return(g.buttons[button].isPressed);
}

bool
arIsButtonReleased(
    ArButton button)
{
    return(g.buttons[button].isReleased);
}

double
arGetTime(void)
{
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);

    return((double)
        (value.QuadPart - g.timeOffset.QuadPart)
        / g.timeFrequency.QuadPart);
}

double
arGetDeltaTime(void)
{
    return(g.deltaTime);
}

float
arGetTimef(void)
{
    return((float)arGetTime());
}

float
arGetDeltaTimef(void)
{
    return((float)g.deltaTime);
}

uint32_t
arGetWindowWidth(void)
{
    return((uint32_t)g.width);
}

uint32_t
arGetWindowHeight(void)
{
    return((uint32_t)g.height);
}

uint32_t
arGetRenderWidth(void)
{
    return(g.extent.width);
}

uint32_t
arGetRenderHeight(void)
{
    return(g.extent.height);
}

int32_t
arGetWindowPositionX(void)
{
    RECT point;
    GetWindowRect(g.hwnd, &point);
    return(point.left);
}

int32_t
arGetWindowPositionY(void)
{
    RECT point;
    GetWindowRect(g.hwnd, &point);
    return(point.top);
}

float
arGetWindowAspectRatio(void)
{
    return(g.width / (float)g.height);
}

float
arGetRenderAspectRatio(void)
{
    return(g.extent.width / (float)g.extent.height);
}

int
arGetGlobalCursorX(void)
{
    return(g.globalCursorX);
}

int
arGetGlobalCursorY(void)
{
    return(g.globalCursorY);
}

int
arGetCursorX(void)
{
    return(g.cursorX);
}

int
arGetCursorY(void)
{
    return(g.cursorY);
}

int
arGetCursorDeltaX(void)
{
    return(g.cursorDeltaX);
}

int
arGetCursorDeltaY(void)
{
    return(g.cursorDeltaY);
}

int
arGetRelativeCursorX(void)
{
    return(g.cursorRelX);
}

int
arGetRelativeCursorY(void)
{
    return(g.cursorRelY);
}

void
arExecute(
    ArApplicationInfo const* pApplicationInfo)
{
    g.pfnUpdate = pApplicationInfo->pfnUpdate;
    g.pfnResize = pApplicationInfo->pfnResize;
    g.pfnRecordCommands = pApplicationInfo->pfnRecordCommands;
    g.vsyncEnabled = pApplicationInfo->enableVsync;
    arWindowCreate(pApplicationInfo->width, pApplicationInfo->height);
    arContextCreate();

    pApplicationInfo->pfnInit();
    arRecordCommands(); 

    for (;;)
    {
        pApplicationInfo->pfnUpdate();

        if (g.windowShouldClose)
        {
            break;
        }

        if (g.vkWaitForFences(g.device, 1, &g.fence, 0, UINT64_MAX) |
            g.vkResetFences(g.device, 1, &g.fence))
        {
            arError("Failed to sync");
        }

        switch (pApplicationInfo->pfnUpdateResources())
        {
        case AR_REQUEST_NONE:
            break;
        case AR_REQUEST_RECORD_COMMANDS:
            arRecordCommands();
            break;
        case AR_REQUEST_VSYNC_DISABLE:
            arSwapchainRecreate(false);
            break;
        case AR_REQUEST_VSYNC_ENABLE:
            arSwapchainRecreate(true);
            break;
        }

        if (g.vkAcquireNextImageKHR(g.device, g.swapchain, UINT64_MAX, g.acqSemaphore.semaphore, NULL, &g.imageIndex))
        {
            arError("Failed to acquire image");
        }

        g.graphicsCommandBufferInfo.commandBuffer = g.frames[g.imageIndex].cmd;
        if (g.vkQueueSubmit2(g.graphicsQueue, 1, &g.submitInfo, g.fence))
        {
            arError("Failed to submit commands");
        }

        if (!g.unifiedQueue)
        {
            g.presentCommandBufferInfo.commandBuffer = g.frames[g.imageIndex].presentCmd;
            
            if (g.vkQueueSubmit2(g.presentQueue, 1, &g.presentSubmitInfo, NULL))
            {
                arError("Failed to submit commands");
            }
        }

        if (g.vkQueuePresentKHR(g.presentQueue, &g.presentInfo))
        {
            arError("Failed to present frame");
        }
    }

    g.vkDeviceWaitIdle(g.device);
    pApplicationInfo->pfnTeardown();
    arContextTeardown();
    arWindowTeardown();
}
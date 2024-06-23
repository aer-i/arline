#include "Arline.hxx"

#define VMA_IMPLEMENTATION

#ifdef _WIN32
#   define VK_USE_PLATFORM_WIN32_KHR
#   include <dwmapi.h>
#   pragma comment(lib, "Dwmapi")
#endif

#include <vulkan/vulkan.h>

#pragma warning(push, 0)
#include "vma.hxx"
#pragma warning(pop)

using namespace arline::types;

#pragma region Declarations

template<class T>
static constexpr auto g_minImageCount{ static_cast<T>(3u) };

template<class T>
static constexpr auto g_maxImageCount{ g_minImageCount<T> * static_cast<T>(2) }; 

struct ArlineContext
{
    #ifdef AR_ENABLE_INFO_CALLBACK
    union DebugMessenger
    {
        VkDebugUtilsMessengerEXT handle;
        u8_t enableValidationLayers;
    };
    #endif

    struct SwapchainImage
    {
        VkImage image;
        VkImageView view;
        VkCommandBuffer graphicsCommandBuffer;
        VkCommandBuffer presentCommandBuffer;
    };

    #ifdef AR_ENABLE_INFO_CALLBACK
    DebugMessenger messenger;
    #endif

    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    u32_t graphicsFamily;
    u32_t presentFamily;
    VkCommandPool graphicsCommandPool;
    VkCommandPool presentCommandPool;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D surfaceExtent;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipelineLayout;
    VkSampler linearToEdgeSampler;
    VkSampler linearRepeatSampler;
    VkSampler nearestToEdgeSampler;
    VkSampler nearestRepeatSampler;
    VkCommandPool transferCommandPool;
    VkCommandBuffer transferCommandBuffer;
    VmaAllocator allocator;
    u32_t allocatedSampledImageCount;

    u32_t imageCount;
    u32_t unifiedPresent;
    u32_t imageIndex;
    VkFence fence;
    VkSemaphore acquireSemaphore;
    VkSemaphore graphicsSemaphore;
    VkSemaphore presentSemaphore;
    VkDevice device;
    VkSwapchainKHR swapchain;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSubmitInfo submitInfo;
    VkPresentInfoKHR presentInfo;

    SwapchainImage images[g_maxImageCount<u32_t>];
};

struct ArlineWindow
{
    HWND hwnd;
    HINSTANCE hinstance;
    i32_t width;
    i32_t height;
    b8_t available;
};

struct ArlineTimer
{
    LARGE_INTEGER timeOffset;
    LARGE_INTEGER frequency;
    f64_t previousTime;
    f64_t deltaTime;
};

struct ArlineKey
{
    b8_t isPressed  : 1;
    b8_t isReleased : 1;
};

struct ArlineKeyboard
{
    ArlineKey keys[static_cast<u8_t>(ar::Key::eMaxEnum)];
    b8_t keysDown[static_cast<u8_t>(ar::Key::eMaxEnum)];
};

struct ArlineButton
{
    b8_t isDown     : 1;
    b8_t isPressed  : 1;
    b8_t isReleased : 1;
};

struct ArlineMouse
{
    i32_t globPosX, globPosY;
    i32_t posX, posY;
    ArlineButton buttons[5];
};

#ifdef AR_ENABLE_INFO_CALLBACK
static callback_t     g_infoCallback;
#endif
static callback_t     g_errorCallback;
static ArlineContext  g_ctx;
static ArlineWindow   g_wnd;
static ArlineTimer    g_timer;
static ArlineKeyboard g_keyboard;
static ArlineMouse    g_mouse;

#ifdef AR_ENABLE_INFO_CALLBACK
#define AR_INFO_CALLBACK(f, ...) \
{ char msg[1024]; std::snprintf(msg, sizeof(msg), f, __VA_ARGS__); g_infoCallback(msg); }
#else
#define AR_INFO_CALLBACK(format, ...)
#endif

namespace arTimer
{
    static auto create() noexcept -> void;
    static auto update() noexcept -> void;
}

namespace arWindow
{
    static auto create(ar::EngineConfig const&) noexcept -> void;
    static auto teardown() noexcept -> void;
}

namespace arContext
{
    static auto create() noexcept -> void;
    static auto teardown() noexcept -> void;
    static auto acquireImage() noexcept -> b8_t;
    static auto presentFrame() noexcept -> b8_t;
    static auto createSwapchain() noexcept -> void;
    static auto teardownSwapchain() noexcept -> void;
    static auto resultCheck(VkResult result) noexcept -> void;
    static auto beginTransfer() noexcept -> void;
    static auto endTransfer() noexcept -> void;
    static auto createImageView(
        VkImage image,
        VkImageViewType type,
        VkFormat format,
        VkImageAspectFlags aspect,
        u32_t mipLevels
    ) noexcept -> VkImageView;
}

#pragma endregion
#pragma region Engine

arline::Engine::Engine(EngineConfig const& config) noexcept
{
    g_ctx = ArlineContext{
        #ifdef AR_ENABLE_INFO_CALLBACK
        .messenger = {
            .enableValidationLayers = config.enableValidationLayers
        },
        #endif
        .submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1u,
            .commandBufferCount = 1u,
            .signalSemaphoreCount = 1u
        }
    };

    #ifdef AR_ENABLE_INFO_CALLBACK
    g_infoCallback = config.infoCallback;
    #endif
    g_errorCallback = config.errorCallback;

    arWindow::create(config);
    arContext::create();
    arTimer::create();
}

arline::Engine::~Engine() noexcept
{
    arContext::teardown();
    arWindow::teardown();
}

auto arline::Engine::execute() noexcept -> int
{
    auto recordCommands{ [this]
    {
        for (auto commands{ GraphicsCommands{} }; commands; )
        {
            this->onCommandsRecord(commands);
        }
    }};
    
    recordCommands();

    while (true)
    {
        if (!arContext::acquireImage()) [[unlikely]]
        {
            this->onResize();
            recordCommands();
        }

        if (this->onResourcesUpdate()) [[unlikely]]
        {
            recordCommands();
        }

        if (!arContext::presentFrame()) [[unlikely]]
        {
            this->onResize();
            recordCommands();
        }

        this->onUpdate();
        arTimer::update();

        if (!g_wnd.available) [[unlikely]]
            break;
    }

    vkDeviceWaitIdle(g_ctx.device);

    return {};
}

#pragma endregion
#pragma region Window Win32
#ifdef _WIN32

static auto arWindow::create(ar::EngineConfig const& config) noexcept -> void
{
    auto error{ "Window creation failed" };

    g_wnd = ArlineWindow{
        .hinstance = ::GetModuleHandleA(nullptr),
        .width = config.width,
        .height = config.height,
        .available = true
    };

    auto const wc{ WNDCLASSEXW{
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_OWNDC,
        .lpfnWndProc = { 
            [](HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
            {
                switch (msg)
                {
                [[unlikely]] case WM_SIZE:
                    g_wnd.width = LOWORD(lp);
                    g_wnd.height = HIWORD(lp);

                    while (!g_wnd.width || !g_wnd.height)
                    {
                        ar::Window::WaitEvents();

                        if (!g_wnd.available)
                        {
                            break;
                        }
                    }
                    break;
                [[unlikely]] case WM_GETMINMAXINFO:
                    reinterpret_cast<PMINMAXINFO>(lp)->ptMinTrackSize.x = 300;
                    reinterpret_cast<PMINMAXINFO>(lp)->ptMinTrackSize.y = 150;
                    return LRESULT{};
                case WM_LBUTTONDOWN:
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eLeft)].isDown = true;
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eLeft)].isPressed = true;
                    break;
                case WM_LBUTTONUP:
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eLeft)].isDown = false;
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eLeft)].isReleased = true;
                    break;
                case WM_RBUTTONDOWN:
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eRight)].isDown = true;
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eRight)].isPressed = true;
                    break;
                case WM_RBUTTONUP:
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eRight)].isDown = false;
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eRight)].isReleased = true;
                    break;
                case WM_MBUTTONDOWN:
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eMiddle)].isDown = true;
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eMiddle)].isPressed = true;
                    break;
                case WM_MBUTTONUP:
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eMiddle)].isDown = false;
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eMiddle)].isReleased = true;
                    break;
                case WM_XBUTTONDOWN:
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eMiddle) + HIWORD(wp)].isDown = true;
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eMiddle) + HIWORD(wp)].isPressed = true;
                    break;
                case WM_XBUTTONUP:
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eMiddle) + HIWORD(wp)].isDown = false;
                    g_mouse.buttons[static_cast<u8_t>(ar::Button::eMiddle) + HIWORD(wp)].isReleased = true;
                    break;
                case WM_SYSKEYDOWN: [[fallthrough]];
                case WM_SYSKEYUP:   [[fallthrough]];
                case WM_KEYDOWN:    [[fallthrough]];
                case WM_KEYUP:
                    if (wp >= +'A' && wp <= +'Z')
                    {
                        wp -= (+'A' - static_cast<WPARAM>(ar::Key::eA));
                    }
                    else if (wp >= +'0' && wp <= +'9')
                    {
                        wp -= (+'0' - static_cast<WPARAM>(ar::Key::e0));
                    }
                    else if (wp >= VK_LEFT && wp <= VK_DOWN)
                    {
                        wp -= (VK_LEFT - static_cast<WPARAM>(ar::Key::eLeft));
                    }
                    else switch (wp)
                    {
                    case VK_ESCAPE:
                        wp = static_cast<WPARAM>(ar::Key::eEscape);
                        break;
                    case VK_SPACE:
                        wp = static_cast<WPARAM>(ar::Key::eSpace);
                        break;
                    case VK_RETURN:
                        wp = static_cast<WPARAM>(ar::Key::eEnter);
                        break;
                    case VK_BACK:
                        wp = static_cast<WPARAM>(ar::Key::eBackspace);
                        break;
                    case VK_TAB:
                        wp = static_cast<WPARAM>(ar::Key::eTab);
                        break;
                    case VK_OEM_MINUS:
                        wp = static_cast<WPARAM>(ar::Key::eMinus);
                        break;
                    case VK_OEM_PLUS:
                        wp = static_cast<WPARAM>(ar::Key::ePlus);
                        break;
                    case VK_SHIFT:
                        if (::MapVirtualKeyW((lp & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX) == VK_LSHIFT)
                            wp = static_cast<WPARAM>(ar::Key::eLeftShift);
                        else
                            wp = static_cast<WPARAM>(ar::Key::eRightShift);
                        break;
                    case VK_CONTROL:
                        if (lp & 0x01000000)
                            wp = static_cast<WPARAM>(ar::Key::eRightCtrl);
                        else
                            wp = static_cast<WPARAM>(ar::Key::eLeftCtrl);
                        break;
                    case VK_MENU:
                        if (lp & 0x01000000)
                            wp = static_cast<WPARAM>(ar::Key::eRightAlt);
                        else
                            wp = static_cast<WPARAM>(ar::Key::eLeftAlt);
                        break;
                    default:
                        return ::DefWindowProcW(hwnd, msg, wp, lp);
                    }

                    if (msg == WM_SYSKEYDOWN || msg == WM_KEYDOWN) [[likely]]
                    {
                        g_keyboard.keys[wp].isPressed = !(lp & (1 << 30)); 
                        g_keyboard.keysDown[wp] = true;
                    }
                    else [[unlikely]]
                    {
                        g_keyboard.keys[wp].isReleased = true;
                        g_keyboard.keysDown[wp] = false;
                    }
                    break;
                [[unlikely]] case WM_DESTROY:
                    ::PostQuitMessage(0);
                    return 0ll;
                }

                return ::DefWindowProcW(hwnd, msg, wp, lp);
            }
        },
        .hInstance = g_wnd.hinstance,
        .hCursor = ::LoadCursorA(nullptr, IDC_ARROW),
        .hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)),
        .lpszClassName = L"ar",
    }};

    if (!::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) || !::RegisterClassExW(&wc))
    {
        g_errorCallback(error);
    }

    wchar_t windowTitleBuffer[1024];
    ::MultiByteToWideChar(0, 0, config.title.data(), -1, windowTitleBuffer, 1024);

    g_wnd.hwnd = ::CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"ar",
        windowTitleBuffer,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        config.width, config.height,
        nullptr, nullptr,
        g_wnd.hinstance,
        nullptr
    );

    if (!g_wnd.hwnd)
    {
        g_errorCallback(error);
    }

    auto const useDarkMode{ BOOL{1} };
    ::DwmSetWindowAttribute(
        g_wnd.hwnd,
        DWMWA_USE_IMMERSIVE_DARK_MODE,
        &useDarkMode,
        sizeof(useDarkMode)
    );

    ::ShowWindow(g_wnd.hwnd, SW_SHOW);
    ::UpdateWindow(g_wnd.hwnd);

    AR_INFO_CALLBACK("Created Win32 Window: width [%d], height [%d], title [%s]", config.width, config.height, config.title.data())
}

static auto arWindow::teardown() noexcept -> void
{
    ::DestroyWindow(g_wnd.hwnd);
    ::UnregisterClassW(L"ar", g_wnd.hinstance);

    AR_INFO_CALLBACK("Destroyed Win32 Window")
}

auto arline::Window::PollEvents() noexcept -> void
{
    memset(g_keyboard.keys, 0, sizeof(g_keyboard.keys));

    g_mouse.buttons[0].isPressed = g_mouse.buttons[0].isReleased = 0;
    g_mouse.buttons[1].isPressed = g_mouse.buttons[1].isReleased = 0;
    g_mouse.buttons[2].isPressed = g_mouse.buttons[2].isReleased = 0;
    g_mouse.buttons[3].isPressed = g_mouse.buttons[3].isReleased = 0;
    g_mouse.buttons[4].isPressed = g_mouse.buttons[4].isReleased = 0;

    for (MSG msg; ::PeekMessageW(&msg, nullptr, 0u, 0u, PM_REMOVE); )
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);

        if (msg.message == WM_QUIT) [[unlikely]]
        {
            g_wnd.available = false;
        }
    }

    POINT cursorPos;
    ::GetPhysicalCursorPos(&cursorPos);

    g_mouse.globPosX = cursorPos.x;
    g_mouse.globPosY = cursorPos.y;

    ::ScreenToClient(g_wnd.hwnd, &cursorPos);

    g_mouse.posX = cursorPos.x;
    g_mouse.posY = cursorPos.y;
}

auto arline::Window::WaitEvents() noexcept -> void
{
    ::WaitMessage();
    Window::PollEvents();
}

auto arline::Window::SetTitle(std::string_view title) noexcept -> void
{
    ::SetWindowTextA(g_wnd.hwnd, title.data());
}

auto arline::Window::GetWidth() noexcept -> i32_t
{
    return g_wnd.width;
}

auto arline::Window::GetHeight() noexcept -> i32_t
{
    return g_wnd.height;
}

auto arline::Window::GetFramebufferWidth() noexcept -> u32_t
{
    return g_ctx.surfaceExtent.width;
}

auto arline::Window::GetFramebufferHeight() noexcept -> u32_t
{
    return g_ctx.surfaceExtent.height;
}

auto arline::Window::GetAspect() noexcept -> f32_t
{
    return static_cast<f32_t>(g_wnd.width) / g_wnd.height;
}

auto arline::Window::GetFramebufferAspect() noexcept -> f32_t
{
    return static_cast<f32_t>(g_ctx.surfaceExtent.width) / g_ctx.surfaceExtent.height;
}

#endif
#pragma endregion
#pragma region Time Win32

static auto arTimer::create() noexcept -> void
{
    g_timer = {};
    ::QueryPerformanceFrequency(&g_timer.frequency);
    ::QueryPerformanceCounter(&g_timer.timeOffset);
}

auto arTimer::update() noexcept -> void
{
    auto now{ ar::getTime() };
    g_timer.deltaTime = now - g_timer.previousTime;
    g_timer.previousTime = now;
}

auto arline::getTime() noexcept -> f64_t
{
    LARGE_INTEGER value;
    ::QueryPerformanceCounter(&value);

    return static_cast<f64_t>(value.QuadPart - g_timer.timeOffset.QuadPart) / g_timer.frequency.QuadPart;
}

auto arline::getTimef() noexcept -> f32_t
{
    return static_cast<f32_t>(ar::getTime());
}

auto arline::getDeltaTime() noexcept -> f64_t
{
    return g_timer.deltaTime;
}

auto arline::getDeltaTimef() noexcept -> f32_t
{
    return static_cast<f32_t>(g_timer.deltaTime);
}

#pragma endregion
#pragma region Input

auto arline::isKeyPressed(Key key) noexcept -> b8_t
{
    return g_keyboard.keys[static_cast<u8_t>(key)].isPressed;
}

auto arline::isKeyReleased(Key key) noexcept -> b8_t
{
    return g_keyboard.keys[static_cast<u8_t>(key)].isReleased;
}

auto arline::isKeyDown(Key key) noexcept -> b8_t
{
    return g_keyboard.keysDown[static_cast<u8_t>(key)];
}

auto arline::isKeyUp(Key key) noexcept -> b8_t
{
    return !g_keyboard.keysDown[static_cast<u8_t>(key)];
}

auto arline::isButtonPressed(Button button) noexcept -> b8_t
{
    return g_mouse.buttons[static_cast<u8_t>(button)].isPressed;
}

auto arline::isButtonReleased(Button button) noexcept -> b8_t
{
    return g_mouse.buttons[static_cast<u8_t>(button)].isReleased;
}

auto arline::isButtonDown(Button button) noexcept -> b8_t
{
    return g_mouse.buttons[static_cast<u8_t>(button)].isDown;
}

auto arline::isButtonUp(Button button) noexcept -> b8_t
{
    return !g_mouse.buttons[static_cast<u8_t>(button)].isDown;
}

auto arline::getGlobalCursorPositionX() noexcept -> i32_t
{
    return g_mouse.globPosX;
}

auto arline::getGlobalCursorPositionY() noexcept -> i32_t
{
    return g_mouse.globPosY;
}

auto arline::getCursorPositionX() noexcept -> i32_t
{
    return g_mouse.posX;
}

auto arline::getCursorPositionY() noexcept -> i32_t
{
    return g_mouse.posY;
}

auto arline::setCursorPosition(i32_t x, i32_t y) noexcept -> void
{
    ::SetCursorPos(x, y);
}

auto arline::showCursor() noexcept -> void
{
    while (::ShowCursor(1) < 0);
}

auto arline::hideCursor() noexcept -> void
{
    while (::ShowCursor(0) >= 0);
}

#pragma endregion
#pragma region Graphics Commands

arline::GraphicsCommands::GraphicsCommands() noexcept
    : m{
        .id = g_ctx.imageCount
    }
{
    auto const commandBufferBI{ VkCommandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    }};

    arContext::resultCheck(vkResetCommandPool(g_ctx.device, g_ctx.graphicsCommandPool, {}));

    for (auto i{ g_ctx.imageCount }; i--; )
    {
        arContext::resultCheck(
            vkBeginCommandBuffer(g_ctx.images[i].graphicsCommandBuffer, &commandBufferBI)
        );

        vkCmdBindDescriptorSets(
            g_ctx.images[i].graphicsCommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            g_ctx.pipelineLayout,
            0u,
            1u,
            &g_ctx.descriptorSet,
            0u,
            nullptr
        );
    }
}

arline::GraphicsCommands::~GraphicsCommands() noexcept
{
    for (auto i{ g_ctx.imageCount }; i--; )
    {
        arContext::resultCheck(
            vkEndCommandBuffer(g_ctx.images[i].graphicsCommandBuffer)
        );
    }
}

arline::GraphicsCommands::operator b8_t() noexcept
{
    return static_cast<b8_t>(m.id--);
}

auto arline::GraphicsCommands::barrier(ImageBarrier barrier) const noexcept -> void
{
    VkImageAspectFlags constexpr aspects[] = { VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_DEPTH_BIT };
    auto usingDepth{ false };

    usingDepth = static_cast<b8_t>(usingDepth + (barrier.oldLayout == ar::ImageLayout::eDepthAttachment));
    usingDepth = static_cast<b8_t>(usingDepth + (barrier.oldLayout == ar::ImageLayout::eDepthReadOnly));
    usingDepth = static_cast<b8_t>(usingDepth + (barrier.newLayout == ar::ImageLayout::eDepthAttachment));
    usingDepth = static_cast<b8_t>(usingDepth + (barrier.newLayout == ar::ImageLayout::eDepthReadOnly));    

    auto imageBarrier{ VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .oldLayout = static_cast<VkImageLayout>(barrier.oldLayout),
        .newLayout = static_cast<VkImageLayout>(barrier.newLayout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = *reinterpret_cast<VkImage*>(&barrier.image),
        .subresourceRange = {
            .aspectMask = aspects[usingDepth],
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        }
    }};

    switch (barrier.oldLayout)
    {
    using enum ar::ImageLayout;
    case eShaderReadOnly:
        imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        imageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        break;
    case eColorAttachment:
        imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        imageBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case eDepthAttachment:
        imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        break;
    }

    switch (barrier.newLayout)
    {
    using enum ar::ImageLayout;
    case eShaderReadOnly:
        imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        imageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        break;
    case eColorAttachment:
        imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        imageBarrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case eDepthAttachment:
        imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        imageBarrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    }

    auto const dependency{ VkDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        .imageMemoryBarrierCount = 1u,
        .pImageMemoryBarriers = &imageBarrier
    }};

    vkCmdPipelineBarrier2(g_ctx.images[m.id].graphicsCommandBuffer, &dependency);
}

auto arline::GraphicsCommands::beginRendering(
    ColorAttachments colorAttachments,
    DepthAttachment depthAttachment
) const noexcept -> void
{
    VkRenderingAttachmentInfo attachments[9];

    for (auto i{ size_t{} }; i < colorAttachments.size(); ++i)
    {
        auto* attachment{ colorAttachments.begin() + i };

        attachments[i] = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = reinterpret_cast<VkImageView*>(&attachment->image)[1],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = static_cast<VkAttachmentLoadOp>(attachment->loadOp),
            .storeOp = static_cast<VkAttachmentStoreOp>(attachment->storeOp),
            .clearValue = {
                .color = VkClearColorValue{
                    .int32 = {
                        attachment->clearColor.int32[0],
                        attachment->clearColor.int32[1],
                        attachment->clearColor.int32[2],
                        attachment->clearColor.int32[3]
                    }
                }
            }
        };
    }

    if (depthAttachment.pImage)
    {
        attachments[8] = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = reinterpret_cast<VkImageView*>(depthAttachment.pImage)[1],
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .loadOp = static_cast<VkAttachmentLoadOp>(depthAttachment.loadOp),
            .storeOp = static_cast<VkAttachmentStoreOp>(depthAttachment.storeOp),
            .clearValue = {
                .depthStencil = {
                    .depth = 1.f
                }
            }
        };
    }

    auto const renderingInfo{ VkRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .extent = {
                .width = colorAttachments.begin()->image.getWidth(),
                .height = colorAttachments.begin()->image.getHeight()
            }
        },
        .layerCount = 1u,
        .colorAttachmentCount = static_cast<u32_t>(colorAttachments.size()),
        .pColorAttachments = attachments,
        .pDepthAttachment = depthAttachment.pImage ? &attachments[8] : nullptr
    }};

    auto const scissor{ VkRect2D{
        .extent = {
            .width = renderingInfo.renderArea.extent.width,
            .height = renderingInfo.renderArea.extent.height
        }
    }};

    auto const viewport{ VkViewport{
        .y = static_cast<f32_t>(renderingInfo.renderArea.extent.height),
        .width = static_cast<f32_t>(renderingInfo.renderArea.extent.width),
        .height = -static_cast<f32_t>(renderingInfo.renderArea.extent.height),
        .maxDepth = 1.f
    }};

    vkCmdBeginRendering(g_ctx.images[m.id].graphicsCommandBuffer, &renderingInfo);
    vkCmdSetScissor(g_ctx.images[m.id].graphicsCommandBuffer, 0u, 1u, &scissor);
    vkCmdSetViewport(g_ctx.images[m.id].graphicsCommandBuffer, 0u, 1u, &viewport);
}

auto arline::GraphicsCommands::endRendering() const noexcept -> void
{
    vkCmdEndRendering(g_ctx.images[m.id].graphicsCommandBuffer);
}

auto arline::GraphicsCommands::beginPresent() const noexcept -> void
{
    auto const imageBarrier{ VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = g_ctx.images[m.id].image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1u,
            .layerCount = 1u
        }
    }};

    auto const dependency{ VkDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        .imageMemoryBarrierCount = 1u,
        .pImageMemoryBarriers = &imageBarrier
    }};

    auto const colorAttachment{ VkRenderingAttachmentInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = g_ctx.images[m.id].view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE
    }};

    auto const renderingInfo{ VkRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .extent = g_ctx.surfaceExtent
        },
        .layerCount = 1u,
        .colorAttachmentCount = 1u,
        .pColorAttachments = &colorAttachment
    }};

    auto const scissor{ VkRect2D{
        .extent = {
            .width = g_ctx.surfaceExtent.width,
            .height = g_ctx.surfaceExtent.height
        }
    }};

    auto const viewport{ VkViewport{
        .y = static_cast<f32_t>(g_ctx.surfaceExtent.height),
        .width = static_cast<f32_t>(g_ctx.surfaceExtent.width),
        .height = -static_cast<f32_t>(g_ctx.surfaceExtent.height),
        .maxDepth = 1.f
    }};

    vkCmdPipelineBarrier2(g_ctx.images[m.id].graphicsCommandBuffer, &dependency);
    vkCmdBeginRendering(g_ctx.images[m.id].graphicsCommandBuffer, &renderingInfo);
    vkCmdSetScissor(g_ctx.images[m.id].graphicsCommandBuffer, 0u, 1u, &scissor);
    vkCmdSetViewport(g_ctx.images[m.id].graphicsCommandBuffer, 0u, 1u, &viewport);
}

auto arline::GraphicsCommands::endPresent() const noexcept -> void
{
    VkImageMemoryBarrier2 imageBarrier;

    if (g_ctx.unifiedPresent)
    {
        imageBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_NONE,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = g_ctx.images[m.id].image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1u,
                .layerCount = 1u
            }
        };
    }
    else
    {
        imageBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = g_ctx.graphicsFamily,
            .dstQueueFamilyIndex = g_ctx.presentFamily,
            .image = g_ctx.images[m.id].image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1u,
                .layerCount = 1u
            }
        };
    }

    auto const dependency{ VkDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        .imageMemoryBarrierCount = 1u,
        .pImageMemoryBarriers = &imageBarrier
    }};

    vkCmdEndRendering(g_ctx.images[m.id].graphicsCommandBuffer);
    vkCmdPipelineBarrier2(g_ctx.images[m.id].graphicsCommandBuffer, &dependency);
}

auto arline::GraphicsCommands::bindPipeline(Pipeline const& pipeline) const noexcept -> void
{
    vkCmdBindPipeline(
        g_ctx.images[m.id].graphicsCommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        *reinterpret_cast<VkPipeline const*>(&pipeline)
    );
}

auto arline::GraphicsCommands::draw(u32_t vertexCount, u32_t instanceCount, u32_t vertex, u32_t instance) const noexcept -> void
{
    vkCmdDraw(
        g_ctx.images[m.id].graphicsCommandBuffer,
        vertexCount,
        instanceCount,
        vertex,
        instance
    );
}

auto arline::GraphicsCommands::drawIndexed(u32_t indexCount, u32_t instanceCount, u32_t index, i32_t vertexOffset, u32_t instance) const noexcept -> void
{
    vkCmdDrawIndexed(
        g_ctx.images[m.id].graphicsCommandBuffer,
        indexCount,
        instanceCount,
        index,
        vertexOffset,
        instance
    );
}

auto arline::GraphicsCommands::drawIndirect(Buffer const& buffer, u32_t drawCount, u32_t stride) const noexcept -> void
{
    vkCmdDrawIndirect(
        g_ctx.images[m.id].graphicsCommandBuffer,
        *reinterpret_cast<VkBuffer const*>(&buffer),
        0ull,
        drawCount,
        stride
    );
}

auto arline::GraphicsCommands::drawIndexedIndirect(Buffer const &buffer, u32_t drawCount, u32_t stride) const noexcept -> void
{
    vkCmdDrawIndexedIndirect(
        g_ctx.images[m.id].graphicsCommandBuffer,
        *reinterpret_cast<VkBuffer const*>(&buffer),
        0ull,
        drawCount,
        stride
    );
}

auto arline::GraphicsCommands::drawIndirectCount(Buffer const& buffer, Buffer const& countBuffer, u32_t maxDraws, u32_t stride) const noexcept -> void
{
    vkCmdDrawIndirectCount(
        g_ctx.images[m.id].graphicsCommandBuffer,
        *reinterpret_cast<VkBuffer const*>(&buffer),
        0ull,
        *reinterpret_cast<VkBuffer const*>(&countBuffer),
        0ull,
        maxDraws,
        stride
    );
}

auto arline::GraphicsCommands::drawIndexedIndirectCount(Buffer const &buffer, Buffer const &countBuffer, u32_t maxDraws, u32_t stride) const noexcept -> void
{
    vkCmdDrawIndexedIndirectCount(
        g_ctx.images[m.id].graphicsCommandBuffer,
        *reinterpret_cast<VkBuffer const*>(&buffer),
        0ull,
        *reinterpret_cast<VkBuffer const*>(&countBuffer),
        0ull,
        maxDraws,
        stride
    );
}

auto arline::GraphicsCommands::pushConstant(void const* pData, u32_t size) const noexcept -> void
{
    vkCmdPushConstants(
        g_ctx.images[m.id].graphicsCommandBuffer,
        g_ctx.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0u,
        size,
        pData
    );
}

#pragma endregion
#pragma region Shaders

arline::Shader::Shader(std::string_view pathToSpirv) noexcept
    : m{}
{
    static_assert(sizeof(*this) == sizeof(VkPipelineShaderStageCreateInfo));

    auto error{ [&]
    {
        char errorMessage[1024];
        std::snprintf(errorMessage, sizeof(errorMessage), "Failed to load shader: [%s]", pathToSpirv.data());

        g_errorCallback(errorMessage);
    }};

    VkShaderStageFlagBits stageFlagBits;

    if (pathToSpirv.ends_with(".vert.spv"))
    {
        stageFlagBits = VK_SHADER_STAGE_VERTEX_BIT;
    }
    else if (pathToSpirv.ends_with(".frag.spv"))
    {
        stageFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    else
    {
        stageFlagBits = {};
        error();
    }

    auto shaderStageCI{ reinterpret_cast<VkPipelineShaderStageCreateInfo*>(this)};

    *shaderStageCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stageFlagBits,
        .pName = "main"
    };

    auto const file{ ::CreateFileA(
        pathToSpirv.data(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    )};

    if (file == INVALID_HANDLE_VALUE)
    {
        error();
    }

    LARGE_INTEGER fileSize;
    if (!::GetFileSizeEx(file, &fileSize))
    {
        ::CloseHandle(file);
        error();
    }

    auto pBuffer{ new u8_t[fileSize.QuadPart] };
    auto bytesRead{ DWORD{} };

    if (!::ReadFile(file, pBuffer, fileSize.LowPart, &bytesRead, nullptr))
    {
        error();
    }
    ::CloseHandle(file);

    auto const shaderModuleCI{ VkShaderModuleCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = static_cast<size_t>(fileSize.QuadPart),
        .pCode = reinterpret_cast<u32_t*>(pBuffer)
    }};

    arContext::resultCheck(vkCreateShaderModule(g_ctx.device, &shaderModuleCI, nullptr, &shaderStageCI->module));

    delete[] pBuffer;
}

arline::Shader::Shader(u32_t const* pSpirvBinary, size_t dataSize, ShaderStage stage) noexcept
    : m{}
{
    auto shaderStageCI{ reinterpret_cast<VkPipelineShaderStageCreateInfo*>(this)};

    *shaderStageCI = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = static_cast<VkShaderStageFlagBits>(stage),
        .pName = "main"
    };

    auto const shaderModuleCI{ VkShaderModuleCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = dataSize,
        .pCode = pSpirvBinary
    }};

    arContext::resultCheck(vkCreateShaderModule(g_ctx.device, &shaderModuleCI, nullptr, &shaderStageCI->module));
}

arline::Shader::~Shader() noexcept
{
    auto shaderStageCI{ reinterpret_cast<VkPipelineShaderStageCreateInfo*>(this)};

    if (shaderStageCI->module)
    {
        vkDestroyShaderModule(g_ctx.device, shaderStageCI->module, nullptr);
    }

    m = {};
}

arline::Shader::Shader(Shader&& other) noexcept
    : m{ other.m }
{
    other.m = {};
}

auto arline::Shader::operator=(Shader&& other) noexcept -> Shader&
{
    this->~Shader();
    this->m = other.m;
    other.m = {};
    return *this;
}

#pragma endregion
#pragma region Pipeline

arline::Pipeline::Pipeline(GraphicsConfig&& config) noexcept
{
    VkDynamicState constexpr dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    auto const dynamicStateCreateInfo{ VkPipelineDynamicStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2u,
        .pDynamicStates = dynamicStates
    }};

    auto const viewportStateCreateInfo{ VkPipelineViewportStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1u,
        .scissorCount  = 1u
    }};

    auto const inputAssemblyStateCreateInfo{ VkPipelineInputAssemblyStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = static_cast<VkPrimitiveTopology>(config.topology)
    }};

    auto const rasterizationStateCreateInfo{ VkPipelineRasterizationStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = static_cast<VkPolygonMode>(config.polygonMode),
        .cullMode = static_cast<VkCullModeFlags>(config.cullMode),
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f
    }};

    auto const multisampleStateCreateInfo{ VkPipelineMultisampleStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    }};

    auto const blendAttachmentState{ VkPipelineColorBlendAttachmentState{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    }};

    auto const colorBlendStateCreateInfo{ VkPipelineColorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1u,
        .pAttachments = &blendAttachmentState
    }};

    auto const depthStencilStateCreateInfo{ VkPipelineDepthStencilStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = static_cast<u32_t>(config.depthStencilState.depthTestEnable),
        .depthWriteEnable = static_cast<u32_t>(config.depthStencilState.depthWriteEnable),
        .depthCompareOp = static_cast<VkCompareOp>(config.depthStencilState.compareOp)
    }};

    auto const vertexInputStateCreateInfo{ VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    }};

    auto const renderingCreateInfo{ VkPipelineRenderingCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1u,
        .pColorAttachmentFormats = &g_ctx.surfaceFormat.format,
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    }};

    auto const pipelineCreateInfo{ VkGraphicsPipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCreateInfo,
        .stageCount = static_cast<u32_t>(config.shaders.size()),
        .pStages = reinterpret_cast<VkPipelineShaderStageCreateInfo const*>(config.shaders.begin()),
        .pVertexInputState = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pDepthStencilState = &depthStencilStateCreateInfo,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = g_ctx.pipelineLayout
    }};

    arContext::resultCheck(vkCreateGraphicsPipelines(
            g_ctx.device,
            nullptr,
            1u,
            &pipelineCreateInfo,
            nullptr,
            reinterpret_cast<VkPipeline*>(&m.pHandle)
        )
    );
}

arline::Pipeline::~Pipeline() noexcept
{
    if (m.pHandle)
    {
        vkDestroyPipeline(g_ctx.device, static_cast<VkPipeline>(m.pHandle), nullptr);
    }

    m = {};
}

arline::Pipeline::Pipeline(Pipeline&& other) noexcept
    : m{ other.m }
{
    other.m = {};
}

auto arline::Pipeline::operator=(Pipeline&& other) noexcept -> Pipeline&
{
    this->~Pipeline();
    this->m = other.m;
    other.m = {};

    return *this;
}

#pragma endregion
#pragma region Buffer

arline::Buffer::Buffer(Buffer&& other) noexcept
    : m{ other.m }
{
    other.m = {};
}

arline::Buffer::~Buffer() noexcept
{
    if (m.pHandle)
    {
        vmaDestroyBuffer(
            g_ctx.allocator,
            static_cast<VkBuffer>(m.pHandle),
            static_cast<VmaAllocation>(m.pAllocation)
        );
    }

    m = {};
}

auto arline::Buffer::operator=(Buffer&& other) noexcept -> Buffer&
{
    this->~Buffer();
    this->m = other.m;
    other.m = {};

    return *this;
}

auto arline::Buffer::getAddress() const noexcept -> u64_t
{
    auto const bufferDAI{ VkBufferDeviceAddressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = static_cast<VkBuffer>(m.pHandle)
    }};

    return vkGetBufferDeviceAddress(g_ctx.device, &bufferDAI);
}

#pragma endregion
#pragma region Dynamic Buffer

arline::DynamicBuffer::DynamicBuffer(size_t capacity) noexcept
{
    static constexpr auto usage{ VkBufferUsageFlags{
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
    }};

    auto const bufferCI{ VkBufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = capacity,
        .usage = usage
    }};

    auto const allocationCI{ VmaAllocationCreateInfo{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    }};

    VkBuffer buffer;
    VmaAllocation allocation;

    arContext::resultCheck(vmaCreateBuffer(
        g_ctx.allocator,
        &bufferCI,
        &allocationCI,
        &buffer,
        &allocation,
        nullptr
    ));

    arContext::resultCheck(vmaMapMemory(
        g_ctx.allocator,
        allocation,
        reinterpret_cast<void**>(&m_pMapped)
    ));

    m = {
        .pHandle = buffer,
        .pAllocation = allocation,
        .capacity = capacity
    };
}

arline::DynamicBuffer::~DynamicBuffer() noexcept
{
    if (m_pMapped)
    {
        vmaUnmapMemory(
            g_ctx.allocator,
            static_cast<VmaAllocation>(m.pAllocation)
        );
    }
}

auto arline::DynamicBuffer::write(void const* pData, size_t size, size_t offset) noexcept -> void
{
    size = size ? size : m.capacity;
    memcpy(m_pMapped + offset, pData, size);
    arContext::resultCheck(vmaFlushAllocation(
        g_ctx.allocator,
        static_cast<VmaAllocation>(m.pAllocation),
        offset,
        size
    ));
}

#pragma endregion
#pragma region Static Buffer

arline::StaticBuffer::StaticBuffer(void const* pData, size_t dataSize)
{
    static constexpr auto usage{ VkBufferUsageFlags{
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
    }};

    auto const bufferCI{ VkBufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = dataSize,
        .usage = usage
    }};

    auto const allocationCI{ VmaAllocationCreateInfo{
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    }};

    VkBuffer buffer;
    VmaAllocation allocation;

    arContext::resultCheck(vmaCreateBuffer(
        g_ctx.allocator,
        &bufferCI,
        &allocationCI,
        &buffer,
        &allocation,
        nullptr
    ));

    VkMemoryPropertyFlags memProperty;
    vmaGetAllocationMemoryProperties(g_ctx.allocator, allocation, &memProperty);

    if (memProperty & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        arContext::resultCheck(vmaCopyMemoryToAllocation(
            g_ctx.allocator,
            pData,
            allocation,
            0ull,
            dataSize
        ));
    }
    else
    {
        auto const stagingBufferCI{ VkBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = dataSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        }};

        auto const stagingAllocationCI{ VmaAllocationCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        }};

        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;

        arContext::resultCheck(vmaCreateBuffer(
            g_ctx.allocator,
            &stagingBufferCI,
            &stagingAllocationCI,
            &stagingBuffer,
            &stagingAllocation,
            nullptr
        ));

        arContext::resultCheck(vmaCopyMemoryToAllocation(
            g_ctx.allocator,
            pData,
            stagingAllocation,
            0ull,
            dataSize
        ));

        arContext::beginTransfer();

        auto const bufferCopy{ VkBufferCopy{
            .size = dataSize
        }};

        vkCmdCopyBuffer(
            g_ctx.transferCommandBuffer,
            stagingBuffer,
            buffer,
            1u,
            &bufferCopy
        );

        arContext::endTransfer();
    }

    m = {
        .pHandle = buffer,
        .pAllocation = allocation,
        .capacity = dataSize
    };
}

#pragma endregion
#pragma region Image

arline::Image::~Image() noexcept
{
    if (m.pView)
    {
        vkDestroyImageView(
            g_ctx.device,
            static_cast<VkImageView>(m.pView),
            nullptr
        );
    }

    if (m.pHandle)
    {
        vmaDestroyImage(
            g_ctx.allocator,
            static_cast<VkImage>(m.pHandle),
            static_cast<VmaAllocation>(m.pAllocation)
        );
    }

    m = {};
}

arline::Image::Image(Image&& other) noexcept
    : m{ other.m }
{
    other.m = {};
}

auto arline::Image::operator=(Image&& other) noexcept -> Image&
{
    auto index{ m.index };
    auto sampler{ m.sampler };

    this->~Image();
    this->m = other.m;
    other.m = {};

    this->makeResident(sampler, index);

    return *this;
}

arline::Image::Image(ImageCreateInfo const& imageCreateInfo) noexcept
    : m{
        .sampler = imageCreateInfo.sampler,
        .width = imageCreateInfo.width ? imageCreateInfo.width : g_ctx.surfaceExtent.width,
        .height = imageCreateInfo.height ? imageCreateInfo.height : g_ctx.surfaceExtent.height
    }
{
    VkFormat const formats[] = { g_ctx.surfaceFormat.format, VK_FORMAT_D32_SFLOAT };
    VkImageAspectFlags constexpr aspects[] = { VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_DEPTH_BIT };
    VkImageUsageFlags constexpr usages[] = {
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    };
    VkImageLayout constexpr layouts[] = { 
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
    };

    auto usage{ static_cast<u8_t>(imageCreateInfo.usage) };

    auto const imageCI{ VkImageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = formats[usage],
        .extent = {
            .width = m.width,
            .height = m.height,
            .depth = 1u
        },
        .mipLevels = 1u,
        .arrayLayers = 1u,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .usage = m.sampler ? usages[usage] | VK_IMAGE_USAGE_SAMPLED_BIT : usages[usage]
    }};

    auto const allocationCI{ VmaAllocationCreateInfo{
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .priority = 1.f
    }};

    arContext::resultCheck(vmaCreateImage(
        g_ctx.allocator,
        &imageCI,
        &allocationCI,
        reinterpret_cast<VkImage*>(&m.pHandle),
        reinterpret_cast<VmaAllocation*>(&m.pAllocation),
        nullptr
    ));

    m.pView = arContext::createImageView(
        static_cast<VkImage>(m.pHandle),
        VK_IMAGE_VIEW_TYPE_2D,
        formats[usage],
        aspects[usage],
        1u
    );

    arContext::beginTransfer();

    auto const imageBarrier{ VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = layouts[!m.sampler * (usage + 1ui8)],
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = static_cast<VkImage>(m.pHandle),
        .subresourceRange = {
            .aspectMask = aspects[usage],
            .levelCount = 1u,
            .layerCount = 1u
        }
    }};

    auto const dependency{ VkDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        .imageMemoryBarrierCount = 1u,
        .pImageMemoryBarriers = &imageBarrier
    }};

    vkCmdPipelineBarrier2(g_ctx.transferCommandBuffer, &dependency);

    arContext::endTransfer();

    this->makeResident(m.sampler, 0u);
}

auto arline::Image::makeResident(Sampler sampler, u32_t index) noexcept -> void
{
    if (sampler)
    {
        if (index)
        {
            m.index = index;
            --g_ctx.allocatedSampledImageCount;
        }
        else
        {
            m.index = ++g_ctx.allocatedSampledImageCount;
        }

        auto const writeImage{ VkDescriptorImageInfo{
            .sampler = *(&g_ctx.linearToEdgeSampler + (static_cast<u8_t>(sampler) - 1ui8)),
            .imageView = static_cast<VkImageView>(m.pView),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        }};

        auto const write{ VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = g_ctx.descriptorSet,
            .dstArrayElement = m.index,
            .descriptorCount = 1u,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &writeImage
        }};

        vkUpdateDescriptorSets(
            g_ctx.device,
            1u,
            &write,
            0u,
            nullptr
        );
    }
}

#pragma endregion
#pragma region Context

static auto arContext::acquireImage() noexcept -> b8_t
{
    arContext::resultCheck(vkWaitForFences(g_ctx.device, 1u, &g_ctx.fence, 0u, ~0ull));
    arContext::resultCheck(vkResetFences(g_ctx.device, 1u, &g_ctx.fence));

    auto result{ vkAcquireNextImageKHR(
        g_ctx.device,
        g_ctx.swapchain,
        ~0ull,
        g_ctx.acquireSemaphore,
        nullptr,
        &g_ctx.imageIndex
    )};

    switch (result)
    {
    case VK_SUBOPTIMAL_KHR: [[fallthrough]];
    [[likely]] case VK_SUCCESS:
        return true;
    [[unlikely]] case VK_ERROR_OUT_OF_DATE_KHR:
        arContext::createSwapchain();
        return false;
    default:
        arContext::resultCheck(result);
        return true;
    }
}

static auto arContext::presentFrame() noexcept -> b8_t
{
    auto stage{ VkPipelineStageFlags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT} };
    g_ctx.submitInfo.pWaitDstStageMask = &stage;
    g_ctx.submitInfo.pWaitSemaphores = &g_ctx.acquireSemaphore;
    g_ctx.submitInfo.pSignalSemaphores = &g_ctx.graphicsSemaphore;
    g_ctx.submitInfo.pCommandBuffers = &g_ctx.images[g_ctx.imageIndex].graphicsCommandBuffer;
    arContext::resultCheck(vkQueueSubmit(g_ctx.graphicsQueue, 1u, &g_ctx.submitInfo, g_ctx.fence));

    if (g_ctx.unifiedPresent)
    {
        g_ctx.presentInfo.pWaitSemaphores = &g_ctx.graphicsSemaphore;
    }
    else
    {
        stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        g_ctx.submitInfo.pWaitSemaphores = &g_ctx.graphicsSemaphore;
        g_ctx.submitInfo.pSignalSemaphores = &g_ctx.presentSemaphore;
        g_ctx.submitInfo.pCommandBuffers = &g_ctx.images[g_ctx.imageIndex].presentCommandBuffer;
        arContext::resultCheck(vkQueueSubmit(g_ctx.presentQueue, 1u, &g_ctx.submitInfo, nullptr));

        g_ctx.presentInfo.pWaitSemaphores = &g_ctx.presentSemaphore;
    }

    g_ctx.presentInfo.pImageIndices = &g_ctx.imageIndex;

    auto result{ vkQueuePresentKHR(g_ctx.presentQueue, &g_ctx.presentInfo) };

    switch (result)
    {
    [[likely]] case VK_SUCCESS:
        return true;
    [[unlikely]] case VK_SUBOPTIMAL_KHR: [[fallthrough]];
    [[unlikely]] case VK_ERROR_OUT_OF_DATE_KHR:
        createSwapchain();
        return false;
    default:
        arContext::resultCheck(result);
        return false;
    }
}

static auto arContext::create() noexcept -> void
{
    {
        u32_t apiVersion;
        arContext::resultCheck(vkEnumerateInstanceVersion(&apiVersion));

        if (apiVersion < VK_API_VERSION_1_3)
        {
            arContext::resultCheck(VK_ERROR_INITIALIZATION_FAILED);
        }

        AR_INFO_CALLBACK("Vulkan API version: %u.%u.%u", VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion))

        char const* extensions[] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            #ifdef VK_KHR_win32_surface
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            #endif
            #ifdef AR_ENABLE_INFO_CALLBACK
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
            #endif
        };

        #ifdef AR_ENABLE_INFO_CALLBACK
        char const* layers[] = {
            "VK_LAYER_KHRONOS_validation"
        };

        auto const enabledValidationFeatrues{ VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT };

        auto const validationFeatures{ VkValidationFeaturesEXT{
            .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
            .enabledValidationFeatureCount = 1u,
            .pEnabledValidationFeatures = &enabledValidationFeatrues
        }};
        #endif

        auto const applicationInfo{ VkApplicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "ar",
            .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .pEngineName = "ar",
            .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .apiVersion = apiVersion
        }};

        auto const instanceCI{ VkInstanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            #ifdef AR_ENABLE_INFO_CALLBACK
            .pNext = g_ctx.messenger.enableValidationLayers ? &validationFeatures : nullptr,
            #endif
            .pApplicationInfo = &applicationInfo,
            #ifdef AR_ENABLE_INFO_CALLBACK
            .enabledLayerCount = static_cast<u32_t>(g_ctx.messenger.enableValidationLayers),
            .ppEnabledLayerNames = layers,
            .enabledExtensionCount = 2u + static_cast<u32_t>(g_ctx.messenger.enableValidationLayers),
            #else
            .enabledExtensionCount = 2u,
            #endif
            .ppEnabledExtensionNames = extensions,
        }};

        arContext::resultCheck(vkCreateInstance(&instanceCI, nullptr, &g_ctx.instance));
    }
    #ifdef AR_ENABLE_INFO_CALLBACK
    if (g_ctx.messenger.enableValidationLayers)
    {
        auto static constexpr severity{ VkDebugUtilsMessageSeverityFlagsEXT{
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        }};

        auto static constexpr type{ VkDebugUtilsMessageTypeFlagsEXT{
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
        }};

        auto const messengerCI{ VkDebugUtilsMessengerCreateInfoEXT{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = severity,
            .messageType = type,
            .pfnUserCallback = {
                [](
                    [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type,
                    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                    void* pUserData
                )
                {
                    static_cast<void(*)(std::string_view)>(
                        pUserData
                    )(pCallbackData->pMessage);

                    return 0u;
                }
            },
            .pUserData = g_infoCallback
        }};

        arContext::resultCheck(
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(g_ctx.instance, "vkCreateDebugUtilsMessengerEXT"))
            (g_ctx.instance, &messengerCI, nullptr, &g_ctx.messenger.handle)
        );

        g_infoCallback("Validation Layers are enabled. Prefer disabling them in release build");
    }
    #endif
    #if defined(VK_KHR_win32_surface)
    {
        auto const surfaceCI{ VkWin32SurfaceCreateInfoKHR{
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = g_wnd.hinstance,
            .hwnd = g_wnd.hwnd
        }};

        arContext::resultCheck(vkCreateWin32SurfaceKHR(g_ctx.instance, &surfaceCI, nullptr, &g_ctx.surface));
    }
    #endif
    {
        VkPhysicalDevice physicalDevices[64];
        u32_t physicalDeviceCount;

        arContext::resultCheck(vkEnumeratePhysicalDevices(g_ctx.instance, &physicalDeviceCount, nullptr));
        arContext::resultCheck(vkEnumeratePhysicalDevices(g_ctx.instance, &physicalDeviceCount, physicalDevices));

        auto meshShaderFeatures{ VkPhysicalDeviceMeshShaderFeaturesEXT{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT
        }};

        auto features{ VkPhysicalDeviceFeatures2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &meshShaderFeatures
        }};

        auto properties{ VkPhysicalDeviceProperties2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
        }};

        for (auto i{ physicalDeviceCount }; i--; )
        {
            vkGetPhysicalDeviceFeatures2(physicalDevices[i], &features);
            vkGetPhysicalDeviceProperties2(physicalDevices[i], &properties);

            if (properties.properties.apiVersion < VK_API_VERSION_1_3)
                continue;

            g_ctx.physicalDevice = physicalDevices[i];

            if (meshShaderFeatures.meshShader)
                break;
        }

        AR_INFO_CALLBACK("Selected GPU: [%s]", properties.properties.deviceName)

        AR_INFO_CALLBACK("GPU Api version: [%u.%u.%u]", 
            VK_VERSION_MAJOR(properties.properties.apiVersion),
            VK_VERSION_MINOR(properties.properties.apiVersion),
            VK_VERSION_PATCH(properties.properties.apiVersion))

        AR_INFO_CALLBACK("GPU Driver version: [%u.%u.%u]",
            VK_VERSION_MAJOR(properties.properties.driverVersion),
            VK_VERSION_MINOR(properties.properties.driverVersion),
            VK_VERSION_PATCH(properties.properties.driverVersion))

        if (!g_ctx.physicalDevice)
        {
            arContext::resultCheck(VK_ERROR_INITIALIZATION_FAILED);
        }
    }
    {
        VkQueueFamilyProperties properties[64];
        u32_t propertyCount;
        u32_t presentQueueIndex;

        vkGetPhysicalDeviceQueueFamilyProperties(g_ctx.physicalDevice, &propertyCount, nullptr);
        vkGetPhysicalDeviceQueueFamilyProperties(g_ctx.physicalDevice, &propertyCount, properties);

        g_ctx.graphicsFamily = ~0u;
        g_ctx.presentFamily = ~0u;

        for (auto i{ propertyCount }; i--; )
        {
            if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                g_ctx.graphicsFamily = i;
        }

        for (auto i{ propertyCount }; i--; )
        {
            VkBool32 presentSupport;
            arContext::resultCheck(vkGetPhysicalDeviceSurfaceSupportKHR(
                g_ctx.physicalDevice, i, g_ctx.surface, &presentSupport
            ));

            if (presentSupport && i != g_ctx.graphicsFamily)
                g_ctx.presentFamily = i;
        }

        presentQueueIndex = 0u;
        if (g_ctx.presentFamily == ~0u)
        {
            g_ctx.presentFamily = g_ctx.graphicsFamily;
            g_ctx.unifiedPresent = 1u;

            if (properties[g_ctx.presentFamily].queueCount > 1u)
            {
                presentQueueIndex = 1u;
            }
        }

        AR_INFO_CALLBACK("Graphics Family: [%u], Graphics Family Index: [%u]", g_ctx.graphicsFamily, 0u)
        AR_INFO_CALLBACK("Present Family: [%u], Present Family Index: [%u]", g_ctx.presentFamily, presentQueueIndex)
        AR_INFO_CALLBACK("Graphics and Present queues are %s", g_ctx.unifiedPresent ? "unified [worse case]" : "separated [better case]")

        f32_t const queuePriorities[] = {
            1.f, 1.f
        };

        VkDeviceQueueCreateInfo queueCI[] = {
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = g_ctx.graphicsFamily,
                .queueCount = 1u + presentQueueIndex,
                .pQueuePriorities = queuePriorities
            },
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = g_ctx.presentFamily,
                .queueCount = 1u,
                .pQueuePriorities = queuePriorities
            }
        };

        auto features11{ VkPhysicalDeviceVulkan11Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        }};

        auto features12{ VkPhysicalDeviceVulkan12Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &features11,
            .descriptorBindingPartiallyBound = 1u,
            .bufferDeviceAddress = 1u
        }};

        auto features13{ VkPhysicalDeviceVulkan13Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &features12,
            .synchronization2 = 1u,
            .dynamicRendering = 1u
        }};

        auto const features{ VkPhysicalDeviceFeatures2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features13,
            .features = {
                .fillModeNonSolid = 1u
            }
        }};

        char const* deviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        auto const deviceCI{ VkDeviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features,
            .queueCreateInfoCount = 2u - g_ctx.unifiedPresent,
            .pQueueCreateInfos = queueCI,
            .enabledExtensionCount = 1u,
            .ppEnabledExtensionNames = deviceExtensions
        }};

        arContext::resultCheck(vkCreateDevice(g_ctx.physicalDevice, &deviceCI, nullptr, &g_ctx.device));

        vkGetDeviceQueue(g_ctx.device, g_ctx.graphicsFamily, 0u, &g_ctx.graphicsQueue);
        vkGetDeviceQueue(g_ctx.device, g_ctx.presentFamily, presentQueueIndex, &g_ctx.presentQueue);
    }
    {
        auto const allocatorCI{ VmaAllocatorCreateInfo{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
            .physicalDevice = g_ctx.physicalDevice,
            .device = g_ctx.device,
            .instance = g_ctx.instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
        }};

        arContext::resultCheck(vmaCreateAllocator(&allocatorCI, &g_ctx.allocator));
    }
    {
        createSwapchain();
    }
    {
        auto const fenceCI{ VkFenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        }};

        auto const semaphoreCI{ VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        }};

        if (!g_ctx.unifiedPresent)
        arContext::resultCheck(vkCreateSemaphore(g_ctx.device, &semaphoreCI, nullptr, &g_ctx.presentSemaphore));
        arContext::resultCheck(vkCreateSemaphore(g_ctx.device, &semaphoreCI, nullptr, &g_ctx.graphicsSemaphore));
        arContext::resultCheck(vkCreateSemaphore(g_ctx.device, &semaphoreCI, nullptr, &g_ctx.acquireSemaphore));
        arContext::resultCheck(vkCreateFence(g_ctx.device, &fenceCI, nullptr, &g_ctx.fence));
    }   
    {
        VkDescriptorPoolSize poolSizes[] = {
            { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 64u * 1024u }
        };

        auto const descriptorPoolCI{ VkDescriptorPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1u,
            .poolSizeCount = 1u,
            .pPoolSizes = poolSizes
        }};

        arContext::resultCheck(vkCreateDescriptorPool(
            g_ctx.device,
            &descriptorPoolCI,
            nullptr,
            &g_ctx.descriptorPool
        ));

        VkDescriptorSetLayoutBinding const bindings[] = {{
            .binding = 0u,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 64u * 1024u,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        }};

        VkDescriptorBindingFlags const bindingFlags[] = {
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
        };

        auto const bindingFlagsCI{ VkDescriptorSetLayoutBindingFlagsCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = 1u,
            .pBindingFlags = bindingFlags
        }};

        auto const descriptorSetLayoutCI{ VkDescriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &bindingFlagsCI,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = 1u,
            .pBindings = bindings
        }};

        arContext::resultCheck(vkCreateDescriptorSetLayout(
            g_ctx.device,
            &descriptorSetLayoutCI,
            nullptr,
            &g_ctx.descriptorSetLayout
        ));

        auto const descriptorSetAI{ VkDescriptorSetAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = g_ctx.descriptorPool,
            .descriptorSetCount = 1u,
            .pSetLayouts = &g_ctx.descriptorSetLayout
        }};

        arContext::resultCheck(vkAllocateDescriptorSets(
            g_ctx.device,
            &descriptorSetAI,
            &g_ctx.descriptorSet
        ));
    }
    {
        auto const pushConstantRange{ VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .size = 128u
        }};

        auto const pipelineLayoutCI{ VkPipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1u,
            .pSetLayouts = &g_ctx.descriptorSetLayout,
            .pushConstantRangeCount = 1u,
            .pPushConstantRanges = &pushConstantRange,
        }};

        arContext::resultCheck(vkCreatePipelineLayout(g_ctx.device, &pipelineLayoutCI, nullptr, &g_ctx.pipelineLayout));
    }
    {
        auto samplerCI{ VkSamplerCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .maxLod = VK_LOD_CLAMP_NONE
        }};

        samplerCI.addressModeU = samplerCI.addressModeV =
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.magFilter = samplerCI.minFilter = VK_FILTER_LINEAR;

        arContext::resultCheck(vkCreateSampler(
            g_ctx.device,
            &samplerCI,
            nullptr,
            &g_ctx.linearToEdgeSampler
        ));

        samplerCI.addressModeU = samplerCI.addressModeV =
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.magFilter = samplerCI.minFilter = VK_FILTER_LINEAR;

        arContext::resultCheck(vkCreateSampler(
            g_ctx.device,
            &samplerCI,
            nullptr,
            &g_ctx.linearRepeatSampler
        ));

        samplerCI.addressModeU = samplerCI.addressModeV =
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.magFilter = samplerCI.minFilter = VK_FILTER_NEAREST;

        arContext::resultCheck(vkCreateSampler(
            g_ctx.device,
            &samplerCI,
            nullptr,
            &g_ctx.nearestToEdgeSampler
        ));

        samplerCI.addressModeU = samplerCI.addressModeV =
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.magFilter = samplerCI.minFilter = VK_FILTER_NEAREST;

        arContext::resultCheck(vkCreateSampler(
            g_ctx.device,
            &samplerCI,
            nullptr,
            &g_ctx.nearestRepeatSampler
        ));
    }
    {
        auto const commandPoolCI{ VkCommandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = g_ctx.graphicsFamily
        }};

        arContext::resultCheck(vkCreateCommandPool(
            g_ctx.device,
            &commandPoolCI,
            nullptr,
            &g_ctx.transferCommandPool
        ));

        auto const commandBufferAI{ VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = g_ctx.transferCommandPool,
            .commandBufferCount = 1u
        }};

        arContext::resultCheck(vkAllocateCommandBuffers(
            g_ctx.device,
            &commandBufferAI,
            &g_ctx.transferCommandBuffer
        ));
    }

    AR_INFO_CALLBACK("Created Context")
}

static auto arContext::teardown() noexcept -> void
{
    if (!g_ctx.unifiedPresent)
    vkDestroySemaphore(g_ctx.device, g_ctx.presentSemaphore, nullptr);
    vkDestroySemaphore(g_ctx.device, g_ctx.graphicsSemaphore, nullptr);
    vkDestroySemaphore(g_ctx.device, g_ctx.acquireSemaphore, nullptr);
    vkDestroyFence(g_ctx.device, g_ctx.fence, nullptr);

    teardownSwapchain();

    vmaDestroyAllocator(g_ctx.allocator);
    vkDestroySampler(g_ctx.device, g_ctx.linearRepeatSampler, nullptr);
    vkDestroySampler(g_ctx.device, g_ctx.linearToEdgeSampler, nullptr);
    vkDestroySampler(g_ctx.device, g_ctx.nearestRepeatSampler, nullptr);
    vkDestroySampler(g_ctx.device, g_ctx.nearestToEdgeSampler, nullptr);
    vkDestroyPipelineLayout(g_ctx.device, g_ctx.pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(g_ctx.device, g_ctx.descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(g_ctx.device, g_ctx.descriptorPool, nullptr);
    vkDestroyCommandPool(g_ctx.device, g_ctx.transferCommandPool, nullptr);
    vkDestroyDevice(g_ctx.device, nullptr);

    vkDestroySurfaceKHR(g_ctx.instance, g_ctx.surface, nullptr);
    #ifdef AR_ENABLE_INFO_CALLBACK
    if (g_ctx.messenger.handle)
    {
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(g_ctx.instance, "vkDestroyDebugUtilsMessengerEXT")
        )(g_ctx.instance, g_ctx.messenger.handle, nullptr);
    }
    #endif
    vkDestroyInstance(g_ctx.instance, nullptr);

    AR_INFO_CALLBACK("Destroyed Context")
}

static auto arContext::createSwapchain() noexcept -> void
{
    arContext::teardownSwapchain();

    VkSurfaceFormatKHR formats[64];
    VkPresentModeKHR presentModes[6];
    VkSurfaceCapabilitiesKHR capabilities;
    VkCompositeAlphaFlagBitsKHR compositeAlpha;

    u32_t minImageCount;
    u32_t formatCount;
    u32_t presentModeCount;

    arContext::resultCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_ctx.physicalDevice, g_ctx.surface, &capabilities));
    arContext::resultCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(g_ctx.physicalDevice, g_ctx.surface, &formatCount, nullptr));
    arContext::resultCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(g_ctx.physicalDevice, g_ctx.surface, &formatCount, formats));
    arContext::resultCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(g_ctx.physicalDevice, g_ctx.surface, &presentModeCount, nullptr));
    arContext::resultCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(g_ctx.physicalDevice, g_ctx.surface, &presentModeCount, presentModes));

    if (capabilities.currentExtent.width == 0xffffffffu)
    {
        g_ctx.surfaceExtent.width = static_cast<u32_t>(g_wnd.width);
        g_ctx.surfaceExtent.height = static_cast<u32_t>(g_wnd.height);

        if (g_ctx.surfaceExtent.width > capabilities.maxImageExtent.width)
        {
            g_ctx.surfaceExtent.width = capabilities.maxImageExtent.width;
        }

        if (g_ctx.surfaceExtent.height > capabilities.maxImageExtent.height)
        {
            g_ctx.surfaceExtent.height = capabilities.maxImageExtent.height;
        }

        if (g_ctx.surfaceExtent.width < capabilities.minImageExtent.width)
        {
            g_ctx.surfaceExtent.width = capabilities.minImageExtent.width;
        }

        if (g_ctx.surfaceExtent.height < capabilities.minImageExtent.height)
        {
            g_ctx.surfaceExtent.height = capabilities.minImageExtent.height;
        }
    }
    else
    {
        g_ctx.surfaceExtent = capabilities.currentExtent;
    }

    minImageCount = g_minImageCount<u32_t>;

    if (capabilities.minImageCount > g_minImageCount<u32_t>)
    {
        minImageCount = capabilities.minImageCount;
    }

    if (capabilities.maxImageCount && capabilities.maxImageCount < g_minImageCount<u32_t>)
    {
        minImageCount = capabilities.maxImageCount;
    }

    if (formatCount == 1u && formats[0].format == VK_FORMAT_UNDEFINED)
    {
        g_ctx.surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
        g_ctx.surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else
    {
        for (auto i{ formatCount }; i--; )
        {
            if ((formats[i].format == VK_FORMAT_R8G8B8A8_UNORM && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) ||
                (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            )
            {
                g_ctx.surfaceFormat = formats[i];
                break;
            }
        }

        if (g_ctx.surfaceFormat.format == VK_FORMAT_UNDEFINED)
        {
            g_ctx.surfaceFormat = formats[0];
        }
    }

    if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
    {
        compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    else if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
    {
        compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
    else
    {
        compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    }

    auto presentMode{ VK_PRESENT_MODE_FIFO_KHR };

    for (auto i{ presentModeCount }; i--; )
    {
        if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    auto const swapchainCI{ VkSwapchainCreateInfoKHR{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = g_ctx.surface,
        .minImageCount = minImageCount,
        .imageFormat = g_ctx.surfaceFormat.format,
        .imageColorSpace = g_ctx.surfaceFormat.colorSpace,
        .imageExtent = g_ctx.surfaceExtent,
        .imageArrayLayers = 1u,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = compositeAlpha,
        .presentMode = presentMode,
        .clipped = 1u
    }};

    arContext::resultCheck(vkCreateSwapchainKHR(g_ctx.device, &swapchainCI, nullptr, &g_ctx.swapchain));

    g_ctx.presentInfo = VkPresentInfoKHR{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1u,
        .swapchainCount = 1u,
        .pSwapchains = &g_ctx.swapchain
    };

    auto commandPoolCI{ VkCommandPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = g_ctx.graphicsFamily
    }};

    arContext::resultCheck(vkCreateCommandPool(g_ctx.device, &commandPoolCI, nullptr, &g_ctx.graphicsCommandPool));

    if (!g_ctx.unifiedPresent)
    {
        commandPoolCI.queueFamilyIndex = g_ctx.presentFamily;

        arContext::resultCheck(vkCreateCommandPool(g_ctx.device, &commandPoolCI, nullptr, &g_ctx.presentCommandPool));
    }

    VkImage images[g_maxImageCount<u32_t>];

    arContext::resultCheck(vkGetSwapchainImagesKHR(g_ctx.device, g_ctx.swapchain, &g_ctx.imageCount, nullptr));
    if (g_ctx.imageCount > g_maxImageCount<u32_t>)
        arContext::resultCheck(VK_ERROR_INITIALIZATION_FAILED);
    arContext::resultCheck(vkGetSwapchainImagesKHR(g_ctx.device, g_ctx.swapchain, &g_ctx.imageCount, images));

    AR_INFO_CALLBACK("Swapchain image count: [%u]", g_ctx.imageCount)

    VkCommandBuffer graphicsCommandBuffers[g_maxImageCount<u32_t>];
    VkCommandBuffer presentCommandBuffers[g_maxImageCount<u32_t>];

    auto commandBufferAI{ VkCommandBufferAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = g_ctx.graphicsCommandPool,
        .commandBufferCount = g_ctx.imageCount,
    }};

    arContext::resultCheck(vkAllocateCommandBuffers(g_ctx.device, &commandBufferAI, graphicsCommandBuffers));

    if (!g_ctx.unifiedPresent)
    {
        commandBufferAI.commandPool = g_ctx.presentCommandPool;

        arContext::resultCheck(vkAllocateCommandBuffers(g_ctx.device, &commandBufferAI, presentCommandBuffers));
    }

    for (auto i{ g_ctx.imageCount }; i--; )
    {
        g_ctx.images[i] = ArlineContext::SwapchainImage{
            .image = images[i],
            .view = arContext::createImageView(
                images[i],
                VK_IMAGE_VIEW_TYPE_2D,
                g_ctx.surfaceFormat.format,
                VK_IMAGE_ASPECT_COLOR_BIT,
                1u
            ),
            .graphicsCommandBuffer = graphicsCommandBuffers[i],
            .presentCommandBuffer = presentCommandBuffers[i]
        };

        if (!g_ctx.unifiedPresent)
        {
            auto const commandBufferBI{ VkCommandBufferBeginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
            }};

            auto const imageBarrier{ VkImageMemoryBarrier2{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = g_ctx.graphicsFamily,
                .dstQueueFamilyIndex = g_ctx.presentFamily,
                .image = images[i],
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = 1u,
                    .layerCount = 1u
                }
            }};

            auto const dependency{ VkDependencyInfo{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .imageMemoryBarrierCount = 1u,
                .pImageMemoryBarriers = &imageBarrier
            }};

            arContext::resultCheck(vkBeginCommandBuffer(g_ctx.images[i].presentCommandBuffer, &commandBufferBI));
            vkCmdPipelineBarrier2(g_ctx.images[i].presentCommandBuffer, &dependency);
            arContext::resultCheck(vkEndCommandBuffer(g_ctx.images[i].presentCommandBuffer));
        }
    }

    AR_INFO_CALLBACK("Created swapchain: width [%u], height [%u]", g_ctx.surfaceExtent.width, g_ctx.surfaceExtent.height)
}

static auto arContext::teardownSwapchain() noexcept -> void
{
    arContext::resultCheck(vkDeviceWaitIdle(g_ctx.device));

    if (g_ctx.swapchain)
    {
        vkDestroySwapchainKHR(g_ctx.device, g_ctx.swapchain, nullptr);

        for (auto& image : g_ctx.images)
        {
            if (image.view)
            {
                vkDestroyImageView(g_ctx.device, image.view, nullptr);
                image.view = nullptr;
            }
        }

        if (!g_ctx.unifiedPresent)
        vkDestroyCommandPool(g_ctx.device, g_ctx.presentCommandPool, nullptr);
        vkDestroyCommandPool(g_ctx.device, g_ctx.graphicsCommandPool, nullptr);

        g_ctx.swapchain = nullptr;
        g_ctx.surfaceFormat = {};
    }
}

static auto arContext::beginTransfer() noexcept -> void
{
    auto const beginInfo{ VkCommandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    }};

    arContext::resultCheck(vkResetCommandPool(g_ctx.device, g_ctx.transferCommandPool, {}));
    arContext::resultCheck(vkBeginCommandBuffer(g_ctx.transferCommandBuffer, &beginInfo));
}

static auto arContext::endTransfer() noexcept -> void
{
    arContext::resultCheck(vkEndCommandBuffer(g_ctx.transferCommandBuffer));

    auto const submitInfo{ VkSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1u,
        .pCommandBuffers = &g_ctx.transferCommandBuffer
    }};

    arContext::resultCheck(vkQueueSubmit(g_ctx.graphicsQueue, 1u, &submitInfo, nullptr));
    arContext::resultCheck(vkQueueWaitIdle(g_ctx.graphicsQueue));
}

static auto arContext::createImageView(
    VkImage image,
    VkImageViewType type,
    VkFormat format,
    VkImageAspectFlags aspect,
    u32_t mipLevels
) noexcept -> VkImageView
{
    auto const viewCI{ VkImageViewCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = type,
        .format = format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_R,
            .g = VK_COMPONENT_SWIZZLE_G,
            .b = VK_COMPONENT_SWIZZLE_B,
            .a = VK_COMPONENT_SWIZZLE_A
        },
        .subresourceRange = {
            .aspectMask = aspect,
            .levelCount = mipLevels,
            .layerCount = 1u
        }
    }};

    VkImageView view;
    arContext::resultCheck(vkCreateImageView(g_ctx.device, &viewCI, nullptr, &view));

    return view;
}

static auto arContext::resultCheck(VkResult result) noexcept -> void
{
    if (result) [[unlikely]]
    {
        auto const message{ [](VkResult result)
        {
            switch (result)
            {
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
            case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
                return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            default:
                return "VK_ERROR_UNKNOWN";
            }
        }};

        g_errorCallback(message(result));
    }
}

#pragma endregion
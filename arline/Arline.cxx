#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#ifdef _WIN32
#   define VK_USE_PLATFORM_WIN32_KHR
#   define NOMINMAX
#   include <dwmapi.h>
#   pragma comment(lib, "dwmapi")
#else
#   error Unsupported platform
#endif

#include "Arline.hxx"

using namespace ar::types;

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
        b8 enableValidationLayers;
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

    b8 vsyncEnabled;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    u32 graphicsFamily;
    u32 presentFamily;
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

    u32 imageCount;
    u32 unifiedPresent;
    u32 imageIndex;
    u32 cmdIndex;
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

    SwapchainImage images[g_maxImageCount<u32>];
};

struct ArlineWindow
{
    HWND hwnd;
    HINSTANCE hinstance;
    s32 width;
    s32 height;
    b8 available;
};

struct ArlineTimer
{
    LARGE_INTEGER timeOffset;
    LARGE_INTEGER frequency;
    f64 previousTime;
    f64 deltaTime;
};

struct ArlineKey
{
    b8 isPressed  : 1;
    b8 isReleased : 1;
};

struct ArlineKeyboard
{
    ArlineKey keys[static_cast<u8>(ar::Key::eMenu) + 1];
    b8 keysDown[static_cast<u8>(ar::Key::eMenu) + 1];
};

struct ArlineButton
{
    b8 isDown     : 1;
    b8 isPressed  : 1;
    b8 isReleased : 1;
};

struct ArlineMouse
{
    s32 deltaX, deltaY;
    s32 globPosX, globPosY;
    s32 posX, posY;
    ArlineButton buttons[5];
};

#ifdef AR_ENABLE_INFO_CALLBACK
static void (*g_infoCallback)(char const*){};
#endif
static void (*g_errorCallback)(char const*){};
static ArlineContext g_ctx{};
static ArlineWindow g_wnd{};
static ArlineTimer g_timer{};
static ArlineMouse g_mouse{};
static ArlineKeyboard g_keyboard{};
static u8 g_vkToKey[255]{};

#ifdef AR_ENABLE_INFO_CALLBACK
#define AR_INFO_CALLBACK(f, ...) \
{ char msg[1024]; std::snprintf(msg, sizeof(msg), f, __VA_ARGS__); g_infoCallback(msg); }
#else
#define AR_INFO_CALLBACK(format, ...)
#endif

namespace arTimer
{
    static void create() noexcept;
    static void update() noexcept;
}

namespace arWindow
{
    static void create(ar::AppInfo const&) noexcept;
    static void teardown() noexcept;
}

namespace arContext
{
    static void create() noexcept;
    static void teardown() noexcept;
    static b8 acquireImage() noexcept;
    static b8 presentFrame() noexcept;
    static void createSwapchain(b8) noexcept;
    static void teardownSwapchain() noexcept;
    static void resultCheck(VkResult) noexcept;
    static void beginTransfer() noexcept;
    static void endTransfer() noexcept;
    static VkPipelineStageFlags2 layoutToStage(ar::ImageLayout) noexcept;
    static VkAccessFlags2 layoutToAccess(ar::ImageLayout) noexcept;
    static VkImageView createImageView(VkImage, VkImageViewType, VkFormat, VkImageAspectFlags, u32) noexcept;
}

#pragma endregion
#pragma region Execution

void ar::execute(AppInfo&& info) noexcept
{
    if (!info.onResize)
        info.onResize = []{};

    if (!info.onResourcesUpdate)
        info.onResourcesUpdate = []{ return Request::eNone; };

    if (!info.onUpdate)
        info.onUpdate = []{ ar::pollEvents(); };

    if (!info.infoCallback)
        info.infoCallback = [](char const*){};

    if (!info.errorCallback)
        info.errorCallback = [](char const*){};

    g_ctx = ArlineContext{
        #ifdef AR_ENABLE_INFO_CALLBACK
        .messenger = {
            .enableValidationLayers = info.enableValidationLayers
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
    g_infoCallback = info.infoCallback;
    #endif
    g_errorCallback = info.errorCallback;

    arWindow::create(info);
    arContext::create();
    arContext::createSwapchain(info.enalbeVsync);
    arTimer::create();

    info.onInit();

    auto record{ [&info]
    {
        auto const commandBufferBI{ VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        }};

        arContext::resultCheck(vkResetCommandPool(g_ctx.device, g_ctx.graphicsCommandPool, {}));

        for (g_ctx.cmdIndex = u32{}; g_ctx.cmdIndex < g_ctx.imageCount; ++g_ctx.cmdIndex)
        {
            arContext::resultCheck(
                vkBeginCommandBuffer(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer, &commandBufferBI)
            );

            vkCmdBindDescriptorSets(
                g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                g_ctx.pipelineLayout,
                0u,
                1u,
                &g_ctx.descriptorSet,
                0u,
                nullptr
            );

            info.onCommandsRecord(ar::GraphicsCommands{});

            arContext::resultCheck(
                vkEndCommandBuffer(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer)
            );
        }
    }};
    
    record();

    while (g_wnd.available) [[likely]]
    {
        if (!arContext::acquireImage()) [[unlikely]]
        {
            info.onResize();
            record();
        }

        switch (info.onResourcesUpdate())
        {
        [[likely]] case Request::eNone:
            break;
        [[unlikely]] case Request::eDisableVsync:
            arContext::createSwapchain(false);
            [[fallthrough]];
        [[unlikely]] case Request::eRecordCommands:
            record();
            break;
        [[unlikely]] case Request::eEnableVsync:
            arContext::createSwapchain(true);
            record();
            break;
        }

        if (!arContext::presentFrame()) [[unlikely]]
        {
            info.onResize();
            record();
        }

        info.onUpdate();
        arTimer::update();
    }

    vkDeviceWaitIdle(g_ctx.device);
    info.onDestroy();
    arContext::teardown();
    arWindow::teardown();
}

#pragma endregion
#pragma region Window


static LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept
{
    switch (msg)
    {
    [[unlikely]] case WM_SIZE:
    {
        g_wnd.width  = static_cast<u32>(static_cast<u64>(lp) & 0xffff);
        g_wnd.height = static_cast<u32>(static_cast<u64>(lp) >> 16);

        while (!g_wnd.width || !g_wnd.height)
        {
            ar::waitEvents();

            if (!g_wnd.available)
                break;
        }

        break;
    }
    [[unlikely]] case WM_GETMINMAXINFO:
    {
        reinterpret_cast<PMINMAXINFO>(lp)->ptMinTrackSize.x = 300;
        reinterpret_cast<PMINMAXINFO>(lp)->ptMinTrackSize.y = 150;
        return LRESULT{};
    }
    case WM_LBUTTONDOWN:
    {
        g_mouse.buttons[static_cast<u8>(ar::Button::eLeft)].isDown = true;
        g_mouse.buttons[static_cast<u8>(ar::Button::eLeft)].isPressed = true;
        break;
    }
    case WM_LBUTTONUP:
    {
        g_mouse.buttons[static_cast<u8>(ar::Button::eLeft)].isDown = false;
        g_mouse.buttons[static_cast<u8>(ar::Button::eLeft)].isReleased = true;
        break;
    }
    case WM_RBUTTONDOWN:
    {
        g_mouse.buttons[static_cast<u8>(ar::Button::eRight)].isDown = true;
        g_mouse.buttons[static_cast<u8>(ar::Button::eRight)].isPressed = true;
        break;
    }
    case WM_RBUTTONUP:
    {
        g_mouse.buttons[static_cast<u8>(ar::Button::eRight)].isDown = false;
        g_mouse.buttons[static_cast<u8>(ar::Button::eRight)].isReleased = true;
        break;
    }
    [[unlikely]] case WM_MBUTTONDOWN:
    {
        g_mouse.buttons[static_cast<u8>(ar::Button::eMiddle)].isDown = true;
        g_mouse.buttons[static_cast<u8>(ar::Button::eMiddle)].isPressed = true;
        break;
    }
    [[unlikely]] case WM_MBUTTONUP:
    {
        g_mouse.buttons[static_cast<u8>(ar::Button::eMiddle)].isDown = false;
        g_mouse.buttons[static_cast<u8>(ar::Button::eMiddle)].isReleased = true;
        break;
    }
    [[unlikely]] case WM_XBUTTONDOWN:
    {
        g_mouse.buttons[static_cast<u8>(ar::Button::eMiddle) + HIWORD(wp)].isDown = true;
        g_mouse.buttons[static_cast<u8>(ar::Button::eMiddle) + HIWORD(wp)].isPressed = true;
        break;   
    }
    [[unlikely]] case WM_XBUTTONUP:
    {
        g_mouse.buttons[static_cast<u8>(ar::Button::eMiddle) + HIWORD(wp)].isDown = false;
        g_mouse.buttons[static_cast<u8>(ar::Button::eMiddle) + HIWORD(wp)].isReleased = true;
        break;
    }
    case WM_SYSKEYDOWN: [[fallthrough]];
    case WM_KEYDOWN:
    {
        u8 key = g_vkToKey[wp];

        g_keyboard.keys[key].isPressed = !(lp & (1 << 30)); 
        g_keyboard.keysDown[key] = true;

        // Because Windows
        if (wp == VK_MENU)
            return 1;

        break;
    }
    case WM_SYSKEYUP: [[fallthrough]];
    case WM_KEYUP:
    {
        u8 key = g_vkToKey[wp];

        g_keyboard.keys[key].isReleased = true;
        g_keyboard.keysDown[key] = false;

        // Because Windows
        if (wp == VK_MENU)
            return 1;

        break;
    }
    [[unlikely]] case WM_DESTROY:
    {
        ::PostQuitMessage(0);
        return LRESULT{ 0 };
    }}

    return ::DefWindowProcW(hwnd, msg, wp, lp);
}

static void arWindow::create(ar::AppInfo const& config) noexcept
{
    auto const error{ "Window creation failed" };

    g_vkToKey[+'A']             = static_cast<u8>(ar::Key::eA);
    g_vkToKey[+'B']             = static_cast<u8>(ar::Key::eB);
    g_vkToKey[+'C']             = static_cast<u8>(ar::Key::eC);
    g_vkToKey[+'D']             = static_cast<u8>(ar::Key::eD);
    g_vkToKey[+'E']             = static_cast<u8>(ar::Key::eE);
    g_vkToKey[+'F']             = static_cast<u8>(ar::Key::eF);
    g_vkToKey[+'G']             = static_cast<u8>(ar::Key::eG);
    g_vkToKey[+'H']             = static_cast<u8>(ar::Key::eH);
    g_vkToKey[+'I']             = static_cast<u8>(ar::Key::eI);
    g_vkToKey[+'J']             = static_cast<u8>(ar::Key::eJ);
    g_vkToKey[+'K']             = static_cast<u8>(ar::Key::eK);
    g_vkToKey[+'L']             = static_cast<u8>(ar::Key::eL);
    g_vkToKey[+'M']             = static_cast<u8>(ar::Key::eM);
    g_vkToKey[+'N']             = static_cast<u8>(ar::Key::eN);
    g_vkToKey[+'O']             = static_cast<u8>(ar::Key::eO);
    g_vkToKey[+'P']             = static_cast<u8>(ar::Key::eP);
    g_vkToKey[+'Q']             = static_cast<u8>(ar::Key::eQ);
    g_vkToKey[+'R']             = static_cast<u8>(ar::Key::eR);
    g_vkToKey[+'S']             = static_cast<u8>(ar::Key::eS);
    g_vkToKey[+'T']             = static_cast<u8>(ar::Key::eT);
    g_vkToKey[+'U']             = static_cast<u8>(ar::Key::eU);
    g_vkToKey[+'V']             = static_cast<u8>(ar::Key::eV);
    g_vkToKey[+'W']             = static_cast<u8>(ar::Key::eW);
    g_vkToKey[+'X']             = static_cast<u8>(ar::Key::eX);
    g_vkToKey[+'Y']             = static_cast<u8>(ar::Key::eY);
    g_vkToKey[+'Z']             = static_cast<u8>(ar::Key::eZ);
    g_vkToKey[+'0']             = static_cast<u8>(ar::Key::e0);
    g_vkToKey[+'1']             = static_cast<u8>(ar::Key::e1);
    g_vkToKey[+'2']             = static_cast<u8>(ar::Key::e2);
    g_vkToKey[+'3']             = static_cast<u8>(ar::Key::e3);
    g_vkToKey[+'4']             = static_cast<u8>(ar::Key::e4);
    g_vkToKey[+'5']             = static_cast<u8>(ar::Key::e5);
    g_vkToKey[+'6']             = static_cast<u8>(ar::Key::e6);
    g_vkToKey[+'7']             = static_cast<u8>(ar::Key::e7);
    g_vkToKey[+'8']             = static_cast<u8>(ar::Key::e8);
    g_vkToKey[+'9']             = static_cast<u8>(ar::Key::e9);
    g_vkToKey[VK_F1]            = static_cast<u8>(ar::Key::eF1);
    g_vkToKey[VK_F2]            = static_cast<u8>(ar::Key::eF2);
    g_vkToKey[VK_F3]            = static_cast<u8>(ar::Key::eF3);
    g_vkToKey[VK_F4]            = static_cast<u8>(ar::Key::eF4);
    g_vkToKey[VK_F5]            = static_cast<u8>(ar::Key::eF5);
    g_vkToKey[VK_F6]            = static_cast<u8>(ar::Key::eF6);
    g_vkToKey[VK_F7]            = static_cast<u8>(ar::Key::eF7);
    g_vkToKey[VK_F8]            = static_cast<u8>(ar::Key::eF8);
    g_vkToKey[VK_F9]            = static_cast<u8>(ar::Key::eF9);
    g_vkToKey[VK_F10]           = static_cast<u8>(ar::Key::eF10);
    g_vkToKey[VK_F11]           = static_cast<u8>(ar::Key::eF11);
    g_vkToKey[VK_F12]           = static_cast<u8>(ar::Key::eF12);
    g_vkToKey[VK_SPACE]         = static_cast<u8>(ar::Key::eSpace);      
    g_vkToKey[VK_OEM_7]         = static_cast<u8>(ar::Key::eApostrophe); 
    g_vkToKey[VK_OEM_COMMA]     = static_cast<u8>(ar::Key::eComma);      
    g_vkToKey[VK_OEM_MINUS]     = static_cast<u8>(ar::Key::eMinus);      
    g_vkToKey[VK_OEM_PERIOD]    = static_cast<u8>(ar::Key::ePeriod);     
    g_vkToKey[VK_OEM_2]         = static_cast<u8>(ar::Key::eSlash);      
    g_vkToKey[VK_OEM_1]         = static_cast<u8>(ar::Key::eSemicolon);  
    g_vkToKey[VK_OEM_PLUS]      = static_cast<u8>(ar::Key::ePlus);       
    g_vkToKey[VK_OEM_4]         = static_cast<u8>(ar::Key::eLBracket);   
    g_vkToKey[VK_OEM_6]         = static_cast<u8>(ar::Key::eRBracket);   
    g_vkToKey[VK_OEM_5]         = static_cast<u8>(ar::Key::eBackslash);  
    g_vkToKey[VK_OEM_3]         = static_cast<u8>(ar::Key::eGraveAccent);
    g_vkToKey[VK_ESCAPE]        = static_cast<u8>(ar::Key::eEscape);     
    g_vkToKey[VK_TAB]           = static_cast<u8>(ar::Key::eTab);        
    g_vkToKey[VK_BACK]          = static_cast<u8>(ar::Key::eBackspace);  
    g_vkToKey[VK_INSERT]        = static_cast<u8>(ar::Key::eInsert);     
    g_vkToKey[VK_DELETE]        = static_cast<u8>(ar::Key::eDelete);     
    g_vkToKey[VK_RIGHT]         = static_cast<u8>(ar::Key::eRight);      
    g_vkToKey[VK_LEFT]          = static_cast<u8>(ar::Key::eLeft);       
    g_vkToKey[VK_DOWN]          = static_cast<u8>(ar::Key::eDown);       
    g_vkToKey[VK_UP]            = static_cast<u8>(ar::Key::eUp);         
    g_vkToKey[VK_NEXT]          = static_cast<u8>(ar::Key::ePageDown);   
    g_vkToKey[VK_PRIOR]         = static_cast<u8>(ar::Key::ePageUp);     
    g_vkToKey[VK_HOME]          = static_cast<u8>(ar::Key::eHome);       
    g_vkToKey[VK_END]           = static_cast<u8>(ar::Key::eEnd);        
    g_vkToKey[VK_CAPITAL]       = static_cast<u8>(ar::Key::eCapsLock);   
    g_vkToKey[VK_SCROLL]        = static_cast<u8>(ar::Key::eScrollLock); 
    g_vkToKey[VK_SNAPSHOT]      = static_cast<u8>(ar::Key::ePrintScreen);
    g_vkToKey[VK_PAUSE]         = static_cast<u8>(ar::Key::ePause);      
    g_vkToKey[VK_APPS]          = static_cast<u8>(ar::Key::eMenu);       
    g_vkToKey[VK_SHIFT]         = static_cast<u8>(ar::Key::eShift);       
    g_vkToKey[VK_CONTROL]       = static_cast<u8>(ar::Key::eCtrl);       
    g_vkToKey[VK_MENU]          = static_cast<u8>(ar::Key::eAlt);       

    g_wnd = ArlineWindow{
        .hinstance = ::GetModuleHandleA(nullptr),
        .width = config.width,
        .height = config.height,
        .available = true
    };

    auto const wc{ WNDCLASSEXW{
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_OWNDC,
        .lpfnWndProc = windowProc,
        .hInstance = g_wnd.hinstance,
        .hCursor = ::LoadCursorA(nullptr, IDC_ARROW),
        .lpszClassName = L"ar",
    }};

    if (!::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) || !::RegisterClassExW(&wc))
    {
        g_errorCallback(error);
    }

    g_wnd.hwnd = ::CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"ar",
        L"",
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

    AR_INFO_CALLBACK("Created Win32 Window: width [%d], height [%d]", config.width, config.height)
}

static void arWindow::teardown() noexcept
{
    ::DestroyWindow(g_wnd.hwnd);
    ::UnregisterClassW(L"ar", g_wnd.hinstance);

    AR_INFO_CALLBACK("Destroyed Win32 Window")
}

void ar::pollEvents() noexcept
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
            g_wnd.available = false;
    }

    POINT cursorPos;
    ::GetPhysicalCursorPos(&cursorPos);

    g_mouse.globPosX = cursorPos.x;
    g_mouse.globPosY = cursorPos.y;

    ::ScreenToClient(g_wnd.hwnd, &cursorPos);

    g_mouse.deltaX = cursorPos.x - g_mouse.posX;
    g_mouse.deltaY = cursorPos.y - g_mouse.posY;

    g_mouse.posX = cursorPos.x;
    g_mouse.posY = cursorPos.y;
}

void ar::waitEvents()                       noexcept { ::WaitMessage(); ar::pollEvents();                                               }
void ar::setTitle(char const* title)        noexcept { ::SetWindowTextA(g_wnd.hwnd, title);                                             }
void ar::messageBoxError(char const* error) noexcept { ::MessageBoxA(nullptr, error, "Error", MB_ICONERROR);                            }
s32 ar::getWidth()                          noexcept { return g_wnd.width;                                                              }
s32 ar::getHeight()                         noexcept { return g_wnd.height;                                                             }
u32 ar::getFramebufferWidth()               noexcept { return g_ctx.surfaceExtent.width;                                                }
u32 ar::getFramebufferHeight()              noexcept { return g_ctx.surfaceExtent.height;                                               }
f32 ar::getAspectRatio()                    noexcept { return static_cast<f32>(g_wnd.width) / g_wnd.height;                             }
f32 ar::getFramebufferAspectRatio()         noexcept { return static_cast<f32>(g_ctx.surfaceExtent.width) / g_ctx.surfaceExtent.height; }

#pragma endregion
#pragma region Time

static void arTimer::create() noexcept
{
    ::QueryPerformanceFrequency(&g_timer.frequency);
    ::QueryPerformanceCounter(&g_timer.timeOffset);
}

f64 ar::getTime() noexcept
{
    LARGE_INTEGER value;
    ::QueryPerformanceCounter(&value);

    return static_cast<f64>(value.QuadPart - g_timer.timeOffset.QuadPart) / g_timer.frequency.QuadPart;
}

static void arTimer::update() noexcept
{
    auto now{ ar::getTime() };
    g_timer.deltaTime = now - g_timer.previousTime;
    g_timer.previousTime = now;
}

f32 ar::getTimef()      noexcept { return static_cast<f32>(ar::getTime());     }
f64 ar::getDeltaTime()  noexcept { return g_timer.deltaTime;                   }
f32 ar::getDeltaTimef() noexcept { return static_cast<f32>(g_timer.deltaTime); }

#pragma endregion
#pragma region Input

b8 ar::isKeyPressed(Key key)             noexcept { return g_keyboard.keys[static_cast<u8>(key)].isPressed;     }
b8 ar::isKeyReleased(Key key)            noexcept { return g_keyboard.keys[static_cast<u8>(key)].isReleased;    }
b8 ar::isKeyDown(Key key)                noexcept { return  g_keyboard.keysDown[static_cast<u8>(key)];          }
b8 ar::isKeyUp(Key key)                  noexcept { return !g_keyboard.keysDown[static_cast<u8>(key)];          }
b8 ar::isButtonPressed(Button button)    noexcept { return g_mouse.buttons[static_cast<u8>(button)].isPressed;  }
b8 ar::isButtonReleased(Button button)   noexcept { return g_mouse.buttons[static_cast<u8>(button)].isReleased; }
b8 ar::isButtonDown(Button button)       noexcept { return  g_mouse.buttons[static_cast<u8>(button)].isDown;    }
b8 ar::isButtonUp(Button button)         noexcept { return !g_mouse.buttons[static_cast<u8>(button)].isDown;    }
s32 ar::getGlobalCursorPositionX()       noexcept { return g_mouse.globPosX;                                    }
s32 ar::getGlobalCursorPositionY()       noexcept { return g_mouse.globPosY;                                    }
s32 ar::getCursorPositionX()             noexcept { return g_mouse.posX;                                        }
s32 ar::getCursorPositionY()             noexcept { return g_mouse.posY;                                        }
s32 ar::getCursorDeltaX()                noexcept { return g_mouse.deltaX;                                      }
s32 ar::getCursorDeltaY()                noexcept { return g_mouse.deltaY;                                      }
void ar::setCursorPosition(s32 x, s32 y) noexcept { ::SetCursorPos(x, y);                                       }
void ar::showCursor()                    noexcept { while (::ShowCursor(1) <  0);                               }
void ar::hideCursor()                    noexcept { while (::ShowCursor(0) >= 0);                               }

#pragma endregion
#pragma region Graphics Commands

void ar::GraphicsCommands::barrier(ImageBarrier barrier) noexcept
{
    VkImageAspectFlags constexpr aspects[] = { VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_ASPECT_DEPTH_BIT };
    auto usingDepth{ false };

    usingDepth = static_cast<b8>(usingDepth + (barrier.oldLayout == ar::ImageLayout::eDepthAttachment));
    usingDepth = static_cast<b8>(usingDepth + (barrier.newLayout == ar::ImageLayout::eDepthAttachment));  

    auto const imageBarrier{ VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = arContext::layoutToStage(barrier.oldLayout),
        .srcAccessMask = arContext::layoutToAccess(barrier.oldLayout),
        .dstStageMask = arContext::layoutToStage(barrier.newLayout),
        .dstAccessMask = arContext::layoutToAccess(barrier.newLayout),
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

    auto const dependency{ VkDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        .imageMemoryBarrierCount = 1u,
        .pImageMemoryBarriers = &imageBarrier
    }};

    vkCmdPipelineBarrier2(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer, &dependency);
}

void ar::GraphicsCommands::beginRendering(std::initializer_list<ColorAttachment> colorAttachments, DepthAttachment depthAttachment) noexcept
{
    VkRenderingAttachmentInfo attachments[9];

    for (auto i{ size_t{} }; i < colorAttachments.size(); ++i)
    {
        auto* attachment{ colorAttachments.begin() + i };

        attachments[i] = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = attachment->image.view,
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
            },
        };

        if (attachment->pResolve)
        {
            attachments[i].resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            attachments[i].resolveImageView = attachment->pResolve->view;
            attachments[i].resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }

    if (depthAttachment.pImage)
    {
        attachments[8] = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = depthAttachment.pImage->view,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .loadOp = static_cast<VkAttachmentLoadOp>(depthAttachment.loadOp),
            .storeOp = static_cast<VkAttachmentStoreOp>(depthAttachment.storeOp),
            .clearValue = {
                .depthStencil = {
                    .depth = 1.f
                }
            }
        };

        if (depthAttachment.pResolve)
        {
            attachments[8].resolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
            attachments[8].resolveImageView = depthAttachment.pResolve->view;
            attachments[8].resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        }
    }

    VkExtent2D extent;

    if (colorAttachments.size())
    {
        extent.width = colorAttachments.begin()->image.width;
        extent.height = colorAttachments.begin()->image.height;
    }
    else
    {
        extent.width = depthAttachment.pImage->width;
        extent.height = depthAttachment.pImage->height;
    }

    auto const renderingInfo{ VkRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {
            .extent = extent
        },
        .layerCount = 1u,
        .colorAttachmentCount = static_cast<u32>(colorAttachments.size()),
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
        .width = static_cast<f32>(renderingInfo.renderArea.extent.width),
        .height = static_cast<f32>(renderingInfo.renderArea.extent.height),
        .maxDepth = 1.f
    }};

    vkCmdBeginRendering(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer, &renderingInfo);
    vkCmdSetScissor(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer, 0u, 1u, &scissor);
    vkCmdSetViewport(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer, 0u, 1u, &viewport);
}

void ar::GraphicsCommands::endRendering() noexcept
{
    vkCmdEndRendering(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer);
}

void ar::GraphicsCommands::beginPresent() noexcept
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
        .image = g_ctx.images[g_ctx.cmdIndex].image,
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
        .imageView = g_ctx.images[g_ctx.cmdIndex].view,
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
        .width = static_cast<f32>(g_ctx.surfaceExtent.width),
        .height = static_cast<f32>(g_ctx.surfaceExtent.height),
        .maxDepth = 1.f
    }};

    vkCmdPipelineBarrier2(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer, &dependency);
    vkCmdBeginRendering(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer, &renderingInfo);
    vkCmdSetScissor(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer, 0u, 1u, &scissor);
    vkCmdSetViewport(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer, 0u, 1u, &viewport);
}

void ar::GraphicsCommands::endPresent() noexcept
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
            .image = g_ctx.images[g_ctx.cmdIndex].image,
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
            .dstStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = g_ctx.graphicsFamily,
            .dstQueueFamilyIndex = g_ctx.presentFamily,
            .image = g_ctx.images[g_ctx.cmdIndex].image,
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

    vkCmdEndRendering(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer);
    vkCmdPipelineBarrier2(g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer, &dependency);
}

void ar::GraphicsCommands::bindPipeline(Pipeline const& pipeline) noexcept
{
    vkCmdBindPipeline(
        g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.handle
    );
}

void ar::GraphicsCommands::bindIndexBuffer16(Buffer const& buffer) noexcept
{
    vkCmdBindIndexBuffer(
        g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
        buffer.handle,
        0ull,
        VK_INDEX_TYPE_UINT16   
    );
}

void ar::GraphicsCommands::bindIndexBuffer32(Buffer const& buffer) noexcept
{
     vkCmdBindIndexBuffer(
        g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
        buffer.handle,
        0ull,
        VK_INDEX_TYPE_UINT32 
    );
}

void ar::GraphicsCommands::draw(u32 vertexCount, u32 instanceCount, u32 vertex, u32 instance) noexcept
{
    vkCmdDraw(
        g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
        vertexCount,
        instanceCount,
        vertex,
        instance
    );
}

void ar::GraphicsCommands::drawIndexed(u32 indexCount, u32 instanceCount, u32 index, s32 vertexOffset, u32 instance) noexcept
{
    vkCmdDrawIndexed(
        g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
        indexCount,
        instanceCount,
        index,
        vertexOffset,
        instance
    );
}

void ar::GraphicsCommands::drawIndirect(Buffer const& buffer, u32 drawCount, u32 stride) noexcept
{
    vkCmdDrawIndirect(
        g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
        buffer.handle,
        0ull,
        drawCount,
        stride
    );
}

void ar::GraphicsCommands::drawIndexedIndirect(Buffer const& buffer, u32 drawCount, u32 stride) noexcept
{
    vkCmdDrawIndexedIndirect(
        g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
        buffer.handle,
        0ull,
        drawCount,
        stride
    );
}

void ar::GraphicsCommands::drawIndirectCount(Buffer const& buffer, Buffer const& countBuffer, u32 maxDraws, u32 stride) noexcept
{
    vkCmdDrawIndirectCount(
        g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
        buffer.handle,
        0ull,
        countBuffer.handle,
        0ull,
        maxDraws,
        stride
    );
}

void ar::GraphicsCommands::drawIndexedIndirectCount(Buffer const& buffer, Buffer const& countBuffer, u32 maxDraws, u32 stride) noexcept
{
    vkCmdDrawIndexedIndirectCount(
        g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
        buffer.handle,
        0ull,
        countBuffer.handle,
        0ull,
        maxDraws,
        stride
    );
}

void ar::GraphicsCommands::pushConstant(void const* pData, u32 size) noexcept
{
    vkCmdPushConstants(
        g_ctx.images[g_ctx.cmdIndex].graphicsCommandBuffer,
        g_ctx.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0u,
        size,
        pData
    );
}

#pragma endregion
#pragma region Shaders

void ar::Shader::create(char const* path, Constants const& constants) noexcept
{
    auto error{ [&]
    {
        char errorMessage[1024];
        std::snprintf(errorMessage, sizeof(errorMessage), "Failed to load shader: [%s]", path);

        g_errorCallback(errorMessage);
    }};

    VkShaderStageFlagBits stageFlagBits;

    auto pathStrLen{ strlen(path) };
    if (!strcmp(path + pathStrLen - 8, "vert.spv"))
    {
        stageFlagBits = VK_SHADER_STAGE_VERTEX_BIT;
    }
    else if (!strcmp(path + pathStrLen - 8, "frag.spv"))
    {
        stageFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    else
    {
        stageFlagBits = {};
        error();
    }

    spec = {};

    if (constants.entries.size())
    {
        memcpy(entries, constants.entries.begin(), constants.entries.size() * sizeof(MapEntry));

        for (auto const& entry : constants.entries)
        {
            spec.dataSize += entry.size;
        }

        spec.pData = constants.pData;
        spec.mapEntryCount = static_cast<u32>(constants.entries.size());
        spec.pMapEntries = entries;
    }

    shaderStage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stageFlagBits,
        .pName = "main",
        .pSpecializationInfo = &spec
    };

    auto const file{ ::CreateFileA(
        path,
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

    auto pBuffer{ new u8[fileSize.QuadPart] };
    auto bytesRead{ DWORD{} };

    if (!::ReadFile(file, pBuffer, fileSize.LowPart, &bytesRead, nullptr))
    {
        ::CloseHandle(file);
        error();
    }

    ::CloseHandle(file);

    auto const shaderModuleCI{ VkShaderModuleCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = static_cast<size_t>(fileSize.QuadPart),
        .pCode = reinterpret_cast<u32*>(pBuffer)
    }};

    arContext::resultCheck(vkCreateShaderModule(
        g_ctx.device,
        &shaderModuleCI,
        nullptr,
        &shaderStage.module
    ));

    delete[] pBuffer;
}

void ar::Shader::create(u32 const* pSpirv, size_t size, ShaderStage stage, Constants const& constants) noexcept
{
    spec = {};

    if (constants.entries.size())
    {
        memcpy(entries, constants.entries.begin(), constants.entries.size() * sizeof(MapEntry));

        for (auto const& entry : constants.entries)
        {
            spec.dataSize += entry.size;
        }

        spec.pData = constants.pData;
        spec.mapEntryCount = static_cast<u32>(constants.entries.size());
        spec.pMapEntries = entries;
    }

    shaderStage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = static_cast<VkShaderStageFlagBits>(stage),
        .pName = "main",
        .pSpecializationInfo = &spec
    };

    auto const shaderModuleCI{ VkShaderModuleCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = pSpirv
    }};

    arContext::resultCheck(vkCreateShaderModule(
        g_ctx.device,
        &shaderModuleCI,
        nullptr,
        &shaderStage.module
    ));
}

void ar::Shader::destroy() noexcept
{
    vkDestroyShaderModule(g_ctx.device, shaderStage.module, nullptr);
}

#pragma endregion
#pragma region Pipeline

void ar::Pipeline::create(GraphicsConfig&& config) noexcept
{
    VkPipelineShaderStageCreateInfo shaderStages[6];
    VkPipelineColorBlendAttachmentState blendAttachments[8];
    VkFormat const colorAttachmentFormats[8] = {
        g_ctx.surfaceFormat.format,
        g_ctx.surfaceFormat.format,
        g_ctx.surfaceFormat.format,
        g_ctx.surfaceFormat.format,
        g_ctx.surfaceFormat.format,
        g_ctx.surfaceFormat.format,
        g_ctx.surfaceFormat.format,
        g_ctx.surfaceFormat.format
    };

    for (auto i{ size_t{} }; i < config.attachments.size(); ++i)
    {
        auto attachment{ config.attachments.begin() + i };

        blendAttachments[i] = {
            .blendEnable = static_cast<VkBool32>(attachment->blendEnable),
            .srcColorBlendFactor = static_cast<VkBlendFactor>(attachment->srcColorFactor),
            .dstColorBlendFactor = static_cast<VkBlendFactor>(attachment->dstColorFactor),
            .colorBlendOp = static_cast<VkBlendOp>(attachment->colorBlendOp),
            .srcAlphaBlendFactor = static_cast<VkBlendFactor>(attachment->srcAlphaFactor),
            .dstAlphaBlendFactor = static_cast<VkBlendFactor>(attachment->dstAlphaFactor),
            .alphaBlendOp = static_cast<VkBlendOp>(attachment->alphaBlendOp),
            .colorWriteMask = static_cast<VkColorComponentFlags>(attachment->colorComponent)
        };
    }

    for (auto i{ u32{} }; auto const& shader : config.shaders)
    {
        shaderStages[i] = shader.shaderStage;
        ++i;
    }

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
        .rasterizationSamples = config.useMsaa ? VK_SAMPLE_COUNT_4_BIT : VK_SAMPLE_COUNT_1_BIT
    }};

    auto const colorBlendStateCreateInfo{ VkPipelineColorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOp = VK_LOGIC_OP_COPY, // TODO
        .attachmentCount = static_cast<u32>(config.attachments.size()),
        .pAttachments = blendAttachments
    }};

    auto const depthStencilStateCreateInfo{ VkPipelineDepthStencilStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = static_cast<u32>(config.depthStencilState.depthTestEnable),
        .depthWriteEnable = static_cast<u32>(config.depthStencilState.depthWriteEnable),
        .depthCompareOp = static_cast<VkCompareOp>(config.depthStencilState.compareOp)
    }};

    auto const vertexInputStateCreateInfo{ VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    }};

    auto const renderingCreateInfo{ VkPipelineRenderingCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = static_cast<u32>(config.attachments.size()),
        .pColorAttachmentFormats = colorAttachmentFormats,
        .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
        .stencilAttachmentFormat = VK_FORMAT_UNDEFINED
    }};

    auto const pipelineCreateInfo{ VkGraphicsPipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCreateInfo,
        .stageCount = static_cast<u32>(config.shaders.size()),
        .pStages = shaderStages,
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
        &handle
    ));
}

void ar::Pipeline::destroy() noexcept
{
    vkDestroyPipeline(g_ctx.device, handle, nullptr);
}

#pragma endregion
#pragma region Buffer

void ar::Buffer::create(size_t bufferCapacity) noexcept
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
        .size = bufferCapacity,
        .usage = usage
    }};

    auto const allocationCI{ VmaAllocationCreateInfo{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    }};

    arContext::resultCheck(vmaCreateBuffer(
        g_ctx.allocator,
        &bufferCI,
        &allocationCI,
        &handle,
        &allocation,
        nullptr
    ));

    arContext::resultCheck(vmaMapMemory(
        g_ctx.allocator,
        allocation,
        reinterpret_cast<void**>(&pMapped)
    ));

    auto const bufferDAI{ VkBufferDeviceAddressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = handle
    }};

    capacity = bufferCapacity;
    address = vkGetBufferDeviceAddress(g_ctx.device, &bufferDAI);
}

void ar::Buffer::create(void const* pData, size_t dataSize) noexcept
{
    pMapped = nullptr;
    capacity = dataSize;

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

    arContext::resultCheck(vmaCreateBuffer(
        g_ctx.allocator,
        &bufferCI,
        &allocationCI,
        &handle,
        &allocation,
        nullptr
    ));
    
    auto const bufferDAI{ VkBufferDeviceAddressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = handle
    }};

    address = vkGetBufferDeviceAddress(g_ctx.device, &bufferDAI);

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
            handle,
            1u,
            &bufferCopy
        );

        arContext::endTransfer();

        vmaDestroyBuffer(g_ctx.allocator, stagingBuffer, stagingAllocation);
    }
}

void ar::Buffer::destroy() noexcept
{
    if (pMapped)
    {
        vmaUnmapMemory(
            g_ctx.allocator,
            allocation
        );
    }
   
    vmaDestroyBuffer(
        g_ctx.allocator,
        handle,
        allocation
    );
}

void ar::Buffer::write(void const* pData, size_t size, size_t offset) noexcept
{
    size = static_cast<b8>(size) * size + !static_cast<b8>(size) * capacity;
    memcpy(pMapped + offset, pData, size);
    arContext::resultCheck(vmaFlushAllocation(
        g_ctx.allocator,
        allocation,
        offset,
        size
    ));
}

#pragma endregion
#pragma region Image

static void makeImageResident(ar::ImageCreateInfo const& imageCI, ar::Image& image) noexcept
{
    if (static_cast<b8>(imageCI.sampler))
    {
        auto const writeImage{ VkDescriptorImageInfo{
            .sampler = *(&g_ctx.linearToEdgeSampler + (static_cast<u8>(imageCI.sampler) - 1)),
            .imageView = image.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        }};

        auto const write{ VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = g_ctx.descriptorSet,
            .dstArrayElement = imageCI.shaderArrayElement,
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

void ar::Image::create(ImageCreateInfo const& imageCreateInfo) noexcept
{
    sampler = imageCreateInfo.sampler;
    width = static_cast<b8>(imageCreateInfo.width) * imageCreateInfo.width +
           !static_cast<b8>(imageCreateInfo.width) * g_ctx.surfaceExtent.width;
    height = static_cast<b8>(imageCreateInfo.height) * imageCreateInfo.height +
            !static_cast<b8>(imageCreateInfo.height) * g_ctx.surfaceExtent.height;

    VkFormat format;
    VkImageAspectFlags aspect;
    VkSampleCountFlagBits sampleCount;
    VkImageUsageFlags usage;

    switch (imageCreateInfo.usage)
    {
    case ImageUsage::eColorAttachment:
        format = g_ctx.surfaceFormat.format;
        aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        break;
    case ImageUsage::eDepthAttachment:
        format = VK_FORMAT_D32_SFLOAT;
        aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
        usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        break;
    default:
        std::unreachable();
    }

    sampleCount = imageCreateInfo.useMsaa ? VK_SAMPLE_COUNT_4_BIT : VK_SAMPLE_COUNT_1_BIT;
    usage |= VK_IMAGE_USAGE_SAMPLED_BIT * static_cast<b8>(imageCreateInfo.sampler);
    usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT * imageCreateInfo.useMsaa;

    auto const imageCI{ VkImageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1u
        },
        .mipLevels = 1u,
        .arrayLayers = 1u,
        .samples = sampleCount,
        .usage = usage
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
        &handle,
        &allocation,
        nullptr
    ));

    view = arContext::createImageView(
        handle,
        VK_IMAGE_VIEW_TYPE_2D,
        format,
        aspect,
        1u
    );

    arContext::beginTransfer();

    auto const imageBarrier{ VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .dstStageMask = arContext::layoutToStage(imageCreateInfo.layout),
        .dstAccessMask = arContext::layoutToAccess(imageCreateInfo.layout),
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = static_cast<VkImageLayout>(imageCreateInfo.layout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = handle,
        .subresourceRange = {
            .aspectMask = aspect,
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

    makeImageResident(imageCreateInfo, *this);
}

void ar::Image::destroy() noexcept
{
    vkDestroyImageView(g_ctx.device, view, nullptr);
    vmaDestroyImage(g_ctx.allocator, handle, allocation);
}

#pragma endregion
#pragma region Context

static b8 arContext::acquireImage() noexcept
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
        arContext::createSwapchain(g_ctx.vsyncEnabled);
        return false;
    default:
        arContext::resultCheck(result);
        return true;
    }
}

static b8 arContext::presentFrame() noexcept
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
        createSwapchain(g_ctx.vsyncEnabled);
        return false;
    default:
        arContext::resultCheck(result);
        return false;
    }
}

static void arContext::create() noexcept
{
    {
        volkInitialize();
    }
    {
        u32 apiVersion;
        arContext::resultCheck(vkEnumerateInstanceVersion(&apiVersion));

        if (apiVersion < VK_API_VERSION_1_3)
        {
            arContext::resultCheck(VK_ERROR_INITIALIZATION_FAILED);
        }

        AR_INFO_CALLBACK("Vulkan API version: %u.%u.%u", VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion))

        char const* extensions[] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
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
            .apiVersion = apiVersion
        }};

        auto const instanceCI{ VkInstanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            #ifdef AR_ENABLE_INFO_CALLBACK
            .pNext = g_ctx.messenger.enableValidationLayers ? &validationFeatures : nullptr,
            #endif
            .pApplicationInfo = &applicationInfo,
            #ifdef AR_ENABLE_INFO_CALLBACK
            .enabledLayerCount = static_cast<u32>(g_ctx.messenger.enableValidationLayers),
            .ppEnabledLayerNames = layers,
            .enabledExtensionCount = 2u + static_cast<u32>(g_ctx.messenger.enableValidationLayers),
            #else
            .enabledExtensionCount = 2u,
            #endif
            .ppEnabledExtensionNames = extensions,
        }};

        arContext::resultCheck(vkCreateInstance(&instanceCI, nullptr, &g_ctx.instance));
        volkLoadInstance(g_ctx.instance);
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
                    [[maybe_unused]] void* pUserData
                )
                {
                    g_infoCallback(pCallbackData->pMessage);
                    return 0u;
                }
            }
        }};

        arContext::resultCheck(vkCreateDebugUtilsMessengerEXT(g_ctx.instance, &messengerCI, nullptr, &g_ctx.messenger.handle));
        g_infoCallback("Validation Layers are enabled. Prefer disabling them in release build");
    }
    #endif
    {
        auto const surfaceCI{ VkWin32SurfaceCreateInfoKHR{
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = g_wnd.hinstance,
            .hwnd = g_wnd.hwnd
        }};

        arContext::resultCheck(vkCreateWin32SurfaceKHR(g_ctx.instance, &surfaceCI, nullptr, &g_ctx.surface));
    }
    {
        VkPhysicalDevice physicalDevices[64];
        u32 physicalDeviceCount;

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
        u32 propertyCount;
        u32 presentQueueIndex;

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

        f32 const queuePriorities[] = { 1.f, 1.f };

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
            .storageBuffer8BitAccess = 1u,
            .shaderInt8 = 1u,
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
        volkLoadDevice(g_ctx.device);

        vkGetDeviceQueue(g_ctx.device, g_ctx.graphicsFamily, 0u, &g_ctx.graphicsQueue);
        vkGetDeviceQueue(g_ctx.device, g_ctx.presentFamily, presentQueueIndex, &g_ctx.presentQueue);
    }
    {
        auto const functions{ VmaVulkanFunctions{
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr
        }};

        auto const allocatorCI{ VmaAllocatorCreateInfo{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
            .physicalDevice = g_ctx.physicalDevice,
            .device = g_ctx.device,
            .pVulkanFunctions = &functions,
            .instance = g_ctx.instance,
            .vulkanApiVersion = VK_API_VERSION_1_3
        }};

        arContext::resultCheck(vmaCreateAllocator(&allocatorCI, &g_ctx.allocator));
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
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
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

    AR_INFO_CALLBACK("%s", "Created Context")
}

static void arContext::teardown() noexcept
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
    vkDestroyDebugUtilsMessengerEXT(g_ctx.instance, g_ctx.messenger.handle, nullptr);
    #endif
    vkDestroyInstance(g_ctx.instance, nullptr);

    AR_INFO_CALLBACK("%s", "Destroyed Context")
}

static void arContext::createSwapchain(b8 vsync) noexcept
{
    arContext::teardownSwapchain();
    g_ctx.vsyncEnabled = vsync;

    VkSurfaceFormatKHR formats[64];
    VkPresentModeKHR presentModes[6];
    VkSurfaceCapabilitiesKHR capabilities;
    VkCompositeAlphaFlagBitsKHR compositeAlpha;

    u32 minImageCount;
    u32 formatCount;
    u32 presentModeCount;

    arContext::resultCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_ctx.physicalDevice, g_ctx.surface, &capabilities));
    arContext::resultCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(g_ctx.physicalDevice, g_ctx.surface, &formatCount, nullptr));
    arContext::resultCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(g_ctx.physicalDevice, g_ctx.surface, &formatCount, formats));
    arContext::resultCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(g_ctx.physicalDevice, g_ctx.surface, &presentModeCount, nullptr));
    arContext::resultCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(g_ctx.physicalDevice, g_ctx.surface, &presentModeCount, presentModes));

    if (capabilities.currentExtent.width == 0xffffffffu)
    {
        g_ctx.surfaceExtent.width = static_cast<u32>(g_wnd.width);
        g_ctx.surfaceExtent.height = static_cast<u32>(g_wnd.height);

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

    minImageCount = g_minImageCount<u32>;

    if (capabilities.minImageCount > g_minImageCount<u32>)
    {
        minImageCount = capabilities.minImageCount;
    }

    if (capabilities.maxImageCount && capabilities.maxImageCount < g_minImageCount<u32>)
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

    if (!vsync)
    for (auto i{ presentModeCount }; i--; )
    {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        }

        if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
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

    VkImage images[g_maxImageCount<u32>];

    arContext::resultCheck(vkGetSwapchainImagesKHR(g_ctx.device, g_ctx.swapchain, &g_ctx.imageCount, nullptr));
    if (g_ctx.imageCount > g_maxImageCount<u32>)
        arContext::resultCheck(VK_ERROR_INITIALIZATION_FAILED);
    arContext::resultCheck(vkGetSwapchainImagesKHR(g_ctx.device, g_ctx.swapchain, &g_ctx.imageCount, images));

    AR_INFO_CALLBACK("Swapchain image count: [%u]", g_ctx.imageCount)

    VkCommandBuffer graphicsCommandBuffers[g_maxImageCount<u32>];
    VkCommandBuffer presentCommandBuffers[g_maxImageCount<u32>];
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
                .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
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

static void arContext::teardownSwapchain() noexcept
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

static void arContext::beginTransfer() noexcept
{
    auto const beginInfo{ VkCommandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    }};

    arContext::resultCheck(vkResetCommandPool(g_ctx.device, g_ctx.transferCommandPool, {}));
    arContext::resultCheck(vkBeginCommandBuffer(g_ctx.transferCommandBuffer, &beginInfo));
}

static void arContext::endTransfer() noexcept
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

VkPipelineStageFlags2 arContext::layoutToStage(ar::ImageLayout layout) noexcept
{
    switch (layout)
    {
    case ar::ImageLayout::eColorAttachment:
        return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    case ar::ImageLayout::eShaderReadOnly:
        return VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    case ar::ImageLayout::eDepthReadOnly: [[fallthrough]];
    case ar::ImageLayout::eDepthAttachment:
        return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    default:
        std::unreachable();
    }
}

VkAccessFlags2 arContext::layoutToAccess(ar::ImageLayout layout) noexcept
{
    switch (layout)
    {
    case ar::ImageLayout::eColorAttachment:
        return VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    case ar::ImageLayout::eShaderReadOnly:
        return VK_ACCESS_2_SHADER_READ_BIT;
    case ar::ImageLayout::eDepthReadOnly:
        return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    case ar::ImageLayout::eDepthAttachment:
        return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    default:
        std::unreachable();
    }
}

static VkImageView arContext::createImageView(VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspect, u32 mipLevels) noexcept
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

static void arContext::resultCheck(VkResult result) noexcept
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
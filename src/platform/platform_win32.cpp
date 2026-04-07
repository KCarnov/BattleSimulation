// Win32
// TODO: its already specified in build.bat, but removing this line greys out all the code ...
#define WIN32
#ifdef WIN32

// Application (Unit build)
#include "../core/application.cpp"

#include <windows.h>
#include <strsafe.h>
#include "platform_win32.h"

static bool g_running;
static win32_backbuffer* g_backbuffer;
static LARGE_INTEGER g_frequency;

#define PRINT(Expression,var) {char str[256]; \
        StringCchPrintfA(str, sizeof(str), Expression, var); \
        OutputDebugStringA(str); }

inline LARGE_INTEGER win32_GetWallClock()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter;
}
inline f32 win32_GetSecondsElapsed(LARGE_INTEGER lastCounter, LARGE_INTEGER endCounter)
{
    return ((f32)(endCounter.QuadPart - lastCounter.QuadPart) / g_frequency.QuadPart); 
}
void win32_PerformanceBegin(LARGE_INTEGER* lastCounter)
{
    QueryPerformanceCounter(lastCounter); // INFO  Retrieves the current value of the performance counter, which is a high resolution (<1us) time stamp, ie, number of clock cycles
    QueryPerformanceFrequency(&g_frequency); // INFO  Retrieves the frequency of the performance counter, ie, number of clock cycles per second
}
void win32_PerformanceEnd(LARGE_INTEGER* lastCounter, f32 targetMsPerFrame, b32 isSleepGranular)
{

    i32 msPerFrame = (i32)(1000.0f * win32_GetSecondsElapsed(*lastCounter, win32_GetWallClock())); 
    DWORD sleepMs;
    if(msPerFrame < targetMsPerFrame)
    {
        sleepMs = (DWORD) (targetMsPerFrame - (f32)msPerFrame);
        Sleep(sleepMs); //TODO: the Sleep() sleeps for double the time, i dont get it
        msPerFrame = (i32)(1000.0f  * win32_GetSecondsElapsed(*lastCounter, win32_GetWallClock())); 
    }
    else
    {
        // TODO: Missed frame rate ! We are too slow
    }

#if 1
    //f32 FPS = ((f32)g_frequency.QuadPart / (f32)win32_GetWallClock());
    msPerFrame = (i32)(1000.0f  * win32_GetSecondsElapsed(*lastCounter, win32_GetWallClock())); 
    char msPerFrame_StringBuffer[256];
    StringCchPrintfA(msPerFrame_StringBuffer, sizeof(msPerFrame_StringBuffer), "ms/frame = %d\n", msPerFrame);
    OutputDebugStringA(msPerFrame_StringBuffer);
 #endif

    *lastCounter = win32_GetWallClock();
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,  int nCmdShow)
{

    // Performance counter BEGIN
    LARGE_INTEGER lastCounter;
    win32_PerformanceBegin(&lastCounter);

    //  BUG  It seems to be working ONLY when window is showing on screen ... ?????
    i32 monitorRefreshHz = 60; //TODO: the Sleep() sleeps for double the time, i dont get it ??
    i32 applicationRefreshHz = monitorRefreshHz; // / 2;
    f32 milliSecondsElpasedPerFrame = 1000.0f / (f32)applicationRefreshHz;
    // NOTE: Set the Windows scheduler granularity to 1ms so that the Sleep() can be more granular
    u32 desiredSchedulerMS = 1;
    b32 isSleepGranular = (timeBeginPeriod(desiredSchedulerMS ) == TIMERR_NOERROR);
    ASSERT(isSleepGranular)

    WNDCLASSW WindowClass = {};
    WindowClass.style         = CS_VREDRAW | CS_HREDRAW;
    WindowClass.lpfnWndProc   = WindowProc;
    WindowClass.hInstance     = hInstance;
    WindowClass.lpszClassName = L"Custom Window";

    if(RegisterClassW(&WindowClass) == NULL)
    {
        MessageBoxW(NULL, L"Error to register WindowClass", L"Failure", MB_OK );
        return 0;
    }
     
    HWND windowHandle = CreateWindowExW(0, WindowClass.lpszClassName, L"My window i guess",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,CW_USEDEFAULT,WIDTH,HEIGHT,
                        0, 0, hInstance, 0);

    if (windowHandle == NULL)
    {
        MessageBoxW(NULL, L"Error in window creation", L"Failure", MB_OK );
        return 0;
    }
    ShowWindow(windowHandle, SW_SHOW); // NOTE: Not necessary if CreateWindowExW with WS_VISIBLE flag
  
    // INFO  Platform Initialization (win32)
    win32_backbuffer win32Backbuffer;
    g_backbuffer = &win32Backbuffer;
    win32_ResizeBuffer(&win32Backbuffer, WIDTH, HEIGHT);

    // INFO  Application Initialization 
    app_backbuffer appBackbuffer = {};
    app_inputs     appInputs = {};
    app_memory     appMemory;
    appMemory.permanentStorageSize = MEGABYTES(64);
    appMemory.permanentStorage = VirtualAlloc(0,appMemory.permanentStorageSize, 
                                                MEM_RESERVE|MEM_COMMIT, 
                                                PAGE_READWRITE);
    appMemory.transientStorageSize = GIGABYTES(2);
    appMemory.transientStorage = VirtualAlloc(0,appMemory.transientStorageSize, 
                                                MEM_RESERVE|MEM_COMMIT, 
                                                PAGE_READWRITE);

    // INFO  Start loop
    g_running = true;
    MSG msg = {};
    while(g_running)
    {
        // Pool events
        app_inputs zeroInputs = {};
        appInputs = zeroInputs;
        win32_ProcessPendingMessages(&appInputs);

        // Render application
        appBackbuffer.memory = win32Backbuffer.memory;
        appBackbuffer.width  = win32Backbuffer.width;
        appBackbuffer.height = win32Backbuffer.height;
        appBackbuffer.stride = win32Backbuffer.stride;
        appBackbuffer.bytesPerPixel = win32Backbuffer.bytesPerPixel;
        ApplicationUpdateAndRender(&appMemory, &appBackbuffer, &appInputs);
  
        // Performance counter END
        win32_PerformanceEnd(&lastCounter, milliSecondsElpasedPerFrame, isSleepGranular);

        // Swap Buffers
        HDC deviceContext = GetDC(windowHandle);
        win32_window_dimensions windowDimensions = win32_GetWindowDimension(windowHandle);
        win32_DisplayBufferToScreen(deviceContext, windowDimensions.width, windowDimensions.height, &win32Backbuffer);
        ReleaseDC(windowHandle, deviceContext);
    }

    return 0;
}


void win32_ProcessPendingMessages(app_inputs* appInputs)
{
    MSG msg = {};
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        switch (msg.message)
        {
            case WM_LBUTTONDOWN:
            {
                // NOTE: (1<<17)-1 is a mask of the first 16 bit => example : 001000 - 1 = 000111
                // X position if lower part and Y position is higher part
                f32  mouseX = (msg.lParam & (1<<16)-1);
                f32  mouseY = (msg.lParam >> 16);
                appInputs->mouseInputs.isPressed = true;
                appInputs->mouseInputs.mousePosition = {mouseX,mouseY};

            }break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                int delta = 5;
                int VKCode = msg.wParam;

                b32 keyDownPreviously = ((msg.lParam & (1 << 30)) != 0);
                b32 keyDownNow        = ((msg.lParam & (1 << 31)) == 0);
                b32 validKeyTransition     = (keyDownNow != keyDownPreviously); // INFO  Means a transition occured (either pressed down, or released up)
                b32 validKeyPressed        = keyDownNow & !keyDownPreviously;   // TODO: Not used for now ?
                if (validKeyPressed)
                {
                    if(VKCode == 'Z') { appInputs->Z.isPressed = 1; }
                    if(VKCode == 'Q') { appInputs->Q.isPressed = 1; }
                    if(VKCode == 'S') { appInputs->S.isPressed = 1; }
                    if(VKCode == 'D') { 
                        appInputs->D.isPressed = 1; 
                        OutputDebugStringA("D is pressed (platform layer)!");
                    }
                    if(VKCode == 'F') { appInputs->F.isPressed = 1; }
                }

            }break;
            default:
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }break;
        }
    }

}

void win32_DisplayBufferToScreen(HDC deviceContext, int windowWidth, int windowHeight, win32_backbuffer *buffer)
{
    //PatBlt(deviceContext,0,0,buffer->width,buffer->height, BLACKNESS);
    StretchDIBits(deviceContext, 
                  0, 0, buffer->width, buffer->height,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

void win32_ResizeBuffer(win32_backbuffer *buffer, u16 width, u16 height)
{
    if(buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }
    buffer->width = width;
    buffer->height = height;
    buffer->bytesPerPixel = 4;

    BITMAPINFO bitMapInfo;
    buffer->info.bmiHeader.biSize = sizeof(bitMapInfo.bmiHeader);
    buffer->info.bmiHeader.biWidth = (LONG) width;
    buffer->info.bmiHeader.biHeight = - (LONG) height; // NOTE: minus sign is needed for top to bottom
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    size_t memorySize = (width * height)*buffer->bytesPerPixel;
    buffer->memory = VirtualAlloc(0, memorySize, MEM_COMMIT, PAGE_READWRITE);

    buffer->stride = width * buffer->bytesPerPixel; // INFO  Stride is in Bytes
}

LRESULT CALLBACK WindowProc(HWND hWindow, UINT Message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (Message)
    {
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            OutputDebugStringW(L"KEY OR SYSKEY event passed to WindowCallback Process WITHOUT GOING THROUGH DISPATCH => SHOULD NO HAPPENED");
            return result;
        }break;
        case WM_SIZE:
        {

        }break;
        case WM_DESTROY:
        case WM_CLOSE:
        {
            g_running = false;
            OutputDebugStringW(L"C_Project hWindow terminated");
            return 0;
        }break;
        case WM_ACTIVATEAPP:
        {

        }break;
        case WM_PAINT: 
        {
            PAINTSTRUCT ps;
            // INFO  All painting occurs here, between BeginPaint and EndPaint.
            HDC hdc = BeginPaint(hWindow, &ps);
            // HBRUSH brush = CreateSolidBrush(RGB(255, 68 , 15)); // INFO  must link with Gdi32.lib to use CreateSolidBrush
            // FillRect(hdc, &ps.rcPaint, brush);
            
            win32_window_dimensions windowDimensions = win32_GetWindowDimension(hWindow);
            win32_DisplayBufferToScreen(hdc, windowDimensions.width,windowDimensions.height, g_backbuffer);
            EndPaint(hWindow, &ps);
        }break;
        default:
        {
            // INFO  Everything we dont handle ourselves is passed down to Windows default procedure
            result = DefWindowProcW(hWindow, Message, wParam, lParam);
        }break;
    }
    return result;
}

win32_window_dimensions win32_GetWindowDimension(HWND window) 
{   
    //NOTE: I keep window width and height if later i plan on using those, for now we use fixed size backbuffer
    win32_window_dimensions dimension;
    RECT clientRect;
    GetClientRect(window, &clientRect);
    dimension.width  = (u16)(clientRect.right - clientRect.left);
    dimension.height = (u16)(clientRect.bottom - clientRect.top);
    return dimension;
}

#endif 
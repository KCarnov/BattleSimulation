#pragma once
#include "../defines.h"

#define WIN32
#ifdef WIN32

struct win32_backbuffer
{
    BITMAPINFO info;
    void* memory;
    u16 width;
    u16 height;
    i32 stride;
    i32 bytesPerPixel;
};

struct win32_window_dimensions {
    u16 width, height;
};

LRESULT CALLBACK WindowProc(HWND hWindow, UINT Message, WPARAM wParam, LPARAM lParam);

void                    win32_ResizeBuffer(win32_backbuffer *buffer, u16 width, u16 height);
void                    win32_DisplayBufferToScreen(HDC deviceContext, i32 windowWidth, i32 windowHeight, win32_backbuffer *buffer);
void                    win32_ProcessPendingMessages(app_inputs* appInputs);
void                    win32_ProcessInputs();
win32_window_dimensions win32_GetWindowDimension(HWND window);

#endif
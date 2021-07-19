#include <Windows.h>
#include <winuser.h>
#include <iostream>
#include <tchar.h>
#include <conio.h>
#include <stdint.h>
#include <strsafe.h>
#include "_3DEngine.h"
#include "Drawings.hpp"
using namespace std;


// COLORS
extern COLORREF red, green, blue, yellow, black, white, grey, dark_grey;


// 3D ENGINE
_3DEngine _3den;


bool m_front, m_back, m_left, m_right, m_up, m_down;
void camera_state()
{
    if (GetAsyncKeyState(0x57))
        m_front = true;
    else
        m_front = false;
    if (GetAsyncKeyState(0x53))
        m_back = true;
    else
        m_back = false;
    if (GetAsyncKeyState(0x41))
        m_left = true;
    else
        m_left = false;
    if (GetAsyncKeyState(0x44))
        m_right = true;
    else
        m_right = false;

    if (GetAsyncKeyState(0x45))
        m_up = true;
    else
        m_up = false;
    if (GetAsyncKeyState(0x51))
        m_down = true;
    else
        m_down = false;
}
void camera_movement()
{
    if (m_front)
        _3den.MoveForward();
    else if (m_back)
        _3den.MoveBackward();
    if (m_left)
        _3den.RotateCamera(-0.04f);
    else if (m_right)
        _3den.RotateCamera( 0.04f);

    if (m_up)
        _3den.MoveCamera(0.0f, 0.2f, 0.0f);
    else if (m_down)
        _3den.MoveCamera(0.0f, -0.2f, 0.0f);
}


int main()
{
    CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(handle, &screenBufferInfo);

    HWND window = GetConsoleWindow();
    // Prevent from resizing
    SetWindowLong(window, GWL_STYLE, GetWindowLong(window, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

    hdc = GetDC(window);

    SetConsoleTitle(TEXT(TITLE));
    disableScrollBar(handle, screenBufferInfo);

    DWORD prev_mode;
    GetConsoleMode(handle, &prev_mode);

    //Draw pixels
    clock_t current_ticks, delta_ticks;
    clock_t fps = 0;

    Clear(dark_grey);

    float z = 0.0f;

    while (GetAsyncKeyState(VK_ESCAPE) == 0) // Dont stop until press scape or click button exit
    {
        current_ticks = clock();
        _3den.render();
        draw();
        Clear(dark_grey);

        clock_t endFrame = clock();
        delta_ticks = clock() - current_ticks; //the time, in ms, that took to render the scene
        if (delta_ticks > 0)
        {
            fps = CLOCKS_PER_SEC / delta_ticks;
            TCHAR szNewTitle[MAX_PATH];
            StringCchPrintf(szNewTitle, MAX_PATH, TEXT(TITLE " - FPS: %ju"), (uintmax_t)(clock_t)fps);
            SetConsoleTitle(szNewTitle);
        }
        camera_state();
        camera_movement();
        
#ifndef _DEBUG
        //_3den.fTheta += 0.05f / (delta_ticks);
#endif
#ifdef _DEBUG
        //_3den.fTheta += 0.2f / (delta_ticks);
#endif
        if (fps < 10) break;
        Sleep(1000 / fps);
    }

    ReleaseDC(window, hdc);
    return 0;
}
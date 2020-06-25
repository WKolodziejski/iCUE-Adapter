#pragma region include and define
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>

#include "iCUE.h"

#ifdef UNICODE
#define stringcopy wcscpy
#else
#define stringcopy strcpy
#endif

#define ID_CONSOLE_CONTEXT_MENU_TOP     1
#define ID_CONSOLE_CONTEXT_MENU_MIRROR  2
#define ID_CONSOLE_CONTEXT_MENU_BOTTOM  3
#define ID_TRAY_APP_ICON                5000
#define ID_CONSOLE_CONTEXT_MENU_ITEM    4000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define WM_TRAYICON (WM_USER + 1)
#define WM_LED_FAIL (WM_USER + 2)
#pragma endregion

#pragma region constants and globals
UINT WM_TASKBARCREATED = 0;
UINT t_leds;
UINT t_temp;
HWND g_hwnd;
HMENU g_menu;
HWND g_console;

NOTIFYICONDATA g_notifyIconData;
#pragma endregion

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void handleCallbackStop();

#pragma region callbacks
VOID CALLBACK UpdateLEDsCallback(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime) {
    if (!UpdateLEDs()) {
        handleCallbackStop();
    }
}

VOID CALLBACK UpdateTempCallback(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime) {
    if (!UpdateTemp()) {
        handleCallbackStop();        
    }
}

void handleCallbackStop() {
    KillTimer(NULL, t_leds);
    KillTimer(NULL, t_temp);

    while (!InitLEDs())
    {
        Sleep(1000);
    }

    t_leds = SetTimer(NULL, 0, 100, &UpdateLEDsCallback);
    t_temp = SetTimer(NULL, 0, 5000, &UpdateTempCallback);
}
#pragma endregion

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR args, int iCmdShow)
{
    TCHAR className[] = TEXT("iCUE Adapter");
    WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated");

#pragma region add a console
     AllocConsole();
     freopen("CON", "w", stdout);
     g_console = FindWindowA("ConsoleWindowClass", NULL);
     ShowWindow(g_console, SW_HIDE);
#pragma endregion

#pragma region get window up
    WNDCLASSEX wnd = { 0 };

    wnd.hInstance = hInstance;
    wnd.lpszClassName = className;
    wnd.lpfnWndProc = WndProc;
    wnd.style = CS_HREDRAW | CS_VREDRAW;
    wnd.cbSize = sizeof(WNDCLASSEX);
    wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
    wnd.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE;

    if (!RegisterClassEx(&wnd))
    {
        FatalAppExit(0, TEXT("Couldn't register window class!"));
    }

    g_hwnd = CreateWindowEx(
        0, className,
        TEXT("Using the system tray"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0,
        NULL, NULL,
        hInstance, NULL
    );

    memset(&g_notifyIconData, 0, sizeof(NOTIFYICONDATA));
    g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    g_notifyIconData.hWnd = g_hwnd;
    g_notifyIconData.uID = ID_TRAY_APP_ICON;
    g_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_notifyIconData.uCallbackMessage = WM_TRAYICON;
    g_notifyIconData.hIcon = (HICON)LoadImage(NULL, TEXT("iCUEicon.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    stringcopy(g_notifyIconData.szTip, TEXT("iCUE Adapter"));

    Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);

    while (!InitLEDs())
    {
        Sleep(1000);
    }

    t_leds = SetTimer(NULL, 0, 50, &UpdateLEDsCallback);
    t_temp = SetTimer(NULL, 0, 5000, &UpdateTempCallback);

#pragma endregion

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (!IsWindowVisible(g_hwnd))
    {
        Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);
    }

    KillTimer(NULL, t_leds);
    KillTimer(NULL, t_temp);

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:

            g_menu = CreatePopupMenu();

            AppendMenu(g_menu, MF_STRING, ID_CONSOLE_CONTEXT_MENU_TOP, TEXT("Top"));
            AppendMenu(g_menu, MF_STRING, ID_CONSOLE_CONTEXT_MENU_MIRROR, TEXT("Mirror"));
            AppendMenu(g_menu, MF_STRING, ID_CONSOLE_CONTEXT_MENU_BOTTOM, TEXT("Bottom"));
            AppendMenu(g_menu, MF_STRING, ID_CONSOLE_CONTEXT_MENU_ITEM, TEXT("Console"));
            AppendMenu(g_menu, MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM, TEXT("Exit"));

            break;

        case WM_SYSCOMMAND:
        case WM_TRAYICON:
        {
            if (lParam == WM_RBUTTONDOWN)
            {
                POINT curPoint;
                GetCursorPos(&curPoint);
                //SetForegroundWindow(hwnd);
                UINT clicked = TrackPopupMenu(
                    g_menu,
                    TPM_RETURNCMD | TPM_NONOTIFY,
                    curPoint.x,
                    curPoint.y,
                    0,
                    hwnd,
                    NULL
                );

                switch (clicked)
                {
                    case ID_CONSOLE_CONTEXT_MENU_ITEM:
                        ShowWindow(g_console, IsWindowVisible(g_console) ? SW_HIDE : SW_SHOW);
                        break;

                    case ID_TRAY_EXIT_CONTEXT_MENU_ITEM:
                        PostQuitMessage(0);
                        break;

                    case ID_CONSOLE_CONTEXT_MENU_TOP:
                        ChangeColorMode('t');
                        break;

                    case ID_CONSOLE_CONTEXT_MENU_MIRROR:
                        ChangeColorMode('m');
                        break;

                    case ID_CONSOLE_CONTEXT_MENU_BOTTOM:
                        ChangeColorMode('b');
                        break;
                }
            }
        }
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
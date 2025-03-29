/*
 * Copyright (C) 2025 Jamie Howell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "TrayIcon.h"

// ID's for the buttons in the icon
#define ID_TRAY_APP_ICON 0x11
#define ID_TRAY_EXIT 0x12
#define ID_TRAY_OPEN 0x13

// Handle and structure for creating the window and creating the tray entry
NOTIFYICONDATA nid;
HWND g_hWndTray;

LRESULT CALLBACK TrayProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // Initialize the tray icon
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hWnd;
        nid.uID = ID_TRAY_APP_ICON;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_MAINICON));
        wcscpy(nid.szTip, L"XPClock");

        Shell_NotifyIcon(NIM_ADD, &nid);
        break;

    case WM_USER + 1:  // Tray icon message handler
        if (lParam == WM_RBUTTONDOWN) {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();

            AppendMenu(hMenu, MF_STRING, ID_TRAY_OPEN, L"Open");
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

            SetForegroundWindow(hWnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_OPEN:
            SetForegroundWindow(g_hWndMain);
            break;
        case ID_TRAY_EXIT:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;
        }
        break;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void StartTray(void) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = TrayProc;
    wc.hInstance = g_hInst;
    wc.lpszClassName = L"ClockTrayClass";
    wc.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_MAINICON));

    if (!RegisterClass(&wc)) {
        return;
    }

    g_hWndTray = CreateWindowEx(0, wc.lpszClassName, L"Tray Icon", 0, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
    if (!g_hWndTray) return;

    // No need to show a window here.
}
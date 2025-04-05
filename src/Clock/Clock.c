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

#include "Clock.h"
#include "MathHelpers.h"
#include "AboutWindow.h"
#include "SettingsWindow.h"
#include "NTPClient.h"
#include "TrayIcon.h"
#include "Colors.h"

 // Version of common controls to link to. Changes the appearance of controls. https://learn.microsoft.com/en-us/windows/win32/controls/common-control-versions
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma region Global Variables
// Application instance and handles
HINSTANCE g_hInst; // Global variable for storing the handle for the instance for use whenever it is needed. I prefer this method over passing an HINSTANCE into each function.
HWND g_hWndMain; // Global variable for storing the window handle for the main window of the application. This is the window where the clock itself is rendered. See MainWndProc. 
HWND g_hWndClockOut; // Global variable for storing the handle of the static control where the clock text is rendered. This is where the magic happens! The control is set to be the same dimensions as the window, thus giving it range of the entire window to move and bounce. See RenderText. 
HMENU g_hMenu; // Global variable for storing the handle for the menu. This is used to create the menu for the application.

// Configuration and UI state
UINT g_Context = 0; // Context for what part of the UI is being shown. Useful for scanning the messages to check if the user clicked escape.
BOOL g_bIsFullScreen = FALSE; // Used for full screen functionality.
RECT g_rcWindow; // Used for full screen functionality.
GradientColor g_GradientColor; // Check the GradientColor comments
COLORREF g_bgColor; // Holds the color that is used if a custom color is parsed, but the gradient effect isn't used
BOOL g_bGetTime = TRUE; // Variable to check if the time is needed to be checked. This stops the application from crashing if an NTP server can't be reached

// Font resources
HFONT g_hfMainFont; // Global variable for storing the main font that will be used to render the clock text.
HFONT g_hfBtnFont; // Same as above, except this one is rendered smaller to fit in buttons. Is used in the settings sub-window.

// Strings
#define szCLASS L"ClockWndClass" // Constant string for registering the main window class. https://learn.microsoft.com/en-us/windows/win32/intl/registering-window-classes
WCHAR* g_szMainFont; // Global variable for the font name. Changed to not be constant to accomodate for custom fonts.

// Control ID's
#define TIMER_ID 0x101 // ID for refresh timer for renderer. See MainWndProc.

// NTP-associated values
DWORD g_tidNTPThread; // Thread ID for the NTP refresh thread
HANDLE g_hNTPThread; // Handle for the thread itself.
#pragma endregion


// Specific errors I disabled because I either don't know how to fix them, or they don't impact the application.
#pragma warning(disable : 4047)	// Different levels of indirection.

#pragma region Main window
BOOL CreateClock(void) {
	RegisterMainClass(g_hInst); // Register the class for the main window

	wprintf(L"Creating main window...\r\n");
	g_hWndMain = CreateWindowW(szCLASS, L"XPClock", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, g_hMenu, g_hInst, NULL);

	if (!g_hWndMain) {
		red();
		wprintf(L"Failed to create main window! GetLastError: 0x%x\r\n", GetLastError());
		reset();
		return FALSE;
	}
	wprintf(L"Successfully created main window!\r\n");

	ShowWindow(g_hWndMain, SW_SHOW);
	UpdateWindow(g_hWndMain);

	return TRUE;
}

ATOM RegisterMainClass(HINSTANCE hInstance) {
	// Pretty standard ATOM for registering a class
	// This is the Microsoft preferred way to do it I guess.
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szCLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		CreateClockControl(hwnd);
		if (DVDLogo()) {
			SetTimer(hwnd, TIMER_ID, 16, NULL); // If the DVD logo effect is used, draw at 60 FPS
		}
		else {
			SetTimer(hwnd, TIMER_ID, 66, NULL); // If not, draw at 15 FPS. Hopefully this helps optmize it for older platforms
		}
		CenterWindow(hwnd, NULL);
		break;
	case WM_DESTROY:
		// Make sure to release everythin before exiting.
		KillTimer(hwnd, TIMER_ID);
		DeleteObject(g_hfMainFont);
		DeleteObject(g_hfBtnFont);
		FreeConsole();
		CloseHandle(g_hNTPThread);
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		ResizeText(hwnd);
		break;
	case WM_DRAWITEM:
		RenderText(lParam);
		break;
	case WM_TIMER: {
		if (wParam == TIMER_ID) {
			WCHAR buffer[64];
			GetCurrentDateTime(buffer, 64);
			SetWindowTextW(g_hWndClockOut, buffer); // Update the text box
		}
		break;
	}
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		else if (wParam == 'F') {
			blue();
			wprintf(L"Changing display format. (%d)\r\n", g_Config.DisplayFormat);
			reset();
			ChangeShowDate();
		}
		else if (wParam == 'T') {
			blue();
			wprintf(L"Changing display format. (%d)\r\n", g_Config.DisplayFormat);
			reset();
			ChangeTimeFormat();
		}
		else if (wParam == VK_F1) {
			wprintf(L"Creating about window.\r\n");
			g_Context = __CONTEXT_ABOUT__;
			InitAbout();
		}
		else if (wParam == VK_F2) {
			wprintf(L"Creating settings window.\r\n");
			g_Context = __CONTEXT_SETTINGS__;
			InitSettings();
		}
		else if (wParam == VK_F11) {
			wprintf(L"Full screening main window.\r\n");
			ToggleFullScreen(hwnd);
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_ABOUT_MENU_BTN) {
			wprintf(L"Creating about window.\r\n");
			g_Context = __CONTEXT_ABOUT__;
			InitAbout();
		}
		else if (LOWORD(wParam) == ID_SETTINGS_MENU_BTN) {
			wprintf(L"Creating settings window.\r\n");
			g_Context = __CONTEXT_SETTINGS__;
			InitSettings();
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL RenderText(LPARAM lParam) {
	if (!g_Config.Gradient) {
		LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
		HDC hdc = pDIS->hDC;
		RECT rect = pDIS->rcItem;

		// Create a memory DC for double buffering
		HDC hMemDC = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

		HBRUSH hBrush;

		if (g_Config.CustomColor) {
			hBrush = CreateSolidBrush(g_bgColor);
		}
		else {
			hBrush = ((HBRUSH)GetStockObject(BLACK_BRUSH));
		}

		FillRect(hMemDC, &rect, hBrush);

		// Set text color to white
		SetTextColor(hMemDC, RGB(255, 255, 255));
		SetBkMode(hMemDC, TRANSPARENT);

		// Get the actual text from the control
		wchar_t text[256];  // Adjust size if needed
		GetWindowTextW(pDIS->hwndItem, text, 256);

		// Select custom font
		if (g_hfMainFont) {
			SelectObject(hMemDC, g_hfMainFont);
		}

		// Calculate text size
		SIZE textSize;
		GetTextExtentPoint32W(hMemDC, text, lstrlenW(text), &textSize);

		// Static variables to keep track of the bouncing text
		static int dx = 2, dy = 2; // Speed of movement
		static int posX = 10, posY = 10; // Initial position

		if (g_Config.DVDLogo) {
			// Update position
			posX += dx;
			posY += dy;

			// Bounce logic
			if (posX + textSize.cx >= rect.right || posX <= rect.left) {
				dx = -dx;
			}
			if (posY + textSize.cy >= rect.bottom || posY <= rect.top) {
				dy = -dy;
			}
		}
		else {
			// Center the text normally
			posX = (rect.right - rect.left - textSize.cx) / 2;
			posY = (rect.bottom - rect.top - textSize.cy) / 2;
		}

		// Draw the text at the calculated position
		TextOutW(hMemDC, posX, posY, text, lstrlenW(text));

		// Copy the memory DC to the actual DC
		BitBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY);

		// Cleanup
		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hMemDC);

		return TRUE;  // Message handled
	}
	else {
		LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
		HDC hdc = pDIS->hDC;
		RECT rect = pDIS->rcItem;

		// Create a memory DC for double buffering
		HDC hMemDC = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

		// Create a gradient from red (top) to black (bottom)
		TRIVERTEX vertex[2];
		GRADIENT_RECT gRect;

		vertex[0].x = rect.left;
		vertex[0].y = rect.top;
		vertex[1].x = rect.right;
		vertex[1].y = rect.bottom;

		if (g_Config.CustomColor) {
			vertex[0].Red = g_GradientColor.tvColor1.Red;
			vertex[0].Green = g_GradientColor.tvColor1.Green;
			vertex[0].Blue = g_GradientColor.tvColor1.Blue;

			vertex[1].Red = g_GradientColor.tvColor2.Red;
			vertex[1].Green = g_GradientColor.tvColor2.Green;
			vertex[1].Blue = g_GradientColor.tvColor2.Blue;
		}
		else {
			// Define the start color
			vertex[0].Red = 0xFFFF;
			vertex[0].Green = 0x0000;
			vertex[0].Blue = 0x0000;
			vertex[0].Alpha = 0x0000;

			// Define the end color (black)
			vertex[1].Red = 0x0000;
			vertex[1].Green = 0x0000;
			vertex[1].Blue = 0x0000;
			vertex[1].Alpha = 0x0000;
		}

		// Gradient direction (vertical)
		gRect.UpperLeft = 0;
		gRect.LowerRight = 1;

		// Fill with gradient
		GradientFill(hMemDC, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

		// Set text color to white
		SetTextColor(hMemDC, RGB(255, 255, 255));
		SetBkMode(hMemDC, TRANSPARENT);

		// Get the actual text from the control
		wchar_t text[256];  // Adjust size if needed
		GetWindowTextW(pDIS->hwndItem, text, 256);

		// Select custom font
		if (g_hfMainFont) {
			SelectObject(hMemDC, g_hfMainFont);
		}

		// Calculate text size
		SIZE textSize;
		GetTextExtentPoint32W(hMemDC, text, lstrlenW(text), &textSize);

		// Static variables to keep track of the bouncing text
		static int dx = 2, dy = 2; // Speed of movement
		static int posX = 10, posY = 10; // Initial position

		if (g_Config.DVDLogo) {
			// Update position
			posX += dx;
			posY += dy;

			// Bounce logic
			if (posX + textSize.cx >= rect.right || posX <= rect.left) {
				dx = -dx;
			}
			if (posY + textSize.cy >= rect.bottom || posY <= rect.top) {
				dy = -dy;
			}
		}
		else {
			// Center the text normally
			posX = (rect.right - rect.left - textSize.cx) / 2;
			posY = (rect.bottom - rect.top - textSize.cy) / 2;
		}

		// Draw the text at the calculated position
		TextOutW(hMemDC, posX, posY, text, lstrlenW(text));

		// Copy the memory DC to the actual DC
		BitBlt(hdc, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, SRCCOPY);

		// Cleanup
		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hMemDC);

		return TRUE;  // Message handled
	}
}

void CreateClockControl(HWND hwnd) {
	// Simply creates the clock control.
	wprintf(L"Creating clock control...\r\n");
	g_hWndClockOut = CreateWindowW(WC_STATIC, L"Please wait while clock initializes...", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_OWNERDRAW, 0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);

	if (g_hfMainFont) {
		SendMessage(g_hWndClockOut, WM_SETFONT, (WPARAM)g_hfMainFont, TRUE);
	}
}

void ResizeText(HWND hwnd) {
	wprintf(L"Resizing main window.\r\n");
	RECT rect;
	GetClientRect(hwnd, &rect);

	SetWindowPos(g_hWndClockOut, NULL, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOZORDER | SWP_NOACTIVATE);
}

void GetCurrentDateTime(WCHAR* buffer, size_t bufferSize) {
	if (!buffer || bufferSize == 0) return;

	if (!g_bGetTime) {
		// Return this function to avoid hanging the application while trying to get NTP time
		// In case of an error with the NTP
		return;
	}

	if (g_TimeConfig.ts == 1 && g_bGetTime) {
		// The user is using network time, so output the NTP time.
		// See NTPClient.h & NTPClient.c for more details.
		OutputNTPTime(buffer, bufferSize);
	}
	else {
		SYSTEMTIME st;
		GetLocalTime(&st);  // Get local time

		switch (g_Config.DisplayFormat) {
		case _SHOW_DATE_24_HOUR_FORMAT_:
			// 24-hour format with date
			swprintf(buffer, bufferSize, L"%04d-%02d-%02d %02d:%02d:%02d",
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond);
			break;

		case _SHOW_DATE_12_HOUR_FORMAT_: {
			// 12-hour format with AM/PM and date
			int hour = st.wHour % 12;
			if (hour == 0) hour = 12; // Convert 0 to 12 for AM/PM format
			const WCHAR* ampm = (st.wHour < 12) ? L"AM" : L"PM";

			swprintf(buffer, bufferSize, L"%04d-%02d-%02d %02d:%02d:%02d %s",
				st.wYear, st.wMonth, st.wDay,
				hour, st.wMinute, st.wSecond, ampm);
			break;
		}

		case _HIDE_DATE_24_HOUR_FORMAT_:
			// 24-hour format, no date
			swprintf(buffer, bufferSize, L"%02d:%02d:%02d",
				st.wHour, st.wMinute, st.wSecond);
			break;

		case _HIDE_DATE_12_HOUR_FORMAT_: {
			// 12-hour format with AM/PM, no date
			int hour = st.wHour % 12;
			if (hour == 0) hour = 12; // Convert 0 to 12 for AM/PM format
			const WCHAR* ampm = (st.wHour < 12) ? L"AM" : L"PM";

			swprintf(buffer, bufferSize, L"%02d:%02d:%02d %s",
				hour, st.wMinute, st.wSecond, ampm);
			break;
		}

		default:
			// Handle unexpected cases (optional, but good practice)
			buffer[0] = L'\0'; // Empty buffer in case of error
			break;
		}
	}
}
#pragma endregion

#pragma region Time Format
void ChangeTimeFormat(void) {
	switch (g_Config.DisplayFormat) {
	case _SHOW_DATE_24_HOUR_FORMAT_:
		g_Config.DisplayFormat = _SHOW_DATE_12_HOUR_FORMAT_;
		break;
	case _HIDE_DATE_24_HOUR_FORMAT_:
		g_Config.DisplayFormat = _HIDE_DATE_12_HOUR_FORMAT_;
		break;
	case _SHOW_DATE_12_HOUR_FORMAT_:
		g_Config.DisplayFormat = _SHOW_DATE_24_HOUR_FORMAT_;
		break;
	case _HIDE_DATE_12_HOUR_FORMAT_:
		g_Config.DisplayFormat = _HIDE_DATE_24_HOUR_FORMAT_;
		break;
	default:
		return; // Handle unexpected values safely
	}
	SaveDisplayFormat();
}

void ChangeShowDate(void) {
	switch (g_Config.DisplayFormat) {
	case _SHOW_DATE_24_HOUR_FORMAT_:
		g_Config.DisplayFormat = _HIDE_DATE_24_HOUR_FORMAT_;
		break;
	case _HIDE_DATE_24_HOUR_FORMAT_:
		g_Config.DisplayFormat = _SHOW_DATE_24_HOUR_FORMAT_;
		break;
	case _SHOW_DATE_12_HOUR_FORMAT_:
		g_Config.DisplayFormat = _HIDE_DATE_12_HOUR_FORMAT_;
		break;
	case _HIDE_DATE_12_HOUR_FORMAT_:
		g_Config.DisplayFormat = _SHOW_DATE_12_HOUR_FORMAT_;
		break;
	default:
		return; // Handle unexpected values safely
	}
	SaveDisplayFormat();
}
#pragma endregion

#pragma region Full screen functions
void ToggleFullScreen(HWND hwnd) {
	DWORD style = GetWindowLong(hwnd, GWL_STYLE);

	if (!g_bIsFullScreen) {
		// Take the menu off the main window
		SetMenu(hwnd, NULL);

		// Save the current window position
		GetWindowRect(hwnd, &g_rcWindow);

		// Remove title bar and borders
		SetWindowLong(hwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);

		// Get the monitor that contains the window
		HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

		MONITORINFO mi = { sizeof(mi) };
		if (GetMonitorInfo(hMonitor, &mi)) {
			// Set window size to full screen on the monitor
			SetWindowPos(hwnd, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
			g_bIsFullScreen = TRUE;
		}
	}
	else {
		// Restore the menu
		SetMenu(hwnd, g_hMenu);

		// Restore the original window style
		SetWindowLong(hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);

		// Restore the original window size
		SetWindowPos(hwnd, HWND_TOP, g_rcWindow.left, g_rcWindow.top,
			g_rcWindow.right - g_rcWindow.left, g_rcWindow.bottom - g_rcWindow.top,
			SWP_FRAMECHANGED);
		g_bIsFullScreen = FALSE;
	}
}
#pragma endregion

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

#define _WIN32_WINNT 0x0501
#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"
#include <wchar.h>
#include <time.h>
#include <CommDlg.h>
#include <ShellAPI.h>
#include <stdio.h>
#include <stdlib.h>

 // Version of common controls to link to. Changes the appearance of controls. https://learn.microsoft.com/en-us/windows/win32/controls/common-control-versions
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Structure to hold color information after being parsed
// Used for the custom color option. See ParseCustomColor
typedef struct GradientColor {
	TRIVERTEX tvColor1;
	TRIVERTEX tvColor2;
} GradientColor;

// Structure to hold registry values to hopefully optimize the memory footprint.
// Prevents repeated calls of Registry keys to check the value
typedef struct Config {
	BOOL Gradient;
	BOOL DVDLogo;
	BOOL CustomColor;
} Config;

// Global variables
const WCHAR* g_szMainClass = L"ClockWndClass"; // Constant string for registering the main window class. https://learn.microsoft.com/en-us/windows/win32/intl/registering-window-classes
const WCHAR* g_szSettingsClass = L"ClockSettingsWndClass"; // Same as above, except this one is for the Settings sub-window.
const WCHAR* g_szMainFont = L"Arial"; // Font to be rendered with the text. Change this to make the text look different!
const WCHAR* g_szRegKey = L"Software\\Jamie\\Clock\\Settings"; // Constant string for getting where the registry values for settings are stored. Based off of the following format: 'Software\Company Name\Application Name\Settings'			
HINSTANCE g_hInst; // Global variable for storing the handle for the instance for use whenever it is needed. I prefer this method over passing an HINSTANCE into each function.
HFONT g_hfMainFont; // Global variable for storing the main font that will be used to render the clock text. 
HFONT g_hfBtnFont; // Same as above, except this one is rendered smaller to fit in buttons. Is used in the settings sub-window. 
HWND g_hWndMain; // Global variable for storing the window handle for the main window of the application. This is the window where the clock itself is rendered. See MainWndProc. 
HWND g_hWndClockOut; // Global variable for storing the handle of the static control where the clock text is rendered. This is where the magic happens! The control is set to be the same dimensions as the window, thus giving it range of the entire window to move and bounce. See RenderText. 
HWND g_hWndSettings; // Global variable for storing the window handle for the settings window of the application. This is where the settings options are rendered. See SettingsWndProc & SaveConfiguration. 
HWND g_hWndSettingsSaveBtn;	// Global variable for storing the handle of the button for saving settings in the settings window. 
HWND g_hWndSettingsGradientCheck; // Global variable for storing the handle of the check box (button. See https://learn.microsoft.com/en-us/windows/win32/menurc/autocheckbox-control) for toggling the gradient rendering option on or off. 
HWND g_hWndSettingsDvdLogoCheck; // Same as above, except this toggles the bouncing, DVD logo effect when the text is rendered.
HWND g_hWndSettingsCustomColorsCheck; // Same as above, except this toggles using a custom color. See 1.5.0 changelog
HACCEL g_hAccel; // Accelerator for keyboard shortcuts for the About dialog and Settings window. Defined in resource script (Resource.rc). 
BOOL g_bIsFullScreen = FALSE; // Used for full screen functionality.
BOOL g_bUse24HourFormat = TRUE;	// Used for the time format switch. Added after everything else in this list.
BOOL g_bShowDate = TRUE; // Used for the date switch. Also added after everything else.
BOOL g_bSettingsShown = FALSE; // Used to prevent the user from creating as many settings windows as they want.
RECT g_rcWindow; // Used for full screen functionality.
GradientColor g_GradientColor; // Check the GradientColor comments
COLORREF g_bgColor; // Holds the color that is used if a custom color is parsed, but the gradient effect isn't used
Config g_Config; // Check the Config comments
#define TIMER_ID 0x101 // ID for refresh timer for renderer. See MainWndProc.
#define SETTINGS_SAVE_BTN_ID			0x1001 // ID for save button in the settings window. Saves configuration to registry upon push.
#define SETTINGS_GRADIENT_CHECK_ID		0x1002 // ID for check box in settings window for the gradient option. Assigned an ID to get it's check state.
#define SETTINGS_DVDLOGO_CHECK_ID		0x1003 // Same as above.
#define SETTINGS_CUSTOMCOLORS_CHECK_ID	0x1004 // Same as above.

// Forward declaration of functions
int Clamp(int, int, int); // Clamping function since native C does not feature one. Used for parsing RGB values in .color
ATOM RegisterMainClass(HINSTANCE); // Registers the class for the main window.
BOOL InitInstance(HINSTANCE, int); // Stores the HINSTANCE, creates the fonts, and shows the main window.
void CreateClock(HWND);	// Creates the static text control that the clock is rendered to. Takes the HWND parameter to use as the parent window for the text.
void ResizeText(HWND); // Function to dynamically resize the text.
void GetCurrentDateTime(WCHAR*, size_t); // Gets the system time and formats a wide-string to display it based off of the formats specified above. Takes a pointer to a WCHAR and a size_t to get the size of the buffer. https://cplusplus.com/reference/cwchar/swprintf/ 
ATOM RegisterSettingsClass(HINSTANCE); // Registers the class for the settings sub-window. 
BOOL InitSettings(void); // Does the same as InitInstance, except for settings. 
void CreateSettingsControls(HWND); // Creates and draws the controls for the settings window. 
void SaveConfiguration(void); // Gets the check state of the toggles and saves it to the registry keys. 
BOOL GradientUsed(void); // Reads the UseGradient value in the registry key for the application. This function determines whether the red gradient background will be rendered. 
BOOL DVDLogo(void); // Does the same as above, except it read the DVDLogoEffect value. Determines whether the text moves and bounces around the screen. 
void RestartApplication(void); // Restarts the application. 
int FormattedMessageBoxW(HWND, LPCWSTR, LPCWSTR, UINT, ...); // Formats a Windows message box. 
void ToggleFullScreen_Monitor1(HWND); // Toggles full screen for the main window. This goes to the first screen on the computer. 
void ToggleFullScreen_Monitor2(HWND); // Same as above, except this one will go to the second-most monitor on the computer.
BOOL RenderText(LPARAM); // Renders the text for the clock. Called by the loop in the MainWndProc. 
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM); // The window procedure for the main window. This is where the timer loop is and where the key events and such are. https://learn.microsoft.com/en-us/windows/win32/winmsg/window-procedures 
LRESULT CALLBACK SettingsWndProc(HWND, UINT, WPARAM, LPARAM); // Same as above, but, it's for, you guessed it, the settings sub-window! 
BOOL CALLBACK About_DLGProc(HWND, UINT, WPARAM, LPARAM); // A little different from the standard window procedure. This doesn't draw it's own controls, it takes messages, but the controls and it's layout are defined in the resource script. https://learn.microsoft.com/en-us/windows/win32/dlgbox/using-dialog-boxes 
void ChangeTimeFormat(void); // Changes the time format. I use these functions for simplicity.
void ChangeShowDate(void); // Same as above.
BOOL CustomColor(void); // Read the 'CustomColor' registry key to check whether the .color file should be parsed
void ParseCustomColor(); // Actually parses the 'CustomColor' registry value. Outputs it's results to g_bgColor & g_GradientColor.
BOOL CALLBACK MonitorEnumProc(HMONITOR, HDC, LPRECT, LPARAM); // Enumerates the monitors connected to the computer. Not exactly sure how this works. https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enumdisplaymonitors

// Specific errors I disabled because I either don't know how to fix them, or they don't impact the application.
#pragma warning(disable : 28251) // Inconsistent annotations. This one is one I disabled because it doesn't make or break anything. The only time I can think of that this error is actually shown is on the WinMain entry point. https://learn.microsoft.com/en-us/cpp/code-quality/c28251?view=msvc-170
#pragma warning(disable : 4047)	// Different levels of indirection. Has something to do with pointers. From what I read, it's how many *'s you have to go through to get to the actual data.
#pragma warning(disable : 4024)	// Different types of formal and actual parameters. If you haven't figured out, I am a novice programmer and have no clue what this does, or how to fix it. All I know, is in debugging, I found no impact that this warning made.
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_WIN95_CLASSES;
	if (!InitCommonControlsEx(&icex)) {
		FormattedMessageBoxW(NULL, L"A severe error occurred while starting up the application! (%lu)\r\nCommon controls failed to initialize.\r\n\r\nPointer to common controls structure: %p\r\n\r\nThe application will now exit.", L"Severe Error!", MB_OK | MB_ICONERROR, GetLastError(), &icex);
		return GetLastError();
	}

	// Register the window class and show the window.
	RegisterMainClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) {
		FormattedMessageBoxW(NULL, L"A severe error occurred while starting up the application! (%lu)\r\nThe instance of the application failed to initialize.\r\n\r\nThe application will now exit.", L"Severe Error!", MB_OK | MB_ICONERROR, GetLastError());
		return GetLastError();
	}

	// Main message loop. https://learn.microsoft.com/en-us/windows/win32/winmsg/using-messages-and-message-queues
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, g_hAccel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}

ATOM RegisterMainClass(HINSTANCE hInstance) {
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
	wcex.lpszClassName = g_szMainClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	g_hInst = hInstance;

	// Create the fonts
	g_hfMainFont = CreateFont(48, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, g_szMainFont);
	g_hfBtnFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, g_szMainFont);

	// Set config structure values to prevent repeated system calls
	g_Config.CustomColor = CustomColor();
	g_Config.DVDLogo = DVDLogo();
	g_Config.Gradient = GradientUsed();

	// Parses the .color file if the setting is set to true
	if (g_Config.CustomColor) {
		ParseCustomColor();
	}

	// Create the main window and show it.
	g_hWndMain = CreateWindowW(g_szMainClass, L"Clock", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
	if (!g_hWndMain) {
		return FALSE;
	}

	ShowWindow(g_hWndMain, nCmdShow);
	UpdateWindow(g_hWndMain);

	return TRUE;
}

BOOL CALLBACK About_DLGProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hwndDlg, IDOK);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		// Bug fix. in 1.4, the about dialog would not close when the X button was clicked, because WM_CLOSE was not handled.
		EndDialog(hwndDlg, 0);
		break;
	case WM_DESTROY:
		EndDialog(hwndDlg, NULL);
		return TRUE;
		break;
	}

	return FALSE;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		g_hAccel = LoadAccelerators(g_hInst, MAKEINTRESOURCE(IDR_MAINACCEL));
		CreateClock(hwnd);
		SetTimer(hwnd, TIMER_ID, 10, NULL);
		break;
	case WM_DESTROY:
		KillTimer(hwnd, TIMER_ID);
		DeleteObject(g_hfMainFont);
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
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_ABOUT: {
			DialogBox(g_hInst, IDD_ABOUTDLG, hwnd, About_DLGProc);
		}
					 break;
		case ID_SETTINGS:
			if (!g_bSettingsShown) {
				InitSettings();
				g_bSettingsShown = TRUE;
			}
			break;
		}
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		else if (wParam == VK_F11) {
			ToggleFullScreen_Monitor1(hwnd);
		}
		else if (wParam == VK_F12) {
			// Note, for those compiling with Visual Studio, pressing F12 will trigger a debug breakpoint.
			// This is hard-coded and requires a registry key to be changed. https://stackoverflow.com/questions/18997754/how-to-disable-f12-to-debug-application-in-visual-studio-2012
			// Also, you need to restart Visual Studio, and if it still does it, you need to restart your computer.
			ToggleFullScreen_Monitor2(hwnd);
		}
		else if (wParam == 'T') {
			// Switches whether 24-hour time format is used.
			ChangeTimeFormat();
		}
		else if (wParam == 'F') {
			// Changes whether the date is shown too.
			ChangeShowDate();
		}
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

void CreateClock(HWND hwnd) {
	g_hWndClockOut = CreateWindowW(WC_STATIC, L"Please wait while clock initializes...", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_OWNERDRAW, 0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);

	if (g_hfMainFont) {
		SendMessage(g_hWndClockOut, WM_SETFONT, (WPARAM)g_hfMainFont, TRUE);
	}
}

void ResizeText(HWND hwnd) {
	RECT rect;
	GetClientRect(hwnd, &rect);

	SetWindowPos(g_hWndClockOut, NULL, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOZORDER | SWP_NOACTIVATE);
}

void GetCurrentDateTime(WCHAR* buffer, size_t bufferSize) {
	if (!buffer || bufferSize == 0) return;

	SYSTEMTIME st;
	GetLocalTime(&st);  // Get local time

	if (g_bShowDate) {
		if (g_bUse24HourFormat) {
			// 24-hour format with date
			swprintf(buffer, bufferSize, L"%04d-%02d-%02d %02d:%02d:%02d",
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond);
		}
		else {
			// 12-hour format with AM/PM and date
			int hour = st.wHour % 12;
			if (hour == 0) hour = 12; // Convert 0 to 12 for AM/PM format
			const WCHAR* ampm = (st.wHour < 12) ? L"AM" : L"PM";

			swprintf(buffer, bufferSize, L"%04d-%02d-%02d %02d:%02d:%02d %s",
				st.wYear, st.wMonth, st.wDay,
				hour, st.wMinute, st.wSecond, ampm);
		}
	}
	else {
		// Only time output if date is not shown
		if (g_bUse24HourFormat) {
			// 24-hour format, no date
			swprintf(buffer, bufferSize, L"%02d:%02d:%02d",
				st.wHour, st.wMinute, st.wSecond);
		}
		else {
			// 12-hour format with AM/PM, no date
			int hour = st.wHour % 12;
			if (hour == 0) hour = 12; // Convert 0 to 12 for AM/PM format
			const WCHAR* ampm = (st.wHour < 12) ? L"AM" : L"PM";

			swprintf(buffer, bufferSize, L"%02d:%02d:%02d %s",
				hour, st.wMinute, st.wSecond, ampm);
		}
	}
}

LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		CreateSettingsControls(hwnd);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == SETTINGS_SAVE_BTN_ID) {
			SaveConfiguration();
		}
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			DestroyWindow(hwnd);
		}
	case WM_DESTROY:
		DestroyWindow(hwnd);
		g_bSettingsShown = FALSE; // Make sure the settings window will still work.
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

ATOM RegisterSettingsClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = SettingsWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = g_szSettingsClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	return RegisterClassEx(&wcex);
}

BOOL InitSettings(void) {
	RegisterSettingsClass(g_hInst);

	g_hWndSettings = CreateWindow(g_szSettingsClass, L"Settings", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 270, 190, g_hWndMain, NULL, g_hInst, NULL);
	if (!g_hWndSettings) {
		return FALSE;
	}

	ShowWindow(g_hWndSettings, SW_SHOW);
	UpdateWindow(g_hWndSettings);

	return TRUE;
}

void CreateSettingsControls(HWND hwnd) {
	g_hWndSettingsSaveBtn = CreateWindow(WC_BUTTON, L"Save", BS_DEFPUSHBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 160, 125, 100, 30, hwnd, (HMENU)SETTINGS_SAVE_BTN_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsSaveBtn, WM_SETFONT, (WPARAM)g_hfBtnFont, TRUE);

	g_hWndSettingsGradientCheck = CreateWindow(WC_BUTTON, L"Enable gradient", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 10, 200, 30, hwnd, (HMENU)SETTINGS_GRADIENT_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsGradientCheck, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	SendMessage(g_hWndSettingsGradientCheck, BM_SETCHECK, g_Config.Gradient ? BST_CHECKED : BST_UNCHECKED, 0);

	g_hWndSettingsDvdLogoCheck = CreateWindow(WC_BUTTON, L"DVD logo", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 40, 200, 30, hwnd, (HMENU)SETTINGS_DVDLOGO_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsDvdLogoCheck, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	SendMessage(g_hWndSettingsDvdLogoCheck, BM_SETCHECK, g_Config.DVDLogo ? BST_CHECKED : BST_UNCHECKED, 0);

	g_hWndSettingsCustomColorsCheck = CreateWindow(WC_BUTTON, L"Use custom colors", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 70, 200, 30, hwnd, (HMENU)SETTINGS_CUSTOMCOLORS_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsCustomColorsCheck, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	SendMessage(g_hWndSettingsCustomColorsCheck, BM_SETCHECK, g_Config.CustomColor ? BST_CHECKED : BST_UNCHECKED, 0);
}

void SaveConfiguration(void) {
	BOOL gradientChecked = SendMessage(g_hWndSettingsGradientCheck, BM_GETCHECK, 0, 0);
	BOOL dvdlogoChecked = SendMessage(g_hWndSettingsDvdLogoCheck, BM_GETCHECK, 0, 0);
	BOOL customcolorChecked = SendMessage(g_hWndSettingsCustomColorsCheck, BM_GETCHECK, 0, 0);
	HKEY hKey = NULL; // Optimized to use only HKEY because it writes to the same key every time.
	DWORD value = 0;

	// Used an LSTATUS to document errors if something goes wrong while writing registry keys.
	// I'm not entirely sure if this works, because it's never had an error while writing a registry key.
	LSTATUS lStatus = RegCreateKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);

	if (lStatus == 0) {
		value = gradientChecked;
		RegSetValueEx(hKey, L"Gradient", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		value = dvdlogoChecked;
		RegSetValueEx(hKey, L"DVDLogo", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		value = customcolorChecked;
		RegSetValueEx(hKey, L"CustomColor", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		RegCloseKey(hKey);
	}
	else {
		wchar_t descBuffer[512];
		// For anyone that doesn't know, the sizeof operator for wide (wchar_t) strings, you must divide the sizeof the buffer itself by the sizeof wchar_t.
		// When handling ANSI strings, you can just write sizeof(string), but when handling wide characters, it is different.
		// This is because Unicode is 16 bit as opposed to 8 bit ASCII characters. This is what allows Unicode to have special characters like ˣ and Д. 
		// https://en.wikipedia.org/wiki/List_of_Unicode_characters
		size_t bufSize = sizeof(descBuffer) / sizeof(wchar_t);
		ZeroMemory(descBuffer, bufSize * sizeof(wchar_t));

		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			lStatus,
			0,
			descBuffer,
			(DWORD)bufSize,
			NULL
		);

		FormattedMessageBoxW(NULL, L"Error while trying to save registry values. (0x%08lx)\r\nThe LSTATUS for RegCreateKeyEx (%p) returned (%lu).\r\n\r\nDescription: %s\r\n\r\nYour configuration has not been saved.", L"Error saving settings", MB_ICONERROR | MB_OK, GetLastError(), &lStatus, lStatus, descBuffer);
	}

	RestartApplication();
}

BOOL GradientUsed(void) {
	HKEY hKey;
	DWORD gradient = 0;
	DWORD size = sizeof(gradient);
	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, L"Gradient", NULL, NULL, (LPBYTE)&gradient, &size);
		RegCloseKey(hKey);
	}

	return gradient;
}

BOOL DVDLogo(void) {
	HKEY hKey;
	DWORD effect = 0;
	DWORD size = sizeof(effect);
	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, L"DVDLogo", NULL, NULL, (LPBYTE)&effect, &size); // Switched the registry value name for personal preference
		RegCloseKey(hKey);
	}

	return effect;
}

void RestartApplication(void) {
	WCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	if (CreateProcess(szPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		ExitProcess(0);  // Exit current process
	}
}

int FormattedMessageBoxW(HWND hwnd, LPCWSTR lpszFormat, LPCWSTR lpszCaption, UINT uType, ...) {
	wchar_t buffer[1024];
	va_list args;
	va_start(args, lpszFormat);
	vswprintf(buffer, 1024, lpszFormat, args);
	va_end(args);
	return MessageBoxW(hwnd, buffer, lpszCaption, uType);
}

void ToggleFullScreen_Monitor1(HWND hwnd) {
	DWORD style = GetWindowLong(hwnd, GWL_STYLE);

	if (!g_bIsFullScreen) {
		// Save the current window position
		GetWindowRect(hwnd, &g_rcWindow);

		// Remove title bar and borders
		SetWindowLong(hwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);

		// Get screen size
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// Set window size to full screen
		SetWindowPos(hwnd, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_FRAMECHANGED);
		g_bIsFullScreen = TRUE;
	}
	else {
		// Restore the original window style
		SetWindowLong(hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);

		// Restore the original window size
		SetWindowPos(hwnd, HWND_TOP, g_rcWindow.left, g_rcWindow.top,
			g_rcWindow.right - g_rcWindow.left, g_rcWindow.bottom - g_rcWindow.top,
			SWP_FRAMECHANGED);
		g_bIsFullScreen = FALSE;
	}
}

void ChangeTimeFormat(void) {
	if (g_bUse24HourFormat == TRUE) {
		g_bUse24HourFormat = FALSE;
	}
	else {
		g_bUse24HourFormat = TRUE;
	}
}

void ChangeShowDate(void) {
	if (g_bShowDate == TRUE) {
		g_bShowDate = FALSE;
	}
	else {
		g_bShowDate = TRUE;
	}
}

BOOL CustomColor(void) {
	HKEY hKey;
	DWORD value = 0;
	DWORD size = sizeof(value);
	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, L"CustomColor", NULL, NULL, (LPBYTE)&value, &size);
		RegCloseKey(hKey);
	}

	return value;
}

int Clamp(int min_value, int max_value, int value) {
	if (value > max_value) return max_value;
	if (value < min_value) return min_value;
	return value;
}

void ParseCustomColor(void) {
	// Pretty unorganized, but, it gets the job done!
	FILE* file = _wfopen(L".color", L"r, ccs=UTF-8"); // Uses _wfopen because we're compiling in Unicode.
	if (!file) {
		// If the file fails to open, set everything to it's default and show an error message.
		g_bgColor = RGB(0, 0, 0);
		g_GradientColor.tvColor1.Red = 255 * 257;
		g_GradientColor.tvColor1.Green = 0;
		g_GradientColor.tvColor1.Blue = 0;
		g_GradientColor.tvColor2.Red = 0;
		g_GradientColor.tvColor2.Green = 0;
		g_GradientColor.tvColor2.Blue = 0;
		FormattedMessageBoxW(NULL, L"Warning: could not open the file '.color'\r\nPlease confirm that .color exists and is in the same directory as the application.\r\n\r\nThe application will fallback to the default color scheme.", L"Warning", MB_OK | MB_ICONWARNING);
		return;
	}

	wchar_t line[128];
	int r1 = 0, g1 = 0, b1 = 0, r2 = 0, g2 = 0, b2 = 0;
	BOOL success;

	ZeroMemory(line, sizeof(line) / sizeof(wchar_t));

	// Parse the first color
	if (fgetws(line, sizeof(line) / sizeof(wchar_t), file)) {
		success = swscanf(line, L"( %d, %d, %d )", &r1, &g1, &b1) == 3;
		if (success) {
			r1 = Clamp(0, 255, r1);
			g1 = Clamp(0, 255, g1);
			b1 = Clamp(0, 255, b1);
		}
	}

	ZeroMemory(line, sizeof(line) / sizeof(wchar_t));

	if (GradientUsed() && fgetws(line, sizeof(line) / sizeof(wchar_t), file)) {
		success = swscanf(line, L"( %d, %d, %d )", &r2, &g2, &b2) == 3;
		if (success) {
			r2 = Clamp(0, 255, r2);
			g2 = Clamp(0, 255, g2);
			b2 = Clamp(0, 255, b2);
		}
	}

	g_bgColor = RGB(r1, g1, b1);
	g_GradientColor.tvColor1.Red = (COLOR16)(r1 * 257);
	g_GradientColor.tvColor1.Green = (COLOR16)(g1 * 257);
	g_GradientColor.tvColor1.Blue = (COLOR16)(b1 * 257);
	g_GradientColor.tvColor2.Red = (COLOR16)(r2 * 257);
	g_GradientColor.tvColor2.Green = (COLOR16)(g2 * 257);
	g_GradientColor.tvColor2.Blue = (COLOR16)(b2 * 257);

	fclose(file);
}

void ToggleFullScreen_Monitor2(HWND hWnd) {
	HMONITOR monitors[16] = { 0 };
	int monitorCount = GetSystemMetrics(SM_CMONITORS);

	if (monitorCount < 2) {
		ToggleFullScreen_Monitor1(hWnd); // Fall back to the regular full screen function if the user doesn't have a 2nd monitor
		return;
	}

	if (!g_bIsFullScreen) {
		GetWindowRect(hWnd, &g_rcWindow); // Store original window size and position
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)monitors);

		MONITORINFO mi = { sizeof(MONITORINFO) };
		if (GetMonitorInfo(monitors[monitorCount - 2], &mi)) {
			SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);
			SetWindowPos(hWnd, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_FRAMECHANGED | SWP_SHOWWINDOW);
			g_bIsFullScreen = TRUE;
		}
	}
	else {
		SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(hWnd, HWND_TOP,
			g_rcWindow.left, g_rcWindow.top,
			g_rcWindow.right - g_rcWindow.left,
			g_rcWindow.bottom - g_rcWindow.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);
		g_bIsFullScreen = FALSE;
	}
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC hDC, LPRECT lprcMonitor, LPARAM lParam) {
	UNREFERENCED_PARAMETER(hDC);
	UNREFERENCED_PARAMETER(lprcMonitor);

	HMONITOR* monitors = (HMONITOR*)lParam;
	int index = 0;
	monitors[index++] = hMon;
	return TRUE;
}
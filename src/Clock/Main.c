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

#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"
#include <wchar.h>
#include <time.h>
#include <CommDlg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __USE_COMMONCTRLS_6__
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif // __USE_COMMONCTRLS_6__


// Note: warning MSB8051 cannot be disabled via the editor. If you are building for Windows XP, and would like to disable the warning,
// you have to edit your Toolset.targets for the project settings.
// https://stackoverflow.com/questions/53841470/how-to-disable-warning-msb8051-support-for-targeting-windows-xp-is-deprecated
// Yes, I know that my code is very messy and unfortunately, I don't know how to fix it.

// To-do: Fix sizing bug where the text will get stuck outside of the screen when sizing.

// Global variables
// Variables should be completely Hungarian Notation-compliant.
const WCHAR* g_szMainClass = L"ClockWndClass";					// Constant string for registering the main window class. https://learn.microsoft.com/en-us/windows/win32/intl/registering-window-classes
const WCHAR* g_szSettingsClass = L"ClockSettingsWndClass";		// Same as above, except this one is for the Settings sub-window.
const WCHAR* g_szMainFont = L"Arial";							// Font to be rendered with the text. Change this to make the text look different!
const WCHAR* g_szRegKey = L"Software\\Jamie\\Clock\\Settings";	// Constant string for getting where the registry values for settings are stored. Based off of the following format: 'Software\Company Name\Application Name\Settings'			
HINSTANCE g_hInst;												// Global variable for storing the handle for the instance for use whenever it is needed. I prefer this method over passing an HINSTANCE into each function.
HFONT g_hfMainFont;												// Global variable for storing the main font that will be used to render the clock text. 
HFONT g_hfBtnFont;												// Same as above, except this one is rendered smaller to fit in buttons. Is used in the settings sub-window. 
HWND g_hWndMain;												// Global variable for storing the window handle for the main window of the application. This is the window where the clock itself is rendered. See MainWndProc. 
HWND g_hWndClockOut;											// Global variable for storing the handle of the static control where the clock text is rendered. This is where the magic happens! The control is set to be the same dimensions as the window, thus giving it range of the entire window to move and bounce. See RenderText. 
HWND g_hWndSettings;											// Global variable for storing the window handle for the settings window of the application. This is where the settings options are rendered. See SettingsWndProc & SaveConfiguration. 
HWND g_hWndSettingsSaveBtn;										// Global variable for storing the handle of the button for saving settings in the settings window. 
HWND g_hWndSettingsGradientCheck;								// Global variable for storing the handle of the check box (button. See https://learn.microsoft.com/en-us/windows/win32/menurc/autocheckbox-control) for toggling the gradient rendering option on or off. 
HWND g_hWndSettingsDvdLogoCheck;								// Same as above, except this toggles the bouncing, DVD logo effect when the text is rendered.
HACCEL g_hAccel;												// Accelerator for keyboard shortcuts for the About dialog and Settings window. Defined in resource script (Resource.rc). 
BOOL g_bIsFullScreen = FALSE;									// Used for full screen functionality.
BOOL g_bUse24HourFormat = TRUE;									// Used for the time format switch. Added after everything else in this list.
BOOL g_bShowDate = TRUE;										// Used for the date switch. Also added after everything else.
RECT g_rcWindow;												// Used for full screen functionality.
#define TIMER_ID 0x101											// ID for refresh timer for renderer. See MainWndProc.
#define SETTINGS_SAVE_BTN_ID			0x1001					// ID for save button in the settings window. Saves configuration to registry upon push.
#define SETTINGS_GRADIENT_CHECK_ID		0x1002					// ID for check box in settings window for the gradient option. Assigned an ID to get it's check state.
#define SETTINGS_DVDLOGO_CHECK_ID		0x1003					// Same as above.

// Forward declaration of functions
ATOM RegisterMainClass(HINSTANCE);								// Registers the class for the main window.
BOOL InitInstance(HINSTANCE, int);								// Stores the HINSTANCE, creates the fonts, and shows the main window.
void CreateClock(HWND);											// Creates the static text control that the clock is rendered to. Takes the HWND parameter to use as the parent window for the text.
void ResizeText(HWND);											// Function to dynamically resize the text.
void GetCurrentDateTime(WCHAR*, size_t);						// Gets the system time and formats a wide-string to display it based off of the formats specified above. Takes a pointer to a WCHAR and a size_t to get the size of the buffer. https://cplusplus.com/reference/cwchar/swprintf/ 
ATOM RegisterSettingsClass(HINSTANCE);							// Registers the class for the settings sub-window. 
BOOL InitSettings(void);										// Does the same as InitInstance, except for settings. 
void CreateSettingsControls(HWND);								// Creates and draws the controls for the settings window. 
void SaveConfiguration(void);									// Gets the check state of the toggles and saves it to the registry keys. 
BOOL GradientUsed(void);										// Reads the UseGradient value in the registry key for the application. This function determines whether the red gradient background will be rendered. 
BOOL DVDLogo(void);												// Does the same as above, except it read the DVDLogoEffect value. Determines whether the text moves and bounces around the screen. 
void RestartApplication(void);									// Restarts the application. 
int FormattedMessageBoxW(HWND, LPCWSTR, LPCWSTR, UINT, ...);	// Formats a Windows message box. 
void ToggleFullScreen_Monitor1(HWND);							// Toggles full screen for the main window. By default, this goes to the first screen on the computer. 
BOOL RenderText(LPARAM);										// Renders the text for the clock. Called by the loop in the MainWndProc. 
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);		// The window procedure for the main window. This is where the timer loop is and where the key events and such are. https://learn.microsoft.com/en-us/windows/win32/winmsg/window-procedures 
LRESULT CALLBACK SettingsWndProc(HWND, UINT, WPARAM, LPARAM);	// Same as above, but, it's for, you guessed it, the settings sub-window! 
BOOL CALLBACK About_DLGProc(HWND, UINT, WPARAM, LPARAM);		// A little different from the standard window procedure. This doesn't draw it's own controls, it takes messages, but the controls and it's layout are defined in the resource script. https://learn.microsoft.com/en-us/windows/win32/dlgbox/using-dialog-boxes 
void ChangeTimeFormat(void);									// Changes the time format. I use these functions for simplicity.
void ChangeShowDate(void);										// Same as above.

// Specific errors I disabled because I either don't know how to fix them, or they don't impact the application.
#pragma warning(disable : 28251)								// Inconsistent annotations. This one is one I disabled because it doesn't make or break anything. The only time I can think of that this error is actually shown is on the WinMain entry point. https://learn.microsoft.com/en-us/cpp/code-quality/c28251?view=msvc-170
#pragma warning(disable : 4047)									// Different levels of indirection. Has something to do with pointers. From what I read, it's how many *'s you have to go through to get to the actual data.
#pragma warning(disable : 4024)									// Different types of formal and actual parameters. If you haven't figured out, I am a novice programmer and have no clue what this does, or how to fix it. All I know, is in debugging, I found no impact that this warning made.
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
	case WM_DESTROY:
		EndDialog(hwndDlg, NULL);
		break;
	}

	return FALSE;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_CREATE:
		g_hAccel = LoadAccelerators(g_hInst, MAKEINTRESOURCE(IDR_MAINACCEL));
		CreateClock(hwnd);
		SetTimer(hwnd, TIMER_ID, 10, NULL); // 10ms for screen effects
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
			InitSettings();
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
		else if (wParam == 'T') {
			ChangeTimeFormat();
		}
		else if (wParam == 'F') {
			ChangeShowDate();
		}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL RenderText(LPARAM lParam) {
	if (!GradientUsed()) {
		LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
		HDC hdc = pDIS->hDC;
		RECT rect = pDIS->rcItem;

		// Create a memory DC for double buffering
		HDC hMemDC = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

		// Fill background with black
		HBRUSH hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
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

		if (DVDLogo()) {
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

		// Define the start color
		vertex[0].x = rect.left;
		vertex[0].y = rect.top;
		vertex[0].Red = 0xFFFF;
		vertex[0].Green = 0x0000;
		vertex[0].Blue = 0x0000;
		vertex[0].Alpha = 0x0000;	  // No transparency, ever


		// Define the end color (black)
		vertex[1].x = rect.right;
		vertex[1].y = rect.bottom;
		vertex[1].Red = 0x0000;       // No Red
		vertex[1].Green = 0x0000;     // No Green
		vertex[1].Blue = 0x0000;      // Black
		vertex[1].Alpha = 0x0000;     // No transparency

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

		if (DVDLogo()) {
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
	// Placeholder text.
	// Changes almost immediately because the refresh timer is by default 10 ms
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

	g_hWndSettings = CreateWindow(g_szSettingsClass, L"Settings", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 240, 150, g_hWndMain, NULL, g_hInst, NULL);
	if (!g_hWndSettings) {
		return FALSE;
	}

	ShowWindow(g_hWndSettings, SW_SHOW);
	UpdateWindow(g_hWndSettings);

	return TRUE;
}

void CreateSettingsControls(HWND hwnd) {
	g_hWndSettingsSaveBtn = CreateWindow(WC_BUTTON, L"Save", BS_DEFPUSHBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 130, 85, 100, 30, hwnd, (HMENU)SETTINGS_SAVE_BTN_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsSaveBtn, WM_SETFONT, (WPARAM)g_hfBtnFont, TRUE);

	g_hWndSettingsGradientCheck = CreateWindow(WC_BUTTON, L"Enable gradient", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 10, 200, 30, hwnd, (HMENU)SETTINGS_GRADIENT_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsGradientCheck, WM_SETFONT, (WPARAM)g_hfBtnFont, TRUE);
	SendMessage(g_hWndSettingsGradientCheck, BM_SETCHECK, GradientUsed() ? BST_CHECKED : BST_UNCHECKED, 0);

	g_hWndSettingsDvdLogoCheck = CreateWindow(WC_BUTTON, L"DVD logo", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 40, 200, 30, hwnd, (HMENU)SETTINGS_DVDLOGO_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsDvdLogoCheck, WM_SETFONT, (WPARAM)g_hfBtnFont, TRUE);
	SendMessage(g_hWndSettingsDvdLogoCheck, BM_SETCHECK, DVDLogo() ? BST_CHECKED : BST_UNCHECKED, 0);
}

void SaveConfiguration(void) {
	BOOL gradientChecked = SendMessage(g_hWndSettingsGradientCheck, BM_GETCHECK, 0, 0);
	BOOL dvdlogoChecked = SendMessage(g_hWndSettingsDvdLogoCheck, BM_GETCHECK, 0, 0);
	HKEY hKey_gradient;
	HKEY hKey_dvdlogo;
	RegCreateKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey_gradient, NULL);
	RegCreateKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey_dvdlogo, NULL);
	DWORD value = gradientChecked;
	RegSetValueEx(hKey_gradient, L"Gradient", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
	RegCloseKey(hKey_gradient);
	value = dvdlogoChecked;
	RegSetValueEx(hKey_dvdlogo, L"DVDLogoEffect", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
	RegCloseKey(hKey_dvdlogo);

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
		RegQueryValueEx(hKey, L"DVDLogoEffect", NULL, NULL, (LPBYTE)&effect, &size);
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

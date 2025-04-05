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

#ifndef __CLOCK_H__
#define __CLOCK_H__

// Ideintifiers for g_Context
#define __CONTEXT_CLOCK__ 0xC1
#define __CONTEXT_ABOUT__ 0xC2
#define __CONTEXT_SETTINGS__ 0xC3
#define __CONTEXT_ALARMS__ 0xC4

// Constants for format identifiers
#define _SHOW_DATE_24_HOUR_FORMAT_	0
#define _HIDE_DATE_24_HOUR_FORMAT_	1
#define _SHOW_DATE_12_HOUR_FORMAT_	2
#define _HIDE_DATE_12_HOUR_FORMAT_	3

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <WinInet.h>
#include <CommCtrl.h>
#include <wchar.h>
#include <time.h>
#include <CommDlg.h>
#include <ShellAPI.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "Config.h"
#include "resource.h"

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif // _WIN32_WINNT

#define _WIN32_WINNT 0x0501

#define szVERSION L"1.7.0-debug"
#define szBUILDDATE L"4/5/25"
#define szBUILDNUMBER L"30"

extern WCHAR* g_szMainFont; // Global variable for the font name. Changed to not be constant to accomodate for custom fonts.
extern UINT g_Context; // Context for what part of the UI is being shown. Useful for scanning the messages to check if the user clicked escape.

extern HINSTANCE g_hInst; // Global variable for storing the handle for the instance for use whenever it is needed. I prefer this method over passing an HINSTANCE into each function.
extern HFONT g_hfMainFont; // Global variable for storing the main font that will be used to render the clock text. 
extern HFONT g_hfBtnFont; // Same as above, except this one is rendered smaller to fit in buttons. Is used in the settings sub-window. 
extern HWND g_hWndMain; // Global variable for storing the window handle for the main window of the application. This is the window where the clock itself is rendered. See MainWndProc. 
extern HMENU g_hMenu; // Global variable for storing the handle for the menu. This is used to create the menu for the main window.

extern BOOL g_bGetTime; // Variable to check if the time is needed to be checked. This stops the application from crashing if an NTP server can't be reached

extern DWORD g_tidNTPThread; // Thread ID for the NTP refresh thread
extern HANDLE g_hNTPThread; // Handle for the thread itself.

// Structure to hold color information after being parsed
// Used for the custom color option. See ParseCustomColor
typedef struct __GradientColor {
	TRIVERTEX tvColor1;
	TRIVERTEX tvColor2;
} GradientColor;

extern GradientColor g_GradientColor; // Check the GradientColor comments
extern COLORREF g_bgColor; // Holds the color that is used if a custom color is parsed, but the gradient effect isn't used

LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM); // The window procedure for the main window. This is where the timer loop is and where the key events and such are. https://learn.microsoft.com/en-us/windows/win32/winmsg/window-procedures 
ATOM RegisterMainClass(HINSTANCE); // Registers the class for the main window.
BOOL InitInstance(HINSTANCE, int); // Stores the HINSTANCE, creates the fonts, and shows the main window.
BOOL CreateClock(void);
void CreateClockControl(HWND);	// Creates the static text control that the clock is rendered to. Takes the HWND parameter to use as the parent window for the text.0
BOOL RenderText(LPARAM); // Renders the text for the clock. Called by the loop in the MainWndProc. 
void ResizeText(HWND); // Function to dynamically resize the text.
void GetCurrentDateTime(WCHAR*, size_t); // Gets the system time and formats a wide-string to display it based off of the formats specified above. Takes a pointer to a WCHAR and a size_t to get the size of the buffer. https://cplusplus.com/reference/cwchar/swprintf/ 
void ToggleFullScreen(HWND); // Revised full screen function to let the user full screen the window on any monitor
void ChangeTimeFormat(void); // Changes time format
void ChangeShowDate(void); // Changes date format

// Formats a Windows message box. 
inline int FormattedMessageBox(HWND hwnd, LPCWSTR lpszFormat, LPCWSTR lpszCaption, UINT uType, ...) {
	wchar_t buffer[1024];
	va_list args;
	va_start(args, lpszFormat);
	vswprintf(buffer, 1024, lpszFormat, args);
	va_end(args);
	return MessageBoxW(hwnd, buffer, lpszCaption, uType);
}

#endif // !__CLOCK_H__

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

#include "AboutWindow.h"
#include "MathHelpers.h"
#include "Drawing.h"

#define szCLASS L"ClockAboutWndClass"

LRESULT CALLBACK AboutWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
		// Disable the main window
		CenterWindow(hwnd, NULL);
		CreateAboutControls(hwnd);
		break;
	case WM_SIZE:
		HandleAboutSize(hwnd);
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		RECT rc;
		GetClientRect(hwnd, &rc);

		// Draw the anti-aliased logo. C++ function that uses the extern "C" marker to be used in C.
		DrawImage(hdc, rc.right - 64 - 10, 10, 64, 64, IDB_ABTICON);

		EndPaint(hwnd, &ps);
	}
		break;
	case WM_DRAWITEM: {
		DRAWITEMSTRUCT* pDrawItem = (DRAWITEMSTRUCT*)lParam;
		if (pDrawItem->CtlID == ABOUT_TEXT_ID) {
			HDC hdc = pDrawItem->hDC;

			// Draw the text transparent
			SetBkMode(hdc, TRANSPARENT);

			SetTextColor(hdc, RGB(0, 0, 0));

			WCHAR buffer[1024];
			GetWindowText(g_hWndAboutText, buffer, 1024);

			DrawText(hdc, buffer, -1, &pDrawItem->rcItem, DT_LEFT);

			return TRUE;
		}
	}
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			DestroyWindow(hwnd);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ABOUT_OK_BTN_ID:
			DestroyWindow(hwnd);
			break;
		}
		break;
	case WM_NOTIFY: {
		// Handle the link click.
		// For some reason, Common Controls doesn't send a message, but it sends a notification.
		// https://learn.microsoft.com/en-us/windows/win32/controls/use-syslink-notifications
		LPNMHDR pnmh = (LPNMHDR)lParam;
		if (pnmh->idFrom == ABOUT_LINK_ID && pnmh->code == NM_CLICK) {
			// Open a link to the github page
			ShellExecute(NULL, L"open", L"https://github.com/Totally-A-Boar/XPClock", NULL, NULL, SW_SHOWNORMAL);
		}

		break;
	}
	case WM_DESTROY:
		g_Context = __CONTEXT_CLOCK__;
		EnableWindow(g_hWndMain, TRUE); // Make sure to re-enable the main window
		ShutdownGDIPlus(); // Shutdown GDI+ to prevent memory leaks. Since it is only used here it is being deactived here.
		SetForegroundWindow(g_hWndMain);
		DestroyWindow(hwnd);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

ATOM RegisterAboutClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = AboutWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szCLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	return RegisterClassEx(&wcex);
}

BOOL InitAbout(void) {
	RegisterAboutClass(g_hInst);

	g_hWndAbout = CreateWindow(szCLASS, L"About - Clock", WS_POPUP | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 270, 235, NULL, NULL, g_hInst, NULL);
	if (!g_hWndAbout) {
		return FALSE;
	}

	EnableWindow(g_hWndMain, FALSE);

	ShowWindow(g_hWndAbout, SW_SHOW);
	UpdateWindow(g_hWndAbout);

	return TRUE;
}

void CreateAboutControls(HWND hwnd) {
	g_hWndAboutOkBtn = CreateWindow(WC_BUTTON, L"Ok", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)ABOUT_OK_BTN_ID, g_hInst, NULL);
	SendMessage(g_hWndAboutOkBtn, WM_SETFONT, (WPARAM)g_hfBtnFont, (LPARAM)TRUE);

	g_hWndAboutText = CreateWindow(WC_STATIC, L"Insert text here", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, 10, 10, 181, 180, hwnd, (HMENU)ABOUT_TEXT_ID, g_hInst, NULL);
	SendMessage(g_hWndAboutText, WM_SETFONT, (WPARAM)g_hfBtnFont, (LPARAM)TRUE);
	WCHAR buffer[1024];
	GetAboutMessage(buffer, sizeof(buffer) / sizeof(wchar_t));
	SetWindowText(g_hWndAboutText, buffer);

	g_hWndAboutLink = CreateWindow(WC_LINK, L"<a>https://github.com/Totally-A-Boar/XPClock</a>", WS_CHILD | WS_VISIBLE | WS_OVERLAPPED, 10, 140, 250, 25, hwnd, (HMENU)ABOUT_LINK_ID, g_hInst, NULL);
	SendMessage(g_hWndAboutLink, WM_SETFONT, (WPARAM)g_hfBtnFont, (LPARAM)TRUE);
}

void HandleAboutSize(HWND hwnd) {
	RECT rc;
	GetClientRect(hwnd, &rc);

	int width = 100;
	int height = 30;

	int x = rc.right - width - 10;
	int y = rc.bottom - height - 10;

	// Move the window
	SetWindowPos(g_hWndAboutOkBtn, NULL, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}

void GetAboutMessage(WCHAR* buffer, size_t size) {
	// For ease of use, the string is defined in this function.
	ZeroMemory(buffer, size);
	swprintf(buffer, size, L"XPClock version %s\r\nBuild date: %s\r\nBuild number: %s\r\n\r\nCopyright© 2025 Jamie Howell. \r\nPublished under GNU GPL v2.0\r\n\r\nProject URL: ", szVERSION, szBUILDDATE, szBUILDNUMBER);
}
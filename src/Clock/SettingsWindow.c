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

#include "SettingsWindow.h"
#include "NTPClient.h"

#pragma warning(disable : 4024)
#pragma warning(disable : 4047)

#define szCLASS L"ClockSettingsWndClass"

const wchar_t* g_szTimeZones[] = {
	L"UTC−12:00 - Baker Island Time (BIT)",
	L"UTC−11:00 - Samoa Standard Time (SST)",
	L"UTC−10:00 - Hawaii-Aleutian Standard Time (HST)",
	L"UTC−09:30 - Marquesas Islands Time (MIT)",
	L"UTC−09:00 - Alaska Standard Time (AKST)",
	L"UTC−08:00 - Pacific Standard Time (PST)",
	L"UTC−07:00 - Mountain Standard Time (MST)",
	L"UTC−06:00 - Central Standard Time (CST)",
	L"UTC−05:00 - Eastern Standard Time (EST)",
	L"UTC−04:30 - Venezuelan Standard Time (VET)",
	L"UTC−04:00 - Atlantic Standard Time (AST)",
	L"UTC−03:30 - Newfoundland Standard Time (NST)",
	L"UTC−03:00 - Argentina/Brazil Time (ART/BRT)",
	L"UTC−02:00 - South Georgia Time (GST)",
	L"UTC−01:00 - Azores Standard Time (AZOT)",
	L"UTC±00:00 - Greenwich Mean Time (GMT)",
	L"UTC+01:00 - Central European Time (CET)",
	L"UTC+02:00 - Eastern European Time (EET)",
	L"UTC+03:00 - Moscow Standard Time (MSK)",
	L"UTC+03:30 - Iran Standard Time (IRST)",
	L"UTC+04:00 - Gulf Standard Time (GST)",
	L"UTC+04:30 - Afghanistan Time (AFT)",
	L"UTC+05:00 - Pakistan Standard Time (PKT)",
	L"UTC+05:30 - India Standard Time (IST)",
	L"UTC+05:45 - Nepal Time (NPT)",
	L"UTC+06:00 - Bangladesh Standard Time (BST)",
	L"UTC+06:30 - Cocos Islands Time (CCT)",
	L"UTC+07:00 - Indochina Time (ICT)",
	L"UTC+08:00 - China Standard Time (CST)",
	L"UTC+08:45 - Australian Central Western Time (ACWST)",
	L"UTC+09:00 - Japan Standard Time (JST)",
	L"UTC+09:30 - Australian Central Standard Time (ACST)",
	L"UTC+10:00 - Australian Eastern Standard Time (AEST)",
	L"UTC+10:30 - Lord Howe Time (LHST)",
	L"UTC+11:00 - Solomon Islands Time (SBT)",
	L"UTC+12:00 - New Zealand Standard Time (NZST)",
	L"UTC+12:45 - Chatham Islands Time (CHAST)",
	L"UTC+13:00 - Tonga Standard Time (TOT)",
	L"UTC+14:00 - Line Islands Time (LINT)"
};

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
	wcex.lpszClassName = szCLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	return RegisterClassEx(&wcex);
}

BOOL InitSettings(void) {
	RegisterSettingsClass(g_hInst);

	g_hWndSettings = CreateWindow(szCLASS, L"Settings", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 450, 330, NULL, NULL, g_hInst, NULL);
	if (!g_hWndSettings) {
		return FALSE;
	}

	ShowWindow(g_hWndSettings, SW_SHOW);
	UpdateWindow(g_hWndSettings);

	return TRUE;
}

void CreateSettingsControls(HWND hwnd) {
	// Long and messy control creation code
	g_hWndSettingsSaveBtn = CreateWindow(WC_BUTTON, L"Save", BS_PUSHBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_SAVE_BTN_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsSaveBtn, WM_SETFONT, (WPARAM)g_hfBtnFont, TRUE);

	g_hWndSettingsGradientCheck = CreateWindow(WC_BUTTON, L"Enable gradient", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_GRADIENT_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsGradientCheck, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	SendMessage(g_hWndSettingsGradientCheck, BM_SETCHECK, g_Config.Gradient ? BST_CHECKED : BST_UNCHECKED, 0);

	g_hWndSettingsDvdLogoCheck = CreateWindow(WC_BUTTON, L"DVD logo", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_DVDLOGO_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsDvdLogoCheck, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	SendMessage(g_hWndSettingsDvdLogoCheck, BM_SETCHECK, g_Config.DVDLogo ? BST_CHECKED : BST_UNCHECKED, 0);

	g_hWndSettingsCustomColorsCheck = CreateWindow(WC_BUTTON, L"Use custom colors", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 70, 200, 30, hwnd, (HMENU)SETTINGS_CUSTOMCOLORS_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsCustomColorsCheck, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	SendMessage(g_hWndSettingsCustomColorsCheck, BM_SETCHECK, g_Config.CustomColor ? BST_CHECKED : BST_UNCHECKED, 0);

	g_hWndSettingsFontPickerBtn = CreateWindow(WC_BUTTON, L"Pick font", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_FONTPICKER_BTN_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsFontPickerBtn, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);

	g_hWndSettingsFileText = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR, WC_EDIT, L"", ES_LEFT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_FILETEXT_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsFileText, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);

	// Check for the file path if the user has one set.
	wchar_t buffer[MAX_PATH];
	ZeroMemory(buffer, sizeof(buffer));
	wcscpy(buffer, GetColorFile());

	// $(LocalDir) was used as a marker to ensure that if the user has a file named clock.col, the app doesn't confuse them.
	// This if statement sets the string to just clock.col to make it look better and be more user-friendly.
	if (wcscmp(buffer, L"$(LocalDir)\\clock.col") == 0) {
		ZeroMemory(buffer, sizeof(buffer));
		wcscpy(buffer, L"clock.col");
	}

	SetWindowText(g_hWndSettingsFileText, buffer);

	g_hWndSettingsBrowseBtn = CreateWindow(WC_BUTTON, L"Browse", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_BROWSE_BTN_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsBrowseBtn, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);

	g_hWndSettingsFontText = CreateWindow(WC_STATIC, L"Font: ", WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_FONTTEXT_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsFontText, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);

	// Check the user's font. Returns Arial if the user doesn't have one set
	wchar_t fontBuffer[256];
	ZeroMemory(fontBuffer, sizeof(fontBuffer));
	wcscpy(fontBuffer, GetCustomFont());
	wchar_t out[256] = L"Font: ";
	wcscat(out, fontBuffer);

	SetWindowText(g_hWndSettingsFontText, out);

	g_hWndDropDownTimeSource = CreateWindow(WC_COMBOBOX, L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_TIME_SOURCE_DROPDOWN_ID, g_hInst, NULL);
	SendMessage(g_hWndDropDownTimeSource, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	SendMessage(g_hWndDropDownTimeSource, CB_ADDSTRING, 0, (LPARAM)L"Local (System Time)");
	SendMessage(g_hWndDropDownTimeSource, CB_ADDSTRING, 1, (LPARAM)L"Network (NTP)");
	SendMessage(g_hWndDropDownTimeSource, CB_SETCURSEL, g_TimeConfig.ts, 0); // Make the user's preference persistent
	
	g_hWndSettingsAddressLabel = CreateWindow(WC_STATIC, L"Address: ", WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_ADDRESS_LABEL_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsAddressLabel, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);

	g_hWndSettingsAddressEdit = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR, WC_EDIT, L"", ES_LEFT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_ADDRESS_EDIT_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsAddressEdit, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	ZeroMemory(buffer, sizeof(buffer));
	wcscpy(buffer, g_TimeConfig.address);
	SetWindowText(g_hWndSettingsAddressEdit, buffer);

	g_hWndSettingsPortLabel = CreateWindow(WC_STATIC, L"Port: ", WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_PORT_LABEL_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsPortLabel, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);

	g_hWndSettingsPortEdit = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR, WC_EDIT, L"", ES_LEFT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_PORT_EDIT_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsPortEdit, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	ZeroMemory(buffer, sizeof(buffer));
	_itow(g_TimeConfig.port, buffer, 10);
	SetWindowText(g_hWndSettingsPortEdit, buffer);

	g_hWndSettingsPortBtn = CreateWindow(UPDOWN_CLASS, NULL, WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_ARROWKEYS | UDS_NOTHOUSANDS, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_PORT_BTN_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsPortBtn, UDM_SETBUDDY, (WPARAM)g_hWndSettingsPortEdit, 0);
	SendMessage(g_hWndSettingsPortBtn, UDM_SETRANGE32, 0, (LPARAM)65535);
	SendMessage(g_hWndSettingsPortBtn, UDM_SETRANGE32, (WPARAM)1, (LPARAM)65535);
	SendMessage(g_hWndSettingsPortBtn, UDM_SETPOS32, 1, (LPARAM)g_TimeConfig.port);
	SendMessage(g_hWndSettingsPortEdit, EM_SETLIMITTEXT, 5, 0);

	g_hWndSettingsSyncLabel = CreateWindow(WC_STATIC, L"Sync (ms):", WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_SYNC_LABEL_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsSyncLabel, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);

	g_hWndSettingsSyncText = CreateWindowEx(WS_EX_CLIENTEDGE | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR, WC_EDIT, L"", ES_LEFT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_SYNC_TEXT_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsSyncText, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	_itow(g_TimeConfig.syncInterval, buffer, 10);
	SetWindowText(g_hWndSettingsSyncText, buffer);
	ZeroMemory(buffer, sizeof(buffer));

	g_hWndSettingsTimeZoneLabel = CreateWindow(WC_STATIC, L"Time zone:", WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_TIMEZONE_LABEL_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsTimeZoneLabel, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);

	g_hWndSettingsTimeZoneCombo = CreateWindow(WC_COMBOBOX, NULL, CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_TIMEZONE_COMBO_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsTimeZoneCombo, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	for (int i = 0; i < _countof(g_szTimeZones); ++i) {
		SendMessage(g_hWndSettingsTimeZoneCombo, CB_ADDSTRING, 0, (LPARAM)g_szTimeZones[i]);
	}
	SendMessage(g_hWndSettingsTimeZoneCombo, CB_SETCURSEL, GetTimeZone(), 0);

	g_hWndSettingsConsoleCheck = CreateWindow(WC_BUTTON, L"Enable console logging", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_CONSOLE_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsConsoleCheck, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	SendMessage(g_hWndSettingsConsoleCheck, BM_SETCHECK, g_Config.ConsoleEnabled ? BST_CHECKED : BST_UNCHECKED, 0);

	g_hWndSettingsTrayIconCheck = CreateWindow(WC_BUTTON, L"Enable tray icon", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_CONSOLE_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsTrayIconCheck, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	SendMessage(g_hWndSettingsTrayIconCheck, BM_SETCHECK, g_Config.TrayIconEnabled ? BST_CHECKED : BST_UNCHECKED, 0);

	g_hWndSettingsMenuCheck = CreateWindow(WC_BUTTON, L"Show menu in main window", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0, 0, 0, 0, hwnd, (HMENU)SETTINGS_MENU_CHECK_ID, g_hInst, NULL);
	SendMessage(g_hWndSettingsMenuCheck, WM_SETFONT, g_hfBtnFont, (LPARAM)TRUE);
	SendMessage(g_hWndSettingsMenuCheck, BM_SETCHECK, g_Config.MenuEnabled ? BST_CHECKED : BST_UNCHECKED, 0);

	// By default, set everything to disabled
	EnableWindow(g_hWndSettingsTimeZoneCombo, FALSE);
	EnableWindow(g_hWndSettingsAddressEdit, FALSE);
	EnableWindow(g_hWndSettingsPortEdit, FALSE);
	EnableWindow(g_hWndSettingsPortBtn, FALSE);
	EnableWindow(g_hWndSettingsSyncText, FALSE);
}

LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
		EnableWindow(g_hWndMain, FALSE);
		CreateSettingsControls(hwnd);
		CenterWindow(hwnd, NULL);
	}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == SETTINGS_SAVE_BTN_ID) {
			// Get the user's choices, even if they're not changed from before
			BOOL b1 = SendMessage(g_hWndSettingsGradientCheck, BM_GETCHECK, 0, 0);
			BOOL b2 = SendMessage(g_hWndSettingsDvdLogoCheck, BM_GETCHECK, 0, 0);
			BOOL b3 = SendMessage(g_hWndSettingsCustomColorsCheck, BM_GETCHECK, 0, 0);
			BOOL b4 = SendMessage(g_hWndSettingsTrayIconCheck, BM_GETCHECK, 0, 0);
			BOOL b5 = SendMessage(g_hWndSettingsConsoleCheck, BM_GETCHECK, 0, 0);
			BOOL b6 = SendMessage(g_hWndSettingsMenuCheck, BM_GETCHECK, 0, 0);
			int sel = SendMessage(g_hWndDropDownTimeSource, CB_GETCURSEL, 0, 0);

			g_TimeConfig.ts = sel;

			// Don't ping the server or update these settings if it's not necessary
			if (sel == 1) {
				wchar_t buf[256];
				ZeroMemory(buf, sizeof(buf));

				GetWindowText(g_hWndSettingsAddressEdit, buf, sizeof(buf) / sizeof(wchar_t));
				g_TimeConfig.address = _wcsdup(buf);

				ZeroMemory(buf, sizeof(buf));

				GetWindowText(g_hWndSettingsPortEdit, buf, sizeof(buf) / sizeof(wchar_t));
				int port = _wtoi(buf);
				g_TimeConfig.port = port;

				ZeroMemory(buf, sizeof(buf));
				GetWindowText(g_hWndSettingsSyncText, buf, sizeof(buf) / sizeof(wchar_t));
				int sync = _wtoi(buf);
				g_TimeConfig.syncInterval = sync;

				// Ensure the user inputted a valid address
				int nResult = PingNTPServer(&g_TimeConfig);
				if (nResult != -4) {
					FormattedMessageBox(NULL, L"Error while saving the current time configuration! PingNTPServer (%p)(%p) returned: %d\r\n\r\nPlease verify that\r\n  1. The address that you inputted is a valid address.\r\n  2. Your computer is connected to the internet and can reach the address you inputted.\r\n\r\nPlease change the information to valid information, or use system time to avoid having to do this.", L"Error while saving time configuration", MB_ICONWARNING | MB_OK, &nResult, &g_TimeConfig, nResult);
					return 1;
				}
			}

			SetTimeZone((int)SendMessage(g_hWndSettingsTimeZoneCombo, CB_GETCURSEL, 0, 0));
			SetTimeConfig(&g_TimeConfig);
			SaveConfiguration(b1, b2, b3, b4, b5, b6);
		}
		else if (LOWORD(wParam) == SETTINGS_FONTPICKER_BTN_ID) {
			PickFont();
		}
		else if (LOWORD(wParam) == SETTINGS_BROWSE_BTN_ID) {
			SetColorFile();
			SetWindowText(g_hWndSettingsFileText, GetColorFile());
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == SETTINGS_TIME_SOURCE_DROPDOWN_ID) {
			int sel = SendMessage(g_hWndDropDownTimeSource, CB_GETCURSEL, 0, 0);

			// Enable specific controls based off of the user selection
			if (sel == 1) {
				EnableWindow(g_hWndSettingsAddressEdit, TRUE);
				EnableWindow(g_hWndSettingsPortEdit, TRUE);
				EnableWindow(g_hWndSettingsPortBtn, TRUE);
				EnableWindow(g_hWndSettingsSyncText, TRUE);
				EnableWindow(g_hWndSettingsTimeZoneCombo, TRUE);
			}
			else {
				EnableWindow(g_hWndSettingsAddressEdit, FALSE);
				EnableWindow(g_hWndSettingsPortEdit, FALSE);
				EnableWindow(g_hWndSettingsPortBtn, FALSE);
				EnableWindow(g_hWndSettingsSyncText, FALSE);
				EnableWindow(g_hWndSettingsTimeZoneCombo, FALSE);
			}
		} 
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			DestroyWindow(hwnd);
		}
		break;
	case WM_SIZE:
		SizeSettingsControls(hwnd); // Windows automatically calls this when the window is created, but I use it to size and position the controls. It's not in the creation code because it's cleaner this way.
		break;
	case WM_DESTROY:
		EnableWindow(g_hWndMain, TRUE);
		SetForegroundWindow(g_hWndMain);
		DestroyWindow(hwnd);
		g_Context = __CONTEXT_CLOCK__;
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void SizeSettingsControls(HWND hwnd) {
	// Pretty messy, but it works pretty nicely.

	RECT rc;
	GetClientRect(hwnd, &rc);

	SetWindowPos(g_hWndSettingsSaveBtn, NULL, rc.right - 110, rc.bottom - 40, 100, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsGradientCheck, NULL, rc.left + 10, rc.top + 10, 125, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsDvdLogoCheck, NULL, rc.left + 10, rc.top + 40, 125, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsCustomColorsCheck, NULL, rc.left + 10, rc.top + 70, 135, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsFontText, NULL, rc.left + 12, rc.bottom - 55, 100, 15, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsFontPickerBtn, NULL, rc.left + 10, rc.bottom - 40, 100, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsFileText, NULL, rc.left + 10, rc.bottom - 90, rc.right - 130, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsBrowseBtn, NULL, rc.right - 110, rc.bottom - 90, 100, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndDropDownTimeSource, NULL, rc.left + 200, rc.top + 10, rc.right - 210, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsAddressLabel, NULL, rc.left + 200, rc.top + 40, 100, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsAddressEdit, NULL, rc.left + 200, rc.top + 55, rc.right - 210, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsPortLabel, NULL, rc.left + 200, rc.top + 78, 125, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsPortEdit, NULL, rc.left + 200, rc.top + 95, 80, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsSyncLabel, NULL, rc.left + 290, rc.top + 78, 115, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsSyncText, NULL, rc.left + 290, rc.top + 95, rc.right - 300, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsTimeZoneLabel, NULL, rc.left + 10, rc.top + 130, 120, 15, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsTimeZoneCombo, NULL, rc.left + 10, rc.top + 145, rc.right - 20, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsConsoleCheck, NULL, rc.left + 10, rc.bottom - 125, 160, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsTrayIconCheck, NULL, rc.left + 170, rc.bottom - 125, 120, 30, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(g_hWndSettingsMenuCheck, NULL, rc.left + 10, rc.top + 100, 178, 30, SWP_NOACTIVATE | SWP_NOZORDER);

	SendMessage(g_hWndSettingsPortBtn, UDM_SETBUDDY, (WPARAM)g_hWndSettingsPortEdit, 0);

	int sel = SendMessage(g_hWndDropDownTimeSource, CB_GETCURSEL, 0, 0);

	if (sel == 1) {
		EnableWindow(g_hWndSettingsAddressEdit, TRUE);
		EnableWindow(g_hWndSettingsPortEdit, TRUE);
		EnableWindow(g_hWndSettingsPortBtn, TRUE);
		EnableWindow(g_hWndSettingsSyncText, TRUE);
		EnableWindow(g_hWndSettingsTimeZoneCombo, TRUE);
	}
	else {
		EnableWindow(g_hWndSettingsAddressEdit, FALSE);
		EnableWindow(g_hWndSettingsPortEdit, FALSE);
		EnableWindow(g_hWndSettingsPortBtn, FALSE);
		EnableWindow(g_hWndSettingsSyncText, FALSE);
		EnableWindow(g_hWndSettingsTimeZoneCombo, FALSE);
	}
}
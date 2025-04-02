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

#ifndef __CLOCK_SETTINGSWINDOW_H__
#define __CLOCK_SETTINGSWINDOW_H__

#include "Clock.h"

// ID constants for the controls & for the WM_COMMAND messages
#define SETTINGS_SAVE_BTN_ID			0x1001
#define SETTINGS_GRADIENT_CHECK_ID		0x1002
#define SETTINGS_DVDLOGO_CHECK_ID		0x1003
#define SETTINGS_CUSTOMCOLORS_CHECK_ID	0x1004
#define SETTINGS_FONTPICKER_BTN_ID		0x1005
#define SETTINGS_FILETEXT_ID			0x1006
#define SETTINGS_BROWSE_BTN_ID			0x1007
#define SETTINGS_FONTTEXT_ID			0x1008
#define SETTINGS_TIME_SOURCE_DROPDOWN_ID 0x1009
#define SETTINGS_ADDRESS_LABEL_ID		0x100A
#define SETTINGS_ADDRESS_EDIT_ID		0x100B
#define SETTINGS_SYNC_LABEL_ID			0x100C
#define SETTINGS_SYNC_TEXT_ID			0x100D
#define SETTINGS_SYNC_BTN_ID			0x100E
#define SETTINGS_PORT_LABEL_ID			0x100F
#define SETTINGS_PORT_EDIT_ID			0x1010
#define SETTINGS_PORT_BTN_ID			0x1011
#define SETTINGS_TIMEZONE_LABEL_ID		0x1012
#define SETTINGS_TIMEZONE_COMBO_ID		0x1013
#define SETTINGS_TRAYICON_CHECK_ID		0x1014
#define SETTINGS_CONSOLE_CHECK_ID		0x1015
#define SETTINGS_MENU_CHECK_ID			0x1016

// Window handles for the controls
// Not adding comments here because there are so much
// These are also not marked as extern because they don't need to be referenced by anything else
HWND g_hWndSettings;
HWND g_hWndSettingsSaveBtn;
HWND g_hWndSettingsGradientCheck;
HWND g_hWndSettingsDvdLogoCheck;
HWND g_hWndSettingsCustomColorsCheck;
HWND g_hWndSettingsFontPickerBtn;
HWND g_hWndSettingsFileText;
HWND g_hWndSettingsBrowseBtn;
HWND g_hWndSettingsFontText;
HWND g_hWndDropDownTimeSource;
HWND g_hWndSettingsAddressLabel;
HWND g_hWndSettingsAddressEdit;
HWND g_hWndSettingsPortLabel;
HWND g_hWndSettingsPortEdit;
HWND g_hWndSettingsPortBtn;
HWND g_hWndSettingsSyncLabel;
HWND g_hWndSettingsSyncText;
HWND g_hWndSettingsSyncBtn;
HWND g_hWndSettingsTimeZoneLabel;
HWND g_hWndSettingsTimeZoneCombo;
HWND g_hWndSettingsTrayIconCheck;
HWND g_hWndSettingsConsoleCheck;
HWND g_hWndSettingsMenuCheck;

extern const wchar_t* g_szTimeZones[]; // Constant string array for every time zone. Defined in SettingsWindow because it is loaded into the combo box but it is also used in the NTP client

ATOM RegisterSettingsClass(HINSTANCE); // Registers the class for the settings sub-window. 
BOOL InitSettings(void); // Does the same as InitInstance, except for settings. 
void CreateSettingsControls(HWND); // Creates and draws the controls for the settings window.
LRESULT CALLBACK SettingsWndProc(HWND, UINT, WPARAM, LPARAM); // Same as above, but, it's for, you guessed it, the settings sub-window! 
void SizeSettingsControls(HWND); // Re-implemented this because it works better and separates the creation code from location code.

#endif // !__CLOCK_SETTINGSWINDOW_H__

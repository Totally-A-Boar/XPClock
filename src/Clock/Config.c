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

#include "Config.h"
#include "Colors.h"

// Define the extern marked variables
Config g_Config;
TimeConfig g_TimeConfig;
int g_nTimeZone;

void SaveConfiguration(BOOL b1, BOOL b2, BOOL b3, BOOL b4, BOOL b5, BOOL b6) {
	// Save the configuration to the registry
	HKEY hKey = NULL;
	DWORD value = 0;

	wprintf(L"Saving configuration. Settings values:\r\nGradientCheck: %d\r\nDVDLogoChecked: %d\r\nCustomColorChecked: %d\r\nTrayIconChecked: %d\r\nConsoleChecked: %d\r\nMenuEnabled: %d\r\n", b1, b2, b3, b4, b5, b6);

	// Create/Open the registry key
	LSTATUS lStatus = RegCreateKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);

	if (lStatus == 0) {
		// Set the values
		value = b1;
		RegSetValueEx(hKey, L"Gradient", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		value = b2;
		RegSetValueEx(hKey, L"DVDLogo", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		value = b3;
		RegSetValueEx(hKey, L"CustomColor", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		value = b4;
		RegSetValueEx(hKey, L"TrayIconEnabled", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		value = b5;
		RegSetValueEx(hKey, L"ConsoleEnabled", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		value = b6;
		RegSetValueEx(hKey, L"MenuEnabled", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

		RegCloseKey(hKey);
	}
	else {
		// Handle the error gracefully and inform the user
		wchar_t descBuffer[512];
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

		red();
		wprintf(L"Error while trying to save registry values. (0x%x)\r\nRegCreateKeyEx (%p) returned (%lu).\r\n\r\nDescription: %s\r\n\r\nYour configuration has not been saved.", GetLastError(), &lStatus, lStatus, descBuffer);
		reset();

		FormattedMessageBox(NULL, L"Error while trying to save registry values. (0x%x)\r\nRegCreateKeyEx (%p) returned (%lu).\r\n\r\nDescription: %s\r\n\r\nYour configuration has not been saved.", L"Clock", MB_ICONERROR | MB_OK, GetLastError(), &lStatus, lStatus, descBuffer);
		return; // Make sure to return the function to prevent the app from restarting
	}

	RestartApplication();
}

void RestartApplication(void) {
	WCHAR szPath[MAX_PATH]; // Get the path to the application.
	GetModuleFileName(NULL, szPath, MAX_PATH);

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	// Create a new process for the application
	if (CreateProcess(szPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		// Create a new process
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		ExitProcess(0);  // Exit current process
	}
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
		RegQueryValueEx(hKey, L"DVDLogo", NULL, NULL, (LPBYTE)&effect, &size);
		RegCloseKey(hKey);
	}

	return effect;
}

void ParseCustomColor(void) {
	wchar_t filePath[MAX_PATH];
	ZeroMemory(filePath, sizeof(filePath));

	// Get the path to the application
	if (GetModuleFileNameW(NULL, filePath, sizeof(filePath) / sizeof(wchar_t))) {
		PathRemoveFileSpecW(filePath);
	}

	size_t pathLen = wcslen(filePath);
	if (pathLen + 1 < MAX_PATH) {
		wcscat(filePath, L"\\"); // Ensure the path ends with a backslash
	}

	wchar_t colFile[MAX_PATH];
	wchar_t buffer[MAX_PATH];
	ZeroMemory(colFile, sizeof(colFile));
	ZeroMemory(buffer, sizeof(buffer));

	wcsncpy(buffer, GetColorFile(), MAX_PATH - 1);
	buffer[MAX_PATH - 1] = L'\0'; // Ensure null termination

	// $(LocalDir) is an identifier to the local directory of the application
	if (wcscmp(buffer, L"$(LocalDir)\\clock.col") == 0) {
		if (wcslen(filePath) + wcslen(L"clock.col") < MAX_PATH) {
			// If so, make it clock.col to look better for displaying
			wcscat(filePath, colFile);
			wcscat(colFile, L"clock.col");
		}
		else {
			red();
			wprintf(L"An error occurred while getting path for color file.\r\nThe buffer for the path overflowed.\r\n\r\nThe application will continue with the default color scheme.\r\n");
			reset();

			FormattedMessageBox(NULL, L"An error occurred while getting path for color file.\r\nThe buffer for the path overflowed.\r\n\r\nThe application will continue with the default color scheme.", L"Clock", MB_OK | MB_ICONERROR);
			g_bgColor = RGB(0, 0, 0);
			g_GradientColor.tvColor1.Red = 255 * 257;
			g_GradientColor.tvColor1.Green = 0;
			g_GradientColor.tvColor1.Blue = 0;
			g_GradientColor.tvColor2.Red = 0;
			g_GradientColor.tvColor2.Green = 0;
			g_GradientColor.tvColor2.Blue = 0;
			return;
		}
	}

	wcscpy(filePath, buffer);

	FILE* file = _wfopen(filePath, L"r, ccs=UTF-8"); // Uses _wfopen because we're compiling in Unicode.
	if (!file) {
		// If the file fails to open, set everything to it's default and show an error message.
		g_bgColor = RGB(0, 0, 0);
		g_GradientColor.tvColor1.Red = 255 * 257;
		g_GradientColor.tvColor1.Green = 0;
		g_GradientColor.tvColor1.Blue = 0;
		g_GradientColor.tvColor2.Red = 0;
		g_GradientColor.tvColor2.Green = 0;
		g_GradientColor.tvColor2.Blue = 0;

		yellow();
		wprintf(L"Could not open the file 'clock.col'\r\nPlease confirm that the file exists and is in the same directory as the application.\r\n\r\nThe application will fallback to the default color scheme.\r\n");
		reset();

		FormattedMessageBox(NULL, L"Could not open the file 'clock.col'\r\nPlease confirm that the file exists and is in the same directory as the application.\r\n\r\nThe application will fallback to the default color scheme.", L"Warning", MB_OK | MB_ICONWARNING);
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

	// If the gradient effect is used, read a second line.
	if (GradientUsed() && fgetws(line, sizeof(line) / sizeof(wchar_t), file)) {
		success = swscanf(line, L"( %d, %d, %d )", &r2, &g2, &b2) == 3;
		if (success) {
			r2 = Clamp(0, 255, r2);
			g2 = Clamp(0, 255, g2);
			b2 = Clamp(0, 255, b2);
		}
	}

	// Set the parse colors
	g_bgColor = RGB(r1, g1, b1);
	g_GradientColor.tvColor1.Red = (COLOR16)(r1 * 257);
	g_GradientColor.tvColor1.Green = (COLOR16)(g1 * 257);
	g_GradientColor.tvColor1.Blue = (COLOR16)(b1 * 257);
	g_GradientColor.tvColor2.Red = (COLOR16)(r2 * 257);
	g_GradientColor.tvColor2.Green = (COLOR16)(g2 * 257);
	g_GradientColor.tvColor2.Blue = (COLOR16)(b2 * 257);

	fclose(file);
}

void SaveDisplayFormat(void) {
	HKEY hKey;
	DWORD value = 0;

	LSTATUS lStatus = RegCreateKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
	if (lStatus == 0) {
		value = g_Config.DisplayFormat;
		RegSetValueEx(hKey, L"DisplayFormat", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
		RegCloseKey(hKey);
	}
	else {
		wchar_t descBuffer[512];
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

		red();
		wprintf(L"An error occurred while trying to save registry value. (0x%08lx)\r\nThe LSTATUS for RegCreateKeyEx (%p) returned (%lu).\r\n\r\nDescription: %s\r\n\r\nYour configuration has not been saved.", GetLastError(), &lStatus, lStatus, descBuffer);
		reset();

		FormattedMessageBox(NULL, L"An error occurred while trying to save registry value. (0x%08lx)\r\nThe LSTATUS for RegCreateKeyEx (%p) returned (%lu).\r\n\r\nDescription: %s\r\n\r\nYour configuration has not been saved.", L"Clock", MB_ICONERROR | MB_OK, GetLastError(), &lStatus, lStatus, descBuffer);
	}
}

int GetDisplayFormat(void) {
	HKEY hKey;
	DWORD value = 0;
	DWORD size = sizeof(value);

	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, L"DisplayFormat", NULL, NULL, (LPBYTE)&value, &size);
		RegCloseKey(hKey);
	}

	return (int)value;
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

void PickFont(void) {
	wprintf(L"Opening font dialog.\r\n");

	LOGFONTW lf;
	CHOOSEFONTW cf;

	ZeroMemory(&lf, sizeof(LOGFONTW));
	ZeroMemory(&cf, sizeof(CHOOSEFONTW));

	wcscpy(lf.lfFaceName, g_szMainFont);

	cf.lStructSize = sizeof(CHOOSEFONTW);
	cf.hwndOwner = g_hWndSettings;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;

	if (ChooseFontW(&cf)) {
		HKEY hKey;
		LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_SET_VALUE, &hKey);
		if (lResult == ERROR_SUCCESS) {
			wprintf(L"User selected %s font.\r\n", lf.lfFaceName);
			lResult = RegSetValueEx(hKey, L"CustomFont", NULL, (DWORD)REG_SZ, (const BYTE*)lf.lfFaceName, (lstrlen(lf.lfFaceName) + 1) * sizeof(wchar_t));
			RegCloseKey(hKey);
		}
	}
}

WCHAR* GetCustomFont(void) {
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 0;
	WCHAR* value = NULL;

	// Open the registry key
	if (RegOpenKeyExW(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		return L"Arial";
	}

	// Query the size of the value
	if (RegQueryValueExW(hKey, L"CustomFont", NULL, &dwType, NULL, &dwSize) != ERROR_SUCCESS || dwType != REG_SZ) {
		RegCloseKey(hKey);
		return L"Arial";
	}

	// Allocate memory for the value
	value = (WCHAR*)malloc(dwSize);
	if (value == NULL) {
		RegCloseKey(hKey);
		return L"Arial";
	}

	// Query the value
	if (RegQueryValueExW(hKey, L"CustomFont", NULL, NULL, (LPBYTE)value, &dwSize) != ERROR_SUCCESS) {
		free(value);
		RegCloseKey(hKey);
		return L"Arial";
	}

	// Close the registry key
	RegCloseKey(hKey);

	return value;
}

void SetColorFile(void) {
	wprintf(L"Opening file dialog for picking a custom color file.\r\n");

	OPENFILENAME ofn;
	wchar_t szFile[MAX_PATH] = L"";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hWndSettings;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Color files\0*.col\0All files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = L"Select .col file";
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		HKEY hKey;
		LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_SET_VALUE, &hKey);
		if (lResult == ERROR_SUCCESS) {
			lResult = RegSetValueEx(hKey, L"ColorFile", NULL, (DWORD)REG_SZ, (const BYTE*)szFile, (lstrlen(szFile) + 1) * sizeof(wchar_t));
			RegCloseKey(hKey);
		}
	}
}

WCHAR* GetColorFile(void) {
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 0;
	WCHAR* value = NULL;

	// Open the registry key
	if (RegOpenKeyExW(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		return L"$(LocalDir)\\clock.col";
	}

	// Query the size of the value
	if (RegQueryValueExW(hKey, L"ColorFile", NULL, &dwType, NULL, &dwSize) != ERROR_SUCCESS || dwType != REG_SZ) {
		RegCloseKey(hKey);
		return L"$(LocalDir)\\clock.col";
	}

	// Allocate memory for the value
	value = (WCHAR*)malloc(dwSize);
	if (value == NULL) {
		RegCloseKey(hKey);
		return L"$(LocalDir)\\clock.col";
	}

	// Query the value
	if (RegQueryValueExW(hKey, L"ColorFile", NULL, NULL, (LPBYTE)value, &dwSize) != ERROR_SUCCESS) {
		free(value);
		RegCloseKey(hKey);
		return L"$(LocalDir)\\clock.col";
	}

	// Close the registry key
	RegCloseKey(hKey);
	return value;
}

void SetTimeConfig(const TimeConfig* tc) {
	HKEY hKey;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
		wchar_t descBuffer[512];
		size_t bufSize = sizeof(descBuffer) / sizeof(wchar_t);
		ZeroMemory(descBuffer, bufSize * sizeof(wchar_t));

		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0,
			descBuffer,
			(DWORD)bufSize,
			NULL
		);

		red();
		wprintf(L"An error occurred while trying to save the time configuration to the registry. (0x%x)\r\n\r\nDescription: %s\r\n\r\nYour settings have not been saved.\r\n", GetLastError(), descBuffer);
		reset();

		FormattedMessageBox(NULL, L"An error occurred while trying to save the time configuration to the registry. (0x%x)\r\n\r\nDescription: %s\r\n\r\nYour settings have not been saved.", L"Clock", MB_ICONERROR | MB_OK, GetLastError(), descBuffer);
		return;
	}

	TimeConfigStorage temp = { 0 };
	temp.ts = (uint8_t)tc->ts;
	wcsncpy(temp.address, tc->address ? tc->address : L"", 64);
	temp.address[63] = L'\0'; // Ensure null-termination
	temp.syncInterval = (uint32_t)tc->syncInterval;
	temp.port = (uint32_t)tc->port;

	RegSetValueEx(hKey, L"TimeConfig", 0, REG_BINARY, (const BYTE*)&temp, sizeof(temp));

	RegCloseKey(hKey);
}

void GetTimeConfig(TimeConfig* tc) {
	HKEY hKey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		wchar_t descBuffer[512];
		size_t bufSize = sizeof(descBuffer) / sizeof(wchar_t);
		ZeroMemory(descBuffer, bufSize * sizeof(wchar_t));

		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0,
			descBuffer,
			(DWORD)bufSize,
			NULL
		);

		red();
		wprintf(L"An error occurred while trying to open the time configuration. (0x%x)\r\n\r\nDescription: %s\r\n\r\nYour settings cannot be read.\r\n", GetLastError(), descBuffer);
		reset();

		FormattedMessageBox(NULL, L"An error occurred while trying to open the time configuration. (0x%x)\r\n\r\nDescription: %s\r\n\r\nYour settings cannot be read.", L"Clock", MB_ICONERROR | MB_OK, GetLastError(), descBuffer);
		return;
	}

	TimeConfigStorage temp = { 0 };
	DWORD size = sizeof(temp);
	LSTATUS lStatus = RegQueryValueEx(hKey, L"TimeConfig", NULL, NULL, (LPBYTE)&temp, &size);
	RegCloseKey(hKey);

	if (lStatus != ERROR_SUCCESS || size != sizeof(temp)) {
		// Default fields.
		// This is pretty basic and if you don't know how to use network time, I advise you stick to this.
		// It's not ridiculously hard to learn though.
		tc->ts = 0;
		tc->address = _wcsdup(L"pool.ntp.org");
		tc->syncInterval = 3600000; // 1 hour
		tc->port = 123;

		return;
	}

	tc->ts = temp.ts;

	size_t addressLen = wcslen(temp.address) + 1;
	tc->address = (wchar_t*)malloc(addressLen * sizeof(wchar_t));
	if (tc->address) {
		wcscpy(tc->address, temp.address);
	}

	tc->syncInterval = (int)temp.syncInterval;
	tc->port = (int)temp.port;
}

int GetMatchingTimeZone(int bias) {
	// This code works despite me not thinking it would. 
	// This is the line that used to be here: sizeof(g_szTimeZones) / sizeof(g_szTimeZones[0])
	// However, this gave both a warning and an error. It still compiled, but the first sizeof statement returned 0
	// and upon removing that line it still worked the same, so I am keeping it out.
	for (int i = 0; i < (sizeof(g_szTimeZones[0])); i++) {
		int tzBiasHours = 0, tzBiasMinutes = 0;
		if (swscanf(g_szTimeZones[i], L"UTC%*[^0-9]%d:%d", &tzBiasHours, &tzBiasMinutes) >= 1) {
			int totalBias = (tzBiasHours * 60 + tzBiasMinutes) * -1; // Convert to Windows bias format
			if (totalBias == bias) {
				return i; // Return the matching index
			}
		}
	}
	return 0;
}


void SetTimeZone(int TimeZone) {
	HKEY hKey;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
		wchar_t descBuffer[512];
		size_t bufSize = sizeof(descBuffer) / sizeof(wchar_t);
		ZeroMemory(descBuffer, bufSize * sizeof(wchar_t));

		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0,
			descBuffer,
			(DWORD)bufSize,
			NULL
		);

		red();
		wprintf(L"An error occurred while trying to save the time zone to the registry. (0x%x)\r\n\r\nDescription: %s\r\n\r\nYour settings have not been saved.\r\n", GetLastError(), descBuffer);
		reset();

		FormattedMessageBox(NULL, L"An error occurred while trying to save the time zone to the registry. (0x%x)\r\n\r\nDescription: %s\r\n\r\nYour settings have not been saved.", L"Clock", MB_ICONERROR | MB_OK, GetLastError(), descBuffer);
		return;
	}

	if (RegSetValueEx(hKey, L"TimeZone", 0, REG_DWORD, (const BYTE*)&TimeZone, sizeof(TimeZone)) != ERROR_SUCCESS) {
		wchar_t descBuffer[512];
		size_t bufSize = sizeof(descBuffer) / sizeof(wchar_t);
		ZeroMemory(descBuffer, bufSize * sizeof(wchar_t));

		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0,
			descBuffer,
			(DWORD)bufSize,
			NULL
		);

		red();
		wprintf(L"An error occurred while trying to save the time zone to the registry. (0x%x)\r\n\r\nDescription: %s\r\n\r\nYour settings have not been saved.\r\n", GetLastError(), descBuffer);
		reset();

		FormattedMessageBox(NULL, L"An error occurred while trying to save the time zone to the registry. (0x%x)\r\n\r\nDescription: %s\r\n\r\nYour settings have not been saved.", L"Clock", MB_ICONERROR | MB_OK, GetLastError(), descBuffer);
		RegCloseKey(hKey);
		return;
	}

	RegCloseKey(hKey);
}

int GetTimeZone(void) {
	HKEY hKey;
	DWORD timeZoneValue;
	DWORD dataSize = sizeof(timeZoneValue);

	// Try to open registry key
	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		// Try to read the TimeZone value
		if (RegQueryValueEx(hKey, L"TimeZone", NULL, NULL, (LPBYTE)&timeZoneValue, &dataSize) == ERROR_SUCCESS) {
			RegCloseKey(hKey);
			return (int)timeZoneValue;
		}
		RegCloseKey(hKey);
	}

	// If registry read fails, fallback to system time zone
	TIME_ZONE_INFORMATION tzInfo;
	DWORD result = GetTimeZoneInformation(&tzInfo);

	int bias = tzInfo.Bias;
	if (result == TIME_ZONE_ID_DAYLIGHT) {
		bias += tzInfo.DaylightBias;
	}
	else if (result == TIME_ZONE_ID_STANDARD) {
		bias += tzInfo.StandardBias;
	}

	return GetMatchingTimeZone(bias);
}

BOOL TrayIconEnabled(void) {
	HKEY hKey;
	DWORD option = 0;
	DWORD size = sizeof(option);

	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, L"TrayIconEnabled", NULL, NULL, (LPBYTE)&option, &size); // Switched the registry value name for personal preference
		RegCloseKey(hKey);
	}

	return option;
}

BOOL ConsoleEnabled(void) {
	HKEY hKey;
	DWORD option = 0;
	DWORD size = sizeof(option);

	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, L"ConsoleEnabled", NULL, NULL, (LPBYTE)&option, &size); // Switched the registry value name for personal preference
		RegCloseKey(hKey);
	}

	return option;
}

BOOL MenuEnabled(void) {
	HKEY hKey;
	DWORD option = 0;
	DWORD size = sizeof(option);

	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey, L"MenuEnabled", NULL, NULL, (LPBYTE)&option, &size); // Switched the registry value name for personal preference
		RegCloseKey(hKey);
	}

	return option;
}

void CreateReg(void) {
	wprintf(L"Making sure registry keys exist.\r\n");
	HKEY hKey;
	LONG lResult;

	// Try to open the registry key
	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey);
	if (lResult == ERROR_SUCCESS) {
		RegCloseKey(hKey); // Key exists. Nothing else left to do here.
	}
	else if (lResult == ERROR_FILE_NOT_FOUND) {
		// If the key doesn't exist, create it
		lResult = RegCreateKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);

		if (lResult != ERROR_SUCCESS) {
			red();
			wprintf(L"An error occurred.\r\n");
			reset();
			FormattedMessageBox(NULL, L"Creating the registry key failed. This is required to run the application correctly.\r\n\r\nError status: 0x%x.\r\nFile: Config.c.\r\nLine: 668.\r\n\r\nRecommended actions:\r\n\tCreate the registry key manually. A guide is available on the Github page.\r\n\tCreate an issue on the Github page.\r\n\r\nExecution cannot proceed.", L"XPClock", MB_OK | MB_ICONERROR, lResult);
			exit(lResult);
		}
	}
	else {
		red();
		wprintf(L"An error occurred.\r\n");
		reset();

		FormattedMessageBox(NULL, L"An unknown error occurred while trying to create the registry key.\r\n\r\nError status: 0x%x.\r\nFile: Config.c.\r\nLine: 673.\r\n\r\nExecution cannot proceed.", L"XPClock", MB_OK | MB_ICONERROR, lResult);
		exit(lResult);
	}

	wprintf(L"Created registry key.\r\n");
}

BOOL ExportSettings(HWND hParent) {
	wchar_t szFile[MAX_PATH] = L"";

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hParent;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"Registry files (*.reg)\0*.reg\0All files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = L"Export settings to file";
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Show save as dialog
	if (!GetSaveFileName(&ofn)) {
		return FALSE;
	}

	// Open the registry key
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		MessageBox(hParent, L"Could not open registry key.\r\n\r\nYour settings have not been copied to a file.", L"XPClock", MB_ICONERROR);
		return FALSE;
	}

	// Save the registry key to the selected file
	LONG result = RegSaveKey(hKey, szFile, NULL);
	RegCloseKey(hKey);

	if (result != ERROR_SUCCESS) {
		FormattedMessageBox(NULL, L"Failed to export registry settings. Error code: 0x%x", L"XPClock", MB_ICONERROR, result);
		return FALSE;
	}

	MessageBox(NULL, L"Registry settings exported successfully!", L"XPClock", MB_ICONINFORMATION);
	return TRUE;
}
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

#ifndef __CLOCK_CONFIG_H__
#define __CLOCK_CONFIG_H__

// Possibly the most un-organized header in this application.

#include "SettingsWindow.h"
#include "Clock.h"
#include "MathHelpers.h"

// Structure to hold registry values to hopefully optimize the memory footprint.
// Prevents repeated calls of Registry keys to check the value
typedef struct __Config {
	BOOL Gradient;
	BOOL DVDLogo;
	BOOL CustomColor;
	int DisplayFormat;
	BOOL ConsoleEnabled;
	BOOL TrayIconEnabled;
	BOOL MenuEnabled;
} Config;

// Time configuration structure
// Stores data for the NTP client
typedef struct __TimeConfig {
	DWORD ts;
	wchar_t* address;
	unsigned int syncInterval;
	unsigned int port;
} TimeConfig;

// Storage structure for the TimeConfig structure
// Better suited for REG_BINARY storage type
#pragma pack(push, 1) // Ensure the structure is packed without padding
typedef struct __TimeConfigStorage {
	uint8_t ts;
	wchar_t address[64];
	uint32_t syncInterval;
	uint32_t port;
} TimeConfigStorage;
#pragma pack(pop)

// Function declarations
void SaveConfiguration(BOOL, BOOL, BOOL, BOOL, BOOL, BOOL); // Gets the check state of the toggles and saves it to the registry keys. 
BOOL GradientUsed(void); // Reads the UseGradient value in the registry key for the application. This function determines whether the red gradient background will be rendered. 
BOOL DVDLogo(void); // Does the same as above, except it read the DVDLogoEffect value. Determines whether the text moves and bounces around the screen. 
BOOL CustomColor(void); // Read the 'CustomColor' registry key to check whether the clock.col file should be parsed
void ParseCustomColor(void); // Parses the 'clock.col' file. Outputs it's results to g_bgColor & g_GradientColor.
int GetDisplayFormat(void); // Reads the display format from the registry
void SaveDisplayFormat(void); // Writes the display format to the registry
void RestartApplication(void); // Does exactly as the title implies and restarts the application
void PickFont(void); // Opens a pick font dialog and saves the result to the registry. 
WCHAR* GetCustomFont(void); // Returns the user picked font. Returns Arial if the user doesn't have a font set
void SetColorFile(void); // Opens a file picker dialog and sets the picked file path to the registry
WCHAR* GetColorFile(void); // Returns the null-terminated string to the file path
void SetTimeConfig(const TimeConfig*); // Sets the time configuration based off of the raw structure. Stores directly in bytes
void GetTimeConfig(TimeConfig*); // Reads the registry value for TimeConfig and restores the binary structure
int GetMatchingTimeZone(int); // Get's a time zone from the g_szTimeZones array based off of a number.
void SetTimeZone(int); // Sets the time zone from an integer
int GetTimeZone(void); // Returns the integer for the time zone
BOOL TrayIconEnabled(void); // Returns whether the tray icon is enabled
BOOL ConsoleEnabled(void); // Returns whether the console is enabled
BOOL MenuEnabled(void); // Returns whether the menu in the main window is enabled
void CreateReg(void); // Create the registry key on first run. Fixes a bug where the application would crash if the registry key didn't exist.

// Extern variables
extern Config g_Config; // Global configuration structure
extern TimeConfig g_TimeConfig; // Global time configuration structure
extern int g_nTimeZone; // Global identifier for the time zone

#endif // !__CLOCK_CONFIG_H__

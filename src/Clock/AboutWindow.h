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

#ifndef __CLOCK_ABOUTWINDOW_H__
#define __CLOCK_ABOUTWINDOW_H__

#include "Clock.h"

// Control ID's
#define ABOUT_OK_BTN_ID	    0x2001
#define ABOUT_TEXT_ID		0x2002
#define ABOUT_LINK_ID		0x2003

// Window handles
HWND g_hWndAbout;
HWND g_hWndAboutOkBtn;
HWND g_hWndAboutText;
HWND g_hWndAboutLink;

LRESULT CALLBACK AboutWndProc(HWND, UINT, WPARAM, LPARAM); // The window procedure for the About window
ATOM RegisterAboutClass(HINSTANCE); // Function to register the class
BOOL InitAbout(void); // Function to initialize the window
void CreateAboutControls(HWND); // Function to create the controls
void HandleAboutSize(HWND); // Set the button to the bottom right automatically
void GetAboutMessage(WCHAR*, size_t); // Function to get the about message

#endif // !__CLOCK_ABOUTWINDOW_H__

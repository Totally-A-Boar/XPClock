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

#ifndef __CLOCK_MATHHELPERS_H__
#define __CLOCK_MATHHELPERS_H__

#include <Windows.h>

// Clamping function since native C does not feature one. Used for parsing RGB values in clock.col
inline int Clamp(int min_value, int max_value, int value) {
	if (value > max_value) return max_value;
	if (value < min_value) return min_value;
	return value;
}

// Function to center a window in a parent.
// If parent is null, it centers the window in the middle of the screen.
inline void CenterWindow(HWND hWnd, HWND hParent) {
	RECT rcParent;
	int parentWidth, parentHeight;
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	RECT rcWindow;
	int windowWidth, windowHeight;
	int xPos, yPos;

	// Get the window's dimensions
	GetClientRect(hWnd, &rcWindow);
	windowWidth = rcWindow.right - rcWindow.left;
	windowHeight = rcWindow.bottom - rcWindow.top;

	if (hParent != NULL) {
		// Get the parent's dimensions if hParent is not NULL
		GetClientRect(hParent, &rcParent);
		parentWidth = rcParent.right - rcParent.left;
		parentHeight = rcParent.bottom - rcParent.top;

		// Calculate position relative to parent window
		xPos = (parentWidth - windowWidth) / 2 + rcParent.left;
		yPos = (parentHeight - windowHeight) / 2 + rcParent.top;
	}
	else {
		// If no parent, center on the screen
		xPos = (screenWidth - windowWidth) / 2;
		yPos = (screenHeight - windowHeight) / 2;
	}

	// Set the window position
	SetWindowPos(hWnd, HWND_TOP, xPos, yPos, 0, 0, SWP_NOSIZE);
}

#endif // !__CLOCK_MATHHELPERS_H__

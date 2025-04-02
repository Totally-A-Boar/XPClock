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

#ifndef __CLOCK_TRAY_ICON_H__
#define __CLOCK_TRAY_ICON_H__

#include "Clock.h"

LRESULT CALLBACK TrayProc(HWND, UINT, WPARAM, LPARAM); // WndProc for the tray icon
void StartTray(void); // Function to start the tray icon

#endif // !__CLOCK_TRAY_ICON_H__

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

#ifndef __CLOCK_DRAWING_H__
#define __CLOCK_DRAWING_H__

#include "Windows.h"

#ifdef __cplusplus
extern "C" { // Make sure to link the functions as C functions, because it uses GDI+.
#endif

	void DrawImage(HDC, int, int, int, int, int); // Draw an image using GDI+. Pulls from RCDATA.
	void ShutdownGDIPlus();

#ifdef __cplusplus
}
#endif

#endif // !__CLOCK_DRAWING_H__

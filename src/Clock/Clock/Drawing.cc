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

#include "Drawing.h"
#include <GdiPlus.h>

using namespace Gdiplus;

static ULONG_PTR gdiplusToken;

void DrawImage(HDC hdc, int x, int y, int width, int height, int resourceId) {
	HRSRC hResInfo;
	HGLOBAL hResData;
	void* pResData;

	// Initialize GDI+
	GdiplusStartupInput gpsi;
	GdiplusStartup(&gdiplusToken, &gpsi, NULL);

	// Load the resource data
	hResInfo = FindResource(NULL, MAKEINTRESOURCE(resourceId), RT_RCDATA);
	if (!hResInfo) {
		MessageBox(NULL, L"Failed to find resource.", L"Error", MB_OK | MB_ICONERROR);
		GdiplusShutdown(gdiplusToken);
		return;
	}

	hResData = LoadResource(NULL, hResInfo);
	if (!hResData) {
		MessageBox(NULL, L"Failed to load resource.", L"Error", MB_OK | MB_ICONERROR);
		GdiplusShutdown(gdiplusToken);
		return;
	}

	pResData = LockResource(hResData);
	if (!pResData) {
		MessageBox(NULL, L"Failed to lock resource.", L"Error", MB_OK | MB_ICONERROR);
		GdiplusShutdown(gdiplusToken);
		return;
	}

    // Create a stream from the resource data
    IStream* pStream = NULL;
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    if (FAILED(hr)) {
        MessageBox(NULL, L"Failed to create stream.", L"Error", MB_OK | MB_ICONERROR);
        GdiplusShutdown(gdiplusToken);
        return;
    }

    // Write resource data to the stream
    ULONG written;
    hr = pStream->Write(pResData, SizeofResource(NULL, hResInfo), &written);
    if (FAILED(hr)) {
        MessageBox(NULL, L"Failed to write to stream.", L"Error", MB_OK | MB_ICONERROR);
        pStream->Release();
        GdiplusShutdown(gdiplusToken);
        return;
    }

    // Load the image from the stream
    Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromStream(pStream);
    pStream->Release();

    if (pBitmap->GetLastStatus() != Gdiplus::Ok) {
        MessageBox(NULL, L"Failed to load PNG from resource.", L"Error", MB_OK | MB_ICONERROR);
        delete pBitmap;
        GdiplusShutdown(gdiplusToken);
        return;
    }

    // Set up the graphics object
    Gdiplus::Graphics graphics(hdc);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    // Draw the image stretched to the given width and height at position (x, y)
    graphics.DrawImage(pBitmap, x, y, width, height);

    // Cleanup
    delete pBitmap;
}

void ShutdownGDIPlus() {
	GdiplusShutdown(gdiplusToken);
}
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

#include "NTPClient.h"
#include "Colors.h"

#pragma warning(disable : 4244)

static time_t lastNTPTime = 0;
static DWORD lastSyncTick = 0;
static BOOL isNTPInitialized = FALSE;

int PingNTPServer(const TimeConfig* config) {
	if (!config || !config->address || config->port == 0) {
		return ERROR_INVALID_PARAMETER;
	}

	wprintf(L"Pinging %s", config->address);

	WSADATA wsaData;
	SOCKET sock = INVALID_SOCKET;
	struct addrinfo* result = NULL, hints;
	int ret = -4;

	// Initialize Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return WSAGetLastError();
	}

	// Prepare hints
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_DGRAM; // UDP
	hints.ai_protocol = IPPROTO_UDP;

	// Convert wchar_t* to char* for getaddrinfo
	char addrStr[256];
	wcstombs(addrStr, config->address, sizeof(addrStr));

	// Convert port number to string
	char portStr[6];
	sprintf(portStr, "%u", config->port);

	// Resolve address
	if (getaddrinfo(addrStr, portStr, &hints, &result) != 0) {
		WSACleanup();
		return WSAGetLastError();
	}

	// Create the socket
	sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sock == INVALID_SOCKET) {
		freeaddrinfo(result);
		WSACleanup();
		return WSAGetLastError();
	}

	// Set the timeout
	DWORD timeout = TIMEOUT_MS;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	// Attempt to send a small UDP packet
	char testPacket[48] = { 0 };
	if (sendto(sock, testPacket, sizeof(testPacket), 0, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
		ret = WSAGetLastError();
	}
	else {
		ret = -4;
	}

	closesocket(sock);
	freeaddrinfo(result);
	WSACleanup();

	if (ret == -4) {
		yellow();
		wprintf(L"Failed to ping %s! Please verify you are connected to the internet and that you inputted a valid web address.\r\n", config->address);
		reset();
	}

	return ret;
}

void GetNTPDateTime(void) {
	blue();
	wprintf(L"Syncing NTP time.\r\n");
	reset();

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		g_bGetTime = FALSE;
		FormattedMessageBox(NULL, L"Error getting network time: WSA Startup Failed (0x%x)", L"Error", MB_OK | MB_ICONERROR, WSAGetLastError());
		return;
	}

	struct sockaddr_in serverAddr;
	struct hostent* server;
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		g_bGetTime = FALSE;
		FormattedMessageBox(NULL, L"Error getting network time: socket error (0x%x)", L"Error", MB_OK | MB_ICONERROR, WSAGetLastError());
		WSACleanup();
		return;
	}

	char serverBuf[256];
	wcstombs(serverBuf, g_TimeConfig.address, 256);

	server = gethostbyname(serverBuf);
	if (!server) {
		g_bGetTime = FALSE;

		closesocket(sock);
		WSACleanup();
		FormattedMessageBox(NULL, L"Error getting network time: Host not found: %s\r\nWSAGetLastError: 0x%x\r\n\r\nPlease ensure you are connected to the internet and the address you are trying to connect to is valid and your computer can connect to it in general or switch to system time.", L"Error", MB_OK | MB_ICONERROR, WSAGetLastError());
		return;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	memcpy(&serverAddr.sin_addr.s_addr, server->h_addr, server->h_length);
	serverAddr.sin_port = htons(g_TimeConfig.port);

	unsigned char ntpPacket[48] = { 0 };
	ntpPacket[0] = 0x1B; // LI=0, Version=3, Mode=3 (Client)

	if (sendto(sock, (char*)ntpPacket, sizeof(ntpPacket), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		g_bGetTime = FALSE;
		FormattedMessageBox(NULL, L"Error getting network time: send failed (0x%x)", L"Error", MB_OK | MB_ICONERROR, WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return;
	}

	struct sockaddr_in recvAddr;
	int recvAddrSize = sizeof(recvAddr);
	if (recvfrom(sock, (char*)ntpPacket, sizeof(ntpPacket), 0, (struct sockaddr*)&recvAddr, &recvAddrSize) == SOCKET_ERROR) {
		g_bGetTime = FALSE;
		FormattedMessageBox(NULL, L"Error getting network time: recieve failed (0x%x)", L"Error", MB_OK | MB_ICONERROR, WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return;
	}

	closesocket(sock);
	WSACleanup();

	uint32_t seconds;
	memcpy(&seconds, &ntpPacket[40], sizeof(seconds));
	seconds = ntohl(seconds) - NTP_TIMESTAMP_DELTA;

	SetNTPTime((time_t)seconds);
}

void SetNTPTime(time_t ntpTime) {
	lastNTPTime = ntpTime;
	lastSyncTick = GetTickCount();
	isNTPInitialized = TRUE;
}

void GetAdjustedTime(struct tm* outputTm) {
	if (!isNTPInitialized) {
		time_t now = time(NULL);
		*outputTm = *gmtime(&now);
		return;
	}

	// Calculate elapsed time since last sync
	DWORD elapsed = GetTickCount() - lastSyncTick;
	time_t adjustedTime = lastNTPTime + (elapsed / 1000); // ms to seconds

	*outputTm = *gmtime(&adjustedTime);
}

void OutputNTPTime(WCHAR* buffer, size_t bufferSize) {
	if (!buffer) return;
	
	struct tm timeinfo;
	GetAdjustedTime(&timeinfo);

	if (g_nTimeZone >= 0 && g_nTimeZone < 39) {
		const wchar_t* tzString = g_szTimeZones[g_nTimeZone];
		int hours = 0, minutes = 0, sign = 1;

		const wchar_t* p = wcsstr(tzString, L"UTC");
		if (p) {
			p += 3; // Move past "UTC"

			// Check for ± sign
			if (*p == L'−' || *p == L'-') { sign = -1; p++; }
			else if (*p == L'+') { sign = 1; p++; }

			// Read hours and minutes
			swscanf(p, L"%d:%d", &hours, &minutes);
		}

		// Convert offset to minutes and adjust the time
		time_t rawTime = _mkgmtime(&timeinfo); // Convert struct tm to UTC time
		rawTime += sign * ((hours * 60 + minutes) * 60); // Apply offset in seconds
		gmtime_s(&timeinfo, &rawTime); // Convert back to struct tm
	}

	switch (g_Config.DisplayFormat) {
	case 0: // Show date, 24-hour format
		swprintf(buffer, bufferSize, L"%04d-%02d-%02d %02d:%02d:%02d",
			timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
			timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
		break;

	case 1: // Hide date, 24-hour format
		swprintf(buffer, bufferSize, L"%02d:%02d:%02d",
			timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
		break;

	case 2: { // Show date, 12-hour format
		int hour = timeinfo.tm_hour % 12;
		if (hour == 0) hour = 12;
		const WCHAR* ampm = (timeinfo.tm_hour < 12) ? L"AM" : L"PM";

		swprintf(buffer, bufferSize, L"%04d-%02d-%02d %02d:%02d:%02d %s",
			timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
			hour, timeinfo.tm_min, timeinfo.tm_sec, ampm);
		break;
	}

	case 3: { // Hide date, 12-hour format
		int hour = timeinfo.tm_hour % 12;
		if (hour == 0) hour = 12;
		const WCHAR* ampm = (timeinfo.tm_hour < 12) ? L"AM" : L"PM";

		swprintf(buffer, bufferSize, L"%02d:%02d:%02d %s",
			hour, timeinfo.tm_min, timeinfo.tm_sec, ampm);
		break;
	}

	default:
		// Fallback in case of an invalid DisplayFormat value
		wcscpy(buffer, L"Invalid format");
		break;
	}
}

DWORD WINAPI NTPThread(LPVOID lpParam) {
	UNREFERENCED_PARAMETER(lpParam);
	while (1) {
		GetNTPDateTime();
		Sleep(g_TimeConfig.syncInterval);
	}
}
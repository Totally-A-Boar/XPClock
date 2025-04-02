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

#ifndef __CLOCK_NTP_CLIENT_H__
#define __CLOCK_NTP_CLIENT_H__

#include "Clock.h"
#include "Config.h"

#define NTP_TIMESTAMP_DELTA 2208988800UL
#define TIMEOUT_MS 5000 // Give the server 5 seconds to respond

int PingNTPServer(const TimeConfig*); // Checks that the address is a valid address and the PC can reach it
void GetNTPDateTime(); // Gets the current time from the NTP server
void SetNTPTime(time_t); // Sets the internal time to a specific time_t
void GetAdjustedTime(struct tm*); // Gets the current time adjusted for local offsets. Prevents the clock from pulling from NTP every time the it needs to be called.
void OutputNTPTime(WCHAR*, size_t); // Outputs the current time from the NTP time source, exactly the same as the system time in Clock.c
DWORD WINAPI NTPThread(LPVOID); // Thread to update the time periodically. Uses the user-defined interval in the config.

#endif // !__CLOCK_NTP_CLIENT_H__

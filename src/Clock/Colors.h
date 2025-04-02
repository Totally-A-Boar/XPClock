/*
 * This file includes code from colors.hpp, originally licensed under the MIT License.
 * Copyright (c) Antônio Hugo. Original Github page: https://github.com/hugorplobo/colors.hpp.
 * The code has been slightly modified to suit the needs of this program.
 * 
 * The original MIT license is included below:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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

#ifndef COLORS_H_
#define COLORS_H_

// For some reason this file doesn't compile well on GCC

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#define COLORS_TARGET_WINDOWS
#include <windows.h>
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define COLORS_TARGET_POSIX
#endif

#if !defined(COLORS_USE_ANSI_ESCAPE) && !defined(COLORS_USE_WINDOWS_API)
#if defined(COLORS_TARGET_POSIX)
#define COLORS_USE_ANSI_ESCAPE
#elif defined(COLORS_TARGET_WINDOWS)
#define COLORS_USE_WINDOWS_API
#endif
#endif

#ifdef COLORS_USE_WINDOWS_API
inline void wset_attributes(int foreground, int background) {
    static WORD defaultAttributes = 0;
    HANDLE handleTerminal = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!defaultAttributes) {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (!GetConsoleScreenBufferInfo(handleTerminal, &info)) return;
        defaultAttributes = info.wAttributes;
    }

    if (foreground == -1 && background == -1) {
        SetConsoleTextAttribute(handleTerminal, defaultAttributes);
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(handleTerminal, &info)) return;

    if (foreground != -1) {
        info.wAttributes &= ~0x0F;
        info.wAttributes |= (WORD)foreground;
    }

    if (background != -1) {
        info.wAttributes &= ~0xF0;
        info.wAttributes |= (WORD)background;
    }

    SetConsoleTextAttribute(handleTerminal, info.wAttributes);
}
#endif

inline void reset() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[00m");
#else
    wset_attributes(-1, -1);
#endif
}

inline void bold() { printf("\033[1m"); }
inline void faint() { printf("\033[2m"); }
inline void italic() { printf("\033[3m"); }
inline void underline() { printf("\033[4m"); }
inline void blink() { printf("\033[5m"); }
inline void reverse() { printf("\033[7m"); }
inline void invisible() { printf("\033[8m"); }
inline void strikethrough() { printf("\033[9m"); }

inline void grey() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[30m");
#else
    wset_attributes(0, -1);
#endif
}

inline void red() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[31m");
#else
    wset_attributes(FOREGROUND_RED, -1);
#endif
}

inline void green() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[32m");
#else
    wset_attributes(FOREGROUND_GREEN, -1);
#endif
}

inline void yellow() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[33m");
#else
    wset_attributes(FOREGROUND_RED | FOREGROUND_GREEN, -1);
#endif
}

inline void blue() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[34m");
#else
    wset_attributes(FOREGROUND_BLUE, -1);
#endif
}

inline void magenta() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[35m");
#else
    wset_attributes(FOREGROUND_RED | FOREGROUND_BLUE, -1);
#endif
}

inline void cyan() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[36m");
#else
    wset_attributes(FOREGROUND_BLUE | FOREGROUND_GREEN, -1);
#endif
}

inline void white() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[37m");
#else
    wset_attributes(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, -1);
#endif
}

inline void on_grey() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[40m");
#else
    wset_attributes(-1, 0);
#endif
}

inline void on_red() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[41m");
#else
    wset_attributes(-1, BACKGROUND_RED);
#endif
}

inline void on_green() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[42m");
#else
    wset_attributes(-1, BACKGROUND_GREEN);
#endif
}

inline void on_yellow() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[43m");
#else
    wset_attributes(-1, BACKGROUND_RED | BACKGROUND_GREEN);
#endif
}

inline void on_blue() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[44m");
#else
    wset_attributes(-1, BACKGROUND_BLUE);
#endif
}

inline void on_magenta() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[45m");
#else
    wset_attributes(-1, BACKGROUND_RED | BACKGROUND_BLUE);
#endif
}

inline void on_cyan() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[46m");
#else
    wset_attributes(-1, BACKGROUND_BLUE | BACKGROUND_GREEN);
#endif
}

inline void on_white() {
#ifdef COLORS_USE_ANSI_ESCAPE
    printf("\033[47m");
#else
    wset_attributes(-1, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
#endif
}

#undef COLORS_TARGET_POSIX
#undef COLORS_TARGET_WINDOWS
#undef COLORS_USE_ANSI_ESCAPE
#undef COLORS_USE_WINDOWS_API

#endif /* COLORS_H_ */

/***************************************************************************
 * Gens: Main Loop. (Win32-specific code)                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef GENS_G_MAIN_WIN32_HPP
#define GENS_G_MAIN_WIN32_HPP

#ifdef __cplusplus
extern "C" {
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Win32 instance
extern HINSTANCE ghInstance;

// Windows version
extern OSVERSIONINFO winVersion;

// If extended Common Controls are enabled, this is set to non-zero.
extern int win32_CommCtrlEx;

// Set actual window size.
void Win32_setActualWindowSize(HWND hWnd, const int reqW, const int reqH);

// Get the default save path.
void get_default_save_path(char *buf, size_t size);

// TODO: Move this stuff to GensUI.
void Win32_centerOnGensWindow(HWND hWnd);
void Win32_clientResize(HWND hWnd, int width, int height);

#ifdef __cplusplus
}
#endif

#endif /* GENS_G_MAIN_WIN32_HPP */

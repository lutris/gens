/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_win32_gdi.h: Win32 convenience functions. (GDI Stuff)              *
 *                                                                         *
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

#ifndef __GSFT_WIN32_GDI_H
#define __GSFT_WIN32_GDI_H

#ifndef _WIN32
#error Do not include libgsft/gsft_win32_gdi.h on non-Win32 platforms!
#else

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "gsft_fncall.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * gsft_win32_gdi_init(): Initialize (or reinitialize) font handling.
 * @param hWnd hWnd for setting up dialog unit conversion.
 */
DLL_LOCAL void WINAPI gsft_win32_gdi_init(HWND hWnd);

/**
 * gsft_win32_gdi_end(): Shut down font handling.
 */
DLL_LOCAL void WINAPI gsft_win32_gdi_end(void);

/**
 * Fonts. DO NOT modify these externally!
 */
extern HFONT w32_fntMessage;		// Message font.
extern HFONT w32_fntTitle;		// Title font.
extern HFONT w32_fntMonospaced;		// Monospaced font.

/**
 * Dialog unit conversion.
 * DO NOT modify these externally!
 */
extern int w32_dlu_x;
extern int w32_dlu_y;

// 4 == horizontal dialog template units.
// 8 == vertical dialog template units.
#define DLU_X(dlu) ((w32_dlu_x * (dlu)) / 4)
#define DLU_Y(dlu) ((w32_dlu_y * (dlu)) / 8)

#ifdef __cplusplus
}
#endif

#endif /* _WIN32 */

#endif /* __MDP_WIN32_H */

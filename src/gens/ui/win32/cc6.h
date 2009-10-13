/***************************************************************************
 * Gens: (Win32) Common Controls 6.                                        *
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

#ifndef GENS_WIN32_CC6_H
#define GENS_WIN32_CC6_H

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>

#ifdef __cplusplus
extern "C" {
#endif

// w32api's headers don't include BUTTON_IMAGELIST or related macros and constants.
#ifndef BCM_SETIMAGELIST
typedef struct
{
	HIMAGELIST himl;
	RECT margin;
	UINT uAlign;
} BUTTON_IMAGELIST, *PBUTTON_IMAGELIST;
#define BUTTON_IMAGELIST_ALIGN_LEFT 0
#define BCM_FIRST 0x1600
#define BCM_SETIMAGELIST (BCM_FIRST + 0x0002)
#define Button_SetImageList(hWnd, pbuttonImageList) \
	SendMessage((hWnd), BCM_SETIMAGELIST, 0, (LPARAM)(pbuttonImageList))
#endif

// comctl32.dll version.
typedef enum _CC6_STATUS_T
{
	CC6_STATUS_UNKNOWN = 0,		// DLL version hasn't been checked yet.
	CC6_STATUS_OLD     = 1,		// Older version of comctl32.dll. (pre-6.0)
	CC6_STATUS_V6      = 2		// Newer version of comctl32.dll. (6.0 or later)
} CC6_STATUS_T;

CC6_STATUS_T cc6_check(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_WIN32_CC6_H */

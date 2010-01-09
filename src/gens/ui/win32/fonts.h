/***************************************************************************
 * Gens: (Win32) Font functions and variables.                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#ifndef GENS_UI_WIN32_FONTS_H
#define GENS_UI_WIN32_FONTS_H

// Win32 includes.
#include "libgsft/w32u/w32u.h"

#ifdef __cplusplus
extern "C" {
#endif

extern HFONT fntMain;
extern HFONT fntTitle;
extern HFONT fntMono;

void WINAPI fonts_init(void);
void WINAPI fonts_end(void);

/** Dialog units. **/
extern int dlu_x;
extern int dlu_y;

void WINAPI dlu_init(HWND hWnd);

// 4 == horizontal dialog template units.
// 8 == vertical dialog template units.
#define DLU_X(dlu) ((dlu_x * (dlu)) / 4)
#define DLU_Y(dlu) ((dlu_y * (dlu)) / 8)

#ifdef __cplusplus
}
#endif

#endif /* GENS_UI_WIN32_FONTS_H */

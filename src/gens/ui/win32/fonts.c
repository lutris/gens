/***************************************************************************
 * Gens: (Win32) Font functions and variables.                             *
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

#include "fonts.h"

// Gens window.
#include "gens/gens_window.h"

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>


// Fonts.
HFONT fntMain;
HFONT fntTitle;
HFONT fntMono;
HFONT fntDebug;


/**
 * fonts_init(): Initialize the Win32 fonts.
 */
void fonts_init(void)
{
	// Get the system message font.
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	
	// Use the system message font as the main font.
	fntMain = CreateFontIndirect(&ncm.lfMessageFont);
	
	// Create the title font.
	// Title font is the main font with bold and italics.
	ncm.lfMessageFont.lfItalic = 1;
	ncm.lfMessageFont.lfWeight = FW_BOLD;
	fntTitle = CreateFontIndirect(&ncm.lfMessageFont);
	
	// Set up the monospaced font. (WinXP: Fixedsys)
	fntMono = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
	
	// Set up the debug font. (WinXP: Terminal)
	fntDebug = (HFONT)GetStockObject(OEM_FIXED_FONT);
}


/**
 * fonts_end(): Delete all Win32 fonts.
 */
void fonts_end(void)
{
	DeleteFont(fntMain);
	fntMain = NULL;
	
	DeleteFont(fntTitle);
	fntTitle = NULL;
}

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
#include <windows.h>
#include <windowsx.h>

// Unused parameter macro.
#include "macros/unused.h"


// Fonts.
HFONT fntMain;
HFONT fntTitle;
HFONT fntMono;

// Font enumeration callbacks.
static int CALLBACK font_enum_callback_fixedsys(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme,
						DWORD FontType, LPARAM lParam);
static int CALLBACK font_enum_callback_courier_new(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme,
						   DWORD FontType, LPARAM lParam);


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
	
	// Enumerate the fonts to find a monospaced font.
	LOGFONT lfMonoFont;
	lfMonoFont.lfCharSet = DEFAULT_CHARSET;
	lfMonoFont.lfPitchAndFamily = 0;
	strcpy(lfMonoFont.lfFaceName, "Fixedsys");
	EnumFontFamiliesEx(GetDC(gens_window), &lfMonoFont, font_enum_callback_fixedsys, 0, 0);
	strcpy(lfMonoFont.lfFaceName, "Courier New");
	EnumFontFamiliesEx(GetDC(gens_window), &lfMonoFont, font_enum_callback_courier_new, 0, 0);
	
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
	
	DeleteFont(fntMono);
	fntMono = NULL;
}


/**
 * font_enum_callback_fixedsys(): Callback for finding the "Fixedsys" font.
 * @param lpelfe
 * @param lpntme
 * @param FontType
 * @param lParam
 */
static int CALLBACK font_enum_callback_fixedsys(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme,
						DWORD FontType, LPARAM lParam)
{
	GENS_UNUSED_PARAMETER(lpntme);
	GENS_UNUSED_PARAMETER(FontType);
	GENS_UNUSED_PARAMETER(lParam);
	
	// Verify that this is the correct font.
	if (strcasecmp(lpelfe->elfLogFont.lfFaceName, "Fixedsys"))
	{
		// Incorrect font. Continue enumeration.
		return 1;
	}
	
	// If a font has been created already, delete it.
	if (fntMono)
		DeleteFont(fntMono);
	
	// Create the "Fixedsys" font.
	LOGFONT lfMono;
	memset(&lfMono, 0x00, sizeof(lfMono));
	strcpy(lfMono.lfFaceName, "Fixedsys");
	fntMono = CreateFontIndirect(&lfMono);
	
	// Stop enumeration.
	return 0;
}


/**
 * font_enum_callback_courier_new(): Callback for finding the "Courier New" font.
 * @param lpelfe
 * @param lpntme
 * @param FontType
 * @param lParam
 */
static int CALLBACK font_enum_callback_courier_new(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme,
						   DWORD FontType, LPARAM lParam)
{
	GENS_UNUSED_PARAMETER(lpntme);
	GENS_UNUSED_PARAMETER(FontType);
	GENS_UNUSED_PARAMETER(lParam);
	
	// Verify that this is the correct font.
	if (strcasecmp(lpelfe->elfLogFont.lfFaceName, "Courier New"))
	{
		// Incorrect font. Continue enumeration.
		return 1;
	}
	
	// If a font has been created already, don't do anything.
	if (fntMono)
		return 0;
	
	// Create the "Courier New" font.
	LOGFONT lfMono;
	memset(&lfMono, 0x00, sizeof(lfMono));
	strcpy(lfMono.lfFaceName, "Courier New");
	fntMono = CreateFontIndirect(&lfMono);
	
	// Stop enumeration.
	return 0;
}

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

#include "fonts.h"

// Gens window.
#include "gens/gens_window.h"

// Win32 includes.
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_windowsx.h"


// Fonts.
HFONT fntMain;
HFONT fntTitle;
HFONT fntMono;

// Dialog base units.
int dlu_x = 4;
int dlu_y = 4;


/**
 * fonts_init(): Initialize the Win32 fonts.
 */
void WINAPI fonts_init(void)
{
	/**
	 * NOTE: This function does not need Unicode translation,
	 * since it passes the text directly from SystemParametersInfo().
	 */
	
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
}


/**
 * fonts_end(): Delete all Win32 fonts.
 */
void WINAPI fonts_end(void)
{
	DeleteFont(fntMain);
	fntMain = NULL;
	
	DeleteFont(fntTitle);
	fntTitle = NULL;
}


/**
 * dlu_init(): Initialize dialog unit calculation.
 * @param hWnd Window.
 */
void WINAPI dlu_init(HWND hWnd)
{
	HDC hDC = GetDC(hWnd);
	HFONT hFontOld = SelectFont(hDC, fntMain);
	
	TEXTMETRICA tm;
	BOOL bRet = GetTextMetricsA(hDC, &tm);
	if (!bRet)
	{
		SelectFont(hDC, hFontOld);
		return;
	}
	
	SIZE size;
	static const char str[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	bRet = GetTextExtentPoint32A(hDC, str, sizeof(str)-1, &size);
	if (!bRet)
	{
		SelectFont(hDC, hFontOld);
		return;
	}
	
	dlu_x = (((size.cx / 26) + 1) / 2);
	dlu_y = tm.tmHeight;
}

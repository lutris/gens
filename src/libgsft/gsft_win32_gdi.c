/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_win32_gdi.c: Win32 convenience functions. (GDI Stuff)              *
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

#include "gsft_win32_gdi.h"
#include <windowsx.h>


/**
 * Fonts. DO NOT modify these externally!
 */
HFONT w32_fntMessage = NULL;		// Message font.
HFONT w32_fntTitle = NULL;		// Title font.
HFONT w32_fntMonospaced = NULL;		// Monospaced font.

/**
 * Dialog unit conversion.
 * DO NOT modify these externally!
 */
int w32_dlu_x = 4;
int w32_dlu_y = 4;


/**
 * gsft_win32_gdi_init(): Initialize (or reinitialize) font handling.
 * @param hWnd hWnd for setting up dialog unit conversion.
 */
void WINAPI gsft_win32_gdi_init(HWND hWnd)
{
	/** Font Initialization. **/
	
	// Make sure gsft_win32_gdi is shut down before reinitializing it.
	gsft_win32_gdi_end();
	
	// Get the system message font.
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	w32_fntMessage = CreateFontIndirectA(&ncm.lfMessageFont);
	
	// Get the title font.
	// This is the same as the system message font, but with bold and italics.
	ncm.lfMessageFont.lfItalic = 1;
	ncm.lfMessageFont.lfWeight = FW_BOLD;
	w32_fntTitle = CreateFontIndirectA(&ncm.lfMessageFont);
	
	// Get the monospaced font.
	// On Windows XP, this is Fixedsys.
	// On Wine, this is Courier New.
	w32_fntMonospaced = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
	
	/** Dialog Unit Conversion. **/
	
	if (!hWnd)
		return;
	
	// Set up dialog unit conversion.
	HDC hDC = GetDC(hWnd);
	HFONT hFontOld = SelectFont(hDC, w32_fntMessage);
	
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
	
	w32_dlu_x = (((size.cx / 26) + 1) / 2);
	w32_dlu_y = tm.tmHeight;
}


void WINAPI gsft_win32_gdi_end(void)
{
	// Make sure the message and title fonts are deleted.
	// (Monospaced font uses a Windows stock object.)
	if (w32_fntMessage)
	{
		DeleteFont(w32_fntMessage);
		w32_fntMessage = NULL;
	}
	if (w32_fntTitle)
	{
		DeleteFont(w32_fntTitle);
		w32_fntTitle = NULL;
	}
	
	// NULL out the monospaced font.
	w32_fntMonospaced = NULL;
}

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
 * gsft_win32_gdi_get_message_font(): Get the message font.
 * @return Main font. (Must be deleted with DeleteFont() when finished.)
 */
HFONT GSFT_FNCALL gsft_win32_gdi_get_message_font(void)
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	
	return (HFONT)(CreateFontIndirect(&ncm.lfMessageFont));
}


/**
 * gsft_win32_gdi_get_title_font(): Get the title font.
 * @return Main font. (Must be deleted with DeleteFont() when finished.)
 */
HFONT GSFT_FNCALL gsft_win32_gdi_get_title_font(void)
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	
	// Title font is the message font with bold and italics.
	ncm.lfMessageFont.lfItalic = 1;
	ncm.lfMessageFont.lfWeight = FW_BOLD;
	
	return (HFONT)(CreateFontIndirect(&ncm.lfMessageFont));
}

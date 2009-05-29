/***************************************************************************
 * MDP: Mega Drive Plugins - Win32 convenience functions.                  *
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

#ifdef _WIN32

#ifndef __MDP_WIN32_H
#define __MDP_WIN32_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <minmax.h>

/**
 * mdp_win32_center_on_window(): Center one window on top of another window.
 * @param hWnd_top Window to center on top.
 * @param hWnd_bottom Window to be centered over.
 */
static void mdp_win32_center_on_window(HWND hWnd_top, HWND hWnd_bottom)
{
	RECT r, r2;
	int dx1, dy1, dx2, dy2;
	
	GetWindowRect(hWnd_bottom, &r);
	dx1 = (r.right - r.left) / 2;
	dy1 = (r.bottom - r.top) / 2;
	
	GetWindowRect(hWnd_top, &r2);
	dx2 = (r2.right - r2.left) / 2;
	dy2 = (r2.bottom - r2.top) / 2;
	
	SetWindowPos(hWnd_top, NULL,
		     max(0, r.left + (dx1 - dx2)),
		     max(0, r.top + (dy1 - dy2)), 0, 0,
		     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

/**
 * mdp_win32_set_actual_window_size(): Set the actual window size, including the non-client area.
 * @param hWnd Window handle.
 * @param reqW Required width.
 * @param reqH Required height.
 */
static void mdp_win32_set_actual_window_size(HWND hWnd, const int reqW, const int reqH)
{
	RECT r;
	SetRect(&r, 0, 0, reqW, reqH);
	
	// Adjust the rectangle.
	AdjustWindowRectEx(&r, GetWindowStyle(hWnd), (GetMenu(hWnd) != NULL), GetWindowExStyle(hWnd));
	
	// Set the window size.
	SetWindowPos(hWnd, NULL, 0, 0, r.right - r.left, r.bottom - r.top,
		     SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

/**
 * mdp_win32_get_message_font(): Get the message font.
 * @return Main font. (Must be deleted with DeleteFont() when finished.)
 */
static HFONT mdp_win32_get_message_font(void)
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	
	return (HFONT)(CreateFontIndirect(&ncm.lfMessageFont));
}

/**
 * mdp_win32_get_title_font(): Get the title font.
 * @return Main font. (Must be deleted with DeleteFont() when finished.)
 */
static HFONT mdp_win32_get_title_font(void)
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	
	// Title font is the message font with bold and italics.
	ncm.lfMessageFont.lfItalic = 1;
	ncm.lfMessageFont.lfWeight = FW_BOLD;
	
	return (HFONT)(CreateFontIndirect(&ncm.lfMessageFont));
}

#endif /* __MDP_WIN32_H */

#endif /* _WIN32 */

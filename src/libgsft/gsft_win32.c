/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_win32.c: Win32 convenience functions. (General Stuff)              *
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

#include "gsft_win32.h"
#include <windowsx.h>

// Internal max() macro.
#define gsft_win32_max(a, b) (((a) < (b)) ? (a) : (b))


/**
 * gsft_win32_center_on_window(): Center one window on top of another window.
 * @param hWnd_top Window to center on top.
 * @param hWnd_bottom Window to be centered over.
 */
void GSFT_FNCALL gsft_win32_center_on_window(HWND hWnd_top, HWND hWnd_bottom)
{
	// TODO: This function doesn't take the taskbar into account.
	// If the window is offscreen on the bottom of the screen,
	// then it will still be obscured by the taskbar.
	
	RECT r_bottom, r_top;
	int dx1, dy1, dx2, dy2;
	
	GetWindowRect(hWnd_bottom, &r_bottom);
	dx1 = (r_bottom.right - r_bottom.left) / 2;
	dy1 = (r_bottom.bottom - r_bottom.top) / 2;
	
	GetWindowRect(hWnd_top, &r_top);
	dx2 = (r_top.right - r_top.left) / 2;
	dy2 = (r_top.bottom - r_top.top) / 2;
	
	int win_left = r_bottom.left + (dx1 - dx2);
	int win_top = r_bottom.top + (dy1 - dy2);
	
	// Window size.
	const int win_width = (r_top.right - r_top.left);
	const int win_height = (r_top.bottom - r_top.top);
	
	// Make sure the window isn't offscreen.
	const int scrn_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	const int scrn_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	
	RECT r_scrn;
	if (scrn_width == 0 || scrn_height == 0)
	{
		// System does not support multiple monitors.
		r_scrn.left = 0;
		r_scrn.top = 0;
		
		// Check the single-monitor size.
		r_scrn.right = GetSystemMetrics(SM_CXSCREEN);
		r_scrn.bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		// System supports multiple monitors.
		
		// Get the left/top.
		r_scrn.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
		r_scrn.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
		
		// Calculate the right/bottom.
		r_scrn.right = r_scrn.left + scrn_width;
		r_scrn.bottom = r_scrn.top + scrn_height;
	}
	
	// Check if the window is offscreen. (X)
	if (win_left + win_width >= r_scrn.right)
		win_left = r_scrn.right - win_width;
	else if (win_left < r_scrn.left)
		win_left = r_scrn.left;
	
	// Check if the window is offscreen. (Y)
	if (win_top + win_height >= r_scrn.bottom)
		win_top = r_scrn.bottom - win_height;
	else if (win_top < r_scrn.top)
		win_top = r_scrn.top;
	
	// Set the window position.
	SetWindowPos(hWnd_top, NULL, win_left, win_top, 0, 0,
		     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


/**
 * gsft_win32_set_actual_window_size(): Set the actual window size, including the non-client area.
 * @param hWnd Window handle.
 * @param cx Requested width.
 * @param cy Requested height.
 */
void GSFT_FNCALL gsft_win32_set_actual_window_size(HWND hWnd, const int cx, const int cy)
{
	// Improved function from http://blogs.msdn.com/oldnewthing/archive/2003/09/11/54885.aspx
	HMENU hMenu = GetMenu(hWnd);
	RECT rcWindow = {0, 0, cx, cy};
	
	/*
	 * First, convert the client rectangle to a window rectangle
	 * the menu-wrap-agnostic way.
	 */
	AdjustWindowRectEx(&rcWindow, GetWindowStyle(hWnd),
			   (hMenu != NULL), GetWindowExStyle(hWnd));
	
	/*
	 * If there is a menu, then check how much wrapping occurs
	 * when we set a window to the width specified by AdjustWindowRectEx()
	 * and an infinite amount of height. An infinite height allows
	 * us to see every single menu wrap.
	 */
	if (hMenu)
	{
		RECT rcTemp = rcWindow;
		rcTemp.bottom = 0x7FFF;		/* "Infinite" height. */
		SendMessage(hWnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rcTemp);
		
		/* Adjust our previous calculation to compensate for menu wrapping. */
		rcWindow.bottom += rcTemp.top;
	}
	
	// Set the window size.
	SetWindowPos(hWnd, NULL, 0, 0,
		     rcWindow.right - rcWindow.left,
		     rcWindow.bottom - rcWindow.top,
		     SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

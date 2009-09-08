/***************************************************************************
 * Gens: (Win32) Debug Window.                                             *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "debug_window.h"
#include "gens/gens_window.h"

// C includes.
#include <stdio.h>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include "ui/win32/fonts.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"

// Unused Parameter macro.
#include "macros/unused.h"

// Gens includes.
#include "emulator/g_main.hpp"


// Window.
HWND debug_window = NULL;

// Window class.
static WNDCLASS debug_wndclass;

// Window size. (TODO: Adjust based on font size.)
#define DEBUG_WINDOW_WIDTH  ((80*8)+24)
#define DEBUG_WINDOW_HEIGHT ((25*12)+8)

// Window procedure.
static LRESULT CALLBACK debug_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	txtDebug;

// Background color.
static HBRUSH	hbrBackground = NULL;

// Widget creation functions.
static void	debug_window_create_child_windows(HWND hWnd);

// Wordwrap function.
static int CALLBACK debug_window_txtDebug_WordBreak(LPTSTR lpch, int ichCurrent, int cch, int code);


/**
 * debug_window_create(): Create the Debug window.
 */
void debug_window_create(void)
{
	if (debug_window)
	{
		// Debug window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(debug_window, SW_SHOW);
		return;
	}
	
	if (debug_wndclass.lpfnWndProc != debug_window_wndproc)
	{
		// Create the background color brush.
		hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
		
		// Create the window class.
		debug_wndclass.style = 0;
		debug_wndclass.lpfnWndProc = debug_window_wndproc;
		debug_wndclass.cbClsExtra = 0;
		debug_wndclass.cbWndExtra = 0;
		debug_wndclass.hInstance = ghInstance;
		debug_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		debug_wndclass.hCursor = NULL;
		debug_wndclass.hbrBackground = hbrBackground;
		debug_wndclass.lpszMenuName = NULL;
		debug_wndclass.lpszClassName = TEXT("debug_window");
		
		RegisterClass(&debug_wndclass);
	}
	
	// Create the window.
	debug_window = CreateWindow(TEXT("debug_window"), TEXT("Gens/GS - Debug Window"),
					WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					CW_USEDEFAULT, CW_USEDEFAULT,
					DEBUG_WINDOW_WIDTH, DEBUG_WINDOW_HEIGHT,
					gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(debug_window, DEBUG_WINDOW_WIDTH, DEBUG_WINDOW_HEIGHT);
	
	// Center the window on the parent window.
	// TODO: Change Win32_centerOnGensWindow to accept two parameters.
	gsft_win32_center_on_window(debug_window, gens_window);
	
	UpdateWindow(debug_window);
	
	// Don't show the Debug window until requested by the main program.
	//ShowWindow(debug_window, SW_SHOW);
}


/**
 * debug_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void debug_window_create_child_windows(HWND hWnd)
{
	// Create the textbox.
	txtDebug = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
				  WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_WANTRETURN | ES_READONLY,
				  0, 0, DEBUG_WINDOW_WIDTH, DEBUG_WINDOW_HEIGHT,
				  hWnd, NULL, ghInstance, NULL);
	SetWindowFont(txtDebug, fntDebug, TRUE);
	Edit_SetWordBreakProc(txtDebug, debug_window_txtDebug_WordBreak);
}


/**
 * debug_window_close(): Close the Debug window.
 */
void debug_window_close(void)
{
	if (!debug_window)
		return;
	
	// Destroy the window.
	DestroyWindow(debug_window);
	debug_window = NULL;
}


/**
 * debug_window_log(): Append a message to the debug window.
 * @param msg Message to append.
 */
void debug_window_log(const char *msg)
{
	// TODO: Remove previous lines.
	int len = GetWindowTextLength(txtDebug);
	Edit_SetSel(txtDebug, len, len);
	Edit_ReplaceSel(txtDebug, msg);
	
	// Set the cursor position to the end of the buffer.
	len = GetWindowTextLength(txtDebug);
	SendMessage(txtDebug, EM_LINESCROLL, 0, len);
}


/**
 * debug_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK debug_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			debug_window_create_child_windows(hWnd);
			break;
		
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
		{
			if ((HWND)lParam != txtDebug)
				break;
			
			HDC hDC = (HDC)wParam;
			SetTextColor(hDC, RGB(170, 170, 170));
			SetBkColor(hDC, RGB(0, 0, 0));
			SelectObject(hDC, hbrBackground);
			return (LRESULT)hbrBackground;
		}
		
		case WM_DESTROY:
			if (hWnd != debug_window)
				break;
			
			debug_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * debug_window_txtDebug_WordBreak(): Wordwrap function.
 * @param lpch
 * @param ichCurrent
 * @param cch
 * @param code
 */
static int CALLBACK debug_window_txtDebug_WordBreak(LPTSTR lpch, int ichCurrent, int cch, int code)
{
	// Disable all wordwrapping.
	return 0;
}

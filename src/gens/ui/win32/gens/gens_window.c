/***************************************************************************
 * Gens: (Win32) Main Window.                                              *
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

#include "gens_window.h"
#include "gens_window_sync.hpp"
#include "gens_window_callbacks.hpp"

// Menus.
#include "gens_menu.hpp"
#include "ui/common/gens/gens_menu.h"

#include "emulator/g_main.hpp"

// Video Drawing.
#include "video/vdraw.h"

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"


// Window.
HWND gens_window = NULL;

// Window class.
static WNDCLASS gens_wndclass;


/**
 * gens_window_init_hWnd(): Initialize the Gens window.
 */
void gens_window_init_hWnd(void)
{
	// This function simply initializes the base window.
	// It's needed because DirectX needs the window handle in order to set cooperative levels.
	
	if (gens_wndclass.lpfnWndProc != Gens_Window_WndProc)
	{
		// Create the background color brush.
		HBRUSH gens_bgcolor = CreateSolidBrush(RGB(0, 0, 0));
		
		// Set the window class information.
		gens_wndclass.style = CS_HREDRAW | CS_VREDRAW;
		gens_wndclass.lpfnWndProc = Gens_Window_WndProc;
		gens_wndclass.cbClsExtra = 0;
		gens_wndclass.cbWndExtra = 0;
		gens_wndclass.hInstance = ghInstance;
		gens_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		gens_wndclass.hCursor = NULL;
		gens_wndclass.hbrBackground = gens_bgcolor;
		gens_wndclass.lpszMenuName = NULL;
		gens_wndclass.lpszClassName = TEXT("Gens");
		
		RegisterClass(&gens_wndclass);
	}
	
	// Create the window.
	gens_window = CreateWindow(TEXT("Gens"), TEXT("Gens"),
				   WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
				   320 * 2, 240 * 2,
				   NULL, NULL, ghInstance, NULL);
	
	// Accept dragged files.
	DragAcceptFiles(gens_window, TRUE);
}


/**
 * gens_window_create(): Create the Gens window.
 */
void gens_window_create(void)
{
	// Create the menu bar.
	gens_window_create_menubar();
}


/**
 * gens_window_create_menubar(): Create the menu bar.
 */
void gens_window_create_menubar(void)
{
	if (MainMenu)
	{
		// Menu bar already exists. Delete it.
		DestroyMenu(MainMenu);
		gens_menu_clear();
	}
	
	DestroyMenu(MainMenu);
	
	// Create the main menu.
	if (!vdraw_get_fullscreen() && Settings.showMenuBar)
	{
		// Create a menu bar.
		MainMenu = CreateMenu();
	}
	else
	{
		// Create a popup menu.
		MainMenu = CreatePopupMenu();
	}
	
	// Parse the menus.
	gens_menu_parse(&gmiMain[0], MainMenu);
	
	// Synchronize the menus.
	Sync_Gens_Window();
	
	// Set the menu bar.
	if (!vdraw_get_fullscreen() && Settings.showMenuBar)
		SetMenu(gens_window, MainMenu);
	else
		SetMenu(gens_window, NULL);
	
	if (!vdraw_get_fullscreen())
	{
		// Resize the window after the menu bar is rebuilt.
		if (vdraw_scale <= 0)
			return;
		const int w = 320 * vdraw_scale;
		const int h = 240 * vdraw_scale;
		gsft_win32_set_actual_window_size(gens_window, w, h);
	}
}


/**
 * gens_window_reinit(): Reinitialize the Gens window.
 */
void gens_window_reinit(void)
{
	// Determine the window size using the scaling factor.
	if (vdraw_scale <= 0)
		return;
	const int w = 320 * vdraw_scale;
	const int h = 240 * vdraw_scale;
	
	LONG_PTR lStyle;
	
	if (vdraw_get_fullscreen())
	{
		// Fullscreen. Hide the mouse cursor and the window borders.
		while (ShowCursor(TRUE) < 1) { }
		while (ShowCursor(FALSE) >= 0) { }
		
		// Hide the window borders.
		lStyle = GetWindowLongPtr(gens_window, GWL_STYLE);
		lStyle &= ~(WS_POPUPWINDOW | WS_OVERLAPPEDWINDOW);
		SetWindowLongPtr(gens_window, GWL_STYLE, lStyle);
		SetWindowPos(gens_window, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		// Windowed. Show the mouse cursor and the window borders.
		while (ShowCursor(FALSE) >= 0) { }
		while (ShowCursor(TRUE) < 1) { }
		
		// Adjust the window style.
		lStyle = GetWindowLongPtr(gens_window, GWL_STYLE);
		if (vdraw_cur_backend_flags & VDRAW_BACKEND_FLAG_WINRESIZE)
		{
			// Backend supports resizable windows.
			lStyle &= ~WS_POPUPWINDOW;
			lStyle |= WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE | WS_CLIPSIBLINGS;
		}
		else
		{
			// Backend does not support resizable windows.
			lStyle &= ~WS_OVERLAPPEDWINDOW;
			lStyle |= WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE | WS_CLIPSIBLINGS;
		}
		SetWindowLongPtr(gens_window, GWL_STYLE, lStyle);
		
		// Reposition the window.
		SetWindowPos(gens_window, NULL, Window_Pos.x, Window_Pos.y, 0, 0,
			     SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	
	// Rebuild the menu bar.
	// This is needed if the mode is switched from windowed to fullscreen, or vice-versa.
	gens_window_create_menubar();
	
	// Synchronize menus.
	Sync_Gens_Window();
}

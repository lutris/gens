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

// Gens Win32 resources
#include "ui/win32/resource.h"


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
		gens_wndclass.style = CS_HREDRAW | CS_VREDRAW;
		gens_wndclass.lpfnWndProc = Gens_Window_WndProc;
		gens_wndclass.cbClsExtra = 0;
		gens_wndclass.cbWndExtra = 0;
		gens_wndclass.hInstance = ghInstance;
		gens_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC));
		gens_wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		gens_wndclass.hbrBackground = NULL;
		gens_wndclass.lpszMenuName = NULL;
		gens_wndclass.lpszClassName = "Gens";
		
		RegisterClass(&gens_wndclass);
	}
	
	// Create the window.
	gens_window = CreateWindow("Gens", "Gens",
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
	// TODO: Popup menu if fullscreen.
	DestroyMenu(MainMenu);
	
	// Create the main menu.
	if (vdraw_get_fullscreen())
		MainMenu = CreatePopupMenu();
	else
		MainMenu = CreateMenu();
	
	// Menus
	gens_menu_parse(&gmiMain[0], MainMenu);
	
	// Set the menu bar.
	if (vdraw_get_fullscreen())
		SetMenu(gens_window, NULL);
	else
		SetMenu(gens_window, MainMenu);
}

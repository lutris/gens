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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gens_window.hpp"
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

WNDCLASS WndClass;
HWND gens_window = NULL;


/**
 * initGens_hWnd(): Initialize the Gens window.
 * @return hWnd.
 */
HWND initGens_hWnd(void)
{
	// This function simply initializes the base window.
	// It's needed because DirectX needs the window handle in order to set cooperative levels.
	
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Gens_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = NULL;
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens";
	
	RegisterClass(&WndClass);
	
	gens_window = CreateWindow("Gens", "Gens", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
				 320 * 2, 240 * 2, NULL, NULL, ghInstance, NULL);
	
	// Accept dragged files.
	DragAcceptFiles(gens_window, TRUE);
	
	return gens_window;
}


HWND create_gens_window(void)
{
	// Create the menu bar.
	create_gens_window_menubar();
	
	return gens_window;
}


/**
 * create_gens_window_menubar(): Create the menu bar.
 */
void create_gens_window_menubar(void)
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

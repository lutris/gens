/***************************************************************************
 * Gens: (Win32) Game Genie Window.                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include "game_genie_window.h"
#include "game_genie_window_callbacks.h"
#include "gens/gens_window.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>

static WNDCLASS WndClass;
HWND game_genie_window = NULL;


// Controls
HWND gg_txtCode = NULL;
HWND gg_txtName = NULL;
HWND gg_lstvCodes = NULL;


// Window width.
static const int wndWidth = 416;


/**
 * create_game_genie_window(): Create the Game Genie Window.
 * @return Game Genie Window.
 */
HWND create_game_genie_window(void)
{
	if (game_genie_window)
	{
		// Game Genie window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(game_genie_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = Game_Genie_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_Game_Genie";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	game_genie_window = CreateWindowEx(NULL, "Gens_Game_Genie", "Game Genie",
					   (WS_POPUP | WS_SYSMENU | WS_CAPTION) & ~(WS_MINIMIZE),
					   CW_USEDEFAULT, CW_USEDEFAULT,
					   wndWidth + 8 + Win32_dw, 300 + Win32_dh, NULL, NULL, ghInstance, NULL);
	
	UpdateWindow(game_genie_window);
	return game_genie_window;
}


void Game_Genie_Window_CreateChildWindows(HWND hWnd)
{
	Win32_centerOnGensWindow(hWnd);
	
	// Description labels.
	HWND lblInfoTitle, lblInfo;
	
	// Strings
	const char* strInfoTitle = "Information about Game Genie / Patch codes";
	const char* strInfo =
			"Both Game Genie codes and Patch codes are supported.\n"
			"Check the box next to the code to activate it.\n"
			"Syntax for Game Genie codes: XXXX-YYYY\n"
			"Syntax for Patch codes: AAAAAA-DDDD (address-data)";
	
	// Info Title
	lblInfoTitle = CreateWindow(WC_STATIC, strInfoTitle,
				    WS_CHILD | WS_VISIBLE | SS_LEFT,
				    8, 8, 256, 12, hWnd, NULL, ghInstance, NULL);
	SendMessage(lblInfoTitle, WM_SETFONT, (WPARAM)fntTitle, 1);
	
	// Info
	lblInfo = CreateWindow(WC_STATIC, strInfo,
			       WS_CHILD | WS_VISIBLE | SS_LEFT,
			       8, 24, 288, 52, hWnd, NULL, ghInstance, NULL);
	SendMessage(lblInfo, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Code and Name boxes, plus "Add Code" button.
	HWND lblCode, btnAddCode;
	HWND lblName;
	
	// Code label
	lblCode = CreateWindow(WC_STATIC, "Code",
			       WS_CHILD | WS_VISIBLE | SS_LEFT,
			       8, 24+52+8+2, 32, 12, hWnd, NULL, ghInstance, NULL);
	SendMessage(lblCode, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Code entry
	gg_txtCode = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
				    WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
				    8+32+8, 24+52+8, wndWidth - (8+32+8+64+8), 20,
				     hWnd, NULL, ghInstance, NULL);
	SendMessage(gg_txtCode, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Add Code
	btnAddCode = CreateWindow(WC_BUTTON, "&Add Code",
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				  wndWidth - 64, 24+52+8, 64, 20,
				  hWnd, IDC_BTN_ADD, ghInstance, NULL);
	SendMessage(btnAddCode, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Name label
	lblName = CreateWindow(WC_STATIC, "Name",
			       WS_CHILD | WS_VISIBLE | SS_LEFT,
			       8, 24+52+8+2+24, 32, 12,
			       hWnd, NULL, ghInstance, NULL);
	SendMessage(lblName, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// Name entry
	gg_txtName = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
				    WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
				    8+32+8, 24+52+8+24, wndWidth - (8+32+8+64+8), 20,
				    hWnd, NULL, ghInstance, NULL);
	SendMessage(gg_txtName, WM_SETFONT, (WPARAM)fntMain, 1);
	
	// ListView
	gg_lstvCodes = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "",
				      WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT,
				      8, 24+52+8+24+24, wndWidth - 8, 128,
				      hWnd, NULL, ghInstance, NULL);
	SendMessage(gg_lstvCodes, WM_SETFONT, (WPARAM)fntMain, 1);
	SendMessage(gg_lstvCodes, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	
	// Create the ListView columns.
	LV_COLUMN lvCol;
	memset(&lvCol, 0, sizeof(lvCol));
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	
	// Code
	lvCol.pszText = "Code";
	lvCol.cx = 128;
	SendMessage(gg_lstvCodes, LVM_INSERTCOLUMN, 0, (LPARAM)&lvCol);
	
	// Name
	lvCol.pszText = "Name";
	lvCol.cx = 256;
	SendMessage(gg_lstvCodes, LVM_INSERTCOLUMN, 1, (LPARAM)&lvCol);
	
	// Buttons
	const int btnTop = 24+52+8+24+24+128+8;
	HWND btnOK, btnApply, btnCancel, btnDeactivateAll, btnDelete;
	
	btnOK = CreateWindow(WC_BUTTON, "&OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
			     8, btnTop, 75, 23,
			     hWnd, (HMENU)IDC_BTN_OK, ghInstance, NULL);
	SendMessage(btnOK, WM_SETFONT, (WPARAM)fntMain, 1);
	
	btnApply = CreateWindow(WC_BUTTON, "&Apply", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				8+75+8, btnTop, 75, 23,
				hWnd, (HMENU)IDC_BTN_APPLY, ghInstance, NULL);
	SendMessage(btnApply, WM_SETFONT, (WPARAM)fntMain, 1);
	
	btnCancel = CreateWindow(WC_BUTTON, "&Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 8+75+8+75+8, btnTop, 75, 23,
				 hWnd, (HMENU)IDC_BTN_CANCEL, ghInstance, NULL);
	SendMessage(btnCancel, WM_SETFONT, (WPARAM)fntMain, 1);
	
	btnDelete = CreateWindow(WC_BUTTON, "&Delete", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 8+75+8+75+8+75+8, btnTop, 75, 23,
				 hWnd, (HMENU)IDC_BTN_DELETE, ghInstance, NULL);
	SendMessage(btnDelete, WM_SETFONT, (WPARAM)fntMain, 1);
	
	btnDeactivateAll = CreateWindow(WC_BUTTON, "Deac&tivate All", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					8+75+8+75+8+75+8+75+8, btnTop, 75, 23,
					hWnd, (HMENU)IDC_BTN_DEACTIVATEALL, ghInstance, NULL);
	SendMessage(btnDeactivateAll, WM_SETFONT, (WPARAM)fntMain, 1);
}

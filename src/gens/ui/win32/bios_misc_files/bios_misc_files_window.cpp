/***************************************************************************
 * Gens: (Win32) BIOS/Misc Files Window.                                   *
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

#include "bios_misc_files_window.hpp"
#include "bios_misc_files_window_callbacks.h"
#include "gens/gens_window.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "emulator/g_main.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>

static WNDCLASS WndClass;
HWND bios_misc_files_window = NULL;


// Frame width.
static const int frameWidth = 360;


// All textboxes to be displayed on the BIOS/Misc Files window are defined here.
const unsigned short BIOSMiscFiles_Count = 14;
const struct BIOSMiscFileEntry_t BIOSMiscFiles[BIOSMiscFiles_Count + 1] =
{
	{"Configure Genesis BIOS File", "md_bios", (FileFilterType)0, NULL},
	{"Genesis", "md_bios", ROMFile, BIOS_Filenames.MD_TMSS},
	{"Configure 32X BIOS Files", "32x_bios", (FileFilterType)0, NULL},
	{"MC68000", "mc68000", ROMFile, BIOS_Filenames._32X_MC68000},
	{"Master SH2", "msh2", ROMFile, BIOS_Filenames._32X_MSH2},
	{"Slave SH2", "ssh2", ROMFile, BIOS_Filenames._32X_SSH2},
	{"Configure SegaCD BIOS Files", "mcd_bios", (FileFilterType)0, NULL},
	{"USA", "mcd_bios_usa", ROMFile, BIOS_Filenames.SegaCD_US},
	{"Europe", "mcd_bios_eur", ROMFile, BIOS_Filenames.MegaCD_EU},
	{"Japan", "mcd_bios_jap", ROMFile, BIOS_Filenames.MegaCD_JP},
	{"Configure Miscellaneous Files", "misc", (FileFilterType)0, NULL},
	{"7z Binary", "_7z", AnyFile, Misc_Filenames._7z_Binary},
	{"GCOffline", "gcoffline", AnyFile, Misc_Filenames.GCOffline},
	{"Manual", "manual", AnyFile, Misc_Filenames.Manual},
	{NULL, NULL, (FileFilterType)0, NULL},
};


// Handles for the Win32 textboxes.
HWND bmf_txtEntry[BIOSMiscFiles_Count];


/**
 * create_bios_misc_files_window(): Create the BIOS/Misc Files Window.
 * @return BIOS/Misc Files Window.
 */
HWND create_bios_misc_files_window(void)
{
	if (bios_misc_files_window)
	{
		// BIOS/Misc Files window is already created. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(bios_misc_files_window, 1);
		return NULL;
	}
	
	// Create the window class.
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = BIOS_Misc_Files_Window_WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = ghInstance;
	WndClass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens_BIOS_Misc_Files";
	
	RegisterClass(&WndClass);
	
	// Create the window.
	bios_misc_files_window = CreateWindowEx(NULL, "Gens_BIOS_Misc_Files", "Configure BIOS/Misc Files",
						(WS_POPUP | WS_SYSMENU | WS_CAPTION) & ~(WS_MINIMIZE),
						CW_USEDEFAULT, CW_USEDEFAULT,
						frameWidth + 16 + Win32_dw, 480 + Win32_dh, NULL, NULL, ghInstance, NULL);
	
	UpdateWindow(bios_misc_files_window);
	return bios_misc_files_window;
}


void BIOS_Misc_Files_Window_CreateChildWindows(HWND hWnd)
{
	Win32_centerOnGensWindow(hWnd);
	
	// Create all frames. This will be fun!
	HWND grpBox = NULL;
	HWND lblTitle, txtEntry, btnChange;
	
	// Positioning.
	int grpBox_Top = 0, grpBox_Height = 0, grpBox_Entry = 0;
	int entryTop;
	int file = 0;
	
	while (BIOSMiscFiles[file].title)
	{
		if (!BIOSMiscFiles[file].entry)
		{
			// No entry buffer. This is a new frame.
			grpBox_Top += grpBox_Height + 8;
			grpBox_Height = 24;
			grpBox_Entry = 0;
			bmf_txtEntry[file] = NULL;
			
			grpBox = CreateWindow(WC_BUTTON, BIOSMiscFiles[file].title,
					      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					      8, grpBox_Top, frameWidth, grpBox_Height,
					      hWnd, NULL, ghInstance, NULL);
			
			// Set the font for the groupbox title.
			SendMessage(grpBox, WM_SETFONT, (WPARAM)fntMain, 1);
		}
		else
		{
			// File entry.
			grpBox_Height += 24;
			entryTop = 20 + (grpBox_Entry * 24);
			SetWindowPos(grpBox, NULL, 0, 0, frameWidth, grpBox_Height, SWP_NOMOVE | SWP_NOZORDER);
			
			// Create the label for the title.
			lblTitle = CreateWindow(WC_STATIC, BIOSMiscFiles[file].title,
						WS_CHILD | WS_VISIBLE | SS_LEFT,
						8, entryTop, 56, 16,
						grpBox, NULL, ghInstance, NULL);
						
			// Set the font for the label.
			SendMessage(lblTitle, WM_SETFONT, (WPARAM)fntMain, 1);
			
			// Create the textbox for the entry.
			txtEntry = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, BIOSMiscFiles[file].entry,
						  WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | SS_LEFT,
						  8+56+8, entryTop, frameWidth - (8+56+8+16+64), 20,
						  grpBox, NULL, ghInstance, NULL);
			
			// Set the font for the entry.
			SendMessage(txtEntry, WM_SETFONT, (WPARAM)fntMain, 1);
			
			// Create the change button for the entry.
			btnChange = CreateWindow(WC_BUTTON, "Change...",
						 WS_CHILD | WS_VISIBLE | WS_TABSTOP,
						 frameWidth - (8+8+56), entryTop, 64, 20,
						 grpBox, (HMENU)(IDC_BTN_CHANGE + file), ghInstance, NULL);
						 
			// Set the font for the button.
			SendMessage(btnChange, WM_SETFONT, (WPARAM)fntMain, 1);
			
			// Save the text entry hWnd for later.
			bmf_txtEntry[file] = txtEntry;
			
			// Next entry.
			grpBox_Entry++;
		}
		
		// Next file.
		file++;
	}
}

/***************************************************************************
 * Gens: (GTK+) Select CD-ROM Drive Window.                                *
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

#include "selcd_window.hpp"

// C includes.
#include <string.h>

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "ui/win32/fonts.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"

// Gens includes.
#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"
#include "ui/gens_ui.hpp"
#include "emulator/g_main.hpp"
#include "emulator/options.hpp"

// SegaCD
#include "emulator/g_mcd.hpp"
#include "segacd/cd_aspi.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"


// Window.
HWND selcd_window = NULL;

// Window class.
static WNDCLASS selcd_wndclass;

// Window size.
#define SELCD_WINDOW_WIDTH  320
#define SELCD_WINDOW_HEIGHT 72

// Window procedure.
static LRESULT CALLBACK selcd_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	cboDeviceName;
static HWND	btnOK, btnCancel, btnApply;
#define IDC_SELCD_CBODEVICENAME 0x2000

// Widget creation functions.
static void	selcd_window_create_child_windows(HWND hWnd);
// Select CD-ROM Drive load/save functions.
static void	selcd_window_init(void);
static int	selcd_window_save(void);


/**
 * selcd_window_show(): Show the Select CD-ROM Drive window.
 */
void selcd_window_show(void)
{
	if (selcd_window)
	{
		// Select CD-ROM Drive window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(selcd_window, SW_SHOW);
		return;
	}
	
	if (selcd_wndclass.lpfnWndProc != selcd_window_wndproc)
	{
		// Create the window class.
		selcd_wndclass.style = 0;
		selcd_wndclass.lpfnWndProc = selcd_window_wndproc;
		selcd_wndclass.cbClsExtra = 0;
		selcd_wndclass.cbWndExtra = 0;
		selcd_wndclass.hInstance = ghInstance;
		selcd_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		selcd_wndclass.hCursor = NULL;
		selcd_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		selcd_wndclass.lpszMenuName = NULL;
		selcd_wndclass.lpszClassName = TEXT("selcd_window");
		
		RegisterClass(&selcd_wndclass);
	}
	
	// Create the window.
	selcd_window = CreateWindow(TEXT("selcd_window"), TEXT("Select CD-ROM Drive"),
				 WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 SELCD_WINDOW_WIDTH, SELCD_WINDOW_HEIGHT,
				 gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(selcd_window, SELCD_WINDOW_WIDTH, SELCD_WINDOW_HEIGHT);
	
	// Center the window on the parent window.
	gsft_win32_center_on_window(selcd_window, gens_window);
	
	UpdateWindow(selcd_window);
	ShowWindow(selcd_window, SW_SHOW);
}
	
/**
 * selcd_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void selcd_window_create_child_windows(HWND hWnd)
{
	HWND lblDeviceName = CreateWindow(WC_STATIC, TEXT("CD-ROM Drive:"),
					  WS_CHILD | WS_VISIBLE | SS_LEFT,
					  8, 8+3, 96, 16,
					  hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblDeviceName, fntMain, true);
	
	// CD-ROM Drive dropdown box
	cboDeviceName = CreateWindow(WC_COMBOBOX, NULL,
				     WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
				     16+96-8, 8, SELCD_WINDOW_WIDTH-8-96-16+8, 23*5,
				     hWnd, (HMENU)(IDC_SELCD_CBODEVICENAME), ghInstance, NULL);
	SetWindowFont(cboDeviceName, fntMain, true);
	
	// Buttons
	const unsigned short btnTop = SELCD_WINDOW_HEIGHT-8-24;
	
	btnOK = CreateWindow(WC_BUTTON, TEXT("&OK"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
			     SELCD_WINDOW_WIDTH-8-75-8-75-8-75, btnTop, 75, 23,
			     hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, true);
	
	btnCancel = CreateWindow(WC_BUTTON, TEXT("&Cancel"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				 SELCD_WINDOW_WIDTH-8-75-8-75, btnTop, 75, 23,
				 hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, true);
	
	btnApply = CreateWindow(WC_BUTTON, TEXT("&Apply"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				SELCD_WINDOW_WIDTH-8-75, btnTop, 75, 23,
				hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, true);
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, false);
	
	// Initialize the CD-ROM device name dropdown.
	selcd_window_init();
}


/**
 * selcd_window_close(): Close the Select CD-ROM Drive window.
 */
void selcd_window_close(void)
{
	if (!selcd_window)
		return;
	
	// Destroy the window.
	DestroyWindow(selcd_window);
	selcd_window = NULL;
}


/**
 * selcd_window_init(): Initialize the Select CD-ROM Drive window.
 */
static void selcd_window_init(void)
{
	// Clear the CD-ROM device name dropdown.
	ComboBox_ResetContent(cboDeviceName);
	
	if (Num_CD_Drive == 0)
	{
		// No CD-ROM drives detected.
		ComboBox_AddString(cboDeviceName, TEXT("No CD-ROM drives detected."));
		Button_Enable(btnOK, false);
		Button_Enable(btnApply, false);
		
		return;
	}
	
	// Populate the dropdown box.
	char driveName[128];
	for (int i = 0; i < Num_CD_Drive; i++)
	{
		ASPI_Get_Drive_Info(i, (unsigned char*)driveName);
		ComboBox_AddString(cboDeviceName, &driveName[8]);
	}
	
	// Set the current CD-ROM drive.
	ComboBox_SetCurSel(cboDeviceName, cdromDeviceID);
	if (ComboBox_GetCurSel(cboDeviceName) == -1)
	{
		// Invalid CD-ROM drive ID. Select the first drive.
		ComboBox_SetCurSel(cboDeviceName, 0);
	}
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, false);
}


/**
 * selcd_window_save(): Save the CD-ROM device name.
 * @return 0 if the settings were saved; non-zero on error.
 */
static int selcd_window_save(void)
{
	if (Num_CD_Drive == 0)
	{
		// No CD-ROM drives detected.
		return 0;
	}
	
	// Save settings.
	bool restartCD = false;
	
	// CD-ROM drive
	int tmpDrive = ComboBox_GetCurSel(cboDeviceName);
	
	// Check if the drive ID was changed.
	if (tmpDrive == cdromDeviceID)
	{
		// Drive ID wasn't changed. Don't do anything.
		// Disable the "Apply" button.
		Button_Enable(btnApply, false);
		return 0;
	}
	
	// Drive ID was changed.
	// This will cause a reset of the ASPI subsystem.
	if (SegaCD_Started && (CD_Load_System == CDROM_))
	{
		// ASPI is currently in use. Ask the user if they want to restart emulation.
		GensUI::MsgBox_Response response;
		response = GensUI::msgBox(
			"The CD-ROM drive is currently in use.\n"
			"Chanigng the CD-ROM drive will force the emulator to reset.\n"
			"Are you sure you want to do this?",
			"CD-ROM Drive in Use",
			GensUI::MSGBOX_ICON_WARNING | GensUI::MSGBOX_BUTTONS_YES_NO,
			selcd_window);
		
		if (response == GensUI::MSGBOX_RESPONSE_NO)
		{
			// Don't change anything.
			return 1;
		}
		
		// Stop SegaCD emulation.
		// TODO: Don't allow this if Netplay is enabled.
		restartCD = true;
		ROM::freeROM(Game);
	}
	
	// Stop ASPI.
	ASPI_End();
	
	cdromDeviceID = tmpDrive;
	
	// TODO: Drive speed (maybe)
	
	// Restart ASPI.
	ASPI_Init();
	
	if (restartCD)
	{
		// Restart SegaCD emulation.
		SegaCD_Started = Init_SegaCD(NULL);
		Options::setGameName();
		Sync_Gens_Window();
	}
	
	// Disable the "Apply" button.
	Button_Enable(btnApply, false);
	
	// Settings saved.
	return 0;
}


/**
 * selcd_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK selcd_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			selcd_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					if (!selcd_window_save())
						DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					selcd_window_save();
					break;
				case IDC_SELCD_CBODEVICENAME:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						// Device name dropdown box was changed.
						// Enable the "Apply" button.
						Button_Enable(btnApply, true);
					}
					break;
				default:
					// Unknown command identifier.
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != selcd_window)
				break;
			
			selcd_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}

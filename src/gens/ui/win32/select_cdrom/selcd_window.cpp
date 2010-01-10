/***************************************************************************
 * Gens: (Win32) Select CD-ROM Drive Window.                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_windowsx.h"
#include "libgsft/w32u/w32u_commctrl.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_win32_gdi.h"

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

// Window size. (NOTE: THESE ARE IN DIALOG UNITS, and must be converted to pixels using DLU_X() / DLU_Y().)
#define SELCD_WINDOW_WIDTH  200
#define SELCD_WINDOW_HEIGHT 44

// Window procedure.
static LRESULT CALLBACK selcd_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	cboDeviceName;
static HWND	btnOK, btnCancel, btnApply;
#define IDC_SELCD_CBODEVICENAME 0x2000

// Widget creation functions.
static void WINAPI selcd_window_create_child_windows(HWND hWnd);
// Select CD-ROM Drive load/save functions.
static void WINAPI selcd_window_init(void);
static int WINAPI selcd_window_save(void);


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
		selcd_wndclass.hIcon = LoadIconA(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		selcd_wndclass.hCursor = NULL;
		selcd_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		selcd_wndclass.lpszMenuName = NULL;
		selcd_wndclass.lpszClassName = "selcd_window";
		
		pRegisterClassU(&selcd_wndclass);
	}
	
	// Create the window.
	selcd_window = pCreateWindowU("selcd_window", "Select CD-ROM Drive",
					WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					CW_USEDEFAULT, CW_USEDEFAULT,
					DLU_X(SELCD_WINDOW_WIDTH), DLU_Y(SELCD_WINDOW_HEIGHT),
					gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(selcd_window, DLU_X(SELCD_WINDOW_WIDTH), DLU_Y(SELCD_WINDOW_HEIGHT));
	
	// Center the window on the parent window.
	gsft_win32_center_on_window(selcd_window, gens_window);
	
	UpdateWindow(selcd_window);
	ShowWindow(selcd_window, SW_SHOW);
}
	
/**
 * selcd_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void WINAPI selcd_window_create_child_windows(HWND hWnd)
{
	HWND lblDeviceName = pCreateWindowU(WC_STATIC, "CD-ROM Drive:",
						WS_CHILD | WS_VISIBLE | SS_LEFT,
						DLU_X(5), DLU_Y(5+1),
						DLU_X(50), DLU_Y(10),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(lblDeviceName, w32_fntMessage, true);
	
	// CD-ROM Drive dropdown box
	cboDeviceName = pCreateWindowU(WC_COMBOBOX, NULL,
					WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
					DLU_X(5+50+5), DLU_Y(5),
					DLU_X(SELCD_WINDOW_WIDTH-5-5-50-5), DLU_Y(14*5),
					hWnd, (HMENU)(IDC_SELCD_CBODEVICENAME), ghInstance, NULL);
	SetWindowFontU(cboDeviceName, w32_fntMessage, true);
	
	// Buttons
	int btnLeft = DLU_X(SELCD_WINDOW_WIDTH-5-50-5-50-5-50);
	int btnInc = DLU_X(5+50);
	const int btnTop = DLU_Y(SELCD_WINDOW_HEIGHT-5-14);
	
	btnOK = pCreateWindowU(WC_BUTTON, "&OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				btnLeft, btnTop,
				DLU_X(50), DLU_Y(14),
				hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFontU(btnOK, w32_fntMessage, true);
	
	btnLeft += btnInc;
	btnCancel = pCreateWindowU(WC_BUTTON, "&Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft, btnTop,
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFontU(btnCancel, w32_fntMessage, true);
	
	btnLeft += btnInc;
	btnApply = pCreateWindowU(WC_BUTTON, "&Apply", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					btnLeft, btnTop,
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFontU(btnApply, w32_fntMessage, true);
	
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
static void WINAPI selcd_window_init(void)
{
	// Clear the CD-ROM device name dropdown.
	ComboBox_ResetContentU(cboDeviceName);
	
	if (Num_CD_Drive == 0)
	{
		// No CD-ROM drives detected.
		ComboBox_AddStringU(cboDeviceName, "No CD-ROM drives detected.");
		Button_Enable(btnOK, false);
		Button_Enable(btnApply, false);
		
		return;
	}
	
	// Populate the dropdown box.
	char driveName[128];
	for (int i = 0; i < Num_CD_Drive; i++)
	{
		ASPI_Get_Drive_Info(i, (unsigned char*)driveName);
		ComboBox_AddStringU(cboDeviceName, &driveName[8]);
	}
	
	// Set the current CD-ROM drive.
	ComboBox_SetCurSelU(cboDeviceName, cdromDeviceID);
	if (ComboBox_GetCurSelU(cboDeviceName) == -1)
	{
		// Invalid CD-ROM drive ID. Select the first drive.
		ComboBox_SetCurSelU(cboDeviceName, 0);
	}
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, false);
}


/**
 * selcd_window_save(): Save the CD-ROM device name.
 * @return 0 if the settings were saved; non-zero on error.
 */
static int WINAPI selcd_window_save(void)
{
	if (Num_CD_Drive == 0)
	{
		// No CD-ROM drives detected.
		return 0;
	}
	
	// Save settings.
	bool restartCD = false;
	
	// CD-ROM drive
	int tmpDrive = ComboBox_GetCurSelU(cboDeviceName);
	
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
	
	return pDefWindowProcU(hWnd, message, wParam, lParam);
}

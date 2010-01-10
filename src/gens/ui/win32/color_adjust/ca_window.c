/***************************************************************************
 * Gens: (Win32) Color Adjustment Window.                                  *
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

#include "ca_window.h"
#include "gens/gens_window.h"

// C includes.
#include <stdio.h>

// Win32 includes.
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_windowsx.h"
#include "libgsft/w32u/w32u_commctrl.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_win32_gdi.h"
#include "libgsft/gsft_szprintf.h"

// Gens includes.
#include "emulator/md_palette.hpp"
#include "emulator/g_main.hpp"
#include "emulator/g_update.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/file/rom.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */


// Window.
HWND ca_window = NULL;

// Window class.
static WNDCLASS ca_wndclass;

// Window size. (NOTE: THESE ARE IN DIALOG UNITS, and must be converted to pixels using DLU_X() / DLU_Y().)
#define CA_WINDOW_WIDTH  192
#define CA_WINDOW_HEIGHT 100

#define CA_TRACKBAR_WIDTH  125
#define CA_TRACKBAR_HEIGHT 15

// Window procedure.
static LRESULT CALLBACK ca_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	trkContrast;
static HWND	lblContrastVal;
static HWND	trkBrightness;
static HWND	lblBrightnessVal;

static HWND	chkGrayscale;
static HWND	chkInverted;
static HWND	cboColorScaleMethod;

// Widget creation functions.
static void WINAPI ca_window_create_child_windows(HWND hWnd);

// Color adjustment load/save functions.
static void WINAPI ca_window_init(void);
static void WINAPI ca_window_save(void);


/**
 * ca_window_show(): Show the Color Adjustment window.
 */
void ca_window_show(void)
{
	if (ca_window)
	{
		// Color Adjustment window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(ca_window, SW_SHOW);
		return;
	}
	
	if (ca_wndclass.lpfnWndProc != ca_window_wndproc)
	{
		// Create the window class.
		ca_wndclass.style = 0;
		ca_wndclass.lpfnWndProc = ca_window_wndproc;
		ca_wndclass.cbClsExtra = 0;
		ca_wndclass.cbWndExtra = 0;
		ca_wndclass.hInstance = ghInstance;
		ca_wndclass.hIcon = LoadIconA(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		ca_wndclass.hCursor = NULL;
		ca_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		ca_wndclass.lpszMenuName = NULL;
		ca_wndclass.lpszClassName = "ca_window";
		
		pRegisterClassU(&ca_wndclass);
	}
	
	// Create the window.
	ca_window = pCreateWindowU("ca_window", "Color Adjustment",
					WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					CW_USEDEFAULT, CW_USEDEFAULT,
					DLU_X(CA_WINDOW_WIDTH), DLU_Y(CA_WINDOW_HEIGHT),
					gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(ca_window, DLU_X(CA_WINDOW_WIDTH), DLU_Y(CA_WINDOW_HEIGHT));
	
	// Center the window on the parent window.
	// TODO: Change Win32_centerOnGensWindow to accept two parameters.
	gsft_win32_center_on_window(ca_window, gens_window);
	
	UpdateWindow(ca_window);
	ShowWindow(ca_window, SW_SHOW);
}


/**
 * ca_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void WINAPI ca_window_create_child_windows(HWND hWnd)
{
	// Style for the trackbars.
	static const unsigned int trkStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ | TBS_BOTTOM;
	
	// "Contrast" label.
	HWND lblContrast = pCreateWindowU(WC_STATIC, "Co&ntrast",
						WS_CHILD | WS_VISIBLE | SS_LEFT,
						DLU_X(5), DLU_Y(10),
						DLU_X(35), DLU_Y(10),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(lblContrast, w32_fntMessage, TRUE);
	
	// "Contrast" trackbar.
	trkContrast = pCreateWindowU(TRACKBAR_CLASS, NULL, trkStyle,
					DLU_X(5+35), DLU_Y(10-1),
					DLU_X(CA_TRACKBAR_WIDTH), DLU_Y(CA_TRACKBAR_HEIGHT),
					hWnd, (HMENU)IDC_TRK_CA_CONTRAST, ghInstance, NULL);
	pSendMessageU(trkContrast, TBM_SETPAGESIZE, 0, 10);
	pSendMessageU(trkContrast, TBM_SETTICFREQ, 25, 0);
	pSendMessageU(trkContrast, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));
	pSendMessageU(trkContrast, TBM_SETPOS, TRUE, 0);
	
	// "Contrast" value label.
	lblContrastVal = pCreateWindowU(WC_STATIC, "0",
					WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
					DLU_X(5+35+CA_TRACKBAR_WIDTH+5), DLU_Y(10),
					DLU_X(20), DLU_Y(10),
					hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(lblContrastVal, w32_fntMessage, TRUE);
	
	// "Brightness" label.
	HWND lblBrightness = pCreateWindowU(WC_STATIC, "&Brightness",
						WS_CHILD | WS_VISIBLE | SS_LEFT,
						DLU_X(5), DLU_Y(10+20),
						DLU_X(35), DLU_Y(10),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(lblBrightness, w32_fntMessage, TRUE);
	
	// "Brightness" trackbar.
	trkBrightness = pCreateWindowU(TRACKBAR_CLASS, NULL, trkStyle,
					DLU_X(5+35), DLU_Y(10-1+20),
					DLU_X(CA_TRACKBAR_WIDTH), DLU_Y(CA_TRACKBAR_HEIGHT),
					hWnd, (HMENU)IDC_TRK_CA_CONTRAST, ghInstance, NULL);
	pSendMessageU(trkBrightness, TBM_SETPAGESIZE, 0, 10);
	pSendMessageU(trkBrightness, TBM_SETTICFREQ, 25, 0);
	pSendMessageU(trkBrightness, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));
	pSendMessageU(trkBrightness, TBM_SETPOS, TRUE, 0);
	
	// "Brightness" value label.
	lblBrightnessVal = pCreateWindowU(WC_STATIC, "0",
						WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
						DLU_X(5+35+CA_TRACKBAR_WIDTH+5), DLU_Y(10+20),
						DLU_X(20), DLU_Y(10),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(lblBrightnessVal, w32_fntMessage, TRUE);
	
	// Center the checkboxes.
	int ctlLeft = DLU_X((CA_WINDOW_WIDTH-50-5-50)/2);
	int ctlTop = DLU_Y(10+20+10+10);
	
	// "Grayscale" checkbox.
	chkGrayscale = pCreateWindowU(WC_BUTTON, "&Grayscale",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					ctlLeft, ctlTop,
					DLU_X(50), DLU_Y(10),
					hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(chkGrayscale, w32_fntMessage, TRUE);
	
	// "Inverted" checkbox.
	ctlLeft += DLU_X(5+50);
	chkInverted = pCreateWindowU(WC_BUTTON, "&Inverted",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
					ctlLeft, ctlTop,
					DLU_X(50), DLU_Y(10),
					hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(chkInverted, w32_fntMessage, TRUE);
	
	// Center the "Color Scale Method" controls.
	ctlLeft = DLU_X((CA_WINDOW_WIDTH-70-10-60)/2);
	
	// Create a label for the "Color Scale Method" dropdown.
	HWND lblColorScaleMethod = pCreateWindowU(WC_STATIC, "Color Sca&le Method:",
							WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_RIGHT,
							ctlLeft, DLU_Y(10+20+20+12+2),
							DLU_X(70), DLU_Y(10),
							hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(lblColorScaleMethod, w32_fntMessage, TRUE);
	
	// Create the "Color Scale Method" dropdown.
	ctlLeft += DLU_X(70+10);
	cboColorScaleMethod = pCreateWindowU(WC_COMBOBOX, NULL,
						WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
						ctlLeft, DLU_Y(10+20+20+12),
						DLU_X(60), DLU_Y(14*3),
						hWnd, NULL, ghInstance, NULL);
	SetWindowFontU(cboColorScaleMethod, w32_fntMessage, TRUE);
	
	// Add the items to the "Color Scale Method" dropdown.
	ComboBox_AddStringU(cboColorScaleMethod, "Raw");
	ComboBox_AddStringU(cboColorScaleMethod, "Full");
	ComboBox_AddStringU(cboColorScaleMethod, "Full with S/H");
	
	// Create the dialog buttons.
	
	// TODO: Center the buttons, or right-align them?
	// They look better center-aligned in this window...
	ctlLeft = DLU_X((CA_WINDOW_WIDTH-50-5-50-5-50)/2);
	ctlTop = DLU_Y(CA_WINDOW_HEIGHT-5-14);
	
	// OK button.
	HWND btnOK = pCreateWindowU(WC_BUTTON, "&OK",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
					ctlLeft, ctlTop,
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFontU(btnOK, w32_fntMessage, TRUE);
	
	// Cancel button.
	ctlLeft += DLU_X(5+50);
	HWND btnCancel = pCreateWindowU(WC_BUTTON, "&Cancel",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					ctlLeft, ctlTop,
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFontU(btnCancel, w32_fntMessage, TRUE);
	
	// Apply button.
	ctlLeft += DLU_X(5+50);
	HWND btnApply = pCreateWindowU(WC_BUTTON, "&Apply",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					ctlLeft, ctlTop,
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFontU(btnApply, w32_fntMessage, TRUE);
	
	// Initialize the color adjustment spinbuttons.
	ca_window_init();

	// Set focus to the "Contrast" trackbar.
	SetFocus(trkContrast);
}


/**
 * ca_window_close(): Close the Color Adjustment window.
 */
void ca_window_close(void)
{
	if (!ca_window)
		return;
	
	// Destroy the window.
	DestroyWindow(ca_window);
	ca_window = NULL;
}


/**
 * ca_window_init(): Initialize the Color Adjustment window widgets.
 */
static void WINAPI ca_window_init(void)
{
	char buf[16];
	
	// Contrast.
	pSendMessageU(trkContrast, TBM_SETPOS, TRUE, (Contrast_Level - 100));
	szprintf(buf, sizeof(buf), "%d", (Contrast_Level - 100));
	Static_SetTextU(lblContrastVal, buf);
	
	// Brightness.
	pSendMessageU(trkBrightness, TBM_SETPOS, TRUE, (Brightness_Level - 100));
	szprintf(buf, sizeof(buf), "%d", (Brightness_Level - 100));
	Static_SetTextU(lblBrightnessVal, buf);
	
	// Checkboxes.
	Button_SetCheckU(chkGrayscale, (Greyscale ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheckU(chkInverted, (Invert_Color ? BST_CHECKED : BST_UNCHECKED));
	ComboBox_SetCurSelU(cboColorScaleMethod, (int)ColorScaleMethod);
}


/**
 * ca_window_save(): Save the color adjustment settings.
 */
static void WINAPI ca_window_save(void)
{
	Contrast_Level = (pSendMessageU(trkContrast, TBM_GETPOS, 0, 0) + 100);
	Brightness_Level = (pSendMessageU(trkBrightness, TBM_GETPOS, 0, 0) + 100);
	
	Greyscale = (Button_GetCheckU(chkGrayscale) == BST_CHECKED);
	Invert_Color = (Button_GetCheckU(chkInverted) == BST_CHECKED);
	
	int csm = ComboBox_GetCurSelU(cboColorScaleMethod);
	if (csm != -1)
		ColorScaleMethod = (ColorScaleMethod_t)csm;
	
	// Recalculate palettes.
	Recalculate_Palettes();
	if (Game)
	{
		// Emulation is running. Update the CRAM.
		VDP_Flags.CRam = 1;
		
		if (!Settings.Paused)
		{
			// TODO: Just update CRAM. Don't update the frame.
			Update_Emulation_One();
#ifdef GENS_DEBUGGER
			if (IS_DEBUGGING())
				Update_Debug_Screen();
#endif
		}
	}
}


/**
 * ca_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK ca_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			ca_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					ca_window_save();
					DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					ca_window_save();
					break;
				default:
					// Unknown command identifier.
					break;
			}
			break;
		
		case WM_HSCROLL:
		{
			// Trackbar scroll.
			char buf[16];
			int scrlPos;
			
			switch (LOWORD(wParam))
			{
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
					// Scroll position is in the high word of wParam.
					scrlPos = (signed short)HIWORD(wParam);
					break;
				
				default:
					// Send TBM_GETPOS to the trackbar to get the position.
					scrlPos = pSendMessageU((HWND)lParam, TBM_GETPOS, 0, 0);
					break;
			}
			
			// Convert the scroll position to a string.
			szprintf(buf, sizeof(buf), "%d", scrlPos);
			
			// Set the value label.
			if ((HWND)lParam == trkContrast)
				Static_SetTextU(lblContrastVal, buf);
			else if ((HWND)lParam == trkBrightness)
				Static_SetTextU(lblBrightnessVal, buf);
			
			break;
		}
		
		case WM_DESTROY:
			if (hWnd != ca_window)
				break;
			
			ca_window = NULL;
			break;
	}
	
	return pDefWindowProcU(hWnd, message, wParam, lParam);
}

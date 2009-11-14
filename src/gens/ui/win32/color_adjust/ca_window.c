/***************************************************************************
 * Gens: (Win32) Color Adjustment Window.                                  *
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

#include "ca_window.h"
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

// Window size.
#define CA_WINDOW_WIDTH  306
#define CA_WINDOW_HEIGHT 164

#define CA_TRACKBAR_WIDTH  201
#define CA_TRACKBAR_HEIGHT 24

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
static void	ca_window_create_child_windows(HWND hWnd);

// Color adjustment load/save functions.
static void	ca_window_init(void);
static void	ca_window_save(void);


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
		ca_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		ca_wndclass.hCursor = NULL;
		ca_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		ca_wndclass.lpszMenuName = NULL;
		ca_wndclass.lpszClassName = TEXT("ca_window");
		
		RegisterClass(&ca_wndclass);
	}
	
	// Create the window.
	ca_window = CreateWindow(TEXT("ca_window"), TEXT("Color Adjustment"),
				 WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				 CW_USEDEFAULT, CW_USEDEFAULT,
				 CA_WINDOW_WIDTH, CA_WINDOW_HEIGHT,
				 gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(ca_window, CA_WINDOW_WIDTH, CA_WINDOW_HEIGHT);
	
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
static void ca_window_create_child_windows(HWND hWnd)
{
	// Style for the trackbars.
	static const unsigned int trkStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ | TBS_BOTTOM;
	
	// "Contrast" label.
	HWND lblContrast = CreateWindow(WC_STATIC, TEXT("Co&ntrast"),
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, 16, 56, 16,
					hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblContrast, fntMain, TRUE);
	
	// "Contrast" trackbar.
	trkContrast = CreateWindow(TRACKBAR_CLASS, NULL, trkStyle,
				   8+56, 14,
				   CA_TRACKBAR_WIDTH, CA_TRACKBAR_HEIGHT,
				   hWnd, (HMENU)IDC_TRK_CA_CONTRAST, ghInstance, NULL);
	SendMessage(trkContrast, TBM_SETPAGESIZE, 0, 10);
	SendMessage(trkContrast, TBM_SETTICFREQ, 25, 0);
	SendMessage(trkContrast, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));
	SendMessage(trkContrast, TBM_SETPOS, TRUE, 0);
	
	// "Contrast" value label.
	lblContrastVal = CreateWindow(WC_STATIC, TEXT("0"),
				      WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
				      8+56+CA_TRACKBAR_WIDTH+8, 16, 32, 16,
				      hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblContrastVal, fntMain, TRUE);
	
	// "Brightness" label.
	HWND lblBrightness = CreateWindow(WC_STATIC, TEXT("&Brightness"),
					WS_CHILD | WS_VISIBLE | SS_LEFT,
					8, 16+32, 56, 16,
					hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblBrightness, fntMain, TRUE);
	
	// "Brightness" trackbar.
	trkBrightness = CreateWindow(TRACKBAR_CLASS, NULL, trkStyle,
				     8+56, 14+32,
				     CA_TRACKBAR_WIDTH, CA_TRACKBAR_HEIGHT,
				     hWnd, (HMENU)IDC_TRK_CA_CONTRAST, ghInstance, NULL);
	SendMessage(trkBrightness, TBM_SETPAGESIZE, 0, 10);
	SendMessage(trkBrightness, TBM_SETTICFREQ, 25, 0);
	SendMessage(trkBrightness, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));
	SendMessage(trkBrightness, TBM_SETPOS, TRUE, 0);
	
	// "Brightness" value label.
	lblBrightnessVal = CreateWindow(WC_STATIC, TEXT("0"),
					WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
					8+56+CA_TRACKBAR_WIDTH+8, 16+32, 32, 16,
					hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblBrightnessVal, fntMain, TRUE);
	
	// Center the checkboxes.
	int ctlLeft = (CA_WINDOW_WIDTH-80-8-96+16)/2;
	
	// "Grayscale" checkbox.
	chkGrayscale = CreateWindow(WC_BUTTON, TEXT("&Grayscale"),
				    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
				    ctlLeft, 16+32+32, 80, 16,
				    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkGrayscale, fntMain, TRUE);
	
	// "Inverted" checkbox.
	chkInverted = CreateWindow(WC_BUTTON, TEXT("&Inverted"),
				   WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
				   ctlLeft+8+80, 16+32+32, 80, 16,
				   hWnd, NULL, ghInstance, NULL);
	SetWindowFont(chkInverted, fntMain, TRUE);
	
	// Center the "Color Scale Method" controls.
	ctlLeft = (CA_WINDOW_WIDTH-104-8-96+16)/2;
	
	// Create a label for the "Color Scale Method" dropdown.
	HWND lblColorScaleMethod = CreateWindow(WC_STATIC, TEXT("Color Sca&le Method:"),
						WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_CENTER,
						ctlLeft, 16+32+32+20+2, 104, 16,
						hWnd, NULL, ghInstance, NULL);
	SetWindowFont(lblColorScaleMethod, fntMain, TRUE);
	
	// Create the "Color Scale Method" dropdown.
	cboColorScaleMethod = CreateWindow(WC_COMBOBOX, NULL,
						WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
						ctlLeft+104+8, 16+32+32+20, 96, 23*3,
						hWnd, NULL, ghInstance, NULL);
	SetWindowFont(cboColorScaleMethod, fntMain, TRUE);
	
	// Add the items to the "Color Scale Method" dropdown.
	ComboBox_AddString(cboColorScaleMethod, TEXT("Raw"));
	ComboBox_AddString(cboColorScaleMethod, TEXT("Full"));
	ComboBox_AddString(cboColorScaleMethod, TEXT("Full with S/H"));
	
	// Create the dialog buttons.
	
	// TODO: Center the buttons, or right-align them?
	// They look better center-aligned in this window...
	ctlLeft = (CA_WINDOW_WIDTH-75-8-75-8-75)/2;
	
	// OK button.
	HWND btnOK = CreateWindow(WC_BUTTON, TEXT("&OK"),
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				  ctlLeft, CA_WINDOW_HEIGHT-8-24,
				  75, 23,
				  hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	// Cancel button.
	HWND btnCancel = CreateWindow(WC_BUTTON, TEXT("&Cancel"),
				      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				      ctlLeft+8+75, CA_WINDOW_HEIGHT-8-24,
				      75, 23,
				      hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
	// Apply button.
	HWND btnApply = CreateWindow(WC_BUTTON, TEXT("&Apply"),
				     WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     ctlLeft+8+75+8+75, CA_WINDOW_HEIGHT-8-24,
				     75, 23,
				     hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
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
static void ca_window_init(void)
{
	TCHAR buf[16];
	
	// Contrast.
	SendMessage(trkContrast, TBM_SETPOS, TRUE, (Contrast_Level - 100));
	_sntprintf(buf, (sizeof(buf)/sizeof(TCHAR)), TEXT("%d"), (Contrast_Level - 100));
	buf[(sizeof(buf)/sizeof(TCHAR))-1] = 0x00;
	SetWindowText(lblContrastVal, buf);
	
	// Brightness.
	SendMessage(trkBrightness, TBM_SETPOS, TRUE, (Brightness_Level - 100));
	_sntprintf(buf, (sizeof(buf)/sizeof(TCHAR)), TEXT("%d"), (Brightness_Level - 100));
	buf[(sizeof(buf)/sizeof(TCHAR))-1] = 0x00;
	SetWindowText(lblBrightnessVal, buf);
	
	// Checkboxes.
	Button_SetCheck(chkGrayscale, (Greyscale ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkInverted, (Invert_Color ? BST_CHECKED : BST_UNCHECKED));
	ComboBox_SetCurSel(cboColorScaleMethod, (int)ColorScaleMethod);
}


/**
 * ca_window_save(): Save the color adjustment settings.
 */
static void ca_window_save(void)
{
	Contrast_Level = (SendMessage(trkContrast, TBM_GETPOS, 0, 0) + 100);
	Brightness_Level = (SendMessage(trkBrightness, TBM_GETPOS, 0, 0) + 100);
	
	Greyscale = (Button_GetCheck(chkGrayscale) == BST_CHECKED);
	Invert_Color = (Button_GetCheck(chkInverted) == BST_CHECKED);
	
	int csm = ComboBox_GetCurSel(cboColorScaleMethod);
	if (csm != -1)
		ColorScaleMethod = (ColorScaleMethod_t)csm;
	
	// Recalculate palettes.
	Recalculate_Palettes();
	if (Game)
	{
		// Emulation is running. Update the CRAM.
		CRam_Flag = 1;
		
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
			TCHAR buf[16];
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
					scrlPos = SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
					break;
			}
			
			// Convert the scroll position to a string.
			_sntprintf(buf, (sizeof(buf)/sizeof(TCHAR)), TEXT("%d"), scrlPos);
			buf[(sizeof(buf)/sizeof(TCHAR))-1] = 0x00;
			
			// Set the value label.
			if ((HWND)lParam == trkContrast)
				SetWindowText(lblContrastVal, buf);
			else if ((HWND)lParam == trkBrightness)
				SetWindowText(lblBrightnessVal, buf);
			
			break;
		}
		
		case WM_DESTROY:
			if (hWnd != ca_window)
				break;
			
			ca_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}

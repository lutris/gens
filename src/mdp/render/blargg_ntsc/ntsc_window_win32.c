/***************************************************************************
 * MDP: Blargg's NTSC Filter. (Window Code) (Win32)                        *
 *                                                                         *
 * Copyright (c) 2006 by Shay Green                                        *
 * MDP version Copyright (c) 2008-2009 by David Korth                      *
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

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>

// C includes.
#include <math.h>
#include <stdio.h>

#include "ntsc_window.h"
#include "ntsc_window_common.h"

#include "mdp_render_blargg_ntsc.h"
#include "mdp_render_blargg_ntsc_plugin.h"
#include "md_ntsc.hpp"

// MDP error codes.
#include "mdp/mdp_error.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_win32_gdi.h"

// Window.
static HWND ntsc_window = NULL;
static WNDCLASS ntsc_window_wndclass;

// Widgets.
static HWND cboPresets;
static HWND chkScanline;
static HWND chkInterp;
static HWND chkCXA2025AS;
static HWND lblCtrlValues[NTSC_CTRL_COUNT];
static HWND hscCtrlValues[NTSC_CTRL_COUNT];

// Widget IDs.
#define IDC_NTSC_PRESETS  	0x1000
#define IDC_NTSC_SCANLINE	0x1001
#define IDC_NTSC_INTERP		0x1002
#define IDC_NTSC_CXA2025AS	0x1003
#define IDC_NTSC_TRACKBAR 	0x1100

// Window Procedure.
static LRESULT CALLBACK ntsc_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Create Child Windows function.
static void ntsc_window_create_child_windows(HWND hWnd);
static BOOL ntsc_window_child_windows_created;

// Font.
static HFONT ntsc_hFont = NULL;

// Callbacks.
static void	ntsc_window_callback_cboPresets_changed(void);
static void	ntsc_window_callback_hscCtrlValues_value_changed(int setting);

static BOOL	ntsc_window_do_callbacks;

// Window size.
#define NTSC_WINDOW_WIDTH  360
#define NTSC_WINDOW_HEIGHT 372

// HINSTANCE.
// TODO: Move to DllMain().
static HINSTANCE ntsc_hInstance;


/**
 * ntsc_window_show(): Show the NTSC Plugin Options window.
 * @param parent Parent window.
 */
void ntsc_window_show(void *parent)
{
	if (ntsc_window)
	{
		// NTSC Plugin Options window is already visible.
		// Set focus.
		// TODO
		return;
	}
	
	ntsc_window_child_windows_created = FALSE;
	
	// If no HINSTANCE was specified, use the main executable's HINSTANCE.
	if (!ntsc_hInstance)
		ntsc_hInstance = GetModuleHandle(NULL);
	
	// Create the window class.
	if (ntsc_window_wndclass.lpfnWndProc != ntsc_window_wndproc)
	{
		ntsc_window_wndclass.style = 0;
		ntsc_window_wndclass.lpfnWndProc = ntsc_window_wndproc;
		ntsc_window_wndclass.cbClsExtra = 0;
		ntsc_window_wndclass.cbWndExtra = 0;
		ntsc_window_wndclass.hInstance = ntsc_hInstance;
		ntsc_window_wndclass.hIcon = NULL; //LoadIcon(ntsc_hInstance, MAKEINTRESOURCE(IDI_NTSC));
		ntsc_window_wndclass.hCursor = NULL;
		ntsc_window_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		ntsc_window_wndclass.lpszMenuName = NULL;
		ntsc_window_wndclass.lpszClassName = TEXT("ntsc_window_wndclass");
		
		RegisterClass(&ntsc_window_wndclass);
	}
	
	// Create the font.
	ntsc_hFont = gsft_win32_gdi_get_message_font();
	
	// Create the window.
	ntsc_window = CreateWindow(TEXT("ntsc_window_wndclass"), TEXT("Blargg's NTSC Filter"),
				   WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				   CW_USEDEFAULT, CW_USEDEFAULT,
				   NTSC_WINDOW_WIDTH, NTSC_WINDOW_HEIGHT,
				   (HWND)parent, NULL, ntsc_hInstance, NULL);
	
	// Load the current settings.
	ntsc_window_load_settings();
	
	// Window adjustment.
	gsft_win32_set_actual_window_size(ntsc_window, NTSC_WINDOW_WIDTH, NTSC_WINDOW_HEIGHT);
	gsft_win32_center_on_window(ntsc_window, (HWND)parent);
	
	UpdateWindow(ntsc_window);
	ShowWindow(ntsc_window, TRUE);
	
	// Register the window with MDP Host Services.
	ntsc_host_srv->window_register(&mdp, ntsc_window);
}


#define NTSC_WIDGETNAME_WIDTH  88
#define NTSC_WIDGETNAME_HEIGHT 16
#define NTSC_VALUELABEL_WIDTH  32
#define NTSC_VALUELABEL_HEIGHT 16
#define NTSC_TRACKBAR_WIDTH  (NTSC_WINDOW_WIDTH-8-16-NTSC_WIDGETNAME_WIDTH-8-NTSC_VALUELABEL_WIDTH-8)
#define NTSC_TRACKBAR_HEIGHT 24
#define NTSC_TRACKBAR_STYLE  (WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_HORZ | TBS_BOTTOM)
/**
 * ntsc_window_create_child_windows(): Create child windows.
 * @param hWnd Parent window.
 */
static void ntsc_window_create_child_windows(HWND hWnd)
{
	if (ntsc_window_child_windows_created)
		return;
	
	// Don't do any callbacks yet.
	ntsc_window_do_callbacks = FALSE;
	
	// Create the main frame.
	HWND grpBox = CreateWindow(WC_BUTTON, TEXT("NTSC Configuration"),
				   WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				   8, 8, NTSC_WINDOW_WIDTH-16, NTSC_WINDOW_HEIGHT-8-16-24,
				   hWnd, NULL, ntsc_hInstance, NULL);
	SetWindowFont(grpBox, ntsc_hFont, TRUE);
	
	// Add a label for the presets dropdown.
	HWND lblPresets = CreateWindow(WC_STATIC, TEXT("&Presets:"),
				       WS_CHILD | WS_VISIBLE | SS_LEFT,
				       8+8, 8+16+4, 64, 16,
				       hWnd, NULL, ntsc_hInstance, NULL);
	SetWindowFont(lblPresets, ntsc_hFont, TRUE);
	
	// Add the presets dropdown.
	cboPresets = CreateWindow(WC_COMBOBOX, NULL,
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
				  8+64+8, 8+16, 96, 23*NTSC_PRESETS_COUNT,
				  hWnd, (HMENU)IDC_NTSC_PRESETS, ntsc_hInstance, NULL);
	SetWindowFont(cboPresets, ntsc_hFont, TRUE);
	
	// Add the presets to the dropdown.
	int i;
	for (i = 0; i < NTSC_PRESETS_COUNT; i++)
	{
		ComboBox_AddString(cboPresets, ntsc_presets[i].name);
	}
	
	// Scanlines checkbox.
	chkScanline = CreateWindow(WC_BUTTON, TEXT("S&canlines"),
				   WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				   8+64+8+96+8, 8+16+2, 64, 16,
				   hWnd, (HMENU)IDC_NTSC_SCANLINE, ntsc_hInstance, NULL);
	SetWindowFont(chkScanline, ntsc_hFont, TRUE);
	
	// Interpolation checkbox.
	chkInterp = CreateWindow(WC_BUTTON, TEXT("&Interpolation"),
				 WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				 8+64+8+96+8+64+8, 8+16+2, 88, 16,
				 hWnd, (HMENU)IDC_NTSC_INTERP, ntsc_hInstance, NULL);
	SetWindowFont(chkInterp, ntsc_hFont, TRUE);
	
	// Interpolation checkbox.
	chkCXA2025AS = CreateWindow(WC_BUTTON, TEXT("Use Sony C&X2025AS US decoder."),
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				8+8, 8+16+24, 204, 16,
				hWnd, (HMENU)IDC_NTSC_CXA2025AS, ntsc_hInstance, NULL);
	SetWindowFont(chkCXA2025AS, ntsc_hFont, TRUE);
	
	// Create the adjustment widgets.
	int hscTop = 8+16+16+24;
	for (i = 0; i < NTSC_CTRL_COUNT; i++)
	{
		// Label.
		HWND lblWidgetName = CreateWindow(WC_STATIC, ntsc_controls[i].name_mnemonic,
						  WS_CHILD | WS_VISIBLE | SS_LEFT,
						  8+8, hscTop+4,
						  NTSC_WIDGETNAME_WIDTH, NTSC_WIDGETNAME_HEIGHT,
						  hWnd, NULL, ntsc_hInstance, NULL);
		SetWindowFont(lblWidgetName, ntsc_hFont, TRUE);
		
		// Value Label.
		lblCtrlValues[i] = CreateWindow(WC_STATIC, NULL,
						WS_CHILD | WS_VISIBLE | SS_RIGHT,
						8+8+NTSC_WIDGETNAME_WIDTH+8, hscTop+4,
						NTSC_VALUELABEL_WIDTH, NTSC_VALUELABEL_HEIGHT,
						hWnd, NULL, ntsc_hInstance, NULL);
		SetWindowFont(lblCtrlValues[i], ntsc_hFont, TRUE);
		
		// Trackbar.
		hscCtrlValues[i] = CreateWindow(TRACKBAR_CLASS, NULL, NTSC_TRACKBAR_STYLE,
						8+8+NTSC_WIDGETNAME_WIDTH+8+NTSC_VALUELABEL_WIDTH, hscTop,
						NTSC_TRACKBAR_WIDTH, NTSC_TRACKBAR_HEIGHT,
						hWnd, (HMENU)(IDC_NTSC_TRACKBAR + i), ntsc_hInstance, NULL);
		SendMessage(hscCtrlValues[i], TBM_SETPAGESIZE, 0, ntsc_controls[i].step);
		SendMessage(hscCtrlValues[i], TBM_SETRANGE, TRUE, MAKELONG(ntsc_controls[i].min, ntsc_controls[i].max));
		SendMessage(hscCtrlValues[i], TBM_SETPOS, TRUE, 0);
		
		// Next widget.
		hscTop += 26;
	}
	
	// Create the "Close" button.
	HWND btnClose = CreateWindow(WC_BUTTON, TEXT("&Close"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     NTSC_WINDOW_WIDTH-75-8, NTSC_WINDOW_HEIGHT-24-8, 75, 23,
				     hWnd, (HMENU)IDCLOSE, ntsc_hInstance, NULL);
	SetWindowFont(btnClose, ntsc_hFont, TRUE);
	
	// Child windows created.
	ntsc_window_child_windows_created = TRUE;
}


/**
 * ntsc_wndproc(): Window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK ntsc_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			ntsc_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCLOSE:
					// Close.
					ntsc_window_close();
					break;
				
				case IDC_NTSC_PRESETS:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						// Presets selection has changed.
						ntsc_window_callback_cboPresets_changed();
					}
					break;
				
				case IDC_NTSC_SCANLINE:
					if (Button_GetCheck(chkScanline) == BST_CHECKED)
						mdp_md_ntsc_effects |= MDP_MD_NTSC_EFFECT_SCANLINE;
					else
						mdp_md_ntsc_effects &= ~MDP_MD_NTSC_EFFECT_SCANLINE;
					break;
				
				case IDC_NTSC_INTERP:
					if (Button_GetCheck(chkInterp) == BST_CHECKED)
						mdp_md_ntsc_effects |= MDP_MD_NTSC_EFFECT_INTERP;
					else
						mdp_md_ntsc_effects &= ~MDP_MD_NTSC_EFFECT_INTERP;
					break;
				
				case IDC_NTSC_CXA2025AS:
					if (Button_GetCheck(chkCXA2025AS) == BST_CHECKED)
						mdp_md_ntsc_effects |= MDP_MD_NTSC_EFFECT_CXA2025AS;
					else
						mdp_md_ntsc_effects &= ~MDP_MD_NTSC_EFFECT_CXA2025AS;
					mdp_md_ntsc_reinit_setup(); // Changing the decoder matrix requires reinitializing the filter.
					break;
				
				default:
					break;
			}
			break;
		
		case WM_HSCROLL:
		{
			// Determine which control this is.
			// TODO: Use a hash table instead of a for loop.
			int i;
			for (i = 0; i < NTSC_CTRL_COUNT; i++)
			{
				if (hscCtrlValues[i] == (HWND)lParam)
				{
					// NTSC control has been adjusted.
					ntsc_window_callback_hscCtrlValues_value_changed(i);
					break;
				}
			}
			break;
		}
		
		case WM_DESTROY:
			ntsc_window_close();
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * ntsc_window_close(): Close the NTSC Plugin Options window.
 */
void ntsc_window_close(void)
{
	if (!ntsc_window)
		return;
	
	// Unregister the window from MDP Host Services.
	ntsc_host_srv->window_unregister(&mdp, ntsc_window);
	
	// Destroy the window.
	HWND tmp = ntsc_window;
	ntsc_window = NULL;
	DestroyWindow(tmp);
	
	// Delete the font.
	DeleteFont(ntsc_hFont);
	ntsc_hFont = NULL;
}


/**
 * ntsc_window_load_settings(): Load the NTSC settings.
 */
void MDP_FNCALL ntsc_window_load_settings(void)
{
	if (!ntsc_window)
		return;
	
	ntsc_window_do_callbacks = FALSE;
	
	// Set the preset dropdown box.
	int i;
	for (i = 0; i < NTSC_PRESETS_COUNT; i++)
	{
		if (!ntsc_presets[i].setup)
		{
			// "Custom". This is the last item in the predefined list.
			// Since the current setup doesn't match anything else,
			// it must be a custom setup.
			ComboBox_SetCurSel(cboPresets, i);
			break;
		}
		else
		{
			// Check if this preset matches the current setup.
			if (!memcmp(mdp_md_ntsc_setup.params,
				    ntsc_presets[i].setup->params,
				    sizeof(mdp_md_ntsc_setup.params)))
			{
				// Match found!
				ComboBox_SetCurSel(cboPresets, i);
				break;
			}
		}
	}
	
	// Scanlines / Interpolation / CXA2025AS
	Button_SetCheck(chkScanline, ((mdp_md_ntsc_effects & MDP_MD_NTSC_EFFECT_SCANLINE) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkInterp, ((mdp_md_ntsc_effects & MDP_MD_NTSC_EFFECT_INTERP) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(chkCXA2025AS, ((mdp_md_ntsc_effects & MDP_MD_NTSC_EFFECT_CXA2025AS) ? BST_CHECKED : BST_UNCHECKED));
	
	// Load all settings.
	for (i = 0; i < NTSC_CTRL_COUNT; i++)
	{
		SendMessage(hscCtrlValues[i], TBM_SETPOS, TRUE,
			    ntsc_internal_to_display(i, mdp_md_ntsc_setup.params[i]));
		ntsc_window_callback_hscCtrlValues_value_changed(i);
	}
	
	ntsc_window_do_callbacks = TRUE;
}


/**
 * ntsc_window_callback_cboPresets_changed(): The "Presets" combo box has been changed.
 */
static void ntsc_window_callback_cboPresets_changed(void)
{
	if (!ntsc_window_do_callbacks)
		return;
	
	// Load the specified preset setup.
	int i = ComboBox_GetCurSel(cboPresets);
	if (i == -1 || i >= NTSC_PRESETS_COUNT)
		return;
	
	if (!ntsc_presets[i].setup)
		return;
	
	// Reinitialize the NTSC filter with the new settings.
	mdp_md_ntsc_setup = *(ntsc_presets[i].setup);
	mdp_md_ntsc_reinit_setup();
	
	// Load the new settings in the window.
	ntsc_window_load_settings();
}


/**
 * ntsc_window_callback_hscCtrlValues_value_changed(): One of the adjustment controls has been changed.
 * @param setting Setting ID.
 */
static void ntsc_window_callback_hscCtrlValues_value_changed(int setting)
{
	if (setting < 0 || setting >= NTSC_CTRL_COUNT)
		return;
	
	// Update the label for the adjustment widget.
	TCHAR tmp[16];
	int val = SendMessage(hscCtrlValues[setting], TBM_GETPOS, 0, 0);
	
	// Adjust the value to have the appropriate number of decimal places.
	if (setting == 0)
	{
		// Hue. No decimal places.
		_sntprintf(tmp, (sizeof(tmp)/sizeof(TCHAR)),
				TEXT("%d"), NTSC_DEGREE_SYMBOL, val);
	}
	else
	{
		// Other adjustment. 2 decimal places.
		_sntprintf(tmp, (sizeof(tmp)/sizeof(TCHAR)),
				TEXT("%0.2f"), ((double)val / 100.0));
	}
	tmp[(sizeof(tmp)/sizeof(TCHAR))-1] = 0x00;
	
	Static_SetText(lblCtrlValues[setting], tmp);
	
	if (!ntsc_window_do_callbacks)
		return;
	
	// Adjust the NTSC filter.
	mdp_md_ntsc_setup.params[setting] = ntsc_display_to_internal(setting, val);
	
	// Set the "Presets" dropdown to "Custom".
	ComboBox_SetCurSel(cboPresets, NTSC_PRESETS_COUNT-1);
	
	// Reinitialize the NTSC filter with the new settings.
	mdp_md_ntsc_reinit_setup();
}

/***************************************************************************
 * Gens: [MDP] VDP Layer Options. (Window Code) (Win32)                    *
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

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

// C includes.
#include <stdio.h>

#include "vlopt_window.h"
#include "vlopt_options.h"
#include "vlopt_plugin.h"
#include "vlopt.h"

// Win32-specific includes.
#include "vlopt_dllmain.h"
#include "resource.h"

// MDP error codes.
#include "mdp/mdp_error.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_win32_gdi.h"

// Response ID for the "Reset" button.
#define VLOPT_RESPONSE_RESET -64

// Window.
static HWND vlopt_window = NULL;
static WNDCLASS vlopt_window_wndclass;

// Checkboxes.
static HWND vlopt_window_checkboxes[VLOPT_OPTIONS_COUNT];

// Window Procedure.
static LRESULT CALLBACK vlopt_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Create Child Windows function.
static void vlopt_window_create_child_windows(HWND hWnd);
static BOOL vlopt_window_child_windows_created;

// Option handling functions.
static void vlopt_window_load_options(void);
static void vlopt_window_save_options(void);

// Font.
static HFONT vlopt_hfont = NULL;

// Window size.
#define VLOPT_WINDOW_WIDTH  216
#define VLOPT_WINDOW_HEIGHT 192

// Command values.
#define IDC_VLOPT_CHECKBOX 	0x1000
#define IDC_VLOPT_RESET		0x1100


/**
 * vlopt_window_show(): Show the VDP Layer Options window.
 * @param parent Parent window.
 */
void vlopt_window_show(void *parent)
{
	if (vlopt_window)
	{
		// VDP Layer Options window is already visible.
		// Set focus.
		// TODO
		return;
	}
	
	vlopt_window_child_windows_created = FALSE;
	
	// If no HINSTANCE was specified, use the main executable's HINSTANCE.
	if (!vlopt_hInstance)
		vlopt_hInstance = GetModuleHandle(NULL);
	
	// Create the window class.
	if (vlopt_window_wndclass.lpfnWndProc != vlopt_window_wndproc)
	{
		vlopt_window_wndclass.style = 0;
		vlopt_window_wndclass.lpfnWndProc = vlopt_window_wndproc;
		vlopt_window_wndclass.cbClsExtra = 0;
		vlopt_window_wndclass.cbWndExtra = 0;
		vlopt_window_wndclass.hInstance = vlopt_hInstance;
		vlopt_window_wndclass.hIcon = LoadIcon(vlopt_hInstance, MAKEINTRESOURCE(IDI_VLOPT));
		vlopt_window_wndclass.hCursor = NULL;
		vlopt_window_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		vlopt_window_wndclass.lpszMenuName = NULL;
		vlopt_window_wndclass.lpszClassName = TEXT("vlopt_window_wndclass");
		
		RegisterClass(&vlopt_window_wndclass);
	}
	
	// Create the font.
	vlopt_hfont = gsft_win32_gdi_get_message_font();
	
	// Create the window.
	vlopt_window = CreateWindow(TEXT("vlopt_window_wndclass"), TEXT("VDP Layer Options"),
				    WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				    CW_USEDEFAULT, CW_USEDEFAULT,
				    VLOPT_WINDOW_WIDTH, VLOPT_WINDOW_HEIGHT,
				    (HWND)parent, NULL, vlopt_hInstance, NULL);
	
	// Window adjustment.
	gsft_win32_set_actual_window_size(vlopt_window, VLOPT_WINDOW_WIDTH, VLOPT_WINDOW_HEIGHT);
	gsft_win32_center_on_window(vlopt_window, (HWND)parent);
	
	UpdateWindow(vlopt_window);
	ShowWindow(vlopt_window, TRUE);
	
	// Register the window with MDP Host Services.
	vlopt_host_srv->window_register(&mdp, vlopt_window);
}


/**
 * vlopt_window_close(): Close the VDP Layer Options window.
 */
void vlopt_window_close(void)
{
	if (!vlopt_window)
		return;
	
	// Unregister the window from MDP Host Services.
	vlopt_host_srv->window_unregister(&mdp, vlopt_window);
	
	// Destroy the window.
	HWND tmp = vlopt_window;
	vlopt_window = NULL;
	DestroyWindow(tmp);
	
	// Delete the font.
	DeleteFont(vlopt_hfont);
	vlopt_hfont = NULL;
}


/**
 * vlopt_wndproc(): Window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK vlopt_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int rval;
	
	switch (message)
	{
		case WM_CREATE:
			vlopt_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (wParam)
			{
				case IDC_VLOPT_RESET:
					// Reset the VDP layer options to the default value.
					rval = vlopt_host_srv->val_set(&mdp, MDP_VAL_VDP_LAYER_OPTIONS, MDP_VDP_LAYER_OPTIONS_DEFAULT);
					if (rval != MDP_ERR_OK)
					{
						fprintf(stderr, "%s(): Error setting MDP_VAL_VDP_LAYER_OPTIONS: 0x%08X\n", __func__, rval);
					}
					
					// Reload the VDP layer options.
					vlopt_window_load_options();
					break;
				
				case IDCANCEL:
				case IDCLOSE:
					// Close.
					vlopt_window_close();
					break;
				
				default:
					if ((wParam >= IDC_VLOPT_CHECKBOX) &&
					    (wParam < (IDC_VLOPT_CHECKBOX + VLOPT_OPTIONS_COUNT)))
					{
						// Checkbox toggled. Save the options.
						vlopt_window_save_options();
					}
					break;
			}
			
			break;
		
		case WM_DESTROY:
			vlopt_window_close();
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


// Grid size constants.
#define VLOPT_GRID_COL1 56
#define VLOPT_GRID_COLX 40
#define VLOPT_GRID_ROW  20

// Assume a checkbox is 16x16.
#define VLOPT_CHECKBOX_SIZE 16

/**
 * vlopt_window_create_child_windows(): Create child windows.
 * @param hWnd Parent window.
 */
static void vlopt_window_create_child_windows(HWND hWnd)
{
	if (vlopt_window_child_windows_created)
		return;
	
	HWND grpBox = CreateWindow(WC_BUTTON, TEXT("VDP Layer Options"),
			      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			      8, 8, VLOPT_WINDOW_WIDTH-16, VLOPT_WINDOW_HEIGHT-8-16-24,
			      hWnd, NULL, vlopt_hInstance, NULL);
	SetWindowFont(grpBox, vlopt_hfont, TRUE);
	
	// Counter variable.
	unsigned int i;
	
	// Create the column and row headers.
	for (i = 0; i < 3; i++)
	{
		HWND lblColHeader, lblRowHeader;
		
		lblColHeader = CreateWindow(WC_STATIC, vlopt_options[i].sublayer,
					    WS_CHILD | WS_VISIBLE | SS_CENTER,
					    8+VLOPT_GRID_COL1+(VLOPT_GRID_COLX*i), 16,
					    VLOPT_GRID_COLX, VLOPT_GRID_ROW,
					    grpBox, NULL, vlopt_hInstance, NULL);
		SetWindowFont(lblColHeader, vlopt_hfont, TRUE);
		
		lblRowHeader = CreateWindow(WC_STATIC, vlopt_options[i * 3].layer,
					    WS_CHILD | WS_VISIBLE | SS_RIGHT,
					    8, 16+(VLOPT_GRID_ROW*(i+1)),
					    VLOPT_GRID_COL1, VLOPT_GRID_ROW,
					    grpBox, NULL, vlopt_hInstance, NULL);
		SetWindowFont(lblRowHeader, vlopt_hfont, TRUE);
	}
	
	// Create the VDP Layer Options checkboxes.
	uint8_t row = 1, col = 0;
	for (i = 0; i < 9; i++)
	{
		vlopt_window_checkboxes[i] = CreateWindow(
				WC_BUTTON, NULL,
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
				8+8+VLOPT_GRID_COL1+(VLOPT_GRID_COLX*col)+((VLOPT_GRID_COLX-VLOPT_CHECKBOX_SIZE)/2),
				8+16+(VLOPT_GRID_ROW*row),
				VLOPT_CHECKBOX_SIZE, VLOPT_CHECKBOX_SIZE,
				hWnd, (HMENU)(IDC_VLOPT_CHECKBOX + i), vlopt_hInstance, NULL);
		SetWindowFont(vlopt_window_checkboxes[i], vlopt_hfont, TRUE);
		
		// Next cell.
		col++;
		if (col >= 3)
		{
			col = 0;
			row++;
		}
	}
	
	// Create the checkboxes for the remaining VDP Layer Options.
	for (i = 9; i < VLOPT_OPTIONS_COUNT; i++)
	{
		vlopt_window_checkboxes[i] = CreateWindow(
				WC_BUTTON, vlopt_options[i].layer,
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
				8+8+8, 8+16+(VLOPT_GRID_ROW*(i-5)),
				VLOPT_WINDOW_WIDTH-16-16-16, VLOPT_GRID_ROW,
				hWnd, (HMENU)(IDC_VLOPT_CHECKBOX + i), vlopt_hInstance, NULL);
		SetWindowFont(vlopt_window_checkboxes[i], vlopt_hfont, TRUE);
	}
	
	// Load the options.
	vlopt_window_load_options();
	
	// Center the buttons within the window.
	const int posBtnLeft = (VLOPT_WINDOW_WIDTH - 75 - 75 - 8) / 2;
	
	// Create the "Reset" button.
	HWND btnReset = CreateWindow(WC_BUTTON, TEXT("&Reset"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     posBtnLeft, VLOPT_WINDOW_HEIGHT-24-8, 75, 23,
				     hWnd, (HMENU)IDC_VLOPT_RESET, vlopt_hInstance, NULL);
	SetWindowFont(btnReset, vlopt_hfont, TRUE);
	
	// Create the "Close" button.
	HWND btnClose = CreateWindow(WC_BUTTON, TEXT("&Close"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     posBtnLeft+75+8, VLOPT_WINDOW_HEIGHT-24-8, 75, 23,
				     hWnd, (HMENU)IDCLOSE, vlopt_hInstance, NULL);
	SetWindowFont(btnClose, vlopt_hfont, TRUE);
	
	// Child windows created.
	vlopt_window_child_windows_created = TRUE;
}


/**
 * vlopt_window_load_options(): Load the options from MDP_VAL_VDP_LAYER_OPTIONS.
 */
static void vlopt_window_load_options(void)
{
	int vdp_layer_options = vlopt_host_srv->val_get(MDP_VAL_VDP_LAYER_OPTIONS);
	if (vdp_layer_options < 0)
	{
		fprintf(stderr, "%s(): Error getting MDP_VAL_VDP_LAYER_OPTIONS: 0x%08X\n", __func__, vdp_layer_options);
		return;
	}
	
	// Go through the options.
	unsigned int i;
	for (i = 0; i < VLOPT_OPTIONS_COUNT; i++)
	{
		unsigned int flag_enabled = ((vdp_layer_options & vlopt_options[i].flag) ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(vlopt_window_checkboxes[i], flag_enabled);
	}
}


/**
 * vlopt_window_save_options(): Save the options to MDP_VAL_VDP_LAYER_OPTIONS.
 */
static void vlopt_window_save_options(void)
{
	int vdp_layer_options = 0;
	
	// Go through the options.
	unsigned int i;
	for (i = 0; i < VLOPT_OPTIONS_COUNT; i++)
	{
		if (Button_GetCheck(vlopt_window_checkboxes[i]) == BST_CHECKED)
			vdp_layer_options |= vlopt_options[i].flag;
	}
	
	// Set the new options.
	int rval = vlopt_host_srv->val_set(&mdp, MDP_VAL_VDP_LAYER_OPTIONS, vdp_layer_options);
	if (rval != MDP_ERR_OK)
	{
		fprintf(stderr, "%s(): Error setting MDP_VAL_VDP_LAYER_OPTIONS: 0x%08X\n", __func__, vdp_layer_options);
	}
}

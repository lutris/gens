/***************************************************************************
 * Gens: (Win32) BIOS/Misc Files Window.                                   *
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

#include "bmf_window.hpp"
#include "ui/common/bmf_window_common.h"
#include "emulator/g_main.hpp"
#include "gens/gens_window.h"

#include "gens_ui.hpp"

// C includes.
#include <stdio.h>
#include <string.h>

// C++ includes.
#include <string>
using std::string;

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


// Window.
HWND bmf_window = NULL;

// Window class.
static WNDCLASS bmf_wndclass;

// Window size.
#define BMF_WINDOW_WIDTH  392
#define BMF_WINDOW_HEIGHT 360

#define BMF_FRAME_WIDTH (BMF_WINDOW_WIDTH-16)

// Widgets.
static HWND	txtFile[12];
static HWND	btnOK, btnCancel, btnApply;
#define IDC_BMF_TXTFILE 0x8000

// Window procedure.
static LRESULT CALLBACK bmf_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widget creation functions.
static void	bmf_window_create_child_windows(HWND hWnd);

// Configuration load/save functions.
static void	bmf_window_init(void);
static void	bmf_window_save(void);

// Callbacks.
static void	bmf_window_callback_btnChange_clicked(int file);


/**
 * bmf_window_show(): Show the BIOS/Misc Files window.
 */
void bmf_window_show(void)
{
	if (bmf_window)
	{
		// BIOS/Misc Files window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(bmf_window, SW_SHOW);
		return;
	}
	
	if (bmf_wndclass.lpfnWndProc != bmf_window_wndproc)
	{
		// Create the window class.
		bmf_wndclass.style = 0;
		bmf_wndclass.lpfnWndProc = bmf_window_wndproc;
		bmf_wndclass.cbClsExtra = 0;
		bmf_wndclass.cbWndExtra = 0;
		bmf_wndclass.hInstance = ghInstance;
		bmf_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		bmf_wndclass.hCursor = NULL;
		bmf_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		bmf_wndclass.lpszMenuName = NULL;
		bmf_wndclass.lpszClassName = TEXT("bmf_window");
		
		RegisterClass(&bmf_wndclass);
	}
	
	// Create the window.
	// Create the window.
	bmf_window = CreateWindow(TEXT("bmf_window"), TEXT("BIOS/Misc Files"),
				  WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				  CW_USEDEFAULT, CW_USEDEFAULT,
				  BMF_WINDOW_WIDTH, BMF_WINDOW_HEIGHT,
				  gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(bmf_window, BMF_WINDOW_WIDTH, BMF_WINDOW_HEIGHT);
	
	// Center the window on the parent window.
	gsft_win32_center_on_window(bmf_window, gens_window);
	
	UpdateWindow(bmf_window);
	ShowWindow(bmf_window, SW_SHOW);
}


/**
 * bmf_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void bmf_window_create_child_windows(HWND hWnd)
{
	// Create all frames.
	HWND grpBox;
	
	// Positioning.
	const int grpBox_Left = 8;
	int grpBox_Top = 0, grpBox_Height = 0, grpBox_Entry = 0;
	int entryTop;
	
	for (int file = 0; bmf_entries[file].title != NULL; file++)
	{
		if (!bmf_entries[file].entry)
		{
			// No entry buffer. This is a new frame.
			grpBox_Top += grpBox_Height + 8;
			grpBox_Height = 24;
			grpBox_Entry = 0;
			
			grpBox = CreateWindow(WC_BUTTON, bmf_entries[file].title,
					      WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					      grpBox_Left, grpBox_Top,
					      BMF_FRAME_WIDTH, grpBox_Height,
					      hWnd, NULL, ghInstance, NULL);
			SetWindowFont(grpBox, fntMain, true);
		}
		else
		{
			// File entry.
			grpBox_Height += 24;
			entryTop = 20 + (grpBox_Entry * 24);
			SetWindowPos(grpBox, NULL, 0, 0, BMF_FRAME_WIDTH, grpBox_Height, SWP_NOMOVE | SWP_NOZORDER);
			
			// Create the label for the file.
			HWND lblFile = CreateWindow(WC_STATIC, bmf_entries[file].title,
						    WS_CHILD | WS_VISIBLE | SS_LEFT,
						    grpBox_Left + 8, grpBox_Top + entryTop,
						    64, 16,
						    hWnd, NULL, ghInstance, NULL);
			SetWindowFont(lblFile, fntMain, true);
			
			// Create the textbox for the file.
			txtFile[file] = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
						  WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_LEFT | ES_AUTOHSCROLL,
						  grpBox_Left+8+56+8, grpBox_Top+entryTop,
						  BMF_FRAME_WIDTH-(8+64+8+72+8), 20,
						  hWnd, (HMENU)(IDC_BMF_TXTFILE + file), ghInstance, NULL);
			SetWindowFont(txtFile[file], fntMain, true);
			SendMessage(txtFile[file], EM_LIMITTEXT, GENS_PATH_MAX-1, 0);
			
			// Create the "Change..." button for the file.
			HWND btnChange = CreateWindow(WC_BUTTON, TEXT("Change..."),
						      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
						      grpBox_Left+BMF_FRAME_WIDTH-(8+72), grpBox_Top + entryTop,
						      72, 20,
						      hWnd, (HMENU)(IDC_BTN_CHANGE + file), ghInstance, NULL);
			SetWindowFont(btnChange, fntMain, true);
			
			// Next entry.
			grpBox_Entry++;
		}
	}
	
	// Create the dialog buttons.
	
	// OK button.
	btnOK = CreateWindow(WC_BUTTON, TEXT("&OK"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
					BMF_WINDOW_WIDTH-8-75-8-75-8-75, BMF_WINDOW_HEIGHT-8-24,
					75, 23,
					hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, true);
	
	// Cancel button.
	btnCancel = CreateWindow(WC_BUTTON, TEXT("&Cancel"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					BMF_WINDOW_WIDTH-8-75-8-75, BMF_WINDOW_HEIGHT-8-24,
					75, 23,
					hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, true);
	
	// Apply button.
	btnApply = CreateWindow(WC_BUTTON, TEXT("&Apply"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					BMF_WINDOW_WIDTH-8-75, BMF_WINDOW_HEIGHT-8-24,
					75, 23,
					hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, true);
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, false);
	
	// Initialize the file textboxes.
	bmf_window_init();
	
	// Set focus to the first textbox.
	// (Index 0 is the frame; Index 1 is the textbox.)
	SetFocus(txtFile[1]);
}


/**
 * bmf_window_close(): Close the About window.
 */
void bmf_window_close(void)
{
	if (!bmf_window)
		return;
	
	// Destroy the window.
	DestroyWindow(bmf_window);
	bmf_window = NULL;
}


/**
 * bmf_window_init(): Initialize the file text boxes.
 */
static void bmf_window_init(void)
{
	for (int file = 0; bmf_entries[file].title != NULL; file++)
	{
		if (!bmf_entries[file].entry)
			continue;
		
		// Set the entry text.
		SetWindowText(txtFile[file], bmf_entries[file].entry);
	}
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, false);
}


/**
 * bmf_window_save(): Save the BIOS/Misc Files.
 */
static void bmf_window_save(void)
{
	int file;
	for (file = 0; bmf_entries[file].title != NULL; file++)
	{
		if (!bmf_entries[file].entry)
			continue;
		
		// Save the entry text.
		GetWindowText(txtFile[file], bmf_entries[file].entry, GENS_PATH_MAX);
		bmf_entries[file].entry[GENS_PATH_MAX-1] = 0x00;
	}
	
	// Disable the "Apply" button.
	Button_Enable(btnApply, false);
}


/**
 * bmf_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK bmf_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			bmf_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					bmf_window_save();
					DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					bmf_window_save();
					break;
				default:
					switch (LOWORD(wParam) & 0xFF00)
					{
						case IDC_BTN_CHANGE:
							bmf_window_callback_btnChange_clicked(LOWORD(wParam) & 0xFF);
							break;
						case IDC_BMF_TXTFILE:
							if (HIWORD(wParam) == EN_CHANGE)
							{
								// File textbox was changed.
								// Enable the "Apply" button.
								Button_Enable(btnApply, true);
							}
							break;
						default:
							// Unknown command identifier.
							break;
					}
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != bmf_window)
				break;
			
			bmf_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * bmf_window_callback_btnChange_clicked(): A "Change..." button was clicked.
 * @param file File ID number.
 */
static void bmf_window_callback_btnChange_clicked(int file)
{
	// Check that this is a valid file entry.
	if (!bmf_entries[file].entry)
		return;
	
	TCHAR tmp[64];
	_sntprintf(tmp, (sizeof(tmp)/sizeof(TCHAR)),
			TEXT("Select %s File"), bmf_entries[file].title);
	tmp[(sizeof(tmp)/sizeof(TCHAR))-1] = 0x00;
	
	// Get the currently entered filename.
	TCHAR cur_file[GENS_PATH_MAX];
	GetWindowText(txtFile[file], cur_file, (sizeof(cur_file)/sizeof(TCHAR)));
	cur_file[(sizeof(cur_file)/sizeof(TCHAR))-1] = 0x00;
	
	// Request a new file.
	string new_file = GensUI::openFile(tmp, cur_file, bmf_entries[file].filter, bmf_window);
	
	// If "Cancel" was selected, don't do anything.
	if (new_file.empty())
		return;
	
	// Set the new file.
	SetWindowText(txtFile[file], new_file.c_str());
	
	// Enable the "Apply" button.
	Button_Enable(btnApply, true);
}

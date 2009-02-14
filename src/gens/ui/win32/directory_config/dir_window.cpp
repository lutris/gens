/***************************************************************************
 * Gens: (GTK+) Directory Configuration Window.                            *
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

#include "dir_window.hpp"
#include "ui/common/dir_window_common.h"

// C includes.
#include <string.h>

// C++ includes.
#include <string>
using std::string;

// Win32 includes.
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

// Gens Win32 resources.
#include "ui/win32/resource.h"

// Unused Parameter macro.
#include "macros/unused.h"

// Gens includes.
#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "util/file/file.hpp"


// Window.
HWND dir_window = NULL;

// Window class.
static WNDCLASS dir_wndclass;

// Window size.
#define DIR_WINDOW_WIDTH  360
#define DIR_WINDOW_HEIGHT ((DIR_ENTRIES_COUNT*24)+16+16+16+24)

// Widgets.
static HWND	txtDirectory[DIR_ENTRIES_COUNT];

// Command value bases.
#define IDC_DIR_BTNCHANGE	0x1100

// Apply button.
#define IDAPPLY 0x0010

// Window procedure.
static LRESULT CALLBACK dir_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widget creation functions.
static void	dir_window_create_child_windows(HWND hWnd);

// Directory configuration load/save functions.
static void	dir_window_init(void);
static void	dir_window_save(void);

// Callbacks.
static void	dir_window_callback_btnChange_clicked(int dir);


/**
 * dir_window_show(): Show the Directory Configuration window.
 * @param parent Parent window.
 */
void dir_window_show(HWND parent)
{
	if (dir_window)
	{
		// Directory Configuration window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(dir_window, SW_SHOW);
		return;
	}
	
	if (dir_wndclass.lpfnWndProc != dir_window_wndproc)
	{
		// Create the window class.
		dir_wndclass.style = 0;
		dir_wndclass.lpfnWndProc = dir_window_wndproc;
		dir_wndclass.cbClsExtra = 0;
		dir_wndclass.cbWndExtra = 0;
		dir_wndclass.hInstance = ghInstance;
		dir_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SONIC_HEAD));
		dir_wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		dir_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		dir_wndclass.lpszMenuName = NULL;
		dir_wndclass.lpszClassName = "dir_window";
		
		RegisterClass(&dir_wndclass);
	}
	
	// Create the window.
	dir_window = CreateWindow("dir_window", "Configure Directories",
				  WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				  CW_USEDEFAULT, CW_USEDEFAULT,
				  DIR_WINDOW_WIDTH, DIR_WINDOW_HEIGHT,
				  parent, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	Win32_setActualWindowSize(dir_window, DIR_WINDOW_WIDTH, DIR_WINDOW_HEIGHT);
	
	// Center the window on the parent window.
	// TODO: Change Win32_centerOnGensWindow to accept two parameters.
	Win32_centerOnGensWindow(dir_window);
	
	UpdateWindow(dir_window);
	ShowWindow(dir_window, SW_SHOW);
	return;
}


/**
 * cc_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void dir_window_create_child_windows(HWND hWnd)
{
	// Create the directory entry frame.
	HWND fraDirectories = CreateWindow(WC_BUTTON, "Configure Directories",
					   WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					   8, 8, DIR_WINDOW_WIDTH-16, DIR_WINDOW_HEIGHT-16-24-8,
					   hWnd, NULL, ghInstance, NULL);
	SetWindowFont(fraDirectories, fntMain, TRUE);
	
	// Add the directory entries.
	for (unsigned int dir = 0; dir < DIR_ENTRIES_COUNT; dir++)
	{
		const int curTop = 8+20+(dir*24);
		
		// Create the label for the directory.
		HWND lblTitle = CreateWindow(WC_STATIC, dir_window_entries[dir].title,
					     WS_CHILD | WS_VISIBLE | SS_LEFT,
					     8+8, curTop, 72, 16,
					     hWnd, NULL, ghInstance, NULL);
		SetWindowFont(lblTitle, fntMain, TRUE);
		
		// Create the textbox for the directory.
		txtDirectory[dir] = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
						   WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,
						   8+8+72+8, curTop,
						   DIR_WINDOW_WIDTH-(8+72+16+72+8+16), 20,
						   hWnd, NULL, ghInstance, NULL);
		SetWindowFont(txtDirectory[dir], fntMain, TRUE);
		
		// Create the "Change" button for the directory.
		// TODO: Use an icon?
		HWND btnChange = CreateWindow(WC_BUTTON, "Change...",
					      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					      DIR_WINDOW_WIDTH-8-72-8, curTop,
					      72, 20,
					      hWnd, (HMENU)(IDC_DIR_BTNCHANGE + dir), ghInstance, NULL);
		SetWindowFont(btnChange, fntMain, TRUE);
	}
	
	// Create the dialog buttons.
	
	// OK button.
	HWND btnOK = CreateWindow(WC_BUTTON, "&OK",
				  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
				  DIR_WINDOW_WIDTH-8-75-8-75-8-75, DIR_WINDOW_HEIGHT-8-24,
				  75, 23,
				  hWnd, (HMENU)(IDOK), ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	// Apply button.
	HWND btnApply = CreateWindow(WC_BUTTON, "&Apply",
				     WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				     DIR_WINDOW_WIDTH-8-75-8-75, DIR_WINDOW_HEIGHT-8-24,
				     75, 23,
				     hWnd, (HMENU)(IDAPPLY), ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	// Cancel button.
	HWND btnCancel = CreateWindow(WC_BUTTON, "&Cancel",
				      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				      DIR_WINDOW_WIDTH-8-75, DIR_WINDOW_HEIGHT-8-24,
				      75, 23,
				      hWnd, (HMENU)(IDCANCEL), ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
	// Initialize the directory entries.
	dir_window_init();
}


/**
 * dir_window_close(): Close the OpenGL Resolution window.
 */
void dir_window_close(void)
{
	if (!dir_window)
		return;
	
	// Destroy the window.
	DestroyWindow(dir_window);
	dir_window = NULL;
}


/**
 * dir_window_init(): Initialize the Directory Configuration entries.
 */
static void dir_window_init(void)
{
	for (unsigned int dir = 0; dir < DIR_ENTRIES_COUNT; dir++)
	{
		Edit_SetText(txtDirectory[dir], dir_window_entries[dir].entry);
	}
}

/**
 * dir_window_save(): Save the Directory Configuration entries.
 */
static void dir_window_save(void)
{
	size_t len;
	
	for (unsigned int dir = 0; dir < DIR_ENTRIES_COUNT; dir++)
	{
		// Get the entry text.
		Edit_GetText(txtDirectory[dir], dir_window_entries[dir].entry, GENS_PATH_MAX);
		
		// Make sure the entry is null-terminated.
		dir_window_entries[dir].entry[GENS_PATH_MAX - 1] = 0x00;
		
		// Make sure the end of the directory has a slash.
		// TODO: Do this in functions that use pathnames.
		len = strlen(dir_window_entries[dir].entry);
		if (len > 0 && dir_window_entries[dir].entry[len - 1] != GENS_DIR_SEPARATOR_CHR)
		{
			// String needs to be less than 1 minus the max path length
			// in order to be able to append the directory separator.
			if (len < (GENS_PATH_MAX - 1))
			{
				dir_window_entries[dir].entry[len] = GENS_DIR_SEPARATOR_CHR;
				dir_window_entries[dir].entry[len + 1] = 0x00;
			}
		}
	}
}


/**
 * dir_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK dir_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			dir_window_create_child_windows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					dir_window_save();
					DestroyWindow(hWnd);
					break;
				case IDCANCEL:
					DestroyWindow(hWnd);
					break;
				case IDAPPLY:
					dir_window_save();
					break;
				default:
					if ((LOWORD(wParam) & 0xFF00) == IDC_DIR_BTNCHANGE)
					{
						// Change a directory.
						dir_window_callback_btnChange_clicked(LOWORD(wParam) & 0xFF);
					}
					break;
			}
			break;
		
		case WM_DESTROY:
			if (hWnd != dir_window)
				break;
			
			dir_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * dir_window_callback_btnChange_clicked(): A "Change" button was clicked.
 * @param dir Directory number.
 */
static void dir_window_callback_btnChange_clicked(int dir)
{
	char cur_dir[GENS_PATH_MAX];
	char tmp[64];
	string new_dir;
	
	// Get the currently entered directory.
	Edit_GetText(txtDirectory[dir], cur_dir, GENS_PATH_MAX);
	cur_dir[GENS_PATH_MAX - 1] = 0x00;
	
	// Request a new directory.
	sprintf(tmp, "Select %s Directory", dir_window_entries[dir].title);
	new_dir = GensUI::selectDir(tmp, cur_dir, dir_window);
	
	// If "Cancel" was selected, don't do anything.
	if (new_dir.empty())
		return;
	
	// Set the new directory.
	SetWindowText(txtDirectory[dir], new_dir.c_str());
}

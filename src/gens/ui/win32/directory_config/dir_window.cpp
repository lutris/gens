/***************************************************************************
 * Gens: (Win32) Directory Configuration Window.                           *
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
#include "gens/gens_window.h"

// Plugin Manager.
#include "plugins/pluginmgr.hpp"

// MDP error codes.
#include "mdp/mdp_error.h"

// C includes.
#include <string.h>

// C++ includes.
#include <string>
#include <vector>
#include <list>
using std::string;
using std::vector;
using std::list;

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
#include "libgsft/gsft_file.h"
#include "libgsft/gsft_szprintf.h"

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
#define DIR_WINDOW_HEIGHT_DEFAULT ((DIR_WINDOW_ENTRIES_COUNT*24)+16+16+16+24)
#define DIR_FRAME_HEIGHT(entries) (((entries)*24)+16+8)

static int	dir_window_height;

// Widgets.
static HWND	btnOK, btnCancel, btnApply;
#define IDC_DIR_DIRECTORY 0x1200

// Directory widgets.
typedef struct _dir_widget_t
{
	HWND	txt;
	string	title;
	bool	is_plugin;
	int	id;
} dir_widget_t;
static vector<dir_widget_t> vectDirs;

// Command value bases.
#define IDC_DIR_BTNCHANGE	0x8000

// Window procedure.
static LRESULT CALLBACK dir_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widget creation functions.
static void	dir_window_create_child_windows(HWND hWnd);
static HWND	dir_window_create_dir_widgets(LPCTSTR title, HWND parent, int y, int id);

// Directory configuration load/save functions.
static void	dir_window_init(void);
static void	dir_window_save(void);

// Callbacks.
static void	dir_window_callback_btnChange_clicked(int dir);


/**
 * dir_window_show(): Show the Directory Configuration window.
 */
void dir_window_show(void)
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
		dir_wndclass.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		dir_wndclass.hCursor = NULL;
		dir_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		dir_wndclass.lpszMenuName = NULL;
		dir_wndclass.lpszClassName = TEXT("dir_window");
		
		RegisterClass(&dir_wndclass);
	}
	
	// Create the window.
	dir_window = CreateWindow(TEXT("dir_window"), TEXT("Configure Directories"),
				  WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
				  CW_USEDEFAULT, CW_USEDEFAULT,
				  DIR_WINDOW_WIDTH, DIR_WINDOW_HEIGHT_DEFAULT,
				  gens_window, NULL, ghInstance, NULL);
	
	UpdateWindow(dir_window);
	ShowWindow(dir_window, SW_SHOW);
}


/**
 * cc_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void dir_window_create_child_windows(HWND hWnd)
{
	// Create the internal directory entry frame.
	HWND fraInternalDirs = CreateWindow(WC_BUTTON, TEXT("Gens/GS Directories"),
					    WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
					    8, 8,
					    DIR_WINDOW_WIDTH-16,
					    DIR_FRAME_HEIGHT(DIR_WINDOW_ENTRIES_COUNT),
					    hWnd, NULL, ghInstance, NULL);
	SetWindowFont(fraInternalDirs, fntMain, TRUE);
	
	// Initialize the directory widget vector.
	vectDirs.clear();
	vectDirs.reserve(DIR_WINDOW_ENTRIES_COUNT + PluginMgr::lstDirectories.size());
	
	// Create all internal directory entry widgets.
	int curTop = 8+20-24;
	dir_widget_t dir_widget;
	dir_widget.is_plugin = false;
	for (unsigned int dir = 0; dir < DIR_WINDOW_ENTRIES_COUNT; dir++)
	{
		curTop += 24;
		
		dir_widget.id = dir;
		dir_widget.title = string(dir_window_entries[dir].title);
		
		dir_widget.txt = dir_window_create_dir_widgets(
					dir_window_entries[dir].title,
					hWnd, curTop, vectDirs.size());
		vectDirs.push_back(dir_widget);
	}
	
	// If any plugin directories exist, create the plugin directory entry frame.
	if (!PluginMgr::lstDirectories.empty())
	{
		// Create the plugin directory entry frame.
		HWND fraPluginDirs = CreateWindow(WC_BUTTON, TEXT("Plugin Directories"),
						  WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
						  8, 8+DIR_FRAME_HEIGHT(DIR_WINDOW_ENTRIES_COUNT)+8,
						  DIR_WINDOW_WIDTH-16,
						  DIR_FRAME_HEIGHT(PluginMgr::lstDirectories.size()),
						  hWnd, NULL, ghInstance, NULL);
		SetWindowFont(fraPluginDirs, fntMain, TRUE);
		
		// Create all plugin directory entry widgets.
		int dir = 0x10;
		curTop += 8+16+8;
		dir_widget.is_plugin = true;
		
		for (list<mdpDir_t>::iterator iter = PluginMgr::lstDirectories.begin();
		     iter != PluginMgr::lstDirectories.end(); iter++, dir++)
		{
			curTop += 24;
			
			dir_widget.id = (*iter).id;
			dir_widget.title = (*iter).name;
			
			dir_widget.txt = dir_window_create_dir_widgets(
						(*iter).name.c_str(),
						hWnd, curTop, vectDirs.size());			
			vectDirs.push_back(dir_widget);
		}
	}
	
	// Calculate the window height.
	dir_window_height = 8 + DIR_FRAME_HEIGHT(DIR_WINDOW_ENTRIES_COUNT) + 8 + 24 + 8;
	if (!PluginMgr::lstDirectories.empty())
	{
		dir_window_height += DIR_FRAME_HEIGHT(PluginMgr::lstDirectories.size()) + 8;
	}
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(hWnd, DIR_WINDOW_WIDTH, dir_window_height);
	
	// Center the window on the parent window.
	gsft_win32_center_on_window(hWnd, gens_window);
	
	// Create the dialog buttons.
	
	// OK button.
	btnOK = CreateWindow(WC_BUTTON, TEXT("&OK"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
					DIR_WINDOW_WIDTH-8-75-8-75-8-75, dir_window_height-8-24,
					75, 23,
					hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFont(btnOK, fntMain, TRUE);
	
	// Cancel button.
	btnCancel = CreateWindow(WC_BUTTON, TEXT("&Cancel"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					DIR_WINDOW_WIDTH-8-75-8-75, dir_window_height-8-24,
					75, 23,
					hWnd, (HMENU)IDCANCEL, ghInstance, NULL);
	SetWindowFont(btnCancel, fntMain, TRUE);
	
	// Apply button.
	btnApply = CreateWindow(WC_BUTTON, TEXT("&Apply"),
					WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					DIR_WINDOW_WIDTH-8-75, dir_window_height-8-24,
					75, 23,
					hWnd, (HMENU)IDAPPLY, ghInstance, NULL);
	SetWindowFont(btnApply, fntMain, TRUE);
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, false);
	
	// Initialize the directory entries.
	dir_window_init();
}


static HWND dir_window_create_dir_widgets(LPCTSTR title, HWND container, int y, int id)
{
	// Create the label for the directory.
	HWND lblTitle = CreateWindow(WC_STATIC, title,
				     WS_CHILD | WS_VISIBLE | SS_LEFT,
				     8+8, y, 72, 16,
				     container, NULL, ghInstance, NULL);
	SetWindowFont(lblTitle, fntMain, TRUE);
	
	// Create the textbox for the directory.
	HWND txtDirectory = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL,
					   WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL,
					   8+8+72+8, y,
					   DIR_WINDOW_WIDTH-(8+72+16+72+8+16), 20,
					   container, (HMENU)(IDC_DIR_DIRECTORY), ghInstance, NULL);
	SetWindowFont(txtDirectory, fntMain, TRUE);
	
	// Create the "Change" button for the directory.
	// TODO: Use an icon?
	HWND btnChange = CreateWindow(WC_BUTTON, TEXT("Change..."),
				      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				      DIR_WINDOW_WIDTH-8-72-8, y,
				      72, 20,
				      container, (HMENU)(IDC_DIR_BTNCHANGE + id), ghInstance, NULL);
	SetWindowFont(btnChange, fntMain, TRUE);
	
	return txtDirectory;
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
	char dir_buf[GENS_PATH_MAX];
	char dir_buf_rel[GENS_PATH_MAX];
	const unsigned int gens_exe_pathlen = strlen(PathNames.Gens_EXE_Path);
	
	// Internal directories.
	for (unsigned int dir = 0; dir < vectDirs.size(); dir++)
	{
		if (!vectDirs[dir].is_plugin)
		{
			// Internal directory.
			Edit_SetText(vectDirs[dir].txt,
				     dir_window_entries[vectDirs[dir].id].entry);
		}
		else
		{
			// Plugin directory.
			mapDirItems::iterator dirIter = PluginMgr::tblDirectories.find(vectDirs[dir].id);
			if (dirIter == PluginMgr::tblDirectories.end())
				continue;
			
			list<mdpDir_t>::iterator lstDirIter = (*dirIter).second;
			const mdpDir_t& mdpDir = *lstDirIter;
			
			// Get the directory.
			if (mdpDir.get(vectDirs[dir].id, dir_buf, sizeof(dir_buf)) != MDP_ERR_OK)
			{
				// Error retrieving the directory.
				continue;
			}
			
			// Directory retrieved.
			// If possible, convert it to a relative pathname.
			gsft_file_abs_to_rel(dir_buf, PathNames.Gens_EXE_Path,
					     dir_buf_rel, sizeof(dir_buf_rel));
			Edit_SetText(vectDirs[dir].txt, dir_buf_rel);
		}
	}
	
	// Disable the "Apply" button initially.
	Button_Enable(btnApply, false);
}

/**
 * dir_window_save(): Save the Directory Configuration entries.
 */
static void dir_window_save(void)
{
	size_t len;
	char dir_buf[GENS_PATH_MAX];
	char dir_buf_abs[GENS_PATH_MAX];
	
	for (unsigned int dir = 0; dir < vectDirs.size(); dir++)
	{
		if (!vectDirs[dir].is_plugin)
		{
			// Internal directory.
			char *entry = dir_window_entries[vectDirs[dir].id].entry;
			
			// Get the entry text.
			Edit_GetText(vectDirs[dir].txt, entry, GENS_PATH_MAX);
			
			// Make sure the end of the directory has a slash.
			// TODO: Do this in functions that use pathnames.
			len = strlen(entry);
			if (len > 0 && entry[len-1] != GSFT_DIR_SEP_CHR)
			{
				// String needs to be less than 1 minus the max path length
				// in order to be able to append the directory separator.
				if (len < (GENS_PATH_MAX-1))
				{
					entry[len] = GSFT_DIR_SEP_CHR;
					entry[len+1] = 0x00;
				}
			}
		}
		else
		{
			// Plugin directory.
			
			// Get the entry text.
			Edit_GetText(vectDirs[dir].txt, dir_buf, sizeof(dir_buf));
			
			// Make sure the entry is null-terminated.
			dir_buf[sizeof(dir_buf)-1] = 0x00;
			
			// Make sure the end of the directory has a slash.
			// TODO: Do this in functions that use pathnames.
			len = strlen(dir_buf);
			if (len > 0 && dir_buf[len-1] != GSFT_DIR_SEP_CHR)
			{
				// String needs to be less than 1 minus the max path length
				// in order to be able to append the directory separator.
				if (len < (GENS_PATH_MAX-1))
				{
					dir_buf[len] = GSFT_DIR_SEP_CHR;
					dir_buf[len+1] = 0x00;
				}
			}
			
			// If necessary, convert the pathname to an absolute pathname.
			gsft_file_rel_to_abs(dir_buf, PathNames.Gens_EXE_Path,
					     dir_buf_abs, sizeof(dir_buf_abs));
			
			mapDirItems::iterator dirIter = PluginMgr::tblDirectories.find(vectDirs[dir].id);
			if (dirIter == PluginMgr::tblDirectories.end())
				continue;
			
			list<mdpDir_t>::iterator lstDirIter = (*dirIter).second;
			const mdpDir_t& mdpDir = *lstDirIter;
			
			// Set the directory.
			mdpDir.set(vectDirs[dir].id, dir_buf_abs);
		}
	}
	
	// Disable the "Apply" button.
	Button_Enable(btnApply, false);
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
					switch (LOWORD(wParam) & 0xFF00)
					{
						case IDC_DIR_BTNCHANGE:
							// Change a directory.
							dir_window_callback_btnChange_clicked(LOWORD(wParam) & 0x7FFF);
							break;
						case IDC_DIR_DIRECTORY:
							if (HIWORD(wParam) == EN_CHANGE)
							{
								// Directory textbox was changed.
								// Enable the "Apply" button.
								Button_Enable(btnApply, true);
							}
							break;
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
	char dir_buf[GENS_PATH_MAX];
	char dir_buf_abs[GENS_PATH_MAX];
	string new_dir;
	
	if (dir < 0 || dir >= vectDirs.size())
		return;
	
	dir_widget_t *dir_widget = &vectDirs[dir];
	
	// Get the currently entered directory.
	Edit_GetText(dir_widget->txt, dir_buf, sizeof(dir_buf));
	dir_buf[sizeof(dir_buf)-1] = 0x00;
	
	// Convert the current directory to an absolute pathname, if necessary.
	gsft_file_rel_to_abs(dir_buf, PathNames.Gens_EXE_Path,
			     dir_buf_abs, sizeof(dir_buf_abs));
	
	// Set the title of the window.
	char tmp[128];
	szprintf(tmp, sizeof(tmp), "Select %s Directory", dir_widget->title.c_str());
	
	// Request a new directory.
	new_dir = GensUI::selectDir(tmp, dir_buf_abs, dir_window);
	
	// If "Cancel" was selected, don't do anything.
	if (new_dir.empty())
		return;
	
	// Make sure the end of the directory has a slash.
	if (new_dir.at(new_dir.length() - 1) != GSFT_DIR_SEP_CHR)
		new_dir += GSFT_DIR_SEP_CHR;
	
	// Convert the new directory to a relative pathname, if possible.
	gsft_file_abs_to_rel(new_dir.c_str(), PathNames.Gens_EXE_Path,
			     dir_buf, sizeof(dir_buf));
	
	// Set the new directory.
	SetWindowText(dir_widget->txt, dir_buf);
	
	// Enable the "Apply" button.
	Button_Enable(btnApply, true);
}

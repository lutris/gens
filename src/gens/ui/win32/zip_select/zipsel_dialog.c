/***************************************************************************
 * Gens: (Win32) Zip File Selection Dialog.                                *
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

#include "zipsel_dialog.h"
#include "gens/gens_window.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "emulator/g_main.hpp"

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"


/**
 * zipsel_dialog_init(): Initialize the Zip Select dialog.
 * @param hWndDlg hWnd of the Zip Select dialog.
 * @param z_list File list.
 */
static void zipsel_dialog_init(HWND hWndDlg, mdp_z_entry_t *z_list)
{
	gsft_win32_center_on_window(hWndDlg, gens_window);
	
	// Get the listbox hWnd.
	HWND lstFiles = GetDlgItem(hWndDlg, IDC_ZIPSELECT_LSTFILES);
	
	// Add all strings.
	mdp_z_entry_t *z_entry_cur = z_list;
	while (z_entry_cur)
	{
		int index = ListBox_InsertString(lstFiles, -1, z_entry_cur->filename);
		ListBox_SetItemData(lstFiles, index, z_entry_cur);
		
		// Next file.
		z_entry_cur = z_entry_cur->next;
	}
	
	// Select the first item by default.
	ListBox_SetCurSel(lstFiles, 0);
}


/**
 * getCurListItemData(): Gets the data of the selected list item.
 * @param hWndDlg Dialog containing the listbox.
 * @param nIDDlgItem ID of the listbox.
 * @return Data of the selected list item. (-1 if nothing is selected.)
 */
static inline int getCurListItemData(HWND hWndDlg, int nIDDlgItem)
{
	HWND lstBox = GetDlgItem(hWndDlg, nIDDlgItem);
	int index = ListBox_GetCurSel(lstBox);
	return ListBox_GetItemData(lstBox, index);
}


static INT_PTR CALLBACK zipsel_dialog_dlgproc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			zipsel_dialog_init(hWndDlg, (mdp_z_entry_t*)lParam);
			return TRUE;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hWndDlg, getCurListItemData(hWndDlg, IDC_ZIPSELECT_LSTFILES));
					return TRUE;
				case IDCANCEL:
					EndDialog(hWndDlg, 0);
					return TRUE;
				case IDC_ZIPSELECT_LSTFILES:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						// Listbox double-click. Interpret this as "OK".
						EndDialog(hWndDlg, getCurListItemData(hWndDlg, IDC_ZIPSELECT_LSTFILES));
						return TRUE;
					}
					break;
			}
			break;
	}
	
	return FALSE;
}


/**
 * zipsel_dialog_get_file(): Opens the Zip File Selection Dialog.
 */
mdp_z_entry_t* zipsel_dialog_get_file(mdp_z_entry_t *z_list)
{
	if (!z_list)
	{
		// NULL list pointer passed. Don't do anything.
		return NULL;
	}
	
	mdp_z_entry_t *file;
	file = (mdp_z_entry_t*)(DialogBoxParam(
				  ghInstance, MAKEINTRESOURCE(IDD_ZIPSELECT),
				  gens_window, zipsel_dialog_dlgproc,
				  (LPARAM)z_list));
	
	// File was selected.
	return file;
}

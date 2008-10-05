/***************************************************************************
 * Gens: (GTK+) Zip File Selection Dialog - Miscellaneous Functions.       *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "zip_select_dialog_misc.hpp"
#include "gens/gens_window.h"

#include "emulator/g_main.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"

// Win32 common controls
#include <commctrl.h>

#include <string>
#include <list>
using std::string;
using std::list;


static void Init_Zip_Select_Dialog(HWND hWndDlg, list<CompressedFile>* lst);
static inline int getCurListItem(HWND hWndDlg, int nIDDlgItem);


LRESULT CALLBACK Zip_Select_Dialog_DlgProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam);


/**
 * Get_Zip_Select(): Opens the Zip File Selection Dialog.
 */
CompressedFile* Open_Zip_Select_Dialog(list<CompressedFile>* lst)
{
	if (!lst)
	{
		// NULL list pointer passed. Don't do anything.
		return NULL;
	}
	
	int file = DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_ZIPSELECT),
				  Gens_hWnd, reinterpret_cast<DLGPROC>(Zip_Select_Dialog_DlgProc),
				  reinterpret_cast<LPARAM>(lst));
	if (file < 0)
	{
		// No file was selected.
		return NULL;
	}
	
	// File was selected.
	if (static_cast<unsigned int>(file) >= lst->size())
	{
		// File index is out of range.
		return NULL;
	}
	
	// Return the file.
	list<CompressedFile>::iterator lstIter = lst->begin();
	for (int i = 0; i < file; i++)
		lstIter++;
	
	return &(*lstIter);
}


LRESULT CALLBACK Zip_Select_Dialog_DlgProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			Init_Zip_Select_Dialog(hWndDlg, reinterpret_cast<list<CompressedFile>*>(lParam));
			return TRUE;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hWndDlg, getCurListItem(hWndDlg, IDC_ZIPSELECT_LSTFILES));
					return TRUE;
				case IDCANCEL:
					EndDialog(hWndDlg, -1);
					return TRUE;
				case IDC_ZIPSELECT_LSTFILES:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						// Listbox double-click. Interpret this as "OK".
						EndDialog(hWndDlg, getCurListItem(hWndDlg, IDC_ZIPSELECT_LSTFILES));
						return TRUE;
					}
			}
			break;
	}
	
	return FALSE;
}


/**
 * Init_Zip_Select_Dialog(): Initialize the Zip Select dialog.
 * @param hWndDlg hWnd of the Zip Select dialog.
 * @param lst List of files in the archive.
 */
static void Init_Zip_Select_Dialog(HWND hWndDlg, list<CompressedFile>* lst)
{
	Win32_centerOnGensWindow(hWndDlg);
	
	// Get the listbox hWnd.
	HWND lstFiles = GetDlgItem(hWndDlg, IDC_ZIPSELECT_LSTFILES);
	
	// Reserve space in the listbox for the file listing.
	// Assuming maximum of 128 bytes per filename.
	SendMessage(lstFiles, LB_INITSTORAGE, static_cast<WPARAM>(lst->size()), 128);
	
	// Add all strings.
	list<CompressedFile>::iterator lstIter;
	for (lstIter = lst->begin(); lstIter != lst->end(); lstIter++)
	{
		SendMessage(lstFiles, LB_ADDSTRING, static_cast<WPARAM>(NULL),
			    reinterpret_cast<LPARAM>((*lstIter).filename.c_str()));
	}
}


/**
 * getCurListItem(): Gets the index of the selected list item.
 * @param hWndDlg Dialog containing the listbox.
 * @param nIDDlgItem ID of the listbox.
 * @return Index of the selected list item. (-1 if nothing is selected.)
 */
static inline int getCurListItem(HWND hWndDlg, int nIDDlgItem)
{
	HWND lstBox = GetDlgItem(hWndDlg, nIDDlgItem);
	return SendMessage(lstBox, LB_GETCURSEL, NULL, NULL);
}

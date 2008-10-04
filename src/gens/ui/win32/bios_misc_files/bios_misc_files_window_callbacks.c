/***************************************************************************
 * Gens: (Win32) BIOS/Misc Files Window - Callback Functions.              *
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

#include "bios_misc_files_window.hpp"
#include "bios_misc_files_window_callbacks.h"
#include "bios_misc_files_window_misc.hpp"

#include "emulator/gens.hpp"

// Gens Win32 resources
#include "ui/win32/resource.h"


LRESULT CALLBACK BIOS_Misc_Files_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			BIOS_Misc_Files_Window_CreateChildWindows(hWnd);
			break;
		
		case WM_CLOSE:
			DestroyWindow(bios_misc_files_window);
			return 0;
		
		case WM_COMMAND:
			// Button press
#if 0
			switch (LOWORD(wParam))
			{
				case IDOK: // Standard dialog button ID
				case IDC_BTN_OK:
				case IDC_BTN_SAVE:
					CA_Save();
					DestroyWindow(hWnd);
					break;
				
				case IDC_BTN_APPLY:
					CA_Save();
					break;
				
				case IDCANCEL: // Standard dialog button ID
				case IDC_BTN_CANCEL:
					DestroyWindow(hWnd);
					break;
			}
#endif
			break;
		
		case WM_DESTROY:
			if (hWnd != bios_misc_files_window)
				break;
			
			bios_misc_files_window = NULL;
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


#if 0
/**
 * Window is closed.
 */
gboolean on_bios_misc_files_window_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(widget);
	GENS_UNUSED_PARAMETER(event);
	GENS_UNUSED_PARAMETER(user_data);
	
	gtk_widget_destroy(bios_misc_files_window);
	bios_misc_files_window = NULL;
	return FALSE;
}


/**
 * Change...
 */
void on_button_bmf_Change_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	
	BIOS_Misc_Files_Change(GPOINTER_TO_INT(user_data));
}


/**
 * Cancel
 */
void on_button_bmf_Cancel_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	gtk_widget_destroy(bios_misc_files_window);
	bios_misc_files_window = NULL;
}


/**
 * Apply
 */
void on_button_bmf_Apply_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	BIOS_Misc_Files_Save();
}


/**
 * Save
 */
void on_button_bmf_Save_clicked(GtkButton *button, gpointer user_data)
{
	GENS_UNUSED_PARAMETER(button);
	GENS_UNUSED_PARAMETER(user_data);
	
	BIOS_Misc_Files_Save();
	gtk_widget_destroy(bios_misc_files_window);
	bios_misc_files_window = NULL;
}
#endif

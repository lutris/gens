/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_windowsxA.c: windowsx.h translation. (ANSI version)                *
 *                                                                         *
 * Copyright (c) 2009 by David Korth.                                      *
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

#include "w32u_windowsxA.h"
#include "w32u_windowsx.h"

// C includes.
#include <stdlib.h>


/** ListBox functions. **/


static int WINAPI ListBox_GetTextUA(HWND hwndCtl, int index, LPSTR lpszBuffer)
{
	// TODO: Make a generic function for this.
	// Main problem is that different GETTEXT messages use different values for WPARAM.
	// - WM_GETTEXT specifies size of the return buffer as WPARAM.
	// - LB_GETTEXT specifies the list index.
	
	// TODO: ANSI conversion.
	return SendMessageA(hwndCtl, LB_GETTEXT, (WPARAM)index, (LPARAM)lpszBuffer);
}


void WINAPI w32u_windowsxA_init(void)
{
	pListBox_GetTextU	= &ListBox_GetTextUA;
}

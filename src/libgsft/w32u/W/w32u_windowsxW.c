/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_windowsxW.c: windowsx.h translation. (Unicode version)             *
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

#include "w32u_windowsxW.h"
#include "w32u_windowsx.h"

// C includes.
#include <stdlib.h>


/** ListBox functions. **/


static int WINAPI ListBox_GetTextUW(HWND hwndCtl, int index, LPSTR lpszBuffer)
{
	// TODO: Make a generic function for this.
	// Main problem is that different GETTEXT messages use different values for WPARAM.
	// - WM_GETTEXT specifies size of the return buffer as WPARAM.
	// - LB_GETTEXT specifies the list index.
	
	if (!lpszBuffer)
	{
		// String not specified.
		return SendMessageW(hwndCtl, LB_GETTEXT, (WPARAM)index, (LPARAM)lpszBuffer);
	}
	
	// Allocate a buffer for the string.
	int textLength = ListBox_GetTextLenU(hwndCtl, index);
	if (textLength <= 0)
	{
		// No text.
		return 0;
	}
	wchar_t *wbuf = (wchar_t*)malloc((textLength + 1) * sizeof(wchar_t));
	
	// Get the string.
	LRESULT lRet = SendMessageW(hwndCtl, LB_GETTEXT, (WPARAM)index, (LPARAM)wbuf);
	if (lRet == LB_ERR)
	{
		// Invalid index.
		free(wbuf);
		return lRet;
	}
	else if (lRet == 0)
	{
		// No text.
		lpszBuffer[0] = 0;
		free(wbuf);
		return lRet;
	}
	
	// lRet contains the number of characters returned.
	// NOTE: No boundary checking is performed here!
	// textLength * 4 is assumed as the worst-case buffer size, since UTF-8
	// allows up to 4-byte characters.
	WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, lpszBuffer, textLength * 4, NULL, NULL);
	free(wbuf);
	return lRet;
}


void WINAPI w32u_windowsxW_init(void)
{
	pListBox_GetTextU	= &ListBox_GetTextUW;
}

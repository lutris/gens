/***************************************************************************
 * Gens: (Win32) Unicode Translation Layer. (windowsx.h)                   *
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

#include "w32_unicode.h"
#include "w32_unicode_x.h"

// C includes.
#include <stdlib.h>


int ComboBox_AddStringU(HWND hwndCtl, LPCSTR lpsz)
{
	if (!isSendMessageUnicode)
		return pSendMessage(hwndCtl, CB_ADDSTRING, 0, (LPARAM)lpsz);
	
	
	// Convert lpsz from UTF-8 to UTF-16.
	int lpszw_len;
	wchar_t *lpszw = NULL;
	
	if (lpsz)
	{
		lpszw_len = MultiByteToWideChar(CP_UTF8, 0, lpsz, -1, NULL, 0);
		lpszw_len *= sizeof(wchar_t);
		lpszw = (wchar_t*)malloc(lpszw_len);
		MultiByteToWideChar(CP_UTF8, 0, lpsz, -1, lpszw, lpszw_len);
	}
	
	LRESULT lRet = pSendMessage(hwndCtl, CB_ADDSTRING, 0, (LPARAM)lpszw);
	free(lpszw);
	return lRet;
}

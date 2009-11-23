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


int TabCtrl_InsertItemU(HWND hWnd, int iItem, const LPTCITEM pItem)
{
	if (!isSendMessageUnicode)
		return pSendMessage(hWnd, TCM_INSERTITEMA, iItem, (LPARAM)pItem);
	
	// Convert pItem->pszText from UTF-8 to UTF-16.
	TCITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	int pszText_len;
	wchar_t *pszwText = NULL;
	
	if (wItem.pszText)
	{
		pszText_len = MultiByteToWideChar(CP_UTF8, 0, pItem->pszText, -1, NULL, 0);
		pszText_len *= sizeof(wchar_t);
		pszwText = (wchar_t*)malloc(pszText_len);
		MultiByteToWideChar(CP_UTF8, 0, pItem->pszText, -1, pszwText, pszText_len);
		wItem.pszText = pszwText;
	}
	
	LRESULT lRet = pSendMessage(hWnd, TCM_INSERTITEMW, iItem, (LPARAM)&wItem);
	free(pszwText);
	return lRet;
}

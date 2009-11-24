/***************************************************************************
 * Gens: (Win32) Unicode Translation Layer. (commctrl.h)                   *
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
#include "w32_unicode_commctrl.h"

// C includes.
#include <stdlib.h>


int TabCtrl_InsertItemU(HWND hWnd, int iItem, const LPTCITEM pItem)
{
	if (!isSendMessageUnicode)
		return pSendMessageU(hWnd, TCM_INSERTITEMA, iItem, (LPARAM)pItem);
	
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
	
	LRESULT lRet = pSendMessageU(hWnd, TCM_INSERTITEMW, iItem, (LPARAM)&wItem);
	free(pszwText);
	return lRet;
}


/** ListView functions. **/


int ListView_GetItemU(HWND hWnd, LVITEM *pItem)
{
	if (!isSendMessageUnicode)
		return pSendMessageU(hWnd, LVM_GETITEMA, 0, (LPARAM)pItem);
	
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText || pItem->cchTextMax <= 0)
	{
		// No text. Send the message as-is.
		return pSendMessageU(hWnd, LVM_GETITEMW, 0, (LPARAM)pItem);
	}
	
	// Copy the LVITEM temporarily.
	LVITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	// Allocate a buffer for the UTF-16 text.
	wchar_t *wbuf = (wchar_t*)malloc((wItem.cchTextMax + 1) * sizeof(wchar_t));
	wItem.pszText = wbuf;
	
	// Get the LVITEM.
	LRESULT lRet = pSendMessageU(hWnd, LVM_GETITEMW, 0, (LPARAM)&wItem);
	if (!lRet)
	{
		// Error retrieving the item.
		free(wbuf);
		return lRet;
	}
	
	// Convert the text to UTF-8.
	// TODO Verify characters vs. bytes.
	char *utf8_buf = pItem->pszText;
	WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, utf8_buf, pItem->cchTextMax, NULL, NULL);
	
	// Copy the LVITEM struct back to pItem.
	memcpy(pItem, &wItem, sizeof(*pItem));
	pItem->pszText = utf8_buf;
	
	free(wbuf);
	return lRet;
}


int ListView_InsertColumnU(HWND hWnd, int iCol, const LV_COLUMN *pCol)
{
	if (!isSendMessageUnicode)
		return pSendMessageU(hWnd, LVM_INSERTCOLUMNA, iCol, (LPARAM)pCol);
	
	// Convert pCol->pszText from UTF-8 to UTF-16.
	TCITEMW wCol;
	memcpy(&wCol, pCol, sizeof(wCol));
	
	int pszText_len;
	wchar_t *pszwText = NULL;
	
	if (wCol.pszText)
	{
		pszText_len = MultiByteToWideChar(CP_UTF8, 0, pCol->pszText, -1, NULL, 0);
		pszText_len *= sizeof(wchar_t);
		pszwText = (wchar_t*)malloc(pszText_len);
		MultiByteToWideChar(CP_UTF8, 0, pCol->pszText, -1, pszwText, pszText_len);
		wCol.pszText = pszwText;
	}
	
	LRESULT lRet = pSendMessageU(hWnd, LVM_INSERTCOLUMNW, iCol, (LPARAM)&wCol);
	free(pszwText);
	return lRet;
}


int ListView_InsertItemU(HWND hWnd, const LVITEM *pItem)
{
	if (!isSendMessageUnicode)
		return pSendMessageU(hWnd, LVM_INSERTITEMA, 0, (LPARAM)pItem);
	
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText)
	{
		// No text. Send the message as-is.
		return pSendMessageU(hWnd, LVM_INSERTITEMW, 0, (LPARAM)pItem);
	}
	
	// Convert pItem->pszText from UTF-8 to UTF-16.
	LVITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	int pszText_len;
	wchar_t *pszwText = NULL;
	
	pszText_len = MultiByteToWideChar(CP_UTF8, 0, pItem->pszText, -1, NULL, 0);
	pszText_len *= sizeof(wchar_t);
	pszwText = (wchar_t*)malloc(pszText_len);
	MultiByteToWideChar(CP_UTF8, 0, pItem->pszText, -1, pszwText, pszText_len);
	wItem.pszText = pszwText;
	
	LRESULT lRet = pSendMessageU(hWnd, LVM_INSERTITEMW, 0, (LPARAM)&wItem);
	free(pszwText);
	return lRet;
}


int ListView_SetItemU(HWND hWnd, const LVITEM *pItem)
{
	if (!isSendMessageUnicode)
		return pSendMessageU(hWnd, LVM_SETITEMA, 0, (LPARAM)pItem);
	
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText)
	{
		// No text. Send the message as-is.
		return pSendMessageU(hWnd, LVM_SETITEMW, 0, (LPARAM)pItem);
	}
	
	// Convert pItem->pszText from UTF-8 to UTF-16.
	LVITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	int pszText_len;
	wchar_t *pszwText = NULL;
	
	pszText_len = MultiByteToWideChar(CP_UTF8, 0, pItem->pszText, -1, NULL, 0);
	pszText_len *= sizeof(wchar_t);
	pszwText = (wchar_t*)malloc(pszText_len);
	MultiByteToWideChar(CP_UTF8, 0, pItem->pszText, -1, pszwText, pszText_len);
	wItem.pszText = pszwText;
	
	LRESULT lRet = pSendMessageU(hWnd, LVM_SETITEMW, 0, (LPARAM)&wItem);
	free(pszwText);
	return lRet;
}

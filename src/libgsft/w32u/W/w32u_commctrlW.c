/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_commctrlW.c: commctrl.h translation. (Unicode version)             *
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

#include "w32u_commctrlW.h"
#include "w32u_commctrl.h"
#include "w32u_charset.h"

// C includes.
#include <stdlib.h>


/** TabCtrl functions. **/


static int WINAPI TabCtrl_InsertItemUW(HWND hWnd, int iItem, const TCITEMA *pItem)
{
	// Convert pItem->pszText from UTF-8 to UTF-16.
	TCITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	wchar_t *pszwText = NULL;
	if (wItem.pszText)
	{
		pszwText = w32u_UTF8toUTF16(pItem->pszText);
		wItem.pszText = pszwText;
	}
	
	LRESULT lRet = SendMessageW(hWnd, TCM_INSERTITEMW, iItem, (LPARAM)&wItem);
	free(pszwText);
	return lRet;
}


/** ListView functions. **/


static int WINAPI ListView_GetItemUW(HWND hWnd, LVITEMA *pItem)
{
	
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText || pItem->cchTextMax <= 0)
	{
		// No text. Send the message as-is.
		return SendMessageW(hWnd, LVM_GETITEMW, 0, (LPARAM)pItem);
	}
	
	// Copy the LVITEM temporarily.
	LVITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	// Allocate a buffer for the UTF-16 text.
	wchar_t *pszwText = (wchar_t*)malloc((wItem.cchTextMax + 1) * sizeof(wchar_t));
	wItem.pszText = pszwText;
	
	// Get the LVITEM.
	LRESULT lRet = SendMessageW(hWnd, LVM_GETITEMW, 0, (LPARAM)&wItem);
	if (!lRet)
	{
		// Error retrieving the item.
		free(pszwText);
		return lRet;
	}
	
	// Copy the LVITEM struct back to pItem.
	char *pszText_tmp = pItem->pszText;
	memcpy(pItem, &wItem, sizeof(*pItem));
	pItem->pszText = pszText_tmp;
	
	// Convert the text to UTF-8.
	// TODO Verify characters vs. bytes.
	w32u_UTF16toUTF8_copy(pItem->pszText, pszwText, pItem->cchTextMax);
	
	free(pszwText);
	return lRet;
}


static int WINAPI ListView_InsertColumnUW(HWND hWnd, int iCol, const LV_COLUMNA *pCol)
{
	// Convert pCol->pszText from UTF-8 to UTF-16.
	TCITEMW wCol;
	memcpy(&wCol, pCol, sizeof(wCol));
	
	wchar_t *pszwText = NULL;
	if (wCol.pszText)
	{
		pszwText = w32u_UTF8toUTF16(pCol->pszText);
		wCol.pszText = pszwText;
	}
	
	LRESULT lRet = SendMessageW(hWnd, LVM_INSERTCOLUMNW, iCol, (LPARAM)&wCol);
	free(pszwText);
	return lRet;
}


static int WINAPI ListView_InsertItemUW(HWND hWnd, const LVITEMA *pItem)
{
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText)
	{
		// No text. Send the message as-is.
		return SendMessageW(hWnd, LVM_INSERTITEMW, 0, (LPARAM)pItem);
	}
	
	// Convert pItem->pszText from UTF-8 to UTF-16.
	LVITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	wchar_t *pszwText = NULL;
	if (pItem->pszText)
	{
		w32u_UTF8toUTF16(pItem->pszText);
		wItem.pszText = pszwText;
	}
	
	LRESULT lRet = SendMessageW(hWnd, LVM_INSERTITEMW, 0, (LPARAM)&wItem);
	free(pszwText);
	return lRet;
}


static int WINAPI ListView_SetItemUW(HWND hWnd, const LVITEM *pItem)
{
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText)
	{
		// No text. Send the message as-is.
		return SendMessageW(hWnd, LVM_SETITEMW, 0, (LPARAM)pItem);
	}
	
	// Convert pItem->pszText from UTF-8 to UTF-16.
	LVITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	wchar_t *pszwText = NULL;
	if (pItem->pszText)
	{
		pszwText = w32u_UTF8toUTF16(pItem->pszText);
		wItem.pszText = pszwText;
	}
	
	LRESULT lRet = SendMessageW(hWnd, LVM_SETITEMW, 0, (LPARAM)&wItem);
	free(pszwText);
	return lRet;
}


int WINAPI w32u_commctrlW_init(void)
{
	// TODO: Error handling.
	
	pTabCtrl_InsertItemU	= &TabCtrl_InsertItemUW;
	
	pListView_GetItemU	= &ListView_GetItemUW;
	pListView_InsertColumnU	= &ListView_InsertColumnUW;
	pListView_InsertItemU	= &ListView_InsertItemUW;
	pListView_SetItemU	= &ListView_SetItemUW;
	
	return 0;
}

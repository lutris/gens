/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_commctrlA.c: commctrl.h translation. (ANSI version)                *
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

#include "w32u_commctrlA.h"
#include "w32u_commctrl.h"
#include "w32u_charset.h"

// C includes.
#include <stdlib.h>


/** TabCtrl functions. **/


static int WINAPI TabCtrl_InsertItemUA(HWND hWnd, int iItem, const TCITEMA *pItem)
{
	// Convert pItem->pszText from UTF-8 to ANSI.
	TCITEMA aItem;
	memcpy(&aItem, pItem, sizeof(aItem));
	
	char *pszaText = NULL;
	if (aItem.pszText)
	{
		pszaText = w32u_UTF8toANSI(pItem->pszText);
		aItem.pszText = pszaText;
	}
	
	LRESULT lRet = SendMessageA(hWnd, TCM_INSERTITEMA, iItem, (LPARAM)&aItem);
	free(pszaText);
	return lRet;
}


/** ListView functions. **/


static int WINAPI ListView_GetItemUA(HWND hWnd, LVITEMA *pItem)
{
	
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText || pItem->cchTextMax <= 0)
	{
		// No text. Send the message as-is.
		return SendMessageA(hWnd, LVM_GETITEMA, 0, (LPARAM)pItem);
	}
	
	// Get the LVITEM.
	LRESULT lRet = SendMessageA(hWnd, LVM_GETITEMA, 0, (LPARAM)&pItem);
	if (!lRet)
	{
		// Error retrieving the item.
		return lRet;
	}
	
	// Convert the text from ANSI to UTF-8 in place.
	// TODO: Check for errors in case the UTF-8 text doesn't fit.
	w32u_ANSItoUTF8_ip(pItem->pszText, pItem->cchTextMax);
	return lRet;
}


static int WINAPI ListView_InsertColumnUA(HWND hWnd, int iCol, const LV_COLUMNA *pCol)
{
	// Convert pCol->pszText from UTF-8 to ANSI.
	TCITEMA aCol;
	memcpy(&aCol, pCol, sizeof(aCol));
	
	char *pszaText = NULL;
	if (aCol.pszText)
	{
		pszaText = w32u_UTF8toANSI(pCol->pszText);
		aCol.pszText = pszaText;
	}
	
	LRESULT lRet = SendMessageA(hWnd, LVM_INSERTCOLUMNA, iCol, (LPARAM)&aCol);
	free(pszaText);
	return lRet;
}


static int WINAPI ListView_InsertItemUA(HWND hWnd, const LVITEMA *pItem)
{
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText)
	{
		// No text. Send the message as-is.
		return SendMessageA(hWnd, LVM_INSERTITEMA, 0, (LPARAM)pItem);
	}
	
	// Convert pItem->pszText from UTF-8 to ANSI.
	LVITEMA aItem;
	memcpy(&aItem, pItem, sizeof(aItem));
	
	char *pszaText = NULL;
	if (pItem->pszText)
	{
		pszaText = w32u_UTF8toANSI(pItem->pszText);
		aItem.pszText = pszaText;
	}
	
	LRESULT lRet = SendMessageA(hWnd, LVM_INSERTITEMA, 0, (LPARAM)&aItem);
	free(pszaText);
	return lRet;
}


static int WINAPI ListView_SetItemUA(HWND hWnd, const LVITEM *pItem)
{
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText)
	{
		// No text. Send the message as-is.
		return SendMessageA(hWnd, LVM_SETITEMA, 0, (LPARAM)pItem);
	}
	
	// Convert pItem->pszText from UTF-8 to ANSI.
	LVITEMA aItem;
	memcpy(&aItem, pItem, sizeof(aItem));
	
	char *pszaText = NULL;
	if (pItem->pszText)
	{
		pszaText = w32u_UTF8toANSI(pItem->pszText);
		aItem.pszText = pszaText;
	}
	
	LRESULT lRet = SendMessageA(hWnd, LVM_SETITEMA, 0, (LPARAM)&aItem);
	free(pszaText);
	return lRet;
}


int WINAPI w32u_commctrlA_init(void)
{
	// TODO: Error handling.
	
	pTabCtrl_InsertItemU	= &TabCtrl_InsertItemUA;
	
	pListView_GetItemU	= &ListView_GetItemUA;
	pListView_InsertColumnU	= &ListView_InsertColumnUA;
	pListView_InsertItemU	= &ListView_InsertItemUA;
	pListView_SetItemU	= &ListView_SetItemUA;
	
	return 0;
}

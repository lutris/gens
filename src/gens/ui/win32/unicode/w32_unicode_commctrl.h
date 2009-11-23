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

#ifndef GENS_W32_UNICODE_COMMCTRL_H
#define GENS_W32_UNICODE_COMMCTRL_H

#include "w32_unicode.h"
#include <commctrl.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TabCtrl macros. **/
int TabCtrl_InsertItemU(HWND hWnd, int iItem, const LPTCITEM pItem);

/** ListView macros. **/
#define ListView_DeleteAllItemsU(w)		(BOOL)pSendMessageU((w),LVM_DELETEALLITEMS,0,0)
#define ListView_GetNextItemU(w,i,f)		(int)pSendMessageU((w),LVM_GETNEXTITEM,i,MAKELPARAM((f),0))
int ListView_GetItemU(HWND hWnd, LVITEM *pItem);
int ListView_InsertColumnU(HWND hWnd, int iCol, const LV_COLUMN *pCol);
int ListView_InsertItemU(HWND hWnd, const LVITEM *pItem);
#define ListView_SetExtendedListViewStyleU(w,s)	(DWORD)pSendMessageU((w),LVM_SETEXTENDEDLISTVIEWSTYLE,0,(s))
#define ListView_SetImageListU(w,h,i)		(HIMAGELIST)(UINT)pSendMessageU((w),LVM_SETIMAGELIST,(i),(LPARAM)(h))
int ListView_SetItemU(HWND hWnd, const LVITEM *pItem);

#ifdef __cplusplus
}
#endif

#endif /* GENS_W32_UNICODE_COMMCTRL_H */

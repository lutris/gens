/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_commctrl.h: commctrl.h translation.                                *
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

#ifndef GSFT_W32U_COMMCTRL_H
#define GSFT_W32U_COMMCTRL_H

#include "w32u.h"
#include <commctrl.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TabCtrl macros. **/
int WINAPI TabCtrl_InsertItemU(HWND hWnd, int iItem, const LPTCITEM pItem);

/** ListView macros. **/
#define ListView_DeleteAllItemsU(w)		(BOOL)pSendMessageU((w),LVM_DELETEALLITEMS,0,0)
#define ListView_GetNextItemU(w,i,f)		(int)pSendMessageU((w),LVM_GETNEXTITEM,i,MAKELPARAM((f),0))
int WINAPI ListView_GetItemU(HWND hWnd, LVITEM *pItem);
int WINAPI ListView_InsertColumnU(HWND hWnd, int iCol, const LV_COLUMN *pCol);
int WINAPI ListView_InsertItemU(HWND hWnd, const LVITEM *pItem);
#define ListView_SetExtendedListViewStyleU(w,s)	(DWORD)pSendMessageU((w),LVM_SETEXTENDEDLISTVIEWSTYLE,0,(s))
#define ListView_SetImageListU(w,h,i)		(HIMAGELIST)(UINT)pSendMessageU((w),LVM_SETIMAGELIST,(i),(LPARAM)(h))
int WINAPI ListView_SetItemU(HWND hWnd, const LVITEM *pItem);

#ifdef __cplusplus
}
#endif

#endif /* GSFT_W32U_COMMCTRL_H */

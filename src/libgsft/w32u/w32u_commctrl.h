/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_commctrl.h: commctrl.h translation. (common code)                  *
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
#include "w32u_windows.h"
#include <commctrl.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TabCtrl macros. **/
#define TabCtrl_AdjustRectU(w,b,p)		(int)pSendMessageU((w),TCM_ADJUSTRECT,(b),(LPARAM)(LPRECT)(p))
#define TabCtrl_GetCurSelU(w)			(int)pSendMessageU((w),TCM_GETCURSEL,0,0)
extern int (WINAPI *pTabCtrl_InsertItemU)(HWND hWnd, int iItem, const TCITEMA *pItem);

/** ListView macros. **/
#define ListView_DeleteAllItemsU(w)		(BOOL)pSendMessageU((w),LVM_DELETEALLITEMS,0,0)
#define ListView_DeleteItemU(w,i)		(BOOL)pSendMessageU((w),LVM_DELETEITEM,i,0)
#define ListView_GetCheckStateU(w,i)		((((UINT)(pSendMessageU((w),LVM_GETITEMSTATE,(WPARAM)(i),LVIS_STATEIMAGEMASK)))>>12)-1)
#define ListView_GetItemCountU(w)		(int)pSendMessageU((w),LVM_GETITEMCOUNT,0,0)
#define ListView_GetNextItemU(w,i,f)		(int)pSendMessageU((w),LVM_GETNEXTITEM,i,MAKELPARAM((f),0))
extern int (WINAPI *pListView_GetItemU)(HWND hWnd, LVITEMA *pItem);
extern int (WINAPI *pListView_InsertColumnU)(HWND HWnd, int iCol, const LV_COLUMNA *pCol);
extern int (WINAPI *pListView_InsertItemU)(HWND hWnd, const LVITEMA *pItem);
#define ListView_SetCheckStateU(w,i,f)		ListView_SetItemStateU(w,i,INDEXTOSTATEIMAGEMASK((f)+1),LVIS_STATEIMAGEMASK)
#define ListView_SetExtendedListViewStyleU(w,s)	(DWORD)pSendMessageU((w),LVM_SETEXTENDEDLISTVIEWSTYLE,0,(s))
#define ListView_SetImageListU(w,h,i)		(HIMAGELIST)(UINT)pSendMessageU((w),LVM_SETIMAGELIST,(i),(LPARAM)(h))
extern int (WINAPI *pListView_SetItemU)(HWND hWnd, const LVITEMA *pItem);

#define ListView_SetItemStateU(w,i,d,m) \
{ \
	LV_ITEM _lvi;\
	_lvi.stateMask=m;\
	_lvi.state=d;\
	pSendMessageU((w),LVM_SETITEMSTATE,i,(LPARAM)(LV_ITEM*)&_lvi);\
}

void WINAPI w32u_commctrl_init(void);

#ifdef __cplusplus
}
#endif

#endif /* GSFT_W32U_COMMCTRL_H */

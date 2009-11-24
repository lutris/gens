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

#ifndef GENS_W32_UNICODE_X_H
#define GENS_W32_UNICODE_X_H

#include "w32_unicode.h"
#include <windowsx.h>
#include <commctrl.h>

#ifdef __cplusplus
extern "C" {
#endif

/** SetWindowText() macros. **/
#define Static_SetTextU(hwndCtl,lpsz)	pSetWindowTextU((hwndCtl),(lpsz))

/** Button macros. **/
#define Button_GetCheckU(hwndCtl)	((int)(DWORD)pSendMessageU((hwndCtl),BM_GETCHECK,0,0))
#define Button_SetCheckU(hwndCtl,check)	((void)pSendMessageU((hwndCtl),BM_SETCHECK,(WPARAM)(int)(check),0))
#define Button_SetTextU(hwndCtl,lpsz)	pSetWindowTextU((hwndCtl),(lpsz))

/** ComboBox macros. **/
int WINAPI ComboBox_AddStringU(HWND hwndCtl, LPCSTR lpsz);
#define ComboBox_GetCurSelU(hwndCtl)		((int)(DWORD)pSendMessageU((hwndCtl),CB_GETCURSEL,0,0))
#define ComboBox_ResetContentU(hwndCtl)		((int)(DWORD)pSendMessageU((hwndCtl),CB_RESETCONTENT,0,0))
#define ComboBox_SetCurSelU(hwndCtl,index)	((int)(DWORD)pSendMessageU((hwndCtl),CB_SETCURSEL,(WPARAM)(int)(index),0))
#define ComboBox_SetTextU(hwndCtl,lpsz)		pSetWindowTextU((hwndCtl),(lpsz))

/** Edit macros. **/
#define Edit_SetReadOnlyU(hwndCtl,fReadOnly)	((BOOL)(DWORD)pSendMessageU((hwndCtl),EM_SETREADONLY,(WPARAM)(BOOL)(fReadOnly),0))
#define Edit_SetTextU(hwndCtl,lpsz)		pSetWindowTextU((hwndCtl),(lpsz))

/** ListBox macros. **/
#define ListBox_DeleteStringU(hwndCtl,index)		((int)(DWORD)pSendMessageU((hwndCtl),LB_DELETESTRING,(WPARAM)(int)(index),0))
#define ListBox_GetCountU(hwndCtl)			((int)(DWORD)pSendMessageU((hwndCtl),LB_GETCOUNT,0,0))
#define ListBox_GetCurSelU(hwndCtl)			((int)(DWORD)pSendMessageU((hwndCtl),LB_GETCURSEL,0,0))
#define ListBox_GetItemDataU(hwndCtl,index)		((LRESULT)(DWORD)pSendMessageU((hwndCtl),LB_GETITEMDATA,(WPARAM)(int)(index),0))
int WINAPI ListBox_GetTextU(HWND hwndCtl, int index, LPSTR lpszBuffer);
#define ListBox_GetTextLenU(hwndCtl,index)		((int)(DWORD)pSendMessageU((hwndCtl),LB_GETTEXTLEN,(WPARAM)(int)(index),0))
int WINAPI ListBox_InsertStringU(HWND hwndCtl, int index, LPCSTR lpsz);
#define ListBox_ResetContentU(hwndCtl)			((BOOL)(DWORD)pSendMessageU((hwndCtl),LB_RESETCONTENT,0,0))
#define ListBox_SetCurSelU(hwndCtl,index)		((int)(DWORD)pSendMessageU((hwndCtl),LB_SETCURSEL,(WPARAM)(int)(index),0))
#define ListBox_SetItemDataU(hwndCtl,index,data)	((int)(DWORD)pSendMessageU((hwndCtl),LB_SETITEMDATA,(WPARAM)(int)(index),(LPARAM)(data)))

#ifdef __cplusplus
}
#endif

#endif /* GENS_W32_UNICODE_X_H */

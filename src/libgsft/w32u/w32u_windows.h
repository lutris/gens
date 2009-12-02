/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_windows.h: windows.h translation. (common code)                    *
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

#ifndef GSFT_W32U_WINDOWS_H
#define GSFT_W32U_WINDOWS_H

#include "w32u.h"

#ifdef __cplusplus
extern "C" {
#endif

/** kernel32.dll **/
MAKE_EXTFUNCPTR2(GetModuleFileNameA,	GetModuleFileNameU);
MAKE_EXTFUNCPTR2(GetModuleHandleA,	GetModuleHandleU);
MAKE_EXTFUNCPTR2(SetCurrentDirectoryA,	SetCurrentDirectoryU);
MAKE_EXTFUNCPTR2(GetVersionExA,		GetVersionExU);

/** user32.dll **/
MAKE_EXTFUNCPTR2(RegisterClassA,	RegisterClassU);
MAKE_EXTFUNCPTR2(CreateWindowExA,	CreateWindowExU);
#define pCreateWindowU(a,b,c,d,e,f,g,h,i,j,k) pCreateWindowExU(0,a,b,c,d,e,f,g,h,i,j,k)
MAKE_EXTFUNCPTR2(SetWindowTextA,	SetWindowTextU);
MAKE_EXTFUNCPTR2(GetWindowTextA,	GetWindowTextU);
MAKE_EXTFUNCPTR2(InsertMenuA,		InsertMenuU);
MAKE_EXTFUNCPTR2(ModifyMenuA,		ModifyMenuU);
MAKE_EXTFUNCPTR2(LoadAcceleratorsA,	LoadAcceleratorsU);
MAKE_EXTFUNCPTR2(LoadBitmapA,		LoadBitmapU);
MAKE_EXTFUNCPTR2(LoadCursorA,		LoadCursorU);
MAKE_EXTFUNCPTR2(LoadIconA,		LoadIconU);
MAKE_EXTFUNCPTR2(LoadImageA,		LoadImageU);
MAKE_EXTFUNCPTR2(MessageBoxA,		MessageBoxU);

/**
 * These functions don't need reimplementation (no string processing),
 * but they have separate A/W versions.
 */
MAKE_EXTFUNCPTR2(DefWindowProcA,	DefWindowProcU);
MAKE_EXTFUNCPTR2(CallWindowProcA,	CallWindowProcU);

MAKE_EXTFUNCPTR2(SendMessageA,		SendMessageU);
MAKE_EXTFUNCPTR2(GetMessageA,		GetMessageU);
MAKE_EXTFUNCPTR2(PeekMessageA,		PeekMessageU);

typedef WINUSERAPI LRESULT WINAPI (*SNDMSGU_LPCSTR)(HWND hWnd, UINT msgA, UINT msgW, WPARAM wParam, LPARAM lParam);
extern SNDMSGU_LPCSTR pSendMessageU_LPCSTR;

MAKE_EXTFUNCPTR2(CreateAcceleratorTableA,	CreateAcceleratorTableU);
MAKE_EXTFUNCPTR2(TranslateAcceleratorA,		TranslateAcceleratorU);

MAKE_EXTFUNCPTR2(IsDialogMessageA,	IsDialogMessageU);
MAKE_EXTFUNCPTR2(DispatchMessageA,	DispatchMessageU);

#ifdef _WIN64
MAKE_EXTFUNCPTR2(GetWindowLongPtrA,	GetWindowLongPtrU);
MAKE_EXTFUNCPTR2(SetWindowLongPtrA,	SetWindowLongPtrU);
#else
MAKE_EXTFUNCPTR2(GetWindowLongA,	GetWindowLongU);
#define pGetWindowLongPtrU pGetWindowLongU
MAKE_EXTFUNCPTR2(SetWindowLongA,	SetWindowLongU);
#define pSetWindowLongPtrU pSetWindowLongU
#endif

/** WARNING: GCL_MENUNAME requires manual conversion! **/
#ifdef _WIN64
MAKE_EXTFUNCPTR2(GetClassLongPtrA,	GetClassLongPtrU);
MAKE_EXTFUNCPTR2(SetClassLongPtrA,	SetClassLongPtrU);
#else
MAKE_EXTFUNCPTR2(GetClassLongA,		GetClassLongU);
#define pGetClassLongPtrU pGetClassLongU
MAKE_EXTFUNCPTR2(SetClassLongA,		SetClassLongU);
#define pSetClassLongPtrU pSetClassLongU
#endif

MAKE_EXTFUNCPTR2(GetWindowTextLengthA,	GetWindowTextLengthU);

int WINAPI w32u_windows_init(void);
int WINAPI w32u_windows_end(void);

#ifdef __cplusplus
}
#endif

#endif /* GSFT_W32U_WINDOWS_H */

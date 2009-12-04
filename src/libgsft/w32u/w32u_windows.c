/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_windows.c: windows.h translation. (common code)                    *
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

#include "w32u_windows.h"
#include "W/w32u_windowsW.h"
#include "A/w32u_windowsA.h"


/** kernel32.dll **/
MAKE_FUNCPTR2(GetModuleFileNameA,	GetModuleFileNameU);
MAKE_FUNCPTR2(GetModuleHandleA,		GetModuleHandleU);
MAKE_FUNCPTR2(SetCurrentDirectoryA,	SetCurrentDirectoryU);
MAKE_FUNCPTR2(GetVersionExA,		GetVersionExU);

/** user32.dll **/
MAKE_FUNCPTR2(RegisterClassA,		RegisterClassU);
MAKE_FUNCPTR2(CreateWindowExA,		CreateWindowExU);
#define pCreateWindowU(a,b,c,d,e,f,g,h,i,j,k) pCreateWindowExU(0,a,b,c,d,e,f,g,h,i,j,k)
MAKE_FUNCPTR2(SetWindowTextA,		SetWindowTextU);
MAKE_FUNCPTR2(GetWindowTextA,		GetWindowTextU);
MAKE_FUNCPTR2(InsertMenuA,		InsertMenuU);
MAKE_FUNCPTR2(ModifyMenuA,		ModifyMenuU);
MAKE_FUNCPTR2(LoadAcceleratorsA,	LoadAcceleratorsU);
MAKE_FUNCPTR2(LoadBitmapA,		LoadBitmapU);
MAKE_FUNCPTR2(LoadCursorA,		LoadCursorU);
MAKE_FUNCPTR2(LoadIconA,		LoadIconU);
MAKE_FUNCPTR2(LoadImageA,		LoadImageU);
MAKE_FUNCPTR2(MessageBoxA,		MessageBoxU);

/**
 * These functions don't need reimplementation (no string processing),
 * but they have separate A/W versions.
 */
MAKE_FUNCPTR2(DefWindowProcA,		DefWindowProcU);
MAKE_FUNCPTR2(CallWindowProcA,		CallWindowProcU);

MAKE_FUNCPTR2(SendMessageA,		SendMessageU);
MAKE_FUNCPTR2(GetMessageA,		GetMessageU);
MAKE_FUNCPTR2(PeekMessageA,		PeekMessageU);
SNDMSGU_LPCSTR pSendMessageU_LPCSTR = NULL;

MAKE_FUNCPTR2(CreateAcceleratorTableA,	CreateAcceleratorTableU);
MAKE_FUNCPTR2(TranslateAcceleratorA,	TranslateAcceleratorU);

MAKE_FUNCPTR2(IsDialogMessageA,		IsDialogMessageU);
MAKE_FUNCPTR2(DispatchMessageA,		DispatchMessageU);

#ifdef _WIN64
MAKE_FUNCPTR2(GetWindowLongPtrA,	GetWindowLongPtrU);
MAKE_FUNCPTR2(SetWindowLongPtrA,	SetWindowLongPtrU);
#else
MAKE_FUNCPTR2(GetWindowLongA,		GetWindowLongU);
#define pGetWindowLongPtrU pGetWindowLongU
MAKE_FUNCPTR2(SetWindowLongA,		SetWindowLongU);
#define pSetWindowLongPtrU pSetWindowLongU
#endif

/** WARNING: GCL_MENUNAME requires manual conversion! **/
#ifdef _WIN64
MAKE_FUNCPTR2(GetClassLongPtrA,		GetClassLongPtrU);
MAKE_FUNCPTR2(SetClassLongPtrA,		SetClassLongPtrU);
#else
MAKE_FUNCPTR2(GetClassLongA,		GetClassLongU);
#define pGetClassLongPtrU pGetClassLongU
MAKE_FUNCPTR2(SetClassLongA,		SetClassLongU);
#define pSetClassLongPtrU pSetClassLongU
#endif

MAKE_FUNCPTR2(GetWindowTextLengthA,	GetWindowTextLengthU);


int WINAPI w32u_windows_init(void)
{
	if (w32u_is_unicode)
		return w32u_windowsW_init();
	else
		return w32u_windowsA_init();
}

/***************************************************************************
 * Gens: (Win32) Unicode Translation Layer.                                *
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

#ifndef GENS_W32_UNICODE_H
#define GENS_W32_UNICODE_H

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAKE_EXTFUNCPTR(f) extern typeof(f) * p##f

MAKE_EXTFUNCPTR(RegisterClassA);
#define pRegisterClass pRegisterClassA
#define pRegisterClassU pRegisterClassA

MAKE_EXTFUNCPTR(CreateWindowExA);
#define pCreateWindowEx pCreateWindowExA
#define pCreateWindowExU pCreateWindowExA
#define pCreateWindowA(a,b,c,d,e,f,g,h,i,j,k) pCreateWindowExU(0,a,b,c,d,e,f,g,h,i,j,k)
#define pCreateWindow pCreateWindowA
#define pCreateWindowU pCreateWindowA

MAKE_EXTFUNCPTR(SetWindowTextA);
#define pSetWindowText pSetWindowTextA
#define pSetWindowTextU pSetWindowTextA

MAKE_EXTFUNCPTR(InsertMenuA);
#define pInsertMenuU pInsertMenuA

MAKE_EXTFUNCPTR(ModifyMenuA);
#define pModifyMenuU pModifyMenuA

MAKE_EXTFUNCPTR(SetCurrentDirectoryA);
#define pSetCurrentDirectoryU pSetCurrentDirectoryA

/**
 * These functions don't need reimplementation (no string processing),
 * but they have separate A/W versions.
 */
MAKE_EXTFUNCPTR(DefWindowProcA);
#define pDefWindowProc pDefWindowProcA
#define pDefWindowProcU pDefWindowProcA

MAKE_EXTFUNCPTR(CallWindowProcA);
#define pCallWindowProc pCallWindowProcA
#define pCallWindowProcU pCallWindowProcA

MAKE_EXTFUNCPTR(SendMessageA);
extern int isSendMessageUnicode;
#define pSendMessage pSendMessageA
#define pSendMessageU pSendMessageA

#ifdef _WIN64
MAKE_EXTFUNCPTR(GetWindowLongPtrA);
#define pGetWindowLongPtrU pGetWindowLongPtrA
MAKE_EXTFUNCPTR(SetWindowLongPtrA);
#define pSetWindowLongPtrU pSetWindowLongPtrA
#else
MAKE_EXTFUNCPTR(GetWindowLongA);
#define pGetWindowLongU pGetWindowLongA
#define pGetWindowLongPtrU pGetWindowLongA
MAKE_EXTFUNCPTR(SetWindowLongA);
#define pSetWindowLongU pSetWindowLongA
#define pSetWindowLongPtrU pSetWindowLongA
#endif

MAKE_EXTFUNCPTR(CreateAcceleratorTableA);
#define pCreateAcceleratorTableU pCreateAcceleratorTableA

int w32_unicode_init(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_W32_UNICODE_H */

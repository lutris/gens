/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_windowsA.c: windows.h (ANSI version)                               *
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

#include "w32u_windowsA.h"
#include "w32u_priv.h"

#include "w32u_windows.h"

// C includes.
#include <stdlib.h>

// Initialization counter.
static int init_counter = 0;


/** kernel32.dll **/


MAKE_STFUNCPTR(GetModuleFileNameA);
static WINBASEAPI DWORD WINAPI GetModuleFileNameUA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	// TODO: ANSI conversion.
	return pGetModuleFileNameA(hModule, lpFilename, nSize);
}


MAKE_STFUNCPTR(GetModuleHandleA);
static WINBASEAPI HMODULE WINAPI GetModuleHandleUA(LPCSTR lpModuleName)
{
	// TODO: ANSI conversion.
	return pGetModuleHandleA(lpModuleName);
}


MAKE_STFUNCPTR(SetCurrentDirectoryA);
static WINUSERAPI BOOL WINAPI SetCurrentDirectoryUA(LPCSTR lpPathName)
{
	// TODO: ANSI conversion.
	return pSetCurrentDirectoryA(lpPathName);
}


MAKE_STFUNCPTR(GetVersionExA);
static WINBASEAPI BOOL WINAPI GetVersionExUA(LPOSVERSIONINFOA lpVersionInfo)
{
	// TODO: ANSI conversion.
	return pGetVersionExA(lpVersionInfo);
}


/** user32.dll **/


MAKE_STFUNCPTR(RegisterClassA);
static WINUSERAPI ATOM WINAPI RegisterClassUA(CONST WNDCLASSA* lpWndClass)
{
	// TODO: ANSI conversion.
	return pRegisterClassA(lpWndClass);
}


MAKE_STFUNCPTR(CreateWindowExA);
static WINUSERAPI HWND WINAPI CreateWindowExUA(
			DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
			DWORD dwStyle, int x, int y, int nWidth, int nHeight,
			HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	// TODO: ANSI conversion.
	return pCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle,
				x, y, nWidth, nHeight,
				hWndParent, hMenu, hInstance, lpParam);
}


MAKE_STFUNCPTR(SetWindowTextA);
static WINUSERAPI BOOL WINAPI SetWindowTextUA(HWND hWnd, LPCSTR lpString)
{
	// TODO: ANSI conversion.
	return pSetWindowTextA(hWnd, lpString);
}


MAKE_STFUNCPTR(GetWindowTextA);
static WINUSERAPI int WINAPI GetWindowTextUA(HWND hWnd, LPSTR lpString, int nMaxCount)
{
	// TODO: ANSI conversion.
	return pGetWindowTextA(hWnd, lpString, nMaxCount);
}


MAKE_STFUNCPTR(InsertMenuA);
static WINUSERAPI BOOL WINAPI InsertMenuUA(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
{
	// TODO: ANSI conversion.
	return pInsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);
}


MAKE_STFUNCPTR(ModifyMenuA);
static WINUSERAPI BOOL WINAPI ModifyMenuUA(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
{
	// TODO: ANSI conversion.
	return pModifyMenuA(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);
}


MAKE_STFUNCPTR(LoadAcceleratorsA);
static WINUSERAPI HACCEL WINAPI LoadAcceleratorsUA(HINSTANCE hInstance, LPCSTR lpTableName)
{
	// TODO: ANSI conversion.
	return pLoadAcceleratorsA(hInstance, lpTableName);
}


MAKE_STFUNCPTR(LoadBitmapA);
static WINUSERAPI HBITMAP WINAPI LoadBitmapUA(HINSTANCE hInstance, LPCSTR lpBitmapName)
{
	// TODO: ANSI conversion.
	return pLoadBitmapA(hInstance, lpBitmapName);
}


MAKE_STFUNCPTR(LoadCursorA);
static WINUSERAPI HCURSOR WINAPI LoadCursorUA(HINSTANCE hInstance, LPCSTR lpCursorName)
{
	// TODO: ANSI conversion.
	return pLoadCursorA(hInstance, lpCursorName);
}


MAKE_STFUNCPTR(LoadIconA);
static WINUSERAPI HICON WINAPI LoadIconUA(HINSTANCE hInstance, LPCSTR lpIconName)
{
	// TODO: ANSI conversion.
	return pLoadIconA(hInstance, lpIconName);
}


MAKE_STFUNCPTR(LoadImageA);
static WINUSERAPI HANDLE WINAPI LoadImageUA(HINSTANCE hInst, LPCSTR lpszName, UINT uType,
						int cxDesired, int cyDesired, UINT fuLoad)
{
	// TODO: ANSI conversion.
	return pLoadImageA(hInst, lpszName, uType, cxDesired, cyDesired, fuLoad);
}


MAKE_STFUNCPTR(MessageBoxA);
static WINUSERAPI int WINAPI MessageBoxUA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	// TODO: ANSI conversion.
	return pMessageBoxA(hWnd, lpText, lpCaption, uType);
}


#define InitFuncPtrUA(hDll, fnU, fnA) \
do { \
	p##fnA = (typeof(p##fnA))GetProcAddress(hDll, #fnA); \
	p##fnU = &(fnU##A); \
} while (0)

#define InitFuncPtrU(hDll, fnU, fn) \
do { \
	p##fnU = (typeof(p##fnU))GetProcAddress(hDll, #fn); \
} while (0)


int WINAPI w32u_windowsA_init(HMODULE hKernel32, HMODULE hUser32)
{
	if (init_counter++ != 0)
		return 0;
	
	// TODO: Error handling.
	InitFuncPtrUA(hKernel32, GetModuleFileNameU, GetModuleFileNameA);
	InitFuncPtrUA(hKernel32, GetModuleHandleU, GetModuleHandleA);
	InitFuncPtrUA(hKernel32, SetCurrentDirectoryU, SetCurrentDirectoryA);
	InitFuncPtrUA(hKernel32, GetVersionExU, GetVersionExA);
	
	InitFuncPtrUA(hUser32, RegisterClassU, RegisterClassA);
	InitFuncPtrUA(hUser32, CreateWindowExU, CreateWindowExA);
	InitFuncPtrUA(hUser32, SetWindowTextU, SetWindowTextA);
	InitFuncPtrUA(hUser32, GetWindowTextU, GetWindowTextA);
	InitFuncPtrUA(hUser32, InsertMenuU, InsertMenuA);
	InitFuncPtrUA(hUser32, ModifyMenuU, ModifyMenuA);
	InitFuncPtrUA(hUser32, LoadAcceleratorsU, LoadAcceleratorsA);
	InitFuncPtrUA(hUser32, LoadBitmapU, LoadBitmapA);
	InitFuncPtrUA(hUser32, LoadCursorU, LoadCursorA);
	InitFuncPtrUA(hUser32, LoadIconU, LoadIconA);
	InitFuncPtrUA(hUser32, LoadImageU, LoadImageA);
	InitFuncPtrUA(hUser32, MessageBoxU, MessageBoxA);
	
	InitFuncPtrU(hUser32, DefWindowProcU, DefWindowProcA);
	InitFuncPtrU(hUser32, CallWindowProcU, CallWindowProcA);
	
	InitFuncPtrU(hUser32, SendMessageU, SendMessageA);
	InitFuncPtrU(hUser32, GetMessageU, GetMessageA);
	InitFuncPtrU(hUser32, PeekMessageU, PeekMessageA);
	
	InitFuncPtrU(hUser32, CreateAcceleratorTableU, CreateAcceleratorTableA);
	InitFuncPtrU(hUser32, TranslateAcceleratorU, TranslateAcceleratorA);
	
	InitFuncPtrU(hUser32, IsDialogMessageU, IsDialogMessageA);
	InitFuncPtrU(hUser32, DispatchMessageU, DispatchMessageA);
	
	InitFuncPtrU(hUser32, GetWindowTextLengthU, GetWindowTextLengthA);
	
#ifdef _WIN64
	InitFuncPtrU(hUser32, GetWindowLongPtrU, GetWindowLongPtrA);
	InitFuncPtrU(hUser32, SetWindowLongPtrU, SetWindowLongPtrA);
#else
	InitFuncPtrU(hUser32, GetWindowLongU, GetWindowLongA);
	InitFuncPtrU(hUser32, SetWindowLongU, SetWindowLongA);
#endif
	
#ifdef _WIN64
	InitFuncPtrU(hUser32, GetClassLongPtrU, GetClassLongPtrA);
	InitFuncPtrU(hUser32, SetClassLongPtrU, SetClassLongPtrA);
#else
	InitFuncPtrU(hUser32, GetClassLongU, GetClassLongA);
	InitFuncPtrU(hUser32, SetClassLongU, SetClassLongA);
#endif
	
	return 0;
}

int WINAPI w32u_windowsA_end(void)
{
	if (init_counter <= 0)
		return 0;
	
	init_counter--;
	if (init_counter > 0)
		return 0;
	
	// TODO: Should the function pointers be NULL'd?
	return 0;
}

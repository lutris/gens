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

#include "w32_unicode.h"
#include "w32_unicode_priv.h"
#include "w32_unicode_shellapi.h"
#include "w32_unicode_libc.h"

// C includes.
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

// DLLs.
static HMODULE hUser32 = NULL;


MAKE_FUNCPTR(RegisterClassA);
MAKE_STFUNCPTR(RegisterClassW);
static WINUSERAPI ATOM WINAPI RegisterClassU(CONST WNDCLASSA* lpWndClass)
{
	// Convert lpWndClass from WNDCLASSA to WNDCLASSW.
	WNDCLASSW wWndClass;
	memcpy(&wWndClass, lpWndClass, sizeof(wWndClass));
	
	// Convert the ANSI strings to Unicode.
	int lpszwMenuName_len, lpszwClassName_len;
	wchar_t *lpszwMenuName = NULL, *lpszwClassName = NULL;
	
	if (lpWndClass->lpszMenuName)
	{
		lpszwMenuName_len = MultiByteToWideChar(CP_UTF8, 0, lpWndClass->lpszMenuName, -1, NULL, 0);
		lpszwMenuName_len *= sizeof(wchar_t);
		lpszwMenuName = (wchar_t*)malloc(lpszwMenuName_len);
		MultiByteToWideChar(CP_UTF8, 0, lpWndClass->lpszMenuName, -1, lpszwMenuName, lpszwMenuName_len);
		wWndClass.lpszMenuName = lpszwMenuName;
	}
	
	if (lpWndClass->lpszClassName)
	{
		lpszwClassName_len = MultiByteToWideChar(CP_UTF8, 0, lpWndClass->lpszClassName, -1, NULL, 0);
		lpszwClassName_len *= sizeof(wchar_t);
		lpszwClassName = (wchar_t*)malloc(lpszwClassName_len);
		MultiByteToWideChar(CP_UTF8, 0, lpWndClass->lpszClassName, -1, lpszwClassName, lpszwClassName_len);
		wWndClass.lpszClassName = lpszwClassName;
	}
	
	ATOM aRet = pRegisterClassW(&wWndClass);
	
	free(lpszwMenuName);
	free(lpszwClassName);
	
	return aRet;
}


MAKE_FUNCPTR(CreateWindowExA);
MAKE_STFUNCPTR(CreateWindowExW);
static WINUSERAPI HWND WINAPI CreateWindowExU(
		DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
		DWORD dwStyle, int x, int y, int nWidth, int nHeight,
		HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	// Convert lpClassName and lpWindowName from UTF-8 to UTF-16.
	int lpwClassName_len, lpwWindowName_len;
	wchar_t *lpwClassName = NULL, *lpwWindowName = NULL;
	
	if (lpClassName)
	{
		lpwClassName_len = MultiByteToWideChar(CP_UTF8, 0, lpClassName, -1, NULL, 0);
		lpwClassName_len *= sizeof(wchar_t);
		lpwClassName = (wchar_t*)malloc(lpwClassName_len);
		MultiByteToWideChar(CP_UTF8, 0, lpClassName, -1, lpwClassName, lpwClassName_len);
	}
	
	if (lpWindowName)
	{
		lpwWindowName_len = MultiByteToWideChar(CP_UTF8, 0, lpWindowName, -1, NULL, 0);
		lpwWindowName_len *= sizeof(wchar_t);
		lpwWindowName = (wchar_t*)malloc(lpwWindowName_len);
		MultiByteToWideChar(CP_UTF8, 0, lpWindowName, -1, lpwWindowName, lpwWindowName_len);
	}
	
	HWND hRet = pCreateWindowExW(dwExStyle, lpwClassName, lpwWindowName,
					dwStyle, x, y, nWidth, nHeight,
					hWndParent, hMenu, hInstance, lpParam);
	
	free(lpwClassName);
	free(lpwWindowName);
	
	return hRet;
}


MAKE_FUNCPTR(SetWindowTextA);
MAKE_STFUNCPTR(SetWindowTextW);
static WINUSERAPI BOOL WINAPI SetWindowTextU(HWND hWnd, LPCSTR lpString)
{
	// Convert lpString from UTF-8 to UTF-16.
	int lpwString_len;
	wchar_t *lpwString = NULL;
	
	if (lpString)
	{
		lpwString_len = MultiByteToWideChar(CP_UTF8, 0, lpString, -1, NULL, 0);
		lpwString_len *= sizeof(wchar_t);
		lpwString = (wchar_t*)malloc(lpwString_len);
		MultiByteToWideChar(CP_UTF8, 0, lpString, -1, lpwString, lpwString_len);
	}
	
	BOOL bRet = pSetWindowTextW(hWnd, lpwString);
	
	free(lpwString);
	
	return bRet;
}


MAKE_FUNCPTR(InsertMenuA);
MAKE_STFUNCPTR(InsertMenuW);
static WINUSERAPI BOOL WINAPI InsertMenuU(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
{
	if ((uFlags & (MF_BITMAP | MF_OWNERDRAW)) || !lpNewItem)
	{
		// String not specified. Don't bother converting anything.
		return pInsertMenuW(hMenu, uPosition, uFlags, uIDNewItem, (LPCWSTR)lpNewItem);
	}
	
	// Convert lpNewItem from UTF-8 to UTF-16.
	int lpNewItem_len;
	wchar_t *lpwNewItem = NULL;
	
	lpNewItem_len = MultiByteToWideChar(CP_UTF8, 0, lpNewItem, -1, NULL, 0);
	lpNewItem_len *= sizeof(wchar_t);
	lpwNewItem = (wchar_t*)malloc(lpNewItem_len);
	MultiByteToWideChar(CP_UTF8, 0, lpNewItem, -1, lpwNewItem, lpNewItem_len);
	
	BOOL bRet = pInsertMenuW(hMenu, uPosition, uFlags, uIDNewItem, lpwNewItem);
	free(lpwNewItem);
	return bRet;
}


MAKE_FUNCPTR(ModifyMenuA);
MAKE_STFUNCPTR(ModifyMenuW);
static WINUSERAPI BOOL WINAPI ModifyMenuU(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
{
	if ((uFlags & (MF_BITMAP | MF_OWNERDRAW)) || !lpNewItem)
	{
		// String not specified. Don't bother converting anything.
		return pModifyMenuW(hMenu, uPosition, uFlags, uIDNewItem, (LPCWSTR)lpNewItem);
	}
	
	// Convert lpNewItem from UTF-8 to UTF-16.
	int lpNewItem_len;
	wchar_t *lpwNewItem = NULL;
	
	lpNewItem_len = MultiByteToWideChar(CP_UTF8, 0, lpNewItem, -1, NULL, 0);
	lpNewItem_len *= sizeof(wchar_t);
	lpwNewItem = (wchar_t*)malloc(lpNewItem_len);
	MultiByteToWideChar(CP_UTF8, 0, lpNewItem, -1, lpwNewItem, lpNewItem_len);
	
	BOOL bRet = pModifyMenuW(hMenu, uPosition, uFlags, uIDNewItem, lpwNewItem);
	free(lpwNewItem);
	return bRet;
}


/**
 * These functions don't need reimplementation (no string processing),
 * but they have separate A/W versions.
 */
MAKE_FUNCPTR(DefWindowProcA);
MAKE_FUNCPTR(CallWindowProcA);
MAKE_FUNCPTR(SendMessageA);
int isSendMessageUnicode = 0;

#ifdef _WIN64
MAKE_FUNCPTR(GetWindowLongPtrA);
MAKE_FUNCPTR(SetWindowLongPtrA);
#else
MAKE_FUNCPTR(GetWindowLongA);
MAKE_FUNCPTR(SetWindowLongA);
#endif

MAKE_FUNCPTR(CreateAcceleratorTableA);


int w32_unicode_init(void)
{
	// Initialize Win32 Unicode.
	
	// TODO: Error handling.
	hUser32 = LoadLibrary("user32.dll");
	
	InitFuncPtrsU(hUser32, "RegisterClass", pRegisterClassW, pRegisterClassA, RegisterClassU);
	InitFuncPtrsU(hUser32, "CreateWindowEx", pCreateWindowExW, pCreateWindowExA, CreateWindowExU);
	InitFuncPtrsU(hUser32, "SetWindowText", pSetWindowTextW, pSetWindowTextA, SetWindowTextU);
	InitFuncPtrsU(hUser32, "InsertMenu", pInsertMenuW, pInsertMenuA, InsertMenuU);
	InitFuncPtrsU(hUser32, "ModifyMenu", pModifyMenuW, pModifyMenuA, ModifyMenuU);
	
	InitFuncPtrs(hUser32, "DefWindowProc", pDefWindowProcA);
	InitFuncPtrs(hUser32, "CallWindowProc", pCallWindowProcA);
	InitFuncPtrs(hUser32, "SendMessage", pSendMessageA);
	
#ifdef _WIN64
	InitFuncPtrs(hUser32, "GetWindowLongPtr", pGetWindowLongPtrA);
	InitFuncPtrs(hUser32, "SetWindowLongPtr", pSetWindowLongPtrA);
#else
	InitFuncPtrs(hUser32, "GetWindowLong", pGetWindowLongA);
	InitFuncPtrs(hUser32, "SetWindowLong", pSetWindowLongA);
#endif
	
	InitFuncPtrs(hUser32, "CreateAcceleratorTable", pCreateAcceleratorTableA);
	
	// Check if SendMessage is Unicode.
	if ((void*)GetProcAddress(hUser32, "SendMessageW") == pSendMessageA)
		isSendMessageUnicode = 1;
	else
		isSendMessageUnicode = 0;
	
	// Other Win32 Unicode modules.
	w32_unicode_shellapi_init();
	w32_unicode_libc_init();
	
	return 0;
}

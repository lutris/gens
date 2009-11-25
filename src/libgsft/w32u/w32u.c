/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u.c: Main Unicode translation code.                                  *
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

#include "w32u.h"
#include "w32u_priv.h"
#include "w32u_shellapi.h"
#include "w32u_libc.h"

// C includes.
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

// DLLs.
static HMODULE hKernel32 = NULL;
MAKE_FUNCPTR(MultiByteToWideChar);
MAKE_FUNCPTR(WideCharToMultiByte);

static HMODULE hUser32 = NULL;


MAKE_FUNCPTR(GetModuleFileNameA);
MAKE_STFUNCPTR(GetModuleFileNameW);
static WINBASEAPI DWORD WINAPI GetModuleFileNameU(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	if (!lpFilename || nSize == 0)
	{
		// String not specified. Don't bother converting anything.
		return pGetModuleFileNameW(hModule, (LPWSTR)lpFilename, nSize);
	}
	
	// Allocate a buffer for the filename.
	wchar_t *lpwFilename = (wchar_t*)malloc(nSize * sizeof(wchar_t));
	DWORD dwRet = pGetModuleFileNameW(hModule, lpwFilename, nSize);
	
	// Convert the filename from UTF-16 to UTF-8.
	pWideCharToMultiByte(CP_UTF8, 0, lpwFilename, -1, lpFilename, nSize, NULL, NULL);
	free(lpwFilename);
	return dwRet;
}


MAKE_FUNCPTR(SetCurrentDirectoryA);
MAKE_STFUNCPTR(SetCurrentDirectoryW);
static WINUSERAPI BOOL WINAPI SetCurrentDirectoryU(LPCSTR lpPathName)
{
	if (!lpPathName)
	{
		// String not specified. Don't bother converting anything.
		return pSetCurrentDirectoryW((LPCWSTR)lpPathName);
	}
	
	// Convert lpPathName from UTF-8 to UTF-16.
	wchar_t *lpwPathName = w32u_mbstowcs(lpPathName);
	
	BOOL bRet = pSetCurrentDirectoryW(lpwPathName);
	free(lpwPathName);
	return bRet;
}


MAKE_FUNCPTR(RegisterClassA);
MAKE_STFUNCPTR(RegisterClassW);
static WINUSERAPI ATOM WINAPI RegisterClassU(CONST WNDCLASSA* lpWndClass)
{
	// Convert lpWndClass from WNDCLASSA to WNDCLASSW.
	WNDCLASSW wWndClass;
	memcpy(&wWndClass, lpWndClass, sizeof(wWndClass));
	
	// Convert the ANSI strings to Unicode.
	wchar_t *lpszwMenuName = NULL, *lpszwClassName = NULL;
	
	if (lpWndClass->lpszMenuName)
	{
		lpszwMenuName = w32u_mbstowcs(lpWndClass->lpszMenuName);
		wWndClass.lpszMenuName = lpszwMenuName;
	}
	
	if (lpWndClass->lpszClassName)
	{
		lpszwClassName = w32u_mbstowcs(lpWndClass->lpszClassName);
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
	wchar_t *lpwClassName = NULL, *lpwWindowName = NULL;
	
	if (lpClassName)
		lpwClassName = w32u_mbstowcs(lpClassName);
	
	if (lpWindowName)
		lpwWindowName = w32u_mbstowcs(lpWindowName);
	
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
	wchar_t *lpwString = NULL;
	
	if (lpString)
		lpwString = w32u_mbstowcs(lpString);
	
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
	wchar_t *lpwNewItem = NULL;
	
	if (lpNewItem)
		lpwNewItem = w32u_mbstowcs(lpNewItem);
	
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
	wchar_t *lpwNewItem = NULL;
	
	if (lpNewItem)
		lpwNewItem = w32u_mbstowcs(lpNewItem);
	
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


int WINAPI w32u_init(void)
{
	// Initialize Win32 Unicode.
	
	// TODO: Error handling.
	hKernel32 = LoadLibrary("kernel32.dll");
	
	// TODO: If either of these GetProcAddress()'s fails, revert to ANSI.
	InitFuncPtr(hKernel32, MultiByteToWideChar);
	InitFuncPtr(hKernel32, WideCharToMultiByte);
	
	InitFuncPtrsU(hKernel32, "GetModuleFileName", pGetModuleFileNameW, pGetModuleFileNameA, GetModuleFileNameU);
	InitFuncPtrsU(hKernel32, "SetCurrentDirectory", pSetCurrentDirectoryW, pSetCurrentDirectoryA, SetCurrentDirectoryU);
	
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
	w32u_shellapi_init();
	w32u_libc_init();
	
	return 0;
}

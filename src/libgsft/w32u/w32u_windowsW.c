/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_windows.c: windows.h (Unicode version)                             *
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

#include "w32u_windowsW.h"
#include "w32u_priv.h"

#include "w32u_windows.h"

// C includes.
#include <stdlib.h>

// Initialization counter.
static int init_counter = 0;


/** kernel32.dll **/


MAKE_STFUNCPTR(GetModuleFileNameW);
static WINBASEAPI DWORD WINAPI GetModuleFileNameUW(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
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
	WideCharToMultiByte(CP_UTF8, 0, lpwFilename, -1, lpFilename, nSize, NULL, NULL);
	free(lpwFilename);
	return dwRet;
}


MAKE_STFUNCPTR(GetModuleHandleW);
static WINBASEAPI HMODULE WINAPI GetModuleHandleUW(LPCSTR lpModuleName)
{
	if (!lpModuleName)
	{
		// String not specified. Don't bother converting anything.
		return pGetModuleHandleW((LPCWSTR)lpModuleName);
	}
	
	// Convert lpModuleName from UTF-8 to UTF-16.
	wchar_t *lpwModuleName = w32u_mbstowcs(lpModuleName);
	
	HMODULE hRet = pGetModuleHandleW(lpwModuleName);
	free(lpwModuleName);
	return hRet;
}


MAKE_STFUNCPTR(SetCurrentDirectoryW);
static WINUSERAPI BOOL WINAPI SetCurrentDirectoryUW(LPCSTR lpPathName)
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


MAKE_STFUNCPTR(GetVersionExW);
static WINBASEAPI BOOL WINAPI GetVersionExUW(LPOSVERSIONINFOA lpVersionInfo)
{
	// Get the version information.
	OSVERSIONINFOEXW wVersionInfo;
	wVersionInfo.dwOSVersionInfoSize = lpVersionInfo->dwOSVersionInfoSize + sizeof(lpVersionInfo->szCSDVersion);
	BOOL bRet = pGetVersionExW((OSVERSIONINFOW*)&wVersionInfo);
	if (bRet == 0)
		return bRet;
	
	// Copy the OSVERSIONINFO struct data.
	lpVersionInfo->dwMajorVersion = wVersionInfo.dwMajorVersion;
	lpVersionInfo->dwMinorVersion = wVersionInfo.dwMinorVersion;
	lpVersionInfo->dwBuildNumber = wVersionInfo.dwBuildNumber;
	lpVersionInfo->dwPlatformId = wVersionInfo.dwPlatformId;
	
	// Convert szCSDVersion from UTF-16 to UTF-8.
	WideCharToMultiByte(CP_UTF8, 0, wVersionInfo.szCSDVersion, 
				sizeof(wVersionInfo.szCSDVersion) / sizeof(wVersionInfo.szCSDVersion[0]),
				lpVersionInfo->szCSDVersion,
				sizeof(lpVersionInfo->szCSDVersion) / sizeof(lpVersionInfo->szCSDVersion[0]),
				NULL, NULL);
	
	if (lpVersionInfo->dwOSVersionInfoSize + sizeof(lpVersionInfo->szCSDVersion) == sizeof(OSVERSIONINFOEXW))
	{
		// OSVERSIONINFOEXW.
		LPOSVERSIONINFOEXA lpVersionInfoEx = (OSVERSIONINFOEXA*)lpVersionInfo;
		lpVersionInfoEx->wServicePackMajor = wVersionInfo.wServicePackMajor;
		lpVersionInfoEx->wServicePackMinor = wVersionInfo.wServicePackMinor;
		lpVersionInfoEx->wSuiteMask = wVersionInfo.wSuiteMask;
		lpVersionInfoEx->wProductType = wVersionInfo.wProductType;
		lpVersionInfoEx->wReserved = wVersionInfo.wReserved;
	}
	
	return bRet;
}


/** user32.dll **/


MAKE_STFUNCPTR(RegisterClassW);
static WINUSERAPI ATOM WINAPI RegisterClassUW(CONST WNDCLASSA* lpWndClass)
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


MAKE_STFUNCPTR(CreateWindowExW);
static WINUSERAPI HWND WINAPI CreateWindowExUW(
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


MAKE_STFUNCPTR(SetWindowTextW);
static WINUSERAPI BOOL WINAPI SetWindowTextUW(HWND hWnd, LPCSTR lpString)
{
	// Convert lpString from UTF-8 to UTF-16.
	wchar_t *lpwString = NULL;
	
	if (lpString)
		lpwString = w32u_mbstowcs(lpString);
	
	BOOL bRet = pSetWindowTextW(hWnd, lpwString);
	free(lpwString);
	return bRet;
}


MAKE_STFUNCPTR(GetWindowTextW);
static WINUSERAPI int WINAPI GetWindowTextUW(HWND hWnd, LPSTR lpString, int nMaxCount)
{
	if (!lpString || nMaxCount <= 0)
	{
		// No return buffer specified.
		return pGetWindowTextW(hWnd, (LPWSTR)lpString, nMaxCount);
	}
	
	// Allocate a temporary UTF-16 return buffer.
	wchar_t *lpwString = (wchar_t*)malloc(nMaxCount * sizeof(wchar_t));
	int ret = pGetWindowTextW(hWnd, lpwString, nMaxCount);
	
	// Convert the window text to UTF-8.
	WideCharToMultiByte(CP_UTF8, 0, lpwString, nMaxCount, lpString, nMaxCount, NULL, NULL);
	
	// Free the buffer.
	free(lpwString);
	return ret;
}


MAKE_STFUNCPTR(InsertMenuW);
static WINUSERAPI BOOL WINAPI InsertMenuUW(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
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


MAKE_STFUNCPTR(ModifyMenuW);
static WINUSERAPI BOOL WINAPI ModifyMenuUW(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
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


MAKE_STFUNCPTR(LoadAcceleratorsW);
static WINUSERAPI HACCEL WINAPI LoadAcceleratorsUW(HINSTANCE hInstance, LPCSTR lpTableName)
{
	if ((DWORD_PTR)lpTableName < 0x10000)
	{
		// lpTableName is a resource ID.
		return pLoadAcceleratorsW(hInstance, (LPCWSTR)lpTableName);
	}
	
	// lpTableName is a string. Convert it from UTF-8 to UTF-16.
	wchar_t *lpwTableName = w32u_mbstowcs(lpTableName);
	
	HACCEL hRet = pLoadAcceleratorsW(hInstance, lpwTableName);
	free(lpwTableName);
	return hRet;
}


MAKE_STFUNCPTR(LoadBitmapW);
static WINUSERAPI HBITMAP WINAPI LoadBitmapUW(HINSTANCE hInstance, LPCSTR lpBitmapName)
{
	if ((DWORD_PTR)lpBitmapName < 0x10000)
	{
		// lpBitmapName is a resource ID.
		return pLoadBitmapW(hInstance, (LPCWSTR)lpBitmapName);
	}
	
	// lpBitmapName is a string. Convert it from UTF-8 to UTF-16.
	wchar_t *lpwBitmapName = w32u_mbstowcs(lpBitmapName);
	
	HBITMAP hRet = pLoadBitmapW(hInstance, lpwBitmapName);
	free(lpwBitmapName);
	return hRet;
}


MAKE_STFUNCPTR(LoadCursorW);
static WINUSERAPI HCURSOR WINAPI LoadCursorUW(HINSTANCE hInstance, LPCSTR lpCursorName)
{
	if ((DWORD_PTR)lpCursorName < 0x10000)
	{
		// lpCursorName is a resource ID.
		return pLoadCursorW(hInstance, (LPCWSTR)lpCursorName);
	}
	
	// lpCursorName is a string. Convert it from UTF-8 to UTF-16.
	wchar_t *lpwCursorName = w32u_mbstowcs(lpCursorName);
	
	HCURSOR hRet = pLoadCursorW(hInstance, lpwCursorName);
	free(lpwCursorName);
	return hRet;
}


MAKE_STFUNCPTR(LoadIconW);
static WINUSERAPI HICON WINAPI LoadIconUW(HINSTANCE hInstance, LPCSTR lpIconName)
{
	if ((DWORD_PTR)lpIconName < 0x10000)
	{
		// lpIconName is a resource ID.
		return pLoadIconW(hInstance, (LPCWSTR)lpIconName);
	}
	
	// lpIconName is a string. Convert it from UTF-8 to UTF-16.
	wchar_t *lpwIconName = w32u_mbstowcs(lpIconName);
	
	HICON hRet = pLoadIconW(hInstance, lpwIconName);
	free(lpwIconName);
	return hRet;
}


MAKE_STFUNCPTR(LoadImageW);
static WINUSERAPI HANDLE WINAPI LoadImageUW(HINSTANCE hInst, LPCSTR lpszName, UINT uType,
						int cxDesired, int cyDesired, UINT fuLoad)
{
	if ((DWORD_PTR)lpszName < 0x10000)
	{
		// lpszName is a resource ID.
		return pLoadImageW(hInst, (LPCWSTR)lpszName, uType, cxDesired, cyDesired, fuLoad);
	}
	
	// lpszName is a string. Convert it from UTF-8 to UTF-16.
	wchar_t *lpszwName = w32u_mbstowcs(lpszName);
	
	HANDLE hRet = pLoadImageW(hInst, lpszwName, uType, cxDesired, cyDesired, fuLoad);
	free(lpszwName);
	return hRet;
}


MAKE_STFUNCPTR(MessageBoxW);
static WINUSERAPI int WINAPI MessageBoxUW(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	// Convert lpText and lpCaption from UTF-8 to UTF-16.
	wchar_t *lpwText = NULL, *lpwCaption = NULL;
	
	if (lpText)
		lpwText = w32u_mbstowcs(lpText);
	if (lpCaption)
		lpwCaption = w32u_mbstowcs(lpCaption);
	
	int ret = pMessageBoxW(hWnd, lpwText, lpwCaption, uType);
	free(lpwText);
	free(lpwCaption);
	return ret;
}


#define InitFuncPtrUW(hDll, fnU, fnW) \
do { \
	p##fnW = (typeof(p##fnW))GetProcAddress(hDll, #fnW); \
	p##fnU = &(fnU##W); \
} while (0)

#define InitFuncPtrU(hDll, fnU, fn) \
do { \
	p##fnU = (typeof(p##fnU))GetProcAddress(hDll, #fn); \
} while (0)


int WINAPI w32u_windowsW_init(HMODULE hKernel32, HMODULE hUser32)
{
	if (init_counter++ != 0)
		return 0;
	
	// TODO: Error handling.
	InitFuncPtrUW(hKernel32, GetModuleFileNameU, GetModuleFileNameW);
	InitFuncPtrUW(hKernel32, GetModuleHandleU, GetModuleHandleW);
	InitFuncPtrUW(hKernel32, SetCurrentDirectoryU, SetCurrentDirectoryW);
	InitFuncPtrUW(hKernel32, GetVersionExU, GetVersionExW);
	
	InitFuncPtrUW(hUser32, RegisterClassU, RegisterClassW);
	InitFuncPtrUW(hUser32, CreateWindowExU, CreateWindowExW);
	InitFuncPtrUW(hUser32, SetWindowTextU, SetWindowTextW);
	InitFuncPtrUW(hUser32, GetWindowTextU, GetWindowTextW);
	InitFuncPtrUW(hUser32, InsertMenuU, InsertMenuW);
	InitFuncPtrUW(hUser32, ModifyMenuU, ModifyMenuW);
	InitFuncPtrUW(hUser32, LoadAcceleratorsU, LoadAcceleratorsW);
	InitFuncPtrUW(hUser32, LoadBitmapU, LoadBitmapW);
	InitFuncPtrUW(hUser32, LoadCursorU, LoadCursorW);
	InitFuncPtrUW(hUser32, LoadIconU, LoadIconW);
	InitFuncPtrUW(hUser32, LoadImageU, LoadImageW);
	InitFuncPtrUW(hUser32, MessageBoxU, MessageBoxW);
	
	InitFuncPtrU(hUser32, DefWindowProcU, DefWindowProcW);
	InitFuncPtrU(hUser32, CallWindowProcU, CallWindowProcW);
	
	InitFuncPtrU(hUser32, SendMessageU, SendMessageW);
	InitFuncPtrU(hUser32, GetMessageU, GetMessageW);
	InitFuncPtrU(hUser32, PeekMessageU, PeekMessageW);
	
	InitFuncPtrU(hUser32, CreateAcceleratorTableU, CreateAcceleratorTableW);
	InitFuncPtrU(hUser32, TranslateAcceleratorU, TranslateAcceleratorW);
	
	InitFuncPtrU(hUser32, IsDialogMessageU, IsDialogMessageW);
	InitFuncPtrU(hUser32, DispatchMessageU, DispatchMessageW);
	
	InitFuncPtrU(hUser32, GetWindowTextLengthU, GetWindowTextLengthW);
	
#ifdef _WIN64
	InitFuncPtrU(hUser32, GetWindowLongPtrU, GetWindowLongPtrW);
	InitFuncPtrU(hUser32, SetWindowLongPtrU, SetWindowLongPtrW);
#else
	InitFuncPtrU(hUser32, GetWindowLongU, GetWindowLongW);
	InitFuncPtrU(hUser32, SetWindowLongU, SetWindowLongW);
#endif
	
#ifdef _WIN64
	InitFuncPtrU(hUser32, GetClassLongPtrU, GetClassLongPtrW);
	InitFuncPtrU(hUser32, SetClassLongPtrU, SetClassLongPtrW);
#else
	InitFuncPtrU(hUser32, GetClassLongU, GetClassLongW);
	InitFuncPtrU(hUser32, SetClassLongU, SetClassLongW);
#endif
	
	return 0;
}

int WINAPI w32u_windowsW_end(void)
{
	if (init_counter <= 0)
		return 0;
	
	init_counter--;
	if (init_counter > 0)
		return 0;
	
	// TODO: Should function pointers be NULL'd?
	return 0;
}

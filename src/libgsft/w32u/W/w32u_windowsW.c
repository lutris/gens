/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_windowsW.c: windows.h translation. (Unicode version)               *
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
#include "w32u_windows.h"
#include "w32u_charset.h"
#include "../gsft_unused.h"

// C includes.
#include <stdlib.h>


/** kernel32.dll **/


static WINBASEAPI DWORD WINAPI GetModuleFileNameUW(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	if (!lpFilename || nSize == 0)
	{
		// String not specified. Don't bother converting anything.
		return GetModuleFileNameW(hModule, (LPWSTR)lpFilename, nSize);
	}
	
	// Allocate a buffer for the filename.
	wchar_t *lpwFilename = (wchar_t*)malloc(nSize * sizeof(wchar_t));
	DWORD dwRet = GetModuleFileNameW(hModule, lpwFilename, nSize);
	if (dwRet == 0)
	{
		free(lpwFilename);
		return dwRet;
	}
	
	// Convert the filename from UTF-16 to UTF-8.
	WideCharToMultiByte(CP_UTF8, 0, lpwFilename, -1, lpFilename, nSize, NULL, NULL);
	free(lpwFilename);
	return dwRet;
}


static WINBASEAPI HMODULE WINAPI GetModuleHandleUW(LPCSTR lpModuleName)
{
	if (!lpModuleName)
	{
		// String not specified. Don't bother converting anything.
		return GetModuleHandleW((LPCWSTR)lpModuleName);
	}
	
	// Convert lpModuleName from UTF-8 to UTF-16.
	wchar_t *lpwModuleName = w32u_UTF8toUTF16(lpModuleName);
	
	HMODULE hRet = GetModuleHandleW(lpwModuleName);
	free(lpwModuleName);
	return hRet;
}


static WINBASEAPI UINT WINAPI GetSystemDirectoryUW(LPSTR lpBuffer, UINT uSize)
{
	if (!lpBuffer || uSize == 0)
	{
		// String not specified. Don't bother converting anything.
		return GetSystemDirectoryW((LPWSTR)lpBuffer, uSize);
	}
	
	// Allocate a buffer for the filename.
	wchar_t *lpwBuffer = (wchar_t*)malloc(uSize * sizeof(wchar_t));
	UINT uRet = GetSystemDirectoryW(lpwBuffer, uSize);
	if (uRet == 0)
	{
		free(lpwBuffer);
		return uRet;
	}
	
	// Convert the filename from UTF-16 to UTF-8.
	WideCharToMultiByte(CP_UTF8, 0, lpwBuffer, -1, lpBuffer, uSize, NULL, NULL);
	free(lpwBuffer);
	return uRet;
}


static WINUSERAPI BOOL WINAPI SetCurrentDirectoryUW(LPCSTR lpPathName)
{
	if (!lpPathName)
	{
		// String not specified. Don't bother converting anything.
		return SetCurrentDirectoryW((LPCWSTR)lpPathName);
	}
	
	// Convert lpPathName from UTF-8 to UTF-16.
	wchar_t *lpwPathName = w32u_UTF8toUTF16(lpPathName);
	
	BOOL bRet = SetCurrentDirectoryW(lpwPathName);
	free(lpwPathName);
	return bRet;
}


static WINBASEAPI BOOL WINAPI GetVersionExUW(LPOSVERSIONINFOA lpVersionInfo)
{
	// Get the version information.
	OSVERSIONINFOEXW wVersionInfo;
	wVersionInfo.dwOSVersionInfoSize = lpVersionInfo->dwOSVersionInfoSize + sizeof(lpVersionInfo->szCSDVersion);
	BOOL bRet = GetVersionExW((OSVERSIONINFOW*)&wVersionInfo);
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


static WINUSERAPI ATOM WINAPI RegisterClassUW(const WNDCLASSA* lpWndClass)
{
	// Convert lpWndClass from WNDCLASSA to WNDCLASSW.
	WNDCLASSW wWndClass;
	memcpy(&wWndClass, lpWndClass, sizeof(wWndClass));
	
	// Convert the UTF-8 strings to UTF-16.
	wchar_t *lpszwMenuName = NULL, *lpszwClassName = NULL;
	
	if (lpWndClass->lpszMenuName)
	{
		lpszwMenuName = w32u_UTF8toUTF16(lpWndClass->lpszMenuName);
		wWndClass.lpszMenuName = lpszwMenuName;
	}
	
	if (lpWndClass->lpszClassName)
	{
		lpszwClassName = w32u_UTF8toUTF16(lpWndClass->lpszClassName);
		wWndClass.lpszClassName = lpszwClassName;
	}
	
	ATOM aRet = RegisterClassW(&wWndClass);
	free(lpszwMenuName);
	free(lpszwClassName);
	return aRet;
}


static WINUSERAPI HWND WINAPI CreateWindowExUW(
			DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
			DWORD dwStyle, int x, int y, int nWidth, int nHeight,
			HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	// Convert lpClassName and lpWindowName from UTF-8 to UTF-16.
	wchar_t *lpwClassName = NULL, *lpwWindowName = NULL;
	
	if (lpClassName)
		lpwClassName = w32u_UTF8toUTF16(lpClassName);
	
	if (lpWindowName)
		lpwWindowName = w32u_UTF8toUTF16(lpWindowName);
	
	HWND hRet = CreateWindowExW(dwExStyle, lpwClassName, lpwWindowName,
					dwStyle, x, y, nWidth, nHeight,
					hWndParent, hMenu, hInstance, lpParam);
	
	free(lpwClassName);
	free(lpwWindowName);
	return hRet;
}


static WINUSERAPI BOOL WINAPI SetWindowTextUW(HWND hWnd, LPCSTR lpString)
{
	// Convert lpString from UTF-8 to UTF-16.
	wchar_t *lpwString = NULL;
	
	if (lpString)
		lpwString = w32u_UTF8toUTF16(lpString);
	
	BOOL bRet = SetWindowTextW(hWnd, lpwString);
	free(lpwString);
	return bRet;
}


static WINUSERAPI int WINAPI GetWindowTextUW(HWND hWnd, LPSTR lpString, int nMaxCount)
{
	if (!lpString || nMaxCount <= 0)
	{
		// No return buffer specified.
		return GetWindowTextW(hWnd, (LPWSTR)lpString, nMaxCount);
	}
	
	// Allocate a temporary UTF-16 return buffer.
	wchar_t *lpwString = (wchar_t*)malloc(nMaxCount * sizeof(wchar_t));
	int ret = GetWindowTextW(hWnd, lpwString, nMaxCount);
	
	// Convert the window text to UTF-8.
	WideCharToMultiByte(CP_UTF8, 0, lpwString, nMaxCount, lpString, nMaxCount, NULL, NULL);
	
	// Free the buffer.
	free(lpwString);
	return ret;
}


static WINUSERAPI BOOL WINAPI InsertMenuUW(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
{
	if ((uFlags & (MF_BITMAP | MF_OWNERDRAW)) || !lpNewItem)
	{
		// String not specified. Don't bother converting anything.
		return InsertMenuW(hMenu, uPosition, uFlags, uIDNewItem, (LPCWSTR)lpNewItem);
	}
	
	// Convert lpNewItem from UTF-8 to UTF-16.
	wchar_t *lpwNewItem = NULL;
	
	if (lpNewItem)
		lpwNewItem = w32u_UTF8toUTF16(lpNewItem);
	
	BOOL bRet = InsertMenuW(hMenu, uPosition, uFlags, uIDNewItem, lpwNewItem);
	free(lpwNewItem);
	return bRet;
}


static WINUSERAPI BOOL WINAPI ModifyMenuUW(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
{
	if ((uFlags & (MF_BITMAP | MF_OWNERDRAW)) || !lpNewItem)
	{
		// String not specified. Don't bother converting anything.
		return ModifyMenuW(hMenu, uPosition, uFlags, uIDNewItem, (LPCWSTR)lpNewItem);
	}
	
	// Convert lpNewItem from UTF-8 to UTF-16.
	wchar_t *lpwNewItem = NULL;
	
	if (lpNewItem)
		lpwNewItem = w32u_UTF8toUTF16(lpNewItem);
	
	BOOL bRet = ModifyMenuW(hMenu, uPosition, uFlags, uIDNewItem, lpwNewItem);
	free(lpwNewItem);
	return bRet;
}


/** gdi32.dll **/


static WINGDIAPI BOOL WINAPI GetTextExtentPoint32UW(HDC hDC, LPCSTR lpString, int c, LPSIZE lpSize)
{
	if (!lpString)
	{
		// String not specified. Don't bother converting anything.
		return GetTextExtentPoint32W(hDC, (LPCWSTR)lpString, c, lpSize);
	}
	
	// Convert lpString from UTF-8 to UTF-16.
	wchar_t *lpwString = w32u_UTF8toUTF16(lpString);
	BOOL bRet = GetTextExtentPoint32W(hDC, lpwString, c, lpSize);
	free(lpwString);
	return bRet;
}


#define LOADRESOURCE_FNW(fnUW, fnWin32, type_ret) \
static WINUSERAPI type_ret WINAPI fnUW(HINSTANCE hInstance, LPCSTR lpResName) \
{ \
	if ((DWORD_PTR)lpResName < 0x10000) \
		return fnWin32(hInstance, (LPCWSTR)lpResName); \
	wchar_t *lpwResName = w32u_UTF8toUTF16(lpResName); \
	type_ret ret = fnWin32(hInstance, lpwResName); \
	free(lpwResName); \
	return ret; \
}

LOADRESOURCE_FNW(LoadAcceleratorsUW,	LoadAcceleratorsW,	HACCEL);
LOADRESOURCE_FNW(LoadBitmapUW,		LoadBitmapW,		HBITMAP);
LOADRESOURCE_FNW(LoadCursorUW,		LoadCursorW,		HCURSOR);
LOADRESOURCE_FNW(LoadIconUW,		LoadIconW,		HICON);


static WINUSERAPI HANDLE WINAPI LoadImageUW(HINSTANCE hInst, LPCSTR lpszName, UINT uType,
						int cxDesired, int cyDesired, UINT fuLoad)
{
	if ((DWORD_PTR)lpszName < 0x10000)
	{
		// lpszName is a resource ID.
		return LoadImageW(hInst, (LPCWSTR)lpszName, uType, cxDesired, cyDesired, fuLoad);
	}
	
	// lpszName is a string. Convert it from UTF-8 to UTF-16.
	wchar_t *lpszwName = w32u_UTF8toUTF16(lpszName);
	HANDLE hRet = LoadImageW(hInst, lpszwName, uType, cxDesired, cyDesired, fuLoad);
	free(lpszwName);
	return hRet;
}


static WINUSERAPI int WINAPI MessageBoxUW(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	// Convert lpText and lpCaption from UTF-8 to UTF-16.
	wchar_t *lpwText = NULL, *lpwCaption = NULL;
	
	if (lpText)
		lpwText = w32u_UTF8toUTF16(lpText);
	if (lpCaption)
		lpwCaption = w32u_UTF8toUTF16(lpCaption);
	
	int ret = MessageBoxW(hWnd, lpwText, lpwCaption, uType);
	free(lpwText);
	free(lpwCaption);
	return ret;
}


/**
 * SendMessageUW_LPCSTR(): Convert LPARAM from UTF-8 to UTF-16, then call SendMessageW().
 * @param hWnd hWnd.
 * @param msgA ANSI message.
 * @param msgW Unicode message.
 * @param wParam wParam.
 * @param lParam lParam. (LPCSTR)
 * @return Result.
 */
static WINUSERAPI LRESULT WINAPI SendMessageUW_LPCSTR(HWND hWnd, UINT msgA, UINT msgW, WPARAM wParam, LPARAM lParam)
{
	GSFT_UNUSED_PARAMETER(msgA);
	
	if (!lParam)
		return SendMessageW(hWnd, msgW, wParam, lParam);
	
	// Convert lParam from UTF-8 to UTF-16.
	wchar_t *lwParam = w32u_UTF8toUTF16((char*)lParam);
	
	// Send the message.
	LRESULT lRet = SendMessageW(hWnd, msgW, wParam, (LPARAM)lwParam);
	free(lwParam);
	return lRet;
}


int WINAPI w32u_windowsW_init(void)
{
	// TODO: Error handling.
	
	pGetModuleFileNameU	= &GetModuleFileNameUW;
	pGetModuleHandleU	= &GetModuleHandleUW;
	pGetSystemDirectoryU	= &GetSystemDirectoryUW;
	pSetCurrentDirectoryU	= &SetCurrentDirectoryUW;
	pGetVersionExU		= &GetVersionExUW;
	
	pRegisterClassU		= &RegisterClassUW;
	pCreateWindowExU	= &CreateWindowExUW;
	pSetWindowTextU		= &SetWindowTextUW;
	pGetWindowTextU		= &GetWindowTextUW;
	pInsertMenuU		= &InsertMenuUW;
	pModifyMenuU		= &ModifyMenuUW;
	pLoadAcceleratorsU	= &LoadAcceleratorsUW;
	pLoadBitmapU		= &LoadBitmapUW;
	pLoadCursorU		= &LoadCursorUW;
	pLoadIconU		= &LoadIconUW;
	pLoadImageU		= &LoadImageUW;
	pMessageBoxU		= &MessageBoxUW;
	
	pGetTextExtentPoint32U	= &GetTextExtentPoint32UW;
	
	pDefWindowProcU		= &DefWindowProcW;
	pCallWindowProcU	= &CallWindowProcW;
	
	pSendMessageU		= &SendMessageW;
	pGetMessageU		= &GetMessageW;
	pPeekMessageU		= &PeekMessageW;
	pSendMessageU_LPCSTR	= &SendMessageUW_LPCSTR;
	
	pCreateAcceleratorTableU	= &CreateAcceleratorTableW;
	pTranslateAcceleratorU		= &TranslateAcceleratorW;
	
	pIsDialogMessageU	= &IsDialogMessageW;
	pDispatchMessageU	= &DispatchMessageW;
	
	pGetWindowTextLengthU	= &GetWindowTextLengthW;
	
#ifdef _WIN64
	pGetWindowLongPtrU	= &GetWindowLongPtrW;
	pSetWindowLongPtrU	= &SetWindowLongPtrW;
#else
	pGetWindowLongU		= &GetWindowLongW;
	pSetWindowLongU		= &SetWindowLongW;
#endif
	
#ifdef _WIN64
	pGetClassLongPtrU	= &GetClassLongPtrW;
	pSetClassLongPtrU	= &SetClassLongPtrW;
#else
	pGetClassLongU		= &GetClassLongW;
	pSetClassLongU		= &SetClassLongW;
#endif
	
	return 0;
}

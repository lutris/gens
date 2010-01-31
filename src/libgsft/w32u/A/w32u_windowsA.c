/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_windowsA.c: windows.h translation. (ANSI version)                  *
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
#include "w32u_windows.h"
#include "w32u_charset.h"
#include "../gsft_unused.h"


// C includes.
#include <stdlib.h>


/** kernel32.dll **/


static WINBASEAPI DWORD WINAPI GetModuleFileNameUA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	if (!lpFilename || nSize == 0)
	{
		// String not specified. Don't bother converting anything.
		return GetModuleFileNameA(hModule, lpFilename, nSize);
	}
	
	// Get the filename.
	DWORD dwRet = GetModuleFileNameA(hModule, lpFilename, nSize);
	if (dwRet == 0)
		return dwRet;
	
	// Convert the filename from ANSI to UTF-8 in place.
	// TODO: Check for errors in case the UTF-8 text doesn't fit.
	w32u_ANSItoUTF8_ip(lpFilename, nSize);
	return dwRet;
}


static WINBASEAPI HMODULE WINAPI GetModuleHandleUA(LPCSTR lpModuleName)
{
	if (!lpModuleName)
	{
		// String not specified. Don't bother converting anything.
		return GetModuleHandleA(lpModuleName);
	}
	
	// Convert lpModuleName from UTF-8 to ANSI.
	char *lpaModuleName = w32u_UTF8toANSI(lpModuleName);
	
	HMODULE hRet = GetModuleHandleA(lpaModuleName);
	free(lpaModuleName);
	return hRet;
}


static WINBASEAPI UINT WINAPI GetSystemDirectoryUA(LPSTR lpBuffer, UINT uSize)
{
	if (!lpBuffer || uSize == 0)
	{
		// String not specified. Don't bother converting anything.
		return GetSystemDirectoryA(lpBuffer, uSize);
	}
	
	// Get the directory name.
	UINT uRet = GetSystemDirectoryA(lpBuffer, uSize);
	if (uRet == 0)
		return uRet;
	
	// Convert the filename from ANSI to UTF-8 in place.
	// TODO: Check for errors in case the UTF-8 text doesn't fit.
	w32u_ANSItoUTF8_ip(lpBuffer, uSize);
	return uRet;
}


static WINUSERAPI BOOL WINAPI SetCurrentDirectoryUA(LPCSTR lpPathName)
{
	if (!lpPathName)
	{
		// String not specified. Don't bother converting anything.
		return SetCurrentDirectoryA(lpPathName);
	}
	
	// Convert lpPathName from UTF-8 to ANSI.
	char *lpaPathName = w32u_UTF8toANSI(lpPathName);
	
	BOOL bRet = SetCurrentDirectoryA(lpaPathName);
	free(lpaPathName);
	return bRet;
}


static WINBASEAPI BOOL WINAPI GetVersionExUA(LPOSVERSIONINFOA lpVersionInfo)
{
	// TODO: ANSI conversion.
	return GetVersionExA(lpVersionInfo);
}


static WINBASEAPI HINSTANCE WINAPI LoadLibraryUA(LPCSTR lpFileName)
{
	if (!lpFileName)
	{
		// String not specified. Don't bother converting anything.
		return LoadLibraryA(lpFileName);
	}
	
	// Convert lpFileName from UTF-8 to ANSI.
	char *lpaFileName = w32u_UTF8toANSI(lpFileName);
	HINSTANCE hRet = LoadLibraryA(lpaFileName);
	free(lpaFileName);
	return hRet;
}


/** user32.dll **/


static WINUSERAPI ATOM WINAPI RegisterClassUA(const WNDCLASSA* lpWndClass)
{
	WNDCLASSA aWndClass;
	memcpy(&aWndClass, lpWndClass, sizeof(aWndClass));
	
	// Convert the UTF-8 strings to ANSI.
	char *lpszaMenuName = NULL, *lpszaClassName = NULL;
	
	if (lpWndClass->lpszMenuName)
	{
		lpszaMenuName = w32u_UTF8toANSI(lpWndClass->lpszMenuName);
		aWndClass.lpszMenuName = lpszaMenuName;
	}
	
	if (lpWndClass->lpszClassName)
	{
		lpszaClassName = w32u_UTF8toANSI(lpWndClass->lpszClassName);
		aWndClass.lpszClassName = lpszaClassName;
	}
	
	ATOM aRet = RegisterClassA(&aWndClass);
	free(lpszaMenuName);
	free(lpszaClassName);
	return aRet;
}


static WINUSERAPI HWND WINAPI CreateWindowExUA(
			DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
			DWORD dwStyle, int x, int y, int nWidth, int nHeight,
			HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	// Convert lpClassName and lpWindowName from UTF-8 to ANSI.
	char *lpaClassName = NULL, *lpaWindowName = NULL;
	
	if (lpClassName)
		lpaClassName = w32u_UTF8toANSI(lpClassName);
	
	if (lpWindowName)
		lpaWindowName = w32u_UTF8toANSI(lpWindowName);
	
	HWND hRet = CreateWindowExA(dwExStyle, lpaClassName, lpaWindowName,
					dwStyle, x, y, nWidth, nHeight,
					hWndParent, hMenu, hInstance, lpParam);
	
	free(lpaClassName);
	free(lpaWindowName);
	return hRet;
}


static WINUSERAPI BOOL WINAPI SetWindowTextUA(HWND hWnd, LPCSTR lpString)
{
	if (!lpString)
	{
		// String not specified. Don't bother converting anything.
		return SetWindowTextA(hWnd, lpString);
	}
	
	// Convert the text from UTF-8 to ANSI.
	char *lpaString = w32u_UTF8toANSI(lpString);
	BOOL bRet = SetWindowTextA(hWnd, lpaString);
	free(lpaString);
	return bRet;
}


static WINUSERAPI int WINAPI GetWindowTextUA(HWND hWnd, LPSTR lpString, int nMaxCount)
{
	// Get the window text.
	int ret = GetWindowTextA(hWnd, lpString, nMaxCount);
	if (ret == 0)
	{
		// Error retrieving the text.
		return ret;
	}
	
	// Convert the filename from ANSI to UTF-8 in place.
	// TODO: Check for errors in case the UTF-8 text doesn't fit.
	w32u_ANSItoUTF8_ip(lpString, nMaxCount);
	return ret;
}


static WINUSERAPI BOOL WINAPI InsertMenuUA(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
{
	if ((uFlags & (MF_BITMAP | MF_OWNERDRAW)) || !lpNewItem)
	{
		// String not specified. Don't bother converting anything.
		return InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);
	}
	
	// Convert lpNewItem from UTF-8 to ANSI.
	char *lpaNewItem = NULL;
	
	if (lpNewItem)
		lpaNewItem = w32u_UTF8toANSI(lpNewItem);
	
	BOOL bRet = InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, lpaNewItem);
	free(lpaNewItem);
	return bRet;
}


static WINUSERAPI BOOL WINAPI ModifyMenuUA(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
{
	if ((uFlags & (MF_BITMAP | MF_OWNERDRAW)) || !lpNewItem)
	{
		// String not specified. Don't bother converting anything.
		return ModifyMenuA(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);
	}
	
	// Convert lpNewItem from UTF-8 to ANSI.
	char *lpaNewItem = NULL;
	
	if (lpNewItem)
		lpaNewItem = w32u_UTF8toANSI(lpNewItem);
	
	BOOL bRet = ModifyMenuA(hMenu, uPosition, uFlags, uIDNewItem, lpaNewItem);
	free(lpaNewItem);
	return bRet;
}


static WINUSERAPI int WINAPI DrawTextUA(HDC hDC, LPCSTR lpchText, int nCount, LPRECT lpRect, UINT uFormat)
{
	if (!lpchText)
	{
		// String not specified. Don't bother converting anything.
		return DrawTextA(hDC, lpchText, nCount, lpRect, uFormat);
	}
	
	// Convert lpchText from UTF-8 to ANSI.
	char *lpchaText = w32u_UTF8toANSI(lpchText);
	int ret = DrawTextA(hDC, lpchaText, nCount, lpRect, uFormat);
	free(lpchaText);
	return ret;
}


/** gdi32.dll **/


static WINGDIAPI BOOL WINAPI GetTextExtentPoint32UA(HDC hDC, LPCSTR lpString, int c, LPSIZE lpSize)
{
	if (!lpString)
	{
		// String not specified. Don't bother converting anything.
		return GetTextExtentPoint32A(hDC, lpString, c, lpSize);
	}
	
	// Convert lpString from UTF-8 to ANSI.
	char *lpaString = w32u_UTF8toANSI(lpString);
	BOOL bRet = GetTextExtentPoint32A(hDC, lpaString, c, lpSize);
	free(lpaString);
	return bRet;
}


#define LOADRESOURCE_FNA(fnUA, fnWin32, type_ret) \
static WINUSERAPI type_ret WINAPI fnUA(HINSTANCE hInstance, LPCSTR lpResName) \
{ \
	if ((DWORD_PTR)lpResName < 0x10000) \
		return fnWin32(hInstance, lpResName); \
	char *lpaResName = w32u_UTF8toANSI(lpResName); \
	type_ret ret = fnWin32(hInstance, lpaResName); \
	free(lpaResName); \
	return ret; \
}

LOADRESOURCE_FNA(LoadAcceleratorsUA,	LoadAcceleratorsA,	HACCEL);
LOADRESOURCE_FNA(LoadBitmapUA,		LoadBitmapA,		HBITMAP);
LOADRESOURCE_FNA(LoadCursorUA,		LoadCursorA,		HCURSOR);
LOADRESOURCE_FNA(LoadIconUA,		LoadIconA,		HICON);


static WINUSERAPI HANDLE WINAPI LoadImageUA(HINSTANCE hInst, LPCSTR lpszName, UINT uType,
						int cxDesired, int cyDesired, UINT fuLoad)
{
	if ((DWORD_PTR)lpszName < 0x10000)
	{
		// lpszName is a resource ID.
		return LoadImageA(hInst, lpszName, uType, cxDesired, cyDesired, fuLoad);
	}
	
	// lpszName is a string. Convert it from UTF-8 to ANSI.
	char *lpszaName = w32u_UTF8toANSI(lpszName);
	HANDLE hRet = LoadImageA(hInst, lpszaName, uType, cxDesired, cyDesired, fuLoad);
	free(lpszaName);
	return hRet;
}


static WINUSERAPI int WINAPI MessageBoxUA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	// Convert lpText and lpCaption from UTF-8 to ANSI.
	char *lpaText = NULL, *lpaCaption = NULL;
	
	if (lpText)
		lpaText = w32u_UTF8toANSI(lpText);
	if (lpCaption)
		lpaCaption = w32u_UTF8toANSI(lpCaption);
	
	int ret = MessageBoxA(hWnd, lpaText, lpaCaption, uType);
	free(lpaText);
	free(lpaCaption);
	return ret;
}


/**
 * SendMessageUA_LPCSTR(): Convert LPARAM from UTF-8 to ANSI, then call SendMessageA().
 * @param hWnd hWnd.
 * @param msgA ANSI message.
 * @param msgW Unicode message.
 * @param wParam wParam.
 * @param lParam lParam. (LPCSTR)
 * @return Result.
 */
static WINUSERAPI LRESULT WINAPI SendMessageUA_LPCSTR(HWND hWnd, UINT msgA, UINT msgW, WPARAM wParam, LPARAM lParam)
{
	GSFT_UNUSED_PARAMETER(msgW);
	
	if (!lParam)
		return SendMessageA(hWnd, msgA, wParam, lParam);
	
	// Convert lParam from UTF-8 to ANSI.
	char *laParam = w32u_UTF8toANSI((char*)lParam);
	
	// Send the message.
	LRESULT lRet = SendMessageA(hWnd, msgA, wParam, (LPARAM)laParam);
	free(laParam);
	return lRet;
}


void WINAPI w32u_windowsA_init(void)
{
	pGetModuleFileNameU	= &GetModuleFileNameUA;
	pGetModuleHandleU	= &GetModuleHandleUA;
	pGetSystemDirectoryU	= &GetSystemDirectoryUA;
	pSetCurrentDirectoryU	= &SetCurrentDirectoryUA;
	pGetVersionExU		= &GetVersionExUA;
	pLoadLibraryU		= &LoadLibraryUA;
	
	pRegisterClassU		= &RegisterClassUA;
	pCreateWindowExU	= &CreateWindowExUA;
	pSetWindowTextU		= &SetWindowTextUA;
	pGetWindowTextU		= &GetWindowTextUA;
	pInsertMenuU		= &InsertMenuUA;
	pModifyMenuU		= &ModifyMenuUA;
	pLoadAcceleratorsU	= &LoadAcceleratorsUA;
	pLoadBitmapU		= &LoadBitmapUA;
	pLoadCursorU		= &LoadCursorUA;
	pLoadIconU		= &LoadIconUA;
	pLoadImageU		= &LoadImageUA;
	pMessageBoxU		= &MessageBoxUA;
	pDrawTextU		= &DrawTextUA;
	
	pGetTextExtentPoint32U	= &GetTextExtentPoint32UA;
	
	pDefWindowProcU		= &DefWindowProcA;
	pCallWindowProcU	= &CallWindowProcA;
	
	pSendMessageU		= &SendMessageA;
	pGetMessageU		= &GetMessageA;
	pPeekMessageU		= &PeekMessageA;
	pSendMessageU_LPCSTR	= &SendMessageUA_LPCSTR;
	
	pCreateAcceleratorTableU	= &CreateAcceleratorTableA;
	pTranslateAcceleratorU		= &TranslateAcceleratorA;
	
	pIsDialogMessageU	= &IsDialogMessageA;
	pDispatchMessageU	= &DispatchMessageA;
	
	pGetWindowTextLengthU	= &GetWindowTextLengthA;
	
#ifdef _WIN64
	pGetWindowLongPtrU	= &GetWindowLongPtrA;
	pSetWindowLongPtrU	= &SetWindowLongPtrA;
#else
	pGetWindowLongU		= &GetWindowLongA;
	pSetWindowLongU		= &SetWindowLongA;
#endif
	
#ifdef _WIN64
	pGetClassLongPtrU	= &GetClassLongPtrA;
	pSetClassLongPtrU	= &SetClassLongPtrA;
#else
	pGetClassLongU		= &GetClassLongA;
	pSetClassLongU		= &SetClassLongA;
#endif
}

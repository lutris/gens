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
#include "w32u_priv.h"
#include "../gsft_unused.h"

#include "w32u_windows.h"

// C includes.
#include <stdlib.h>


/** kernel32.dll **/


static WINBASEAPI DWORD WINAPI GetModuleFileNameUA(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	// TODO: ANSI conversion.
	return GetModuleFileNameA(hModule, lpFilename, nSize);
}


static WINBASEAPI HMODULE WINAPI GetModuleHandleUA(LPCSTR lpModuleName)
{
	// TODO: ANSI conversion.
	return GetModuleHandleA(lpModuleName);
}


static WINUSERAPI BOOL WINAPI SetCurrentDirectoryUA(LPCSTR lpPathName)
{
	// TODO: ANSI conversion.
	return SetCurrentDirectoryA(lpPathName);
}


static WINBASEAPI BOOL WINAPI GetVersionExUA(LPOSVERSIONINFOA lpVersionInfo)
{
	// TODO: ANSI conversion.
	return GetVersionExA(lpVersionInfo);
}


/** user32.dll **/


static WINUSERAPI ATOM WINAPI RegisterClassUA(CONST WNDCLASSA* lpWndClass)
{
	// TODO: ANSI conversion.
	return RegisterClassA(lpWndClass);
}


static WINUSERAPI HWND WINAPI CreateWindowExUA(
			DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
			DWORD dwStyle, int x, int y, int nWidth, int nHeight,
			HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	// TODO: ANSI conversion.
	return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle,
				x, y, nWidth, nHeight,
				hWndParent, hMenu, hInstance, lpParam);
}


static WINUSERAPI BOOL WINAPI SetWindowTextUA(HWND hWnd, LPCSTR lpString)
{
	// TODO: ANSI conversion.
	return SetWindowTextA(hWnd, lpString);
}


static WINUSERAPI int WINAPI GetWindowTextUA(HWND hWnd, LPSTR lpString, int nMaxCount)
{
	// TODO: ANSI conversion.
	return GetWindowTextA(hWnd, lpString, nMaxCount);
}


static WINUSERAPI BOOL WINAPI InsertMenuUA(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
{
	// TODO: ANSI conversion.
	return InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);
}


static WINUSERAPI BOOL WINAPI ModifyMenuUA(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem)
{
	// TODO: ANSI conversion.
	return ModifyMenuA(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);
}


static WINUSERAPI HACCEL WINAPI LoadAcceleratorsUA(HINSTANCE hInstance, LPCSTR lpTableName)
{
	// TODO: ANSI conversion.
	return LoadAcceleratorsA(hInstance, lpTableName);
}


static WINUSERAPI HBITMAP WINAPI LoadBitmapUA(HINSTANCE hInstance, LPCSTR lpBitmapName)
{
	// TODO: ANSI conversion.
	return LoadBitmapA(hInstance, lpBitmapName);
}


static WINUSERAPI HCURSOR WINAPI LoadCursorUA(HINSTANCE hInstance, LPCSTR lpCursorName)
{
	// TODO: ANSI conversion.
	return LoadCursorA(hInstance, lpCursorName);
}


static WINUSERAPI HICON WINAPI LoadIconUA(HINSTANCE hInstance, LPCSTR lpIconName)
{
	// TODO: ANSI conversion.
	return LoadIconA(hInstance, lpIconName);
}


static WINUSERAPI HANDLE WINAPI LoadImageUA(HINSTANCE hInst, LPCSTR lpszName, UINT uType,
						int cxDesired, int cyDesired, UINT fuLoad)
{
	// TODO: ANSI conversion.
	return LoadImageA(hInst, lpszName, uType, cxDesired, cyDesired, fuLoad);
}


static WINUSERAPI int WINAPI MessageBoxUA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	// TODO: ANSI conversion.
	return MessageBoxA(hWnd, lpText, lpCaption, uType);
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
	
	// TODO: ANSI conversions.
	return SendMessageA(hWnd, msgA, wParam, lParam);
}


int WINAPI w32u_windowsA_init(void)
{
	// TODO: Error handling.
	
	pGetModuleFileNameU	= &GetModuleFileNameUA;
	pGetModuleHandleU	= &GetModuleHandleUA;
	pSetCurrentDirectoryU	= &SetCurrentDirectoryUA;
	pGetVersionExU		= &GetVersionExUA;
	
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
	
	return 0;
}

int WINAPI w32u_windowsA_end(void)
{
	// TODO: Should the function pointers be NULL'd?
	return 0;
}

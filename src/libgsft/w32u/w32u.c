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
#include "w32u_commdlg.h"
#include "w32u_shlobj.h"

// shlwapi.h needed for DLLVERSIONINFO and DLLGETVERSIONPROC.
#include <shlwapi.h>

// C includes.
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

// Initialization counter.
static int init_counter = 0;

// DLLs.
static HMODULE hKernel32 = NULL;
MAKE_FUNCPTR(MultiByteToWideChar);
MAKE_FUNCPTR(WideCharToMultiByte);

static HMODULE hUser32 = NULL;
static HMODULE hShell32 = NULL;

/** DLL versions. (0xMMNNRRRR) **/
DWORD shell32_dll_version = 0;


/** kernel32.dll **/


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


MAKE_FUNCPTR(GetVersionExA);
MAKE_STFUNCPTR(GetVersionExW);
static WINBASEAPI BOOL WINAPI GetVersionExU(LPOSVERSIONINFOA lpVersionInfo)
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
	pWideCharToMultiByte(CP_UTF8, 0, wVersionInfo.szCSDVersion, 
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


MAKE_FUNCPTR(GetWindowTextA);
MAKE_STFUNCPTR(GetWindowTextW);
static WINUSERAPI int WINAPI GetWindowTextU(HWND hWnd, LPSTR lpString, int nMaxCount)
{
	if (!lpString || nMaxCount <= 0)
	{
		// No return buffer specified.
		return pGetWindowTextW(hWnd, (LPWSTR)lpString, nMaxCount);
	}
	
	// Allocate a temporary UTF-16 return buffer.
	wchar_t *lpwString = (wchar_t*)malloc(nMaxCount * sizeof(lpwString));
	
	// Get the window text.
	int ret = pGetWindowTextW(hWnd, lpwString, nMaxCount);
	
	// Convert the window text to UTF-8.
	pWideCharToMultiByte(CP_UTF8, 0, lpwString, nMaxCount, lpString, nMaxCount, NULL, NULL);
	
	// Free the buffer.
	free(lpwString);
	return ret;
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


MAKE_FUNCPTR(LoadAcceleratorsA);
MAKE_STFUNCPTR(LoadAcceleratorsW);
static WINUSERAPI HACCEL WINAPI LoadAcceleratorsU(HINSTANCE hInstance, LPCSTR lpTableName)
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


MAKE_FUNCPTR(LoadBitmapA);
MAKE_STFUNCPTR(LoadBitmapW);
static WINUSERAPI HBITMAP WINAPI LoadBitmapU(HINSTANCE hInstance, LPCSTR lpBitmapName)
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


MAKE_FUNCPTR(LoadCursorA);
MAKE_STFUNCPTR(LoadCursorW);
static WINUSERAPI HCURSOR WINAPI LoadCursorU(HINSTANCE hInstance, LPCSTR lpCursorName)
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


MAKE_FUNCPTR(LoadIconA);
MAKE_STFUNCPTR(LoadIconW);
static WINUSERAPI HICON WINAPI LoadIconU(HINSTANCE hInstance, LPCSTR lpIconName)
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


MAKE_FUNCPTR(LoadImageA);
MAKE_STFUNCPTR(LoadImageW);
static WINUSERAPI HANDLE WINAPI LoadImageU(HINSTANCE hInst, LPCSTR lpszName, UINT uType,
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


MAKE_FUNCPTR(MessageBoxA);
MAKE_STFUNCPTR(MessageBoxW);
static WINUSERAPI int WINAPI MessageBoxU(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
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


/**
 * SendMessageU_LPCSTR(): Convert LPARAM from UTF-8 to UTF-16, then call SendMessageW().
 * @param hWnd hWnd.
 * @param msgA ANSI message.
 * @param msgW Unicode message.
 * @param wParam wParam.
 * @param lParam lParam. (LPCSTR)
 * @return Result.
 */
WINUSERAPI LRESULT WINAPI SendMessageU_LPCSTR(HWND hWnd, UINT msgA, UINT msgW, WPARAM wParam, LPARAM lParam)
{
	if (!isSendMessageUnicode)
		return SendMessageA(hWnd, msgA, wParam, lParam);
	if (!lParam)
		return pSendMessageU(hWnd, msgW, wParam, lParam);
	
	// Convert lParam from UTF-8 to UTF-16.
	wchar_t *lwParam = w32u_mbstowcs((char*)lParam);
	
	// Send the message.
	LRESULT lRet = pSendMessageU(hWnd, msgW, wParam, (LPARAM)lwParam);
	free(lwParam);
	return lRet;
}


/**
 * These functions don't need reimplementation (no string processing),
 * but they have separate A/W versions.
 */

#ifdef _WIN64
MAKE_FUNCPTR(GetWindowLongPtrA);
MAKE_FUNCPTR(SetWindowLongPtrA);
#else
MAKE_FUNCPTR(GetWindowLongA);
MAKE_FUNCPTR(SetWindowLongA);
#endif

#ifdef _WIN64
MAKE_FUNCPTR(GetClassLongPtrA);
MAKE_FUNCPTR(SetClassLongPtrA);
#else
MAKE_FUNCPTR(GetClassLongA);
MAKE_FUNCPTR(SetClassLongA);
#endif

MAKE_FUNCPTR(CreateAcceleratorTableA);
MAKE_FUNCPTR(TranslateAcceleratorA);

MAKE_FUNCPTR(DefWindowProcA);
MAKE_FUNCPTR(CallWindowProcA);

MAKE_FUNCPTR(SendMessageA);
BOOL isSendMessageUnicode = 0;
MAKE_FUNCPTR(GetMessageA);
MAKE_FUNCPTR(PeekMessageA);

MAKE_FUNCPTR(IsDialogMessageA);
MAKE_FUNCPTR(DispatchMessageA);


/**
 * GetDllVersionNumber(): Get a DLL's version number via DllGetVersion().
 * @param hDLL Handle to DLL.
 * @return DLL version number, or 0 on error.
 */
static DWORD WINAPI GetDllVersionNumber(HMODULE hDLL)
{
	DLLVERSIONINFO dllvi;
	DLLGETVERSIONPROC dllviproc;
	HRESULT hRet;
	dllvi.cbSize = sizeof(dllvi);
	
	dllviproc = (DLLGETVERSIONPROC)GetProcAddress(hDLL, "DllGetVersion");
	if (!dllviproc)
		return 0;
	
	// DllGetVersion() found.
	hRet = dllviproc(&dllvi);
	if (hRet != S_OK)
		return 0;
	
	// Received the version number.
	return  (dllvi.dwMajorVersion & 0xFF) << 24 |
		(dllvi.dwMinorVersion & 0xFF) << 16 |
		(dllvi.dwBuildNumber & 0xFFFF);
}


int WINAPI w32u_init(void)
{
	// Initialize the Win32 Unicode Translation Layer.
	if (init_counter++ != 0)
	{
		// The Win32 Unicode Translation Layer is already initialized.
		return 0;
	}
	
	// TODO: Error handling.
	hKernel32 = LoadLibrary("kernel32.dll");
	
	// TODO: If either of these GetProcAddress()'s fails, revert to ANSI.
	InitFuncPtr(hKernel32, MultiByteToWideChar);
	InitFuncPtr(hKernel32, WideCharToMultiByte);
	
	InitFuncPtrsU(hKernel32, "GetModuleFileName", pGetModuleFileNameW, pGetModuleFileNameA, GetModuleFileNameU);
	InitFuncPtrsU(hKernel32, "SetCurrentDirectory", pSetCurrentDirectoryW, pSetCurrentDirectoryA, SetCurrentDirectoryU);
	InitFuncPtrsU(hKernel32, "GetVersionEx", pGetVersionExW, pGetVersionExA, GetVersionExU);
	
	hUser32 = LoadLibrary("user32.dll");
	
	InitFuncPtrsU(hUser32, "RegisterClass", pRegisterClassW, pRegisterClassA, RegisterClassU);
	InitFuncPtrsU(hUser32, "CreateWindowEx", pCreateWindowExW, pCreateWindowExA, CreateWindowExU);
	InitFuncPtrsU(hUser32, "SetWindowText", pSetWindowTextW, pSetWindowTextA, SetWindowTextU);
	InitFuncPtrsU(hUser32, "GetWindowText", pGetWindowTextW, pGetWindowTextA, GetWindowTextU);
	InitFuncPtrsU(hUser32, "InsertMenu", pInsertMenuW, pInsertMenuA, InsertMenuU);
	InitFuncPtrsU(hUser32, "ModifyMenu", pModifyMenuW, pModifyMenuA, ModifyMenuU);
	InitFuncPtrsU(hUser32, "LoadAccelerators", pLoadAcceleratorsW, pLoadAcceleratorsA, LoadAcceleratorsU);
	InitFuncPtrsU(hUser32, "LoadBitmap", pLoadBitmapW, pLoadBitmapA, LoadBitmapU);
	InitFuncPtrsU(hUser32, "LoadCursor", pLoadCursorW, pLoadCursorA, LoadCursorU);
	InitFuncPtrsU(hUser32, "LoadIcon", pLoadIconW, pLoadIconA, LoadIconU);
	InitFuncPtrsU(hUser32, "LoadImage", pLoadImageW, pLoadImageA, LoadImageU);
	InitFuncPtrsU(hUser32, "MessageBox", pMessageBoxW, pMessageBoxA, MessageBoxU);
	
	InitFuncPtrs(hUser32, "DefWindowProc", pDefWindowProcA);
	InitFuncPtrs(hUser32, "CallWindowProc", pCallWindowProcA);
	
	InitFuncPtrs(hUser32, "GetMessage", pGetMessageA);
	InitFuncPtrs(hUser32, "PeekMessage", pPeekMessageA);
	
	InitFuncPtrs(hUser32, "CreateAcceleratorTable", pCreateAcceleratorTableA);
	InitFuncPtrs(hUser32, "TranslateAccelerator", pTranslateAcceleratorA);
	
	InitFuncPtrs(hUser32, "IsDialogMessage", pIsDialogMessageA);
	InitFuncPtrs(hUser32, "DispatchMessage", pDispatchMessageA);
	
#ifdef _WIN64
	InitFuncPtrs(hUser32, "GetWindowLongPtr", pGetWindowLongPtrA);
	InitFuncPtrs(hUser32, "SetWindowLongPtr", pSetWindowLongPtrA);
#else
	InitFuncPtrs(hUser32, "GetWindowLong", pGetWindowLongA);
	InitFuncPtrs(hUser32, "SetWindowLong", pSetWindowLongA);
#endif
	
#ifdef _WIN64
	InitFuncPtrs(hUser32, "GetClassLongPtr", pGetClassLongPtrA);
	InitFuncPtrs(hUser32, "SetClassLongPtr", pSetClassLongPtrA);
#else
	InitFuncPtrs(hUser32, "GetClassLong", pGetClassLongA);
	InitFuncPtrs(hUser32, "SetClassLong", pSetClassLongA);
#endif
	
	// Check if SendMessage is Unicode.
	pSendMessageA = (typeof(pSendMessageA))GetProcAddress(hUser32, "SendMessageW");
	if ((typeof(pSendMessageA))GetProcAddress(hUser32, "SendMessageA") == &SendMessageA)
		isSendMessageUnicode = 0;
	else
		isSendMessageUnicode = 1;
	
	// Other DLLs.
	hShell32 = LoadLibrary("shell32.dll");
	
	// Get DLL version numbers.
	shell32_dll_version = GetDllVersionNumber(hShell32);
	
	// Other Win32 Unicode modules.
	w32u_shellapi_init(hShell32);
	w32u_libc_init();
	w32u_commdlg_init();
	w32u_shlobj_init(hShell32);
	
	return 0;
}


int WINAPI w32u_end(void)
{
	if (init_counter <= 0)
		return 0;
	
	init_counter--;
	if (init_counter > 0)
		return 0;
	
	// Unload the libraries.
	FreeLibrary(hKernel32);
	hKernel32 = NULL;
	FreeLibrary(hUser32);
	hUser32 = NULL;
	FreeLibrary(hShell32);
	hShell32 = NULL;
	
	// Clear the DLL versions.
	shell32_dll_version = 0;
	
	// TODO: Should function pointers be NULL'd?
	// TODO: Should shellapi and shlobj have end functions?
	//w32u_shellapi_end();
	w32u_libc_end();
	w32u_commdlg_end();
	//w32u_shlobj_end();
	
	return 0;
}

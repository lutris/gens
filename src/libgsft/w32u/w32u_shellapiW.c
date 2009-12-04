/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_shellapiW.c: shellapi.h translation. (Unicode version)             *
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
#include "w32u_priv.h"
#include "w32u_charset.h"
#include "w32u_shellapiW.h"

#include "w32u_shellapi.h"

// C includes.
#include <stdlib.h>


static UINT WINAPI DragQueryFileUW(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cch)
{
	if (!lpszFile || cch == 0)
	{
		// String not specified. Don't bother converting anything.
		return DragQueryFileW(hDrop, iFile, (LPWSTR)lpszFile, cch);
	}
	
	// Allocate a buffer for the filename.
	wchar_t *lpszwFile = (wchar_t*)malloc(cch * sizeof(wchar_t));
	UINT uRet = DragQueryFileW(hDrop, iFile, lpszwFile, cch);
	
	// Convert the filename from UTF-16 to UTF-8.
	WideCharToMultiByte(CP_UTF8, 0, lpszwFile, -1, lpszFile, cch, NULL, NULL);
	free(lpszwFile);
	return uRet;
}


static HINSTANCE WINAPI ShellExecuteUW(HWND hWnd, LPCSTR lpOperation, LPCSTR lpFile,
					LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	// Convert the four strings from UTF-8 to UTF-16.
	wchar_t *lpwOperation = NULL, *lpwFile = NULL;
	wchar_t *lpwParameters = NULL, *lpwDirectory = NULL;
	
	if (lpOperation)
		lpwOperation = w32u_UTF8toUTF16(lpOperation);
	if (lpFile)
		lpwFile = w32u_UTF8toUTF16(lpFile);
	if (lpParameters)
		lpwParameters = w32u_UTF8toUTF16(lpParameters);
	if (lpDirectory)
		lpwDirectory = w32u_UTF8toUTF16(lpDirectory);
	
	HINSTANCE hRet = ShellExecuteW(hWnd, lpwOperation, lpwFile, lpwParameters, lpwDirectory, nShowCmd);
	free(lpwOperation);
	free(lpwFile);
	free(lpwParameters);
	free(lpwDirectory);
	return hRet;
}


int WINAPI w32u_shellapiW_init(void)
{
	// TODO: Error handling.
	
	pDragQueryFileU		= &DragQueryFileUW;
	pShellExecuteU		= &ShellExecuteUW;
	
	return 0;
}

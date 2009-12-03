/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_shellapiA.c: shellapi.h translation. (ANSI version)                *
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
#include "w32u_shellapiA.h"

#include "w32u_shellapi.h"

// C includes.
#include <stdlib.h>


static UINT WINAPI DragQueryFileUA(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cch)
{
	if (!lpszFile || cch == 0)
	{
		// String not specified. Don't bother converting anything.
		return DragQueryFileA(hDrop, iFile, lpszFile, cch);
	}
	
	// Get the filename.
	UINT uRet = DragQueryFileA(hDrop, iFile, lpszFile, cch);
	if (uRet == 0)
	{
		// Error retrieving the filename.
		return uRet;
	}
	
	// Convert the filename from ANSI to UTF-16.
	int cchwFile = MultiByteToWideChar(CP_ACP, 0, lpszFile, -1, NULL, 0);
	if (cchwFile <= 0)
		return 0;
	
	wchar_t *lpszwFile = (wchar_t*)malloc(cchwFile * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, lpszFile, -1, lpszwFile, cchwFile);
	
	// Convert the filename from UTF-16 to UTF-8.
	WideCharToMultiByte(CP_UTF8, 0, lpszwFile, cchwFile, lpszFile, cch, NULL, NULL);
	
	free(lpszwFile);
	return uRet;
}


static HINSTANCE WINAPI ShellExecuteUA(HWND hWnd, LPCSTR lpOperation, LPCSTR lpFile,
					LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	// Convert the four strings from UTF-8 to ANSI.
	char *lpaOperation = NULL, *lpaFile = NULL;
	char *lpaParameters = NULL, *lpaDirectory = NULL;
	
	if (lpOperation)
		lpaOperation = w32u_UTF8toANSI(lpOperation);
	if (lpFile)
		lpaFile = w32u_UTF8toANSI(lpFile);
	if (lpParameters)
		lpaParameters = w32u_UTF8toANSI(lpParameters);
	if (lpDirectory)
		lpaDirectory = w32u_UTF8toANSI(lpDirectory);
	
	HINSTANCE hRet = ShellExecuteA(hWnd, lpaOperation, lpaFile, lpaParameters, lpaDirectory, nShowCmd);
	free(lpaOperation);
	free(lpaFile);
	free(lpaParameters);
	free(lpaDirectory);
	return hRet;
	
	// TODO: ANSI conversion.
	return ShellExecuteA(hWnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}


int WINAPI w32u_shellapiA_init(void)
{
	// TODO: Error handling.
	
	pDragQueryFileU		= &DragQueryFileUA;
	pShellExecuteU		= &ShellExecuteUA;
	
	return 0;
}

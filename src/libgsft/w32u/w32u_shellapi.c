/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_shellapi.c: shellapi.h translation.                                *
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

// C includes.
#include <stdlib.h>

// DLLs.
static HMODULE hShell32 = NULL;


MAKE_FUNCPTR(DragQueryFileA);
MAKE_STFUNCPTR(DragQueryFileW);
static UINT WINAPI DragQueryFileU(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cch)
{
	if (!lpszFile || cch == 0)
	{
		// String not specified. Don't bother converting anything.
		return pDragQueryFileW(hDrop, iFile, (LPWSTR)lpszFile, cch);
	}
	
	// Allocate a buffer for the filename.
	wchar_t *lpszwFile = (wchar_t*)malloc(cch * sizeof(wchar_t));
	UINT uRet = DragQueryFileW(hDrop, iFile, lpszwFile, cch);
	
	// Convert the filename from UTF-16 to UTF-8.
	pWideCharToMultiByte(CP_UTF8, 0, lpszwFile, -1, lpszFile, cch, NULL, NULL);
	free(lpszwFile);
	return uRet;
}


int WINAPI w32u_shellapi_init(void)
{
	// TODO: Error handling.
	hShell32 = LoadLibrary("shell32.dll");
	
	InitFuncPtrsU(hShell32, "DragQueryFile", pDragQueryFileW, pDragQueryFileA, DragQueryFileU);
	
	return 0;
}

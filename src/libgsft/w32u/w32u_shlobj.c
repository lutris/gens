/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_shlobj.c: shlobj.h translation.                                    *
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
#include "w32u_shlobj.h"

// C includes.
#include <stdlib.h>


MAKE_FUNCPTR(SHBrowseForFolderA);
MAKE_STFUNCPTR(SHBrowseForFolderW);
static LPITEMIDLIST WINAPI SHBrowseForFolderU(PBROWSEINFO lpbi)
{
	// BROWSEINFOA and BROWSEINFOW don't contain any actual string data,
	// so the structs can be copied.
	BROWSEINFOW wbi;
	memcpy(&wbi, lpbi, sizeof(wbi));
	
	// Convert constant strings from UTF-8 to UTF-16.
	wchar_t *lpszwTitle = NULL;
	
	if (lpbi->lpszTitle)
	{
		lpszwTitle = w32u_mbstowcs(lpbi->lpszTitle);
		wbi.lpszTitle = lpszwTitle;
	}
	
	// Allocate the return buffer.
	wchar_t *pszwDisplayName = NULL;
	if (lpbi->pszDisplayName)
	{
		// This is assumed to be MAX_PATH characters.
		pszwDisplayName = (wchar_t*)malloc(MAX_PATH * sizeof(wchar_t));
		wbi.pszDisplayName = pszwDisplayName;
	}
	
	// Get the directory list entry.
	LPITEMIDLIST pidl = pSHBrowseForFolderW(&wbi);
	
	// Convert the non-constant strings from UTF-16 to UTF-8.
	if (wbi.pszDisplayName)
	{
		pWideCharToMultiByte(CP_UTF8, 0, wbi.pszDisplayName, MAX_PATH,
				     lpbi->pszDisplayName, MAX_PATH, NULL, NULL);
	}
	
	// Free the strings.
	free(lpszwTitle);
	free(pszwDisplayName);
	return pidl;
}


MAKE_FUNCPTR(SHGetPathFromIDListA);
MAKE_STFUNCPTR(SHGetPathFromIDListW);
static BOOL WINAPI SHGetPathFromIDListU(LPCITEMIDLIST pidl, LPSTR pszPath)
{
	if (!pszPath)
	{
		// No return buffer was specified.
		return pSHGetPathFromIDListW(pidl, (LPWSTR)pszPath);
	}
	
	// Allocate a wide character buffer.
	// Size is assumed to be MAX_PATH characters.
	wchar_t *pszwPath = (wchar_t*)malloc(MAX_PATH * sizeof(wchar_t));
	
	// Get the directory.
	BOOL bRet = pSHGetPathFromIDListW(pidl, pszwPath);
	
	// Convert the text to UTF-8.
	pWideCharToMultiByte(CP_UTF8, 0, pszwPath, MAX_PATH, pszPath, MAX_PATH, NULL, NULL);
	
	// Free the buffer.
	free(pszwPath);
	return bRet;
}


int WINAPI w32u_shlobj_init(HMODULE hShell32)
{
	InitFuncPtrsU(hShell32, "SHBrowseForFolder", pSHBrowseForFolderW, pSHBrowseForFolderA, SHBrowseForFolderU);
	InitFuncPtrsU(hShell32, "SHGetPathFromIDList", pSHGetPathFromIDListW, pSHGetPathFromIDListA, SHGetPathFromIDListU);
	return 0;
}

/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_commdlgW.c: commdlg.h translation. (Unicode version)               *
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

#include "w32u_commdlgW.h"
#include "w32u_commdlg.h"
#include "w32u_charset.h"

// C includes.
#include <stdlib.h>
#include <string.h>


typedef BOOL (WINAPI *GETOPENFILENAME_PROC)(void *lpofn);
static inline BOOL WINAPI GetOpenFileNameUW_int(LPOPENFILENAMEA lpofn, GETOPENFILENAME_PROC fn)
{
	// OPENFILENAMEA and OPENFILENAMEW don't contain any actual string data,
	// so the structs can be copied.
	OPENFILENAMEW wofn;
	memcpy(&wofn, lpofn, sizeof(wofn));
	
	// Convert constant strings from UTF-8 to UTF-16.
	wchar_t *lpstrwFilter = NULL, *lpstrwInitialDir = NULL;
	wchar_t *lpstrwTitle = NULL, *lpstrwDefExt = NULL;
	
	// lpstrFilter is terminated with *two* NULLs.
	if (lpofn->lpstrFilter)
	{
		const char *tmp = lpofn->lpstrFilter;
		while (tmp[0] || tmp[1])
			tmp++;
		
		size_t cchFilter = (size_t)(tmp - lpofn->lpstrFilter + 2);
		if (cchFilter > 0)
		{
			lpstrwFilter = (wchar_t*)malloc(cchFilter * sizeof(wchar_t));
			MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrFilter, cchFilter,
						lpstrwFilter, cchFilter);
			wofn.lpstrFilter = lpstrwFilter;
		}
	}
	
	// Convert the rest of the constant strings.
	if (lpofn->lpstrInitialDir)
	{
		lpstrwInitialDir = w32u_UTF8toUTF16(lpofn->lpstrInitialDir);
		wofn.lpstrInitialDir = lpstrwInitialDir;
	}
	if (lpofn->lpstrTitle)
	{
		lpstrwTitle = w32u_UTF8toUTF16(lpofn->lpstrTitle);
		wofn.lpstrTitle = lpstrwTitle;
	}
	if (lpofn->lpstrDefExt)
	{
		lpstrwDefExt = w32u_UTF8toUTF16(lpofn->lpstrDefExt);
		wofn.lpstrDefExt = lpstrwDefExt;
	}
	
	// Allocate buffers for the non-constant strings.
	wchar_t *lpstrwCustomFilter = NULL, *lpstrwFile = NULL;
	wchar_t *lpstrwFileTitle = NULL;
	
	if (lpofn->lpstrCustomFilter && lpofn->nMaxCustFilter > 0)
	{
		// Pair of null-terminated strings. Copy the whole buffer.
		lpstrwCustomFilter = (wchar_t*)malloc(lpofn->nMaxCustFilter * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrCustomFilter, lpofn->nMaxCustFilter,
					lpstrwCustomFilter, lpofn->nMaxCustFilter * sizeof(wchar_t));
		wofn.lpstrCustomFilter = lpstrwCustomFilter;
	}
	
	if (lpofn->lpstrFile && lpofn->nMaxFile > 0)
	{
		lpstrwFile = (wchar_t*)malloc(lpofn->nMaxFile * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrFile, -1,
					lpstrwFile, lpofn->nMaxFile * sizeof(wchar_t));
		wofn.lpstrFile = lpstrwFile;
	}
	
	if (lpofn->lpstrFileTitle && lpofn->nMaxFileTitle > 0)
	{
		lpstrwFileTitle = (wchar_t*)malloc(lpofn->nMaxFileTitle * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrFileTitle, -1,
					lpstrwFileTitle, lpofn->nMaxFileTitle * sizeof(wchar_t));
		wofn.lpstrFileTitle = lpstrwFileTitle;
	}
	
	// Get the filename.
	BOOL bRet = fn(&wofn);
	
	// Convert the non-constant strings from UTF-16 to UTF-8.
	if (wofn.lpstrCustomFilter && wofn.nMaxCustFilter > 0)
	{
		// Pair of null-terminated strings. Copy the whole buffer.
		WideCharToMultiByte(CP_UTF8, 0, wofn.lpstrCustomFilter, wofn.nMaxCustFilter,
					lpofn->lpstrCustomFilter, lpofn->nMaxCustFilter, NULL, NULL);
	}
	if (wofn.lpstrFile && wofn.nMaxFile > 0)
	{
		WideCharToMultiByte(CP_UTF8, 0, wofn.lpstrFile, -1,
					lpofn->lpstrFile, lpofn->nMaxFile, NULL, NULL);
	}
	if (wofn.lpstrFileTitle && wofn.nMaxFileTitle > 0)
	{
		WideCharToMultiByte(CP_UTF8, 0, wofn.lpstrFileTitle, -1,
					lpofn->lpstrFileTitle, lpofn->nMaxFileTitle, NULL, NULL);
	}
	
	// Free the constant strings.
	free(lpstrwFilter);
	free(lpstrwInitialDir);
	free(lpstrwTitle);
	free(lpstrwDefExt);
	
	// Free the non-constant strings.
	free(lpstrwCustomFilter);
	free(lpstrwFile);
	free(lpstrwFileTitle);
	
	return bRet;
}


static BOOL WINAPI GetOpenFileNameUW(LPOPENFILENAMEA lpofn)
{
	return GetOpenFileNameUW_int(lpofn, (GETOPENFILENAME_PROC)&GetOpenFileNameW);
}


static BOOL WINAPI GetSaveFileNameUW(LPOPENFILENAMEA lpofn)
{
	return GetOpenFileNameUW_int(lpofn, (GETOPENFILENAME_PROC)&GetSaveFileNameW);
}


int WINAPI w32u_commdlgW_init(void)
{
	// TODO: Error handling.
	
	pGetOpenFileNameU	= &GetOpenFileNameUW;
	pGetSaveFileNameU	= &GetSaveFileNameUW;
	
	return 0;
}

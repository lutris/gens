/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_commdlgA.c: commdlg.h translation. (ANSI version)                  *
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
#include "w32u_commdlgA.h"

#include "w32u_commdlg.h"

// C includes.
#include <stdlib.h>
#include <string.h>


typedef BOOL (WINAPI *GETOPENFILENAME_PROC)(void *lpofn);
static inline BOOL WINAPI GetOpenFileNameUA_int(LPOPENFILENAMEA lpofn, GETOPENFILENAME_PROC fn)
{
	OPENFILENAMEA aofn;
	memcpy(&aofn, lpofn, sizeof(aofn));
	
	// Convert constant strings from UTF-8 to ANSI.
	char *lpstraFilter = NULL, *lpstraInitialDir = NULL;
	char *lpstraTitle = NULL, *lpstraDefExt = NULL;
	
	// lpstrFilter is terminated with *two* NULLs.
	if (lpofn->lpstrFilter)
	{
		const char *tmp = lpofn->lpstrFilter;
		while (tmp[0] || tmp[1])
			tmp++;
		
		size_t cchFilter = (size_t)(tmp - lpofn->lpstrFilter + 2);
		if (cchFilter > 0)
		{
			// Convert from UTF-8 to UTF-16 first.
			wchar_t *lpstrwFilter = (wchar_t*)malloc(cchFilter * sizeof(lpstrwFilter));
			MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrFilter, cchFilter,
						lpstrwFilter, cchFilter);
			
			// Convert from UTF-16 to ANSI.
			lpstraFilter = (char*)malloc(cchFilter);
			WideCharToMultiByte(CP_ACP, 0, lpstrwFilter, cchFilter,
						lpstraFilter, cchFilter, NULL, NULL);
			aofn.lpstrFilter = lpstraFilter;
			free(lpstrwFilter);
		}
	}
	
	// Convert the rest of the constant strings.
	if (lpofn->lpstrInitialDir)
	{
		lpstraInitialDir = w32u_UTF8toANSI(lpofn->lpstrInitialDir);
		aofn.lpstrInitialDir = lpstraInitialDir;
	}
	if (lpofn->lpstrTitle)
	{
		lpstraTitle = w32u_UTF8toANSI(lpofn->lpstrTitle);
		aofn.lpstrTitle = lpstraTitle;
	}
	if (lpofn->lpstrDefExt)
	{
		lpstraDefExt = w32u_UTF8toANSI(lpofn->lpstrDefExt);
		aofn.lpstrDefExt = lpstraDefExt;
	}
	
	// Allocate buffers for the non-constant strings.
	char *lpstraCustomFilter = NULL, *lpstraFile = NULL;
	char *lpstraFileTitle = NULL;
	
	if (lpofn->lpstrCustomFilter && lpofn->nMaxCustFilter > 0)
	{
		lpstraCustomFilter = w32u_UTF8toANSI_sz(lpofn->lpstrCustomFilter, lpofn->nMaxCustFilter);
		aofn.lpstrCustomFilter = lpstraCustomFilter;
	}
	
	if (lpofn->lpstrFile && lpofn->nMaxFile > 0)
	{
		lpstraFile = w32u_UTF8toANSI_sz(lpofn->lpstrFile, lpofn->nMaxFile);
		aofn.lpstrFile = lpstraFile;
	}
	
	if (lpofn->lpstrFileTitle && lpofn->nMaxFileTitle > 0)
	{
		lpstraFileTitle = w32u_UTF8toANSI_sz(lpofn->lpstrFileTitle, lpofn->nMaxFileTitle);
		aofn.lpstrFileTitle = lpstraFileTitle;
	}
	
	// Get the filename.
	BOOL bRet = fn(&aofn);
	
	// Convert the non-constant strings from UTF-16 to UTF-8.
	if (aofn.lpstrCustomFilter && aofn.nMaxCustFilter > 0)
	{
		w32u_ANSItoUTF8_copy(lpofn->lpstrCustomFilter,
					aofn.lpstrCustomFilter,
					lpofn->nMaxCustFilter);
	}
	if (aofn.lpstrFile && aofn.nMaxFile > 0)
	{
		w32u_ANSItoUTF8_copy(lpofn->lpstrFile,
					aofn.lpstrFile,
					lpofn->nMaxFile);
	}
	if (aofn.lpstrFileTitle && aofn.nMaxFileTitle > 0)
	{
		w32u_ANSItoUTF8_copy(lpofn->lpstrFileTitle,
					aofn.lpstrFileTitle,
					lpofn->nMaxFileTitle);
	}
	
	// Free the constant strings.
	free(lpstraFilter);
	free(lpstraInitialDir);
	free(lpstraTitle);
	free(lpstraDefExt);
	
	// Free the non-constant strings.
	free(lpstraCustomFilter);
	free(lpstraFile);
	free(lpstraFileTitle);
	
	return bRet;
}


static BOOL WINAPI GetOpenFileNameUA(LPOPENFILENAMEA lpofn)
{
	return GetOpenFileNameUA_int(lpofn, (GETOPENFILENAME_PROC)&GetOpenFileNameA);
}


static BOOL WINAPI GetSaveFileNameUA(LPOPENFILENAMEA lpofn)
{
	return GetOpenFileNameUA_int(lpofn, (GETOPENFILENAME_PROC)&GetSaveFileNameA);
}


int WINAPI w32u_commdlgA_init(void)
{
	// TODO: Error handling.
	
	pGetOpenFileNameU	= &GetOpenFileNameUA;
	pGetSaveFileNameU	= &GetSaveFileNameUA;
	
	return 0;
}


int WINAPI w32u_commdlgA_end(void)
{
	// TODO: Should function pointers be NULL'd?
	return 0;
}

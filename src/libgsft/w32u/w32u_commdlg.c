/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_commdlg.c: commdlg.h translation.                                  *
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
#include "w32u_commdlg.h"

// C includes.
#include <stdlib.h>
#include <string.h>

// Initialization counter.
static int init_counter = 0;

// DLLs.
static HMODULE hComDlg32 = NULL;


MAKE_FUNCPTR(GetOpenFileNameA);
MAKE_STFUNCPTR(GetOpenFileNameW);
static BOOL WINAPI GetOpenFileNameU(LPOPENFILENAMEA lpofn)
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
		
		size_t lpstrFilter_len = (size_t)(tmp - lpofn->lpstrFilter + 2);
		if (lpstrFilter_len > 0)
		{
			lpstrwFilter = (wchar_t*)malloc(lpstrFilter_len * sizeof(wchar_t));
			MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrFilter, lpstrFilter_len,
						lpstrwFilter, lpstrFilter_len);
			wofn.lpstrFilter = lpstrwFilter;
		}
	}
	
	// Convert the rest of the constant strings.
	if (lpofn->lpstrInitialDir)
	{
		lpstrwInitialDir = w32u_mbstowcs(lpofn->lpstrInitialDir);
		wofn.lpstrInitialDir = lpstrwInitialDir;
	}
	if (lpofn->lpstrTitle)
	{
		lpstrwTitle = w32u_mbstowcs(lpofn->lpstrTitle);
		wofn.lpstrTitle = lpstrwTitle;
	}
	if (lpofn->lpstrDefExt)
	{
		lpstrwDefExt = w32u_mbstowcs(lpofn->lpstrDefExt);
		wofn.lpstrDefExt = lpstrwDefExt;
	}
	
	// Allocate buffers for the non-constant strings.
	wchar_t *lpstrwCustomFilter = NULL, *lpstrwFile = NULL;
	wchar_t *lpstrwFileTitle = NULL;
	
	if (lpofn->lpstrCustomFilter && lpofn->nMaxCustFilter > 0)
	{
		lpstrwCustomFilter = (wchar_t*)malloc(lpofn->nMaxCustFilter * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrCustomFilter, lpofn->nMaxCustFilter,
					lpstrwCustomFilter, lpofn->nMaxCustFilter * sizeof(wchar_t));
		wofn.lpstrCustomFilter = lpstrwCustomFilter;
	}
	
	if (lpofn->lpstrFile && lpofn->nMaxFile > 0)
	{
		lpstrwFile = (wchar_t*)malloc(lpofn->nMaxFile * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrFile, lpofn->nMaxFile,
					lpstrwFile, lpofn->nMaxFile * sizeof(wchar_t));
		wofn.lpstrFile = lpstrwFile;
	}
	
	if (lpofn->lpstrFileTitle && lpofn->nMaxFileTitle > 0)
	{
		lpstrwFileTitle = (wchar_t*)malloc(lpofn->nMaxFileTitle * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrFileTitle, lpofn->nMaxFileTitle,
					lpstrwFileTitle, lpofn->nMaxFileTitle * sizeof(wchar_t));
		wofn.lpstrFileTitle = lpstrwFileTitle;
	}
	
	// Get the filename.
	BOOL bRet = pGetOpenFileNameW(&wofn);
	
	// Convert the non-constant strings from UTF-16 to UTF-8.
	if (wofn.lpstrCustomFilter && wofn.nMaxCustFilter > 0)
	{
		WideCharToMultiByte(CP_UTF8, 0, wofn.lpstrCustomFilter, wofn.nMaxCustFilter,
					lpofn->lpstrCustomFilter, lpofn->nMaxCustFilter, NULL, NULL);
	}
	if (wofn.lpstrFile && wofn.nMaxFile > 0)
	{
		WideCharToMultiByte(CP_UTF8, 0, wofn.lpstrFile, wofn.nMaxFile,
					lpofn->lpstrFile, lpofn->nMaxFile, NULL, NULL);
	}
	if (wofn.lpstrFileTitle && wofn.nMaxFileTitle > 0)
	{
		WideCharToMultiByte(CP_UTF8, 0, wofn.lpstrFileTitle, wofn.nMaxFileTitle,
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


MAKE_FUNCPTR(GetSaveFileNameA);
MAKE_STFUNCPTR(GetSaveFileNameW);
static BOOL WINAPI GetSaveFileNameU(LPOPENFILENAMEA lpofn)
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
		
		size_t lpstrFilter_len = (size_t)(tmp - lpofn->lpstrFilter + 2);
		if (lpstrFilter_len > 0)
		{
			lpstrwFilter = (wchar_t*)malloc(lpstrFilter_len * sizeof(wchar_t));
			MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrFilter, lpstrFilter_len,
						lpstrwFilter, lpstrFilter_len);
			wofn.lpstrFilter = lpstrwFilter;
		}
	}
	
	// Convert the rest of the constant strings.
	if (lpofn->lpstrInitialDir)
	{
		lpstrwInitialDir = w32u_mbstowcs(lpofn->lpstrInitialDir);
		wofn.lpstrInitialDir = lpstrwInitialDir;
	}
	if (lpofn->lpstrTitle)
	{
		lpstrwTitle = w32u_mbstowcs(lpofn->lpstrTitle);
		wofn.lpstrTitle = lpstrwTitle;
	}
	if (lpofn->lpstrDefExt)
	{
		lpstrwDefExt = w32u_mbstowcs(lpofn->lpstrDefExt);
		wofn.lpstrDefExt = lpstrwDefExt;
	}
	
	// Allocate buffers for the non-constant strings.
	wchar_t *lpstrwCustomFilter = NULL, *lpstrwFile = NULL;
	wchar_t *lpstrwFileTitle = NULL;
	
	if (lpofn->lpstrCustomFilter && lpofn->nMaxCustFilter > 0)
	{
		lpstrwCustomFilter = (wchar_t*)malloc(lpofn->nMaxCustFilter * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrCustomFilter, lpofn->nMaxCustFilter,
					lpstrwCustomFilter, lpofn->nMaxCustFilter * sizeof(wchar_t));
		wofn.lpstrCustomFilter = lpstrwCustomFilter;
	}
	
	if (lpofn->lpstrFile && lpofn->nMaxFile > 0)
	{
		lpstrwFile = (wchar_t*)malloc(lpofn->nMaxFile * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrFile, lpofn->nMaxFile,
					lpstrwFile, lpofn->nMaxFile * sizeof(wchar_t));
		wofn.lpstrFile = lpstrwFile;
	}
	
	if (lpofn->lpstrFileTitle && lpofn->nMaxFileTitle > 0)
	{
		lpstrwFileTitle = (wchar_t*)malloc(lpofn->nMaxFileTitle * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, lpofn->lpstrFileTitle, lpofn->nMaxFileTitle,
					lpstrwFileTitle, lpofn->nMaxFileTitle * sizeof(wchar_t));
		wofn.lpstrFileTitle = lpstrwFileTitle;
	}
	
	// Get the filename.
	BOOL bRet = pGetSaveFileNameW(&wofn);
	
	// Convert the non-constant strings from UTF-16 to UTF-8.
	if (wofn.lpstrCustomFilter && wofn.nMaxCustFilter > 0)
	{
		WideCharToMultiByte(CP_UTF8, 0, wofn.lpstrCustomFilter, wofn.nMaxCustFilter,
					lpofn->lpstrCustomFilter, lpofn->nMaxCustFilter, NULL, NULL);
	}
	if (wofn.lpstrFile && wofn.nMaxFile > 0)
	{
		WideCharToMultiByte(CP_UTF8, 0, wofn.lpstrFile, wofn.nMaxFile,
					lpofn->lpstrFile, lpofn->nMaxFile, NULL, NULL);
	}
	if (wofn.lpstrFileTitle && wofn.nMaxFileTitle > 0)
	{
		WideCharToMultiByte(CP_UTF8, 0, wofn.lpstrFileTitle, wofn.nMaxFileTitle,
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


int WINAPI w32u_commdlg_init(void)
{
	if (init_counter++ != 0)
		return 0;
	
	// TODO: Error handling.
	hComDlg32 = LoadLibrary("comdlg32.dll");
	
	InitFuncPtrsU(hComDlg32, "GetOpenFileName", pGetOpenFileNameW, pGetOpenFileNameA, GetOpenFileNameU);
	InitFuncPtrsU(hComDlg32, "GetSaveFileName", pGetSaveFileNameW, pGetSaveFileNameA, GetSaveFileNameU);
	
	return 0;
}


int WINAPI w32u_commdlg_end(void)
{
	if (init_counter <= 0)
		return 0;
	
	init_counter--;
	if (init_counter > 0)
		return 0;
	
	FreeLibrary(hComDlg32);
	hComDlg32 = NULL;
	
	// TODO: Should function pointers be NULL'd?
	return 0;
}

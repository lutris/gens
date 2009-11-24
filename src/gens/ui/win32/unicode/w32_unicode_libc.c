/***************************************************************************
 * Gens: (Win32) Unicode Translation Layer. (shellapi.h)                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#include "w32_unicode.h"
#include "w32_unicode_priv.h"
#include "w32_unicode_libc.h"

// C includes.
#include <stdlib.h>

// DLLs.
static HMODULE hMsvcrt = NULL;


MAKE_FUNCPTR(access);
MAKE_STFUNCPTR(_waccess);
static int Uaccess(const char *path, int mode)
{
	if (!path)
	{
		// String not specified. Don't bother converting anything.
		return p_waccess((const wchar_t*)path, mode);
	}
	
	// Convert lpNewItem from UTF-8 to UTF-16.
	int wpath_len;
	wchar_t *wpath = NULL;
	
	wpath_len = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
	wpath_len *= sizeof(wchar_t);
	wpath = (wchar_t*)malloc(wpath_len);
	MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, wpath_len);
	
	UINT uRet = p_waccess(wpath, mode);
	free(wpath);
	return uRet;
}


MAKE_FUNCPTR(fopen);
MAKE_STFUNCPTR(_wfopen);
static FILE* Ufopen(const char *path, const char *mode)
{
	// Convert path and mode from UTF-8 to UTF-16.
	int wpath_len, wmode_len;
	wchar_t *wpath = NULL, *wmode = NULL;
	
	if (path)
	{
		wpath_len = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
		wpath_len *= sizeof(wchar_t);
		wpath = (wchar_t*)malloc(wpath_len);
		MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, wpath_len);
	}
	
	if (mode)
	{
		wmode_len = MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);
		wmode_len *= sizeof(wchar_t);
		wmode = (wchar_t*)malloc(wmode_len);
		MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, wmode_len);
	}
	
	FILE *fRet = p_wfopen(wpath, wmode);
	free(wpath);
	free(wmode);
	return fRet;
}


int WINAPI w32_unicode_libc_init(void)
{
	// TODO: Error handling.
	hMsvcrt = LoadLibrary("msvcrt.dll");
	
	InitFuncPtrsU_libc(hMsvcrt, "access", "_waccess", p_waccess, paccess, Uaccess);
	InitFuncPtrsU_libc(hMsvcrt, "fopen", "_wfopen", p_wfopen, pfopen, Ufopen);
	
	return 0;
}

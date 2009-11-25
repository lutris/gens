/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_libc.c: libc translation.                                          *
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
#include "w32u_libc.h"

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
	wchar_t *wpath = w32u_mbstowcs(path);
	
	UINT uRet = p_waccess(wpath, mode);
	free(wpath);
	return uRet;
}


MAKE_FUNCPTR(fopen);
MAKE_STFUNCPTR(_wfopen);
static FILE* Ufopen(const char *path, const char *mode)
{
	// Convert path and mode from UTF-8 to UTF-16.
	wchar_t *wpath = NULL, *wmode = NULL;
	
	if (path)
		wpath = w32u_mbstowcs(path);
	
	if (mode)
		wmode = w32u_mbstowcs(mode);
	
	FILE *fRet = p_wfopen(wpath, wmode);
	free(wpath);
	free(wmode);
	return fRet;
}


int WINAPI w32u_libc_init(void)
{
	// TODO: Error handling.
	hMsvcrt = LoadLibrary("msvcrt.dll");
	
	InitFuncPtrsU_libc(hMsvcrt, "access", "_waccess", p_waccess, paccess, Uaccess);
	InitFuncPtrsU_libc(hMsvcrt, "fopen", "_wfopen", p_wfopen, pfopen, Ufopen);
	
	return 0;
}

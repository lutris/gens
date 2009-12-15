/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_libcW.c: libc translation. (Unicode version)                       *
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

#include "w32u_libcW.h"
#include "w32u_libc.h"
#include "w32u_charset.h"

// C includes.
#include <stdlib.h>


static int accessUW(const char *path, int mode)
{
	if (!path)
	{
		// String not specified. Don't bother converting anything.
		return _waccess((const wchar_t*)path, mode);
	}
	
	// Convert path from UTF-8 to UTF-16.
	wchar_t *wpath = w32u_UTF8toUTF16(path);
	UINT uRet = _waccess(wpath, mode);
	free(wpath);
	return uRet;
}


static FILE* fopenUW(const char *path, const char *mode)
{
	// Convert path and mode from UTF-8 to UTF-16.
	wchar_t *wpath = NULL, *wmode = NULL;
	
	if (path)
		wpath = w32u_UTF8toUTF16(path);
	
	if (mode)
		wmode = w32u_UTF8toUTF16(mode);
	
	FILE *fRet = _wfopen(wpath, wmode);
	free(wpath);
	free(wmode);
	return fRet;
}


static int statUW(const char *path, struct stat *buf)
{
	if (!path)
	{
		// String not specified. Don't bother converting anything.
		return _wstat((const wchar_t*)path, buf);
	}
	
	// Convert path from UTF-8 to UTF-16.
	wchar_t *wpath = w32u_UTF8toUTF16(path);
	int ret = _wstat(wpath, buf);
	free(wpath);
	return ret;
}


void WINAPI w32u_libcW_init(void)
{
	paccess		= &accessUW;
	pfopen		= &fopenUW;
	pstat		= &statUW;
	
	p_wcsicmp	= &_wcsicmp;
}

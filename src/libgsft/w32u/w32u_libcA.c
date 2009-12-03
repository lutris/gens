/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_libcA.c: libc translation. (ANSI version)                          *
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
#include "w32u_libcA.h"

#include "w32u_libc.h"

// C includes.
#include <stdlib.h>

// Make sure fopen isn't defined as a macro.
#ifdef fopen
#undef fopen
#endif


static int accessUA(const char *path, int mode)
{
	if (!path)
	{
		// String not specified. Don't bother converting anything.
		return _access(path, mode);
	}
	
	// Convert path from UTF-8 to ANSI.
	char *apath = w32u_UTF8toANSI(path);
	UINT uRet = _access(apath, mode);
	free(apath);
	return uRet;
}


static FILE* fopenUA(const char *path, const char *mode)
{
	// Convert path from UTF-8 to ANSI.
	// mode doesn't need any conversion, since it's ASCII.
	
	if (!path)
	{
		// String not specified. Don't bother converting anything.
		return fopen(path, mode);
	}
	
	char *apath = w32u_UTF8toANSI(path);
	FILE *fRet = fopen(apath, mode);
	free(apath);
	return fRet;
}


int WINAPI w32u_libcA_init(void)
{
	// TODO: Error handling.
	
	paccess		= &accessUA;
	pfopen		= &fopenUA;
	p_wcsicmp	= &_wcsicmp;
	
	return 0;
}


int WINAPI w32u_libcA_end(void)
{
	// TODO: Should the function pointers be NULL'd?
	return 0;
}

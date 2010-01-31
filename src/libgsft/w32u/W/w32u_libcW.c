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
#include <wchar.h>	/* contains _wmkdir() */


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
		return _wstat((const wchar_t*)path, (struct _stat*)buf);
	}
	
	// Convert path from UTF-8 to UTF-16.
	wchar_t *wpath = w32u_UTF8toUTF16(path);
	int ret = _wstat(wpath, (struct _stat*)buf);
	free(wpath);
	return ret;
}


static int mkdirUW(const char *dirname)
{
	if (!dirname)
	{
		// String not specified. Don't bother converting anything.
		return _wmkdir((const wchar_t*)dirname);
	}
	
	// Convert dirname from UTF-8 to UTF-16.
	wchar_t *wdirname = w32u_UTF8toUTF16(dirname);
	int ret = _wmkdir(wdirname);
	free(wdirname);
	return ret;
}


static int unlinkUW(const char *filename)
{
	if (!filename)
	{
		// String not specified. Don't bother converting anything.
		return _wunlink((const wchar_t*)filename);
	}
	
	// Convert filename from UTF-8 to UTF-16.
	wchar_t *wfilename = w32u_UTF8toUTF16(filename);
	int ret = _wunlink(wfilename);
	free(wfilename);
	return ret;
}


static DIR *opendirUW(const char *name)
{
	if (!name)
	{
		// String not specified. Don't bother converting anything.
		return (DIR*)_wopendir((const wchar_t*)name);
	}
	
	// Convert name from UTF-8 to UTF-16.
	wchar_t *wname = w32u_UTF8toUTF16(name);
	_WDIR *ret = _wopendir(wname);
	free(wname);
	
	// NOTE: opendir() returns a struct of type DIR*.
	// _wopendir() returns a struct of type _WDIR*.
	// DO NOT use the return value except for readdir()!
	return (DIR*)ret;
}


static struct dirent *readdirUW(DIR *dirp)
{
	if (!dirp)
	{
		// Directory not specified. Don't bother converting anything.
		return (struct dirent*)_wreaddir((_WDIR*)dirp);
	}
	
	// Internal buffer. This buffer is returned.
	// Note that readdir() is not reentrant, so this is fine.
	static struct dirent dirUW;
	
	// Read the directory.
	struct _wdirent *dirW = _wreaddir((_WDIR*)dirp);
	if (!dirW)
		return NULL;
	
	// Copy the numeric fields.
	dirUW.d_ino	= dirW->d_ino;
	dirUW.d_reclen	= dirW->d_reclen;
	
	// Convert the filename from UTF-16 to UTF-8.
	w32u_UTF16toUTF8_copy(dirUW.d_name, dirW->d_name, sizeof(dirUW.d_name));
	dirUW.d_namlen = strlen(dirUW.d_name);
	
	// Return a pointer to dirUW.
	return &dirUW;
}


void WINAPI w32u_libcW_init(void)
{
	paccess		= &accessUW;
	pfopen		= &fopenUW;
	pstat		= &statUW;
	pmkdir		= &mkdirUW;
	punlink		= &unlinkUW;
	popendir	= &opendirUW;
	preaddir	= &readdirUW;
	
	p_wcsicmp	= &_wcsicmp;
}

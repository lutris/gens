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

#include "w32u_libcA.h"
#include "w32u_libc.h"
#include "w32u_charset.h"

// C includes.
#include <stdlib.h>

// Make sure fopen(), stat(), opendir(), and readdir() aren't defined as macros.
#ifdef fopen
#undef fopen
#endif
#ifdef stat
#undef stat
#endif
#ifdef opendir
#undef opendir
#endif
#ifdef readdir
#undef readdir
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


static int statUA(const char *path, struct stat *buf)
{
	if (!path)
	{
		// String not specified. Don't bother converting anything.
		return stat(path, buf);
	}
	
	// Convert path from UTF-8 to ANSI.
	char *apath = w32u_UTF8toANSI(path);
	int ret = stat(apath, buf);
	free(apath);
	return ret;
}


static int mkdirUA(const char *dirname)
{
	if (!dirname)
	{
		// String not specified. Don't bother converting anything.
		return _mkdir(dirname);
	}
	
	// Convert dirname from UTF-8 to ANSI.
	char *adirname = w32u_UTF8toANSI(dirname);
	int ret = _mkdir(adirname);
	free(adirname);
	return ret;
}


static int unlinkUA(const char *filename)
{
	if (!filename)
	{
		// String not specified. Don't bother converting anything.
		return _unlink(filename);
	}
	
	// Convert filename from UTF-8 to ANSI.
	char *afilename = w32u_UTF8toANSI(filename);
	int ret = _unlink(afilename);
	free(afilename);
	return ret;
}


static DIR *opendirUA(const char *name)
{
	if (!name)
	{
		// String not specified. Don't bother converting anything.
		return opendir(name);
	}
	
	// Convert name from UTF-8 to ANSI.
	char *aname = w32u_UTF8toANSI(name);
	DIR *ret = opendir(aname);
	free(aname);
	return ret;
}


static struct dirent *readdirUA(DIR *dirp)
{
	if (!dirp)
	{
		// Directory not specified. Don't bother converting anything.
		return readdir(dirp);
	}
	
	// Internal buffer. This buffer is returned.
	// Note that readdir() is not reentrant, so this is fine.
	static struct dirent dirUA;
	
	// Read the directory.
	struct dirent *dirA = readdir(dirp);
	if (!dirA)
		return NULL;
	
	// Copy the numeric fields.
	dirUA.d_ino	= dirA->d_ino;
	dirUA.d_reclen	= dirA->d_reclen;
	
	// Convert the filename from ANSI to UTF-8.
	w32u_ANSItoUTF8_copy(dirUA.d_name, dirA->d_name, sizeof(dirUA.d_name));
	dirUA.d_namlen = strlen(dirUA.d_name);
	
	// Return a pointer to dirUA.
	return &dirUA;
}


void WINAPI w32u_libcA_init(void)
{
	paccess		= &accessUA;
	pfopen		= &fopenUA;
	pstat		= &statUA;
	pmkdir		= &mkdirUA;
	punlink		= &unlinkUA;
	popendir	= &opendirUA;
	preaddir	= &readdirUA;
	
	p_wcsicmp	= &_wcsicmp;
}

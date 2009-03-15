/***************************************************************************
 * Gens: Dynamic Library Loader.                                           *
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

#ifndef GENS_LD_H
#define GENS_LD_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef GENS_OS_WIN32

/* UNIX system. */
#include <dlfcn.h>
#define GENS_DL_EXT ".so"

#define gens_dlopen(filename)		dlopen(filename, RTLD_NOW | RTLD_LOCAL)
#define gens_dlclose(handle)		dlclose(handle)

static inline void* gens_dlsym(void *handle, const char *symbol)
{
	dlerror();
	return dlsym(handle, symbol);
}

static inline const char* gens_dlerror(void)
{
	const char *err = dlerror();
	if (!err)
		return "Unknown error.";
	
	/* Remove the pathname from the error. */
	/* NOTE: This isn't perfect, and may result in garbage  */
	/* for some filenames if the plugin author is retarded. */
	const char *endofpath = strstr(err, GENS_DL_EXT ": ");
	if (!endofpath)
		return err;
	
	return (endofpath + sizeof(GENS_DL_EXT) + 1);
}

#define gens_dlerror_str_free(errstr)

#else

/* Win32 system. */
#include <windows.h>
#include <tchar.h>
#define GENS_DL_EXT ".dll"

#define gens_dlopen(filename)		(void*)(LoadLibrary(filename))
#define gens_dlclose(handle)		FreeLibrary((HMODULE)(handle))

#define gens_dlsym(handle, symbol)	(void*)(GetProcAddress((HMODULE)(handle), (symbol)))

#ifdef __cplusplus
extern "C" {
#endif

static inline const char* gens_dlerror(void)
{
	LPTSTR lpBuf;
	
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpBuf,
		0, NULL);
	
	return lpBuf;
}

#define gens_dlerror_str_free(errstr)	LocalFree((void*)(errstr))

#ifdef __cplusplus
}
#endif

#endif

#endif /* GENS_LD_H */

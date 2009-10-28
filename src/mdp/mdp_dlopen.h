/***************************************************************************
 * MDP: Mega Drive Plugins - Dynamic Library Loader.                       *
 *                                                                         *
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

#ifndef MDP_DLOPEN_H
#define MDP_DLOPEN_H

#if !defined(_WIN32)

/* Linux/UNIX system. */
#include <dlfcn.h>
#define MDP_DLOPEN_EXT ".so"

#define mdp_dlopen(filename)		dlopen(filename, RTLD_NOW | RTLD_LOCAL)
#define mdp_dlopen_lazy(filename)	dlopen(filename, RTLD_LAZY | RTLD_LOCAL)
#define mdp_dlclose(handle)		dlclose(handle)

#ifdef __cplusplus
extern "C" {
#endif

static inline void* mdp_dlsym(void *handle, const char *symbol)
{
	dlerror();
	return dlsym(handle, symbol);
}

static inline const char* mdp_dlerror(void)
{
	const char *err = dlerror();
	if (!err)
		return "Unknown error.";
	
	/* Remove the pathname from the error. */
	/* NOTE: This isn't perfect, and may result in garbage  */
	/* for some filenames if the plugin author is retarded. */
	const char *endofpath = strstr(err, MDP_DLOPEN_EXT ": ");
	if (!endofpath)
		return err;
	
	return (endofpath + sizeof(MDP_DLOPEN_EXT) + 1);
}

#ifdef __cplusplus
}
#endif

#define mdp_dlerror_str_free(errstr)

#else /* defined(_WIN32) */

/* Win32 system. */
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <tchar.h>
#define MDP_DLOPEN_EXT ".dll"

#define mdp_dlopen(filename)		(void*)(LoadLibrary(filename))
#define mdp_dlopen_lazy(filename)	mdp_dlopen(filename)
#define mdp_dlclose(handle)		FreeLibrary((HMODULE)(handle))

#define mdp_dlsym(handle, symbol)	(void*)(GetProcAddress((HMODULE)(handle), (symbol)))

#ifdef __cplusplus
extern "C" {
#endif

static inline LPCTSTR mdp_dlerror(void)
{
	LPTSTR lpBuf;
	int len, i;
	
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpBuf,
		0, NULL);
	
	// Eliminate any newlines or carriage returns at the end of the message.
	len = _tcslen(lpBuf);
	for (i = len - 1; i >= 0; i--)
	{
		if (lpBuf[i] == '\r' || lpBuf[i] == '\n')
		{
			lpBuf[i] = 0x00;
			break;
		}
	}
	
	return lpBuf;
}

#define mdp_dlerror_str_free(errstr)	LocalFree((void*)(errstr))

#ifdef __cplusplus
}
#endif

#endif /* defined(_WIN32) */

#endif /* MDP_DLOPEN_H */

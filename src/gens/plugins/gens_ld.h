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
#define gens_dlsym(handle, symbol)	dlsym(handle, symbol)
#define gens_dlclose(handle)		dlclose(handle)

#else

/* Win32 system. */
#include <windows.h>
#define GENS_DL_EXT ".dll"

#define gens_dlopen(filename)		(void*)(LoadLibrary(filename))
#define gens_dlsym(handle, symbol)	(void*)(GetProcAddress((HMODULE)(handle), (symbol)))
#define gens_dlclose(handle)		FreeLibrary((HMODULE)(handle))

#endif

#endif /* GENS_LD_H */

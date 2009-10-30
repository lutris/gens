/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_png_dll.c: PNG dlopen() functions.                                 *
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

#include "gsft_png_dll.h"

// TODO: Log to console.
#define LOG_MSG_ONCE(channel, level, msg, ...)

// Simple reimplementation of mdp_dlopen.h.
#if !defined(_WIN32)

/* Linux / UNIX */
#include <dlfcn.h>
#define gsft_png_dlopen(filename)	dlopen(filename, RTLD_NOW | RTLD_LOCAL)
#define gsft_png_dlopen_lazy(filename)	dlopen(filename, RTLD_LAZY | RTLD_LOCAL)
#define gsft_png_dlclose(handle)	dlclose(handle)
#define gsft_png_dlsym(handle, symbol)	dlsym(handle, symbol)

#else

/* Win32 */
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#define gsft_png_dlopen(filename)	(void*)(LoadLibrary(filename))
#define gsft_png_dlopen_lazy(filename)	gsft_png_dlopen(filename)
#define gsft_png_dlclose(handle)	FreeLibrary((HMODULE)(handle))
#define gsft_png_dlsym(handle, symbol)	(void*)(GetProcAddress((HMODULE)(handle), (symbol)))

#endif

// libpng DLL handle.
static void *png_dll = NULL;

#define MAKE_FUNCPTR(f) typeof(f) * p##f = NULL
MAKE_FUNCPTR(png_set_read_fn);
MAKE_FUNCPTR(png_get_valid);
MAKE_FUNCPTR(png_create_read_struct);
MAKE_FUNCPTR(png_destroy_read_struct);
MAKE_FUNCPTR(png_create_info_struct);
MAKE_FUNCPTR(png_init_io);
MAKE_FUNCPTR(png_set_filter);
MAKE_FUNCPTR(png_write_rows);
MAKE_FUNCPTR(png_read_update_info);
MAKE_FUNCPTR(png_set_compression_level);
MAKE_FUNCPTR(png_set_tRNS_to_alpha);
MAKE_FUNCPTR(png_set_swap);
MAKE_FUNCPTR(png_set_filler);
MAKE_FUNCPTR(png_set_strip_16);
MAKE_FUNCPTR(png_read_image);
MAKE_FUNCPTR(png_write_end);
MAKE_FUNCPTR(png_set_bgr);
MAKE_FUNCPTR(png_read_info);
MAKE_FUNCPTR(png_destroy_write_struct);
MAKE_FUNCPTR(png_set_gray_to_rgb);
MAKE_FUNCPTR(png_get_IHDR);
MAKE_FUNCPTR(png_write_row);
MAKE_FUNCPTR(png_get_io_ptr);
MAKE_FUNCPTR(png_create_write_struct);
MAKE_FUNCPTR(png_set_invert_alpha);
MAKE_FUNCPTR(png_write_info);
MAKE_FUNCPTR(png_set_IHDR);
MAKE_FUNCPTR(png_set_palette_to_rgb);


#define DLL_LOAD_SYMBOL(handle, symname) \
do { \
	p##symname = (typeof(p##symname))gsft_png_dlsym(handle, #symname); \
	if (!p##symname) \
	{ \
		LOG_MSG_ONCE(gens, LOG_MSG_LEVEL_ERROR, \
			     "%s is missing symbol '%s'. Disabling PNG support.", \
			     png_dllname, #symname); \
		gsft_png_dlclose(handle); \
		handle = NULL; \
		return -4; \
	} \
} while (0)


/**
 * gsft_png_dll_init(): Initialize libpng dynamic linking.
 * @return 0 on success; non-zero on error.
 */
int gsft_png_dll_init(void)
{
#ifdef _WIN32
	// TODO: Verify this!
	static const char png_dllname[] = "libpng.dll";
#else
	static const char png_dllname[] = "libpng12.so";
#endif
	
	if (png_dll)
	{
		// libpng is already initialized.
		return 0;
	}
	
	// Attempt to dlopen() libpng.
	png_dll = gsft_png_dlopen_lazy(png_dllname);
	if (!png_dll)
	{
		// libpng not found.
		LOG_MSG_ONCE(gens, LOG_MSG_LEVEL_ERROR,
			     "%s not found. PNG images will not be usable.", png_dllname);
		return -1;
	}
	
	// libpng dlopen()'d.
	
	// Check the library version.
	MAKE_FUNCPTR(png_access_version_number);
	ppng_access_version_number = (typeof(ppng_access_version_number))gsft_png_dlsym(png_dll, "png_access_version_number");
	if (!ppng_access_version_number)
	{
		// libpng doesn't have png_access_version_number().
		// That means it's way too old.
		LOG_MSG_ONCE(gens, LOG_MSG_LEVEL_ERROR,
				"%s is missing symbol '%s'. Disabling PNG support.",
				png_dllname, "png_access_version_number");
		gsft_png_dlclose(png_dll);
		return -2;
	}
	
	png_uint_32 ver = ppng_access_version_number();
	if ((ver / 100) != 102)
	{
		// libpng is not 1.2.x.
		const int ver_major = ver / 10000;
		const int ver_minor = (ver / 100) % 100;
		const int ver_revision = ver % 100;
		
		LOG_MSG_ONCE(gens, LOG_MSG_LEVEL_ERROR,
				"%s is version %d.%d.%d; should be 1.2.x. Disabling PNG support.",
				png_dllname, ver_major, ver_minor, ver_revision);
		gsft_png_dlclose(png_dll);
		return -3;
	}
	
	// Load the symbols.
	DLL_LOAD_SYMBOL(png_dll, png_set_read_fn);
	DLL_LOAD_SYMBOL(png_dll, png_get_valid);
	DLL_LOAD_SYMBOL(png_dll, png_create_read_struct);
	DLL_LOAD_SYMBOL(png_dll, png_destroy_read_struct);
	DLL_LOAD_SYMBOL(png_dll, png_create_info_struct);
	DLL_LOAD_SYMBOL(png_dll, png_init_io);
	DLL_LOAD_SYMBOL(png_dll, png_set_filter);
	DLL_LOAD_SYMBOL(png_dll, png_write_rows);
	DLL_LOAD_SYMBOL(png_dll, png_read_update_info);
	DLL_LOAD_SYMBOL(png_dll, png_set_compression_level);
	DLL_LOAD_SYMBOL(png_dll, png_set_tRNS_to_alpha);
	DLL_LOAD_SYMBOL(png_dll, png_set_swap);
	DLL_LOAD_SYMBOL(png_dll, png_set_filler);
	DLL_LOAD_SYMBOL(png_dll, png_set_strip_16);
	DLL_LOAD_SYMBOL(png_dll, png_read_image);
	DLL_LOAD_SYMBOL(png_dll, png_write_end);
	DLL_LOAD_SYMBOL(png_dll, png_set_bgr);
	DLL_LOAD_SYMBOL(png_dll, png_read_info);
	DLL_LOAD_SYMBOL(png_dll, png_destroy_write_struct);
	DLL_LOAD_SYMBOL(png_dll, png_set_gray_to_rgb);
	DLL_LOAD_SYMBOL(png_dll, png_get_IHDR);
	DLL_LOAD_SYMBOL(png_dll, png_write_row);
	DLL_LOAD_SYMBOL(png_dll, png_get_io_ptr);
	DLL_LOAD_SYMBOL(png_dll, png_create_write_struct);
	DLL_LOAD_SYMBOL(png_dll, png_set_invert_alpha);
	DLL_LOAD_SYMBOL(png_dll, png_write_info);
	DLL_LOAD_SYMBOL(png_dll, png_set_IHDR);
	DLL_LOAD_SYMBOL(png_dll, png_set_palette_to_rgb);
	
	// libpng initialized.
	LOG_MSG_ONCE(gens, LOG_MSG_LEVEL_INFO,
			"%s version %d.%d.%d initialized.",
			png_dllname, ver_major, ver_minor, ver_revision);
	return 0;
}


/**
 * gsft_png_dll_end(): Shut down libpng.
 */
void gsft_png_dll_end(void)
{
	if (!png_dll)
		return;
	
	gsft_png_dlclose(png_dll);
	png_dll = NULL;
}

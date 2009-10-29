/***************************************************************************
 * Gens: Dynamic Linking for libpng.                                       *
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

#include "dll_png.h"
#include "mdp/mdp_dlopen.h"
#include "macros/log_msg.h"

// libpng DLL handle.
static void *dll_png = NULL;

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
	p##symname = (typeof(p##symname))mdp_dlsym(handle, #symname); \
	if (!p##symname) \
	{ \
		LOG_MSG_ONCE(gens, LOG_MSG_LEVEL_ERROR, \
			     "%s is missing symbol '%s'. Disabling PNG support.", \
			     png_dllname, #symname); \
		mdp_dlclose(handle); \
		handle = NULL; \
		return -4; \
	} \
} while (0)


/**
 * dll_png_init(): Initialize libpng dynamic linking.
 * @return 0 on success; non-zero on error.
 */
int dll_png_init(void)
{
#ifdef _WIN32
	// TODO: Verify this!
	static const char png_dllname[] = "libpng.dll";
#else
	static const char png_dllname[] = "libpng12.so";
#endif
	
	if (dll_png)
	{
		// libpng is already initialized.
		return 0;
	}
	
	// Attempt to dlopen() libpng.
	dll_png = mdp_dlopen_lazy(png_dllname);
	if (!dll_png)
	{
		// libpng not found.
		LOG_MSG_ONCE(gens, LOG_MSG_LEVEL_ERROR,
			     "%s not found. PNG images will not be usable.", png_dllname);
		return -1;
	}
	
	// libpng dlopen()'d.
	
	// Check the library version.
	MAKE_FUNCPTR(png_access_version_number);
	ppng_access_version_number = (typeof(ppng_access_version_number))mdp_dlsym(dll_png, "png_access_version_number");
	if (!ppng_access_version_number)
	{
		// libpng doesn't have png_access_version_number().
		// That means it's way too old.
		LOG_MSG_ONCE(gens, LOG_MSG_LEVEL_ERROR,
				"%s is missing symbol '%s'. Disabling PNG support.",
				png_dllname, "png_access_version_number");
		mdp_dlclose(dll_png);
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
		mdp_dlclose(dll_png);
		return -3;
	}
	
	// Load the symbols.
	DLL_LOAD_SYMBOL(dll_png, png_set_read_fn);
	DLL_LOAD_SYMBOL(dll_png, png_get_valid);
	DLL_LOAD_SYMBOL(dll_png, png_create_read_struct);
	DLL_LOAD_SYMBOL(dll_png, png_destroy_read_struct);
	DLL_LOAD_SYMBOL(dll_png, png_create_info_struct);
	DLL_LOAD_SYMBOL(dll_png, png_init_io);
	DLL_LOAD_SYMBOL(dll_png, png_set_filter);
	DLL_LOAD_SYMBOL(dll_png, png_write_rows);
	DLL_LOAD_SYMBOL(dll_png, png_read_update_info);
	DLL_LOAD_SYMBOL(dll_png, png_set_compression_level);
	DLL_LOAD_SYMBOL(dll_png, png_set_tRNS_to_alpha);
	DLL_LOAD_SYMBOL(dll_png, png_set_swap);
	DLL_LOAD_SYMBOL(dll_png, png_set_filler);
	DLL_LOAD_SYMBOL(dll_png, png_set_strip_16);
	DLL_LOAD_SYMBOL(dll_png, png_read_image);
	DLL_LOAD_SYMBOL(dll_png, png_write_end);
	DLL_LOAD_SYMBOL(dll_png, png_set_bgr);
	DLL_LOAD_SYMBOL(dll_png, png_read_info);
	DLL_LOAD_SYMBOL(dll_png, png_destroy_write_struct);
	DLL_LOAD_SYMBOL(dll_png, png_set_gray_to_rgb);
	DLL_LOAD_SYMBOL(dll_png, png_get_IHDR);
	DLL_LOAD_SYMBOL(dll_png, png_write_row);
	DLL_LOAD_SYMBOL(dll_png, png_get_io_ptr);
	DLL_LOAD_SYMBOL(dll_png, png_create_write_struct);
	DLL_LOAD_SYMBOL(dll_png, png_set_invert_alpha);
	DLL_LOAD_SYMBOL(dll_png, png_write_info);
	DLL_LOAD_SYMBOL(dll_png, png_set_IHDR);
	DLL_LOAD_SYMBOL(dll_png, png_set_palette_to_rgb);
	
	// libpng initialized.
	LOG_MSG_ONCE(gens, LOG_MSG_LEVEL_INFO,
			"%s version %d.%d.%d initialized.",
			png_dllname, ver_major, ver_minor, ver_revision);
	return 0;
}


/**
 * dll_png_end(): Shut down libpng.
 */
void dll_png_end(void)
{
	if (!dll_png)
		return;
	
	mdp_dlclose(dll_png);
	dll_png = NULL;
}

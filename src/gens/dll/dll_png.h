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

#ifndef GENS_DLL_PNG_H
#define GENS_DLL_PNG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_PNG

#include <png.h>

#ifdef GENS_PNG_INTERNAL

// libpng is linked statically into the Gens/GS executable.
// TODO: Define the PNG function pointers to match the function names.

#else

// libpng is dynamically loaded at runtime.

#define MAKE_EXTFUNCPTR(f) extern typeof(f) * p##f
#define MAKE_FUNCPTR(f) typeof(f) * p##f = NULL

#ifdef __cplusplus
extern "C" {
#endif

int dll_png_init(void);
void dll_png_end(void);

MAKE_EXTFUNCPTR(png_set_read_fn);
MAKE_EXTFUNCPTR(png_get_valid);
MAKE_EXTFUNCPTR(png_create_read_struct);
MAKE_EXTFUNCPTR(png_destroy_read_struct);
MAKE_EXTFUNCPTR(png_create_info_struct);
MAKE_EXTFUNCPTR(png_init_io);
MAKE_EXTFUNCPTR(png_set_filter);
MAKE_EXTFUNCPTR(png_write_rows);
MAKE_EXTFUNCPTR(png_read_update_info);
MAKE_EXTFUNCPTR(png_set_compression_level);
MAKE_EXTFUNCPTR(png_set_tRNS_to_alpha);
MAKE_EXTFUNCPTR(png_set_swap);
MAKE_EXTFUNCPTR(png_set_filler);
MAKE_EXTFUNCPTR(png_set_strip_16);
MAKE_EXTFUNCPTR(png_read_image);
MAKE_EXTFUNCPTR(png_write_end);
MAKE_EXTFUNCPTR(png_set_bgr);
MAKE_EXTFUNCPTR(png_read_info);
MAKE_EXTFUNCPTR(png_destroy_write_struct);
MAKE_EXTFUNCPTR(png_set_gray_to_rgb);
MAKE_EXTFUNCPTR(png_get_IHDR);
MAKE_EXTFUNCPTR(png_write_row);
MAKE_EXTFUNCPTR(png_get_io_ptr);
MAKE_EXTFUNCPTR(png_create_write_struct);
MAKE_EXTFUNCPTR(png_set_invert_alpha);
MAKE_EXTFUNCPTR(png_write_info);
MAKE_EXTFUNCPTR(png_set_IHDR);
MAKE_EXTFUNCPTR(png_set_palette_to_rgb);

#ifdef __cplusplus
}
#endif

#endif /* GENS_PNG_INTERNAL */

#endif /* GENS_PNG */

#endif /* GENS_DLL_PNG_H */

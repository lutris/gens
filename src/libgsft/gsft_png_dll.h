/***************************************************************************
 * libgsft: Common functions.                                              *
 * gsft_png_dll.h: PNG dlopen() functions.                                 *
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

#ifndef __GSFT_PNG_DLL_H
#define __GSFT_PNG_DLL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_PNG

#include <png.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(GENS_PNG_INTERNAL) || !defined(GENS_PNG_DLOPEN)

// Don't use dlopen() to open libpng.
// This is either because libpng is linked statically into the Gens/GS executable,
// or the operating system has libpng as a system library.
static inline int gsft_png_dll_init(void) { return 0; }
#define gsft_png_dll_end() do { } while (0)

#define ppng_set_read_fn		png_set_read_fn
#define ppng_get_valid			png_get_valid
#define ppng_create_read_struct		png_create_read_struct
#define ppng_destroy_read_struct	png_destroy_read_struct
#define ppng_create_info_struct		png_create_info_struct
#define ppng_init_io			png_init_io
#define ppng_set_filter			png_set_filter
#define ppng_write_rows			png_write_rows
#define ppng_read_update_info		png_read_update_info
#define ppng_set_compression_level	png_set_compression_level
#define ppng_set_tRNS_to_alpha		png_set_tRNS_to_alpha
#define ppng_set_swap			png_set_swap
#define ppng_set_filler			png_set_filler
#define ppng_set_strip_16		png_set_strip_16
#define ppng_read_image			png_read_image
#define ppng_write_end			png_write_end
#define ppng_set_bgr			png_set_bgr
#define ppng_read_info			png_read_info
#define ppng_destroy_write_struct	png_destroy_write_struct
#define ppng_set_gray_to_rgb		png_set_gray_to_rgb
#define ppng_get_IHDR			png_get_IHDR
#define ppng_write_row			png_write_row
#define ppng_get_io_ptr			png_get_io_ptr
#define ppng_create_write_struct	png_create_write_struct
#define ppng_set_invert_alpha		png_set_invert_alpha
#define ppng_write_info			png_write_info
#define ppng_set_IHDR			png_set_IHDR
#define ppng_set_palette_to_rgb		png_set_palette_to_rgb

#else /* !(defined(GENS_PNG_INTERNAL) || !defined(GENS_PNG_DLOPEN)) */

// Use dlopen() to dynamically load libpng at runtime.

int gsft_png_dll_init(void);
void gsft_png_dll_end(void);

#define MAKE_EXTFUNCPTR(f) extern typeof(f) * p##f
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

#endif /* defined(GENS_PNG_INTERNAL) || !defined(GENS_PNG_DLOPEN) */

#ifdef __cplusplus
}
#endif

#endif /* GENS_PNG */

#endif /* __GSFT_PNG_DLL_H */

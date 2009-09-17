/***************************************************************************
 * libgsft: Common Functions.                                              *
 * gsft_malloc_align.h: Aligned memory allocation functions.               *
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

#ifndef __GSFT_MALLOC_ALIGN_H
#define __GSFT_MALLOC_ALIGN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(_WIN32)

// Win32 supports aligned malloc() using _aligned_malloc().
#include <malloc.h>
#ifdef __cplusplus
extern "C" {
#endif

static inline void* gsft_malloc_align(size_t size, size_t alignment)
{
	return _aligned_malloc(size, alignment);
}

#ifdef __cplusplus
}
#endif

#else /* !defined(_WIN32) */

// POSIX systems may or may not support aligned memory allocation.
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

static inline void* gsft_malloc_align(size_t size, size_t alignment)
{
#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
	void *mem;
	posix_memalign(&mem, alignment, size);
	return mem;
#else
	// TODO: Write a wrapper class for aligned malloc.
	((void)alignment);
	return malloc(size);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* defined(_WIN32) */

#endif /* __GSFT_MALLOC_ALIGN_H */

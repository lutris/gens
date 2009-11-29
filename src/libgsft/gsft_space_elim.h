/***************************************************************************
 * libgsft: Common Functions.                                              *
 * gsft_space_elim.h: Space elimination algorithm for ROM header names.    *
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

#ifndef __GSFT_SPACE_ELIM_H
#define __GSFT_SPACE_ELIM_H

// C includes.
#include <string.h>

#ifdef __GNUC__
#ifndef NONNULL
#define NONNULL __attribute__ ((nonnull))
#endif
#else
#define NONNULL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * gsft_space_elim(): Space elimination algorithm for ROM header names.
 * @param src Source name.
 * @param src_len Size of the source name.
 * @param dest Destination buffer. (Must be src_len+1 or greater!)
 * @return 0 on success; non-zero on error.
 */
int gsft_space_elim(const char *src, size_t src_len, char *dest) NONNULL;

#ifdef __cplusplus
}
#endif

#endif /* __GSFT_SPACE_ELIM_H */

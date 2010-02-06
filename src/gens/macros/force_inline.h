/***************************************************************************
 * Gens: Force Inline macro.                                               *
 *                                                                         *
 * Copyright (c) 2010 by David Korth                                       *
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

#ifndef GENS_FORCE_INLINE_H
#define GENS_FORCE_INLINE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Force inlining if we're not making a debug build.
#if !defined(GENS_DEBUG) && defined(__GNUC__) && (__GNUC__ >= 4)
#define FORCE_INLINE __attribute__ ((always_inline))
#else
#define FORCE_INLINE __inline__
#endif

#endif /* GENS_FORCE_INLINE_H */

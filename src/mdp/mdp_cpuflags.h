/***************************************************************************
 * MDP: Mega Drive Plugins - CPU Flag Definitions.                         *
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

/* The definitions in this file match gens_core/misc/cpuflags.h. */

#ifndef __MDP_CPUFLAGS_H
#define __MDP_CPUFLAGS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdint.h>

/* CPU flags (IA32/x86_64) */
#define MDP_CPUFLAG_MMX		((uint32_t)(1 << 0))
#define MDP_CPUFLAG_SSE		((uint32_t)(1 << 1))
#define MDP_CPUFLAG_SSE2	((uint32_t)(1 << 2))
#define MDP_CPUFLAG_SSE3	((uint32_t)(1 << 3))
#define MDP_CPUFLAG_SSSE3	((uint32_t)(1 << 4))
#define MDP_CPUFLAG_SSE41	((uint32_t)(1 << 5))
#define MDP_CPUFLAG_SSE42	((uint32_t)(1 << 6))
#define MDP_CPUFLAG_SSE4A	((uint32_t)(1 << 7))
#define MDP_CPUFLAG_SSE5	((uint32_t)(1 << 8))

/* CPU flags (IA32/x86_64; AMD only) */
#define MDP_CPUFLAG_MMXEXT	((uint32_t)(1 << 9))
#define MDP_CPUFLAG_3DNOW	((uint32_t)(1 << 10))
#define MDP_CPUFLAG_3DNOWEXT	((uint32_t)(1 << 11))

#endif /* __MDP_CPUFLAGS_H */

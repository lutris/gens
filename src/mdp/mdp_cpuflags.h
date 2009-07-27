/***************************************************************************
 * MDP: Mega Drive Plugins - CPU Flag Definitions.                         *
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

/* The definitions in this file match src/gens/gens_core/misc/cpuflags.h. */

#ifndef __MDP_CPUFLAGS_H
#define __MDP_CPUFLAGS_H

#include "mdp_stdint.h"

/* CPU flags (IA32/x86_64) */
#if defined(__i386__) || defined(__amd64__)

/*! BEGIN: MDP v1.0 CPU flags. !*/
#define MDP_CPUFLAG_X86_MMX		((uint32_t)(1 << 0))
#define MDP_CPUFLAG_X86_MMXEXT		((uint32_t)(1 << 1))	/* AMD only */
#define MDP_CPUFLAG_X86_3DNOW		((uint32_t)(1 << 2))	/* AMD only */
#define MDP_CPUFLAG_X86_3DNOWEXT	((uint32_t)(1 << 3))	/* AMD only */
#define MDP_CPUFLAG_X86_SSE		((uint32_t)(1 << 4))
#define MDP_CPUFLAG_X86_SSE2		((uint32_t)(1 << 5))
#define MDP_CPUFLAG_X86_SSE3		((uint32_t)(1 << 6))
#define MDP_CPUFLAG_X86_SSSE3		((uint32_t)(1 << 7))
#define MDP_CPUFLAG_X86_SSE41		((uint32_t)(1 << 8))
#define MDP_CPUFLAG_X86_SSE42		((uint32_t)(1 << 9))
#define MDP_CPUFLAG_X86_SSE4A		((uint32_t)(1 << 10))
/*! END: MDP v1.0 CPU flags. !*/

#endif /* defined(__i386__) || defined(__amd64__) */

#endif /* __MDP_CPUFLAGS_H */

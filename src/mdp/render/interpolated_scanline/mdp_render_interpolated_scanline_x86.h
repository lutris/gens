/***************************************************************************
 * MDP: Interpolated Scanline renderer. (x86 asm function prototypes)      *
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

#ifndef MDP_RENDER_INTERPOLATED_SCANLINE_X86_H
#define MDP_RENDER_INTERPOLATED_SCANLINE_X86_H

#include "mdp/mdp_stdint.h"
#include "mdp/mdp_fncall.h"

#ifdef __cplusplus
extern "C" {
#endif

DLL_LOCAL void MDP_FNCALL mdp_render_interpolated_scanline_16_x86(
				uint16_t *destScreen, uint16_t *mdScreen,
				int destPitch, int srcPitch,
				int width, int height, uint32_t mask);

DLL_LOCAL void MDP_FNCALL mdp_render_interpolated_scanline_16_x86_mmx(
				uint16_t *destScreen, uint16_t *mdScreen,
				int destPitch, int srcPitch,
				int width, int height, uint32_t vmodeFlags);

#if 0
DLL_LOCAL void MDP_FNCALL mdp_render_interpolated_scanline_32_x86(
				uint16_t *destScreen, uint16_t *mdScreen,
				int destPitch, int srcPitch,
				int width, int height);

DLL_LOCAL void MDP_FNCALL mdp_render_interpolated_scanline_32_x86_mmx(
				uint16_t *destScreen, uint16_t *mdScreen,
				int destPitch, int srcPitch,
				int width, int height);
#endif

#ifdef __cplusplus
}
#endif

#endif /* MDP_RENDER_INTERPOLATED_SCANLINE_X86_H */

/***************************************************************************
 * Gens: VDP Renderer. (Mode 5)                                            *
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

#ifndef GENS_VDP_REND_M5_HPP
#define GENS_VDP_REND_M5_HPP

#ifdef __cplusplus
extern "C" {
#endif

void VDP_Render_Line_m5(void);
void VDP_Render_Line_m5_32X(void);

// Interlaced rendering mode.
typedef enum
{
	INTREND_EVEN	= 0,	// Even lines only. (Old Gens)
	INTREND_ODD	= 1,	// Odd lines only.
	INTREND_FLICKER	= 2,	// Alternating fields. (Flickering Interlaced)
	INTREND_2X	= 3,	// 2x Resolution. (TODO)
} IntRend_Mode_t;
extern IntRend_Mode_t VDP_IntRend_Mode;

#ifdef __cplusplus
}
#endif

#endif /* GENS_VDP_REND_M5_HPP */

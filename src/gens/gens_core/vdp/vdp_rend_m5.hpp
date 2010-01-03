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

typedef struct
{
	unsigned int Pattern_Adr;
	unsigned int Line_7;	// NOTE: OBSOLETE - Replace with Y_FineOffset.
	int X;			// Render_Line_*() needs to check for less-than-zero condition. (jns)
	int Cell;		// VSRam cell can be -1 or -2.
	unsigned int Start_A;
	unsigned int Length_A;
	unsigned int Start_W;
	unsigned int Length_W;
	unsigned int Mask;
	unsigned int Spr_End;	// Contains sprite index, in multiples of sizeof(Sprite_Struct_t) == 8*4 == 32.
	unsigned int Next_Cell;
	unsigned int Palette;
	unsigned int Borne;
	
	unsigned int Y_FineOffset;	// Fine offset. (0-7 normal; 0-15 interlaced.)
} VDP_Data_Misc_t;
extern VDP_Data_Misc_t VDP_Data_Misc;

#ifdef __cplusplus
}
#endif

#endif /* GENS_VDP_REND_M5_HPP */

/***************************************************************************
 * Gens: 32X VDP rendering functions.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#ifndef GENS_VDP_32X_H
#define GENS_VDP_32X_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _VDP_32X_t
{
	unsigned int Mode;
	unsigned int State;
	unsigned int AF_Data;
	unsigned int AF_St;
	unsigned int AF_Len;
	unsigned int AF_Line;
} VDP_32X_t;
extern VDP_32X_t _32X_VDP;

// VDP RAM and CRam.
typedef union
{
	uint8_t  u8[0x100*1024];
	uint16_t u16[(0x100*1024)>>1];
	uint32_t u32[(0x100*1024)>>2];
} _32X_VDP_Ram_t;
extern _32X_VDP_Ram_t _32X_VDP_Ram;
extern uint16_t  _32X_VDP_CRam[0x100];

// Full 32X palette.
typedef union
{
	uint16_t u16[0x10000];
	uint32_t u32[0x10000];
} _32X_Palette_t;
extern _32X_Palette_t _32X_Palette;

// Adjusted CRam.
typedef union
{
	uint16_t u16[0x100];
	uint32_t u32[0x100];
} _32X_VDP_CRam_Adjusted_t;
extern _32X_VDP_CRam_Adjusted_t _32X_VDP_CRam_Adjusted;

void _32X_VDP_Reset(void);
void _32X_VDP_Draw(int FB_Num);

#ifdef __cplusplus
}
#endif

#endif /* GENS_VDP_32X_H */

/***************************************************************************
 * Gens: 32X VDP rendering functions.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include "vdp_32x.h"

// C includes.
#include <stdint.h>
#include <string.h>


// VDP RAM and CRam.
uint8_t  _32X_VDP_Ram[0x100 * 1024];
uint16_t _32X_VDP_CRam[0x100];

// Full 32X palettes.
uint16_t _32X_Palette_16B[0x10000];
uint32_t _32X_Palette_32B[0x10000];

// Adjusted CRam.
uint16_t _32X_VDP_CRam_Adjusted[0x100];
uint32_t _32X_VDP_CRam_Adjusted32[0x100];


/**
 * _32X_VDP_Reset(): Reset the 32X VDP.
 */
void _32X_VDP_Reset(void)
{
	memset(&_32X_VDP, 0x00, sizeof(_32X_VDP));
	memset(&_32X_VDP_Ram, 0x00, sizeof(_32X_VDP_Ram));
}

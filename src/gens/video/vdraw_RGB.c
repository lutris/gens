/***************************************************************************
 * Gens: Video Drawing - RGB lookup tables.                                *
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

#include "vdraw_RGB.h"

// C includes.
#include <stdlib.h>


/**
 * vdraw_build_RGB16to32(): Build a 16-bit RGB to 32-bit RGB table.
 * @return RGB16to32.
 */
uint32_t* vdraw_build_RGB16to32(void)
{
	uint32_t *RGB16to32 = (uint32_t*)(malloc(65536 * sizeof(uint32_t)));
	
	// Initialize the 16-bit to 32-bit conversion table.
	int i;
	for (i = 0; i < 65536; i++)
		RGB16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
	
	// Return the pointer.
	return RGB16to32;
}

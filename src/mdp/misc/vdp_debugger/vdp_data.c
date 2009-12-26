/***************************************************************************
 * MDP: VDP Debugger. (Data)                                               *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vdp_data.h"

/**
 * vdp_color_MDtoSys[]: Converts MD colors to System colors (32-bit).
 * Array index is (MD color & 0xFFF); value is System color.
 */
uint32_t vdp_color_MDtoSys[0x1000];


/**
 * CREATE_SYS_COLOR(): Create a system color from three 8-bit color components.
 * @param r Red component.
 * @param g Green component.
 * @param b Blue component.
 * @return System color.
 */
#if defined(GENS_UI_GTK)
// GTK+ colors. (ABGR: alpha 0xFF == fully opaque)
#define CREATE_SYS_COLOR(r, g, b) \
	(0xFF000000U | (b << 16) | (g << 8) | r)
#elif defined(GENS_UI_WIN32)
// TODO: Win32 colors.
#endif

/**
 * CONSTRAIN_COMPONENT_8BIT(): Constrain a color component to 8-bit.
 * @param x Color component to constrain.
 */
#define CONSTRAIN_COMPONENT_8BIT(x) \
do { \
	if (x < 0) \
		x = 0; \
	else if (x > 0xFF) \
		x = 0xFF; \
} while (0)


void MDP_FNCALL vdpdbg_data_init(void)
{
	static int is_init = 0;
	if (is_init)
		return;
	
	// Set is_init so we don't reinitialize the data structures again.
	is_init = 1;
	
	// Initialize vdp_color_MDtoSys[].
	// Algorithm is borrowed from Gens/GS. (Full with S/H)
	int r, g, b;
	for (unsigned int i = 0x0000; i < 0x1000; i++)
	{
		// Process using 8-bit color components.
		r = (i & 0x000F) << 4;
		g = (i & 0x00F0);
		b = (i >> 4) & 0xF0;
		
		// Scale the colors to full RGB using "Full with S/H" scaling.
		r = (r * 0xFF) / 0xF0;
		g = (g * 0xFF) / 0xF0;
		b = (b * 0xFF) / 0xF0;
		
		// Constrain the color components.
		CONSTRAIN_COMPONENT_8BIT(r);
		CONSTRAIN_COMPONENT_8BIT(g);
		CONSTRAIN_COMPONENT_8BIT(b);
		
		// Create the color.
		vdp_color_MDtoSys[i] = CREATE_SYS_COLOR(r, g, b);
	}
}

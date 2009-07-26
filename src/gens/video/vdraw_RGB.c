/***************************************************************************
 * Gens: Video Drawing - RGB Color Conversion Functions.                   *
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
#include "vdraw.h"

#include "emulator/g_main.hpp"

// MDP includes.
#include "mdp/mdp_render.h"

// C includes.
#include <stdlib.h>


/**
 * vdraw_rgb_convert(): RGB conversion function.
 * @param rInfo Render information.
 */
void vdraw_rgb_convert(mdp_render_info_t *rInfo)
{
	static uint32_t	*RGB_LUT = NULL;
	static void	*surface = NULL;
	static int	last_scale = 0;
	static uint8_t	last_bppOut = 0;
	static uint8_t	last_bppMD = 0;
	
	static uint32_t	pitch = 0;
	
	// TODO: This function only works for 15/16-bit to 32-bit.
	if ((bppMD != 15 && bppMD != 16) || bppOut != 32)
		return;
	
	// Make sure the conversion buffer is initialized.
	if (last_scale != vdraw_scale ||
	    last_bppOut != bppOut)
	{
		// Scaling factor and/or output color depth has changed.
		
		// Free the surface.
		free(surface);
		surface = NULL;
		
		if (vdraw_scale <= 0)
			return;
		
		// Allocate a new surface.
		last_scale = vdraw_scale;
		pitch = 320 * last_scale * (bppMD == 15 ? 2 : bppMD / 8);
		surface = malloc(pitch * 240 * last_scale);
	}
	
	// Make sure the lookup table is initialized.
	if (last_bppMD != bppMD)
	{
		// MD bpp has changed.
		
		// Free the lookup table.
		free(RGB_LUT);
		
		// Initialize a new lookup table.
		if (bppMD == 15)
		{
			// 15-bit color.
			RGB_LUT = (uint32_t*)malloc(65536 * sizeof(uint32_t));
			int i;
			for (i = 0; i < 65536; i++)
				RGB_LUT[i] = ((i & 0x7C00) << 9) | ((i & 0x03E0) << 6) | ((i & 0x001F) << 3);
		}
		else if (bppMD == 16)
		{
			// 16-bit color.
			RGB_LUT = (uint32_t*)malloc(65536 * sizeof(uint32_t));
			int i;
			for (i = 0; i < 65536; i++)
				RGB_LUT[i] = ((i & 0xF800) << 8) | ((i & 0x07E0) << 5) | ((i & 0x001F) << 3);
		}
		else
		{
			// Other. No lookup table required.
			RGB_LUT = NULL;
		}
		
		// Save the MD bpp.
		last_bppMD = bppMD;
	}
	
	// First, blit the image to the conversion surface.
	void *realDestScreen = rInfo->destScreen;
	int realDestPitch = rInfo->destPitch;
	rInfo->destScreen = surface;
	rInfo->destPitch = pitch;
	if (vdraw_get_fullscreen())
		vdraw_blitFS(rInfo);
	else
		vdraw_blitW(rInfo);
	
	// Next, do color conversion.
	
	// Multiply the width and height by the scaling factor.
	unsigned int width = (rInfo->width * vdraw_scale);
	const unsigned int height = (rInfo->height * vdraw_scale);
	
	// Calculate the pitch differences based on the conversion being used.
	const int pitchSrcDiff = ((pitch / (bppMD == 15 ? 2 : bppMD / 8)) - width);
	const int pitchDestDiff = ((realDestPitch / (bppOut == 15 ? 2 : bppOut / 8)) - width);
	
	// Process four pixels at a time.
	width >>= 2;
	
	const uint16_t *src16 = (const uint16_t*)surface;
	uint32_t *dest32 = (uint32_t*)realDestScreen;
	
	unsigned int x, y;
	for (y = height; y != 0; y--)
	{
		for (x = width; x != 0; x--)
		{
			*(dest32 + 0) = RGB_LUT[*(src16 + 0)];
			*(dest32 + 1) = RGB_LUT[*(src16 + 1)];
			*(dest32 + 2) = RGB_LUT[*(src16 + 2)];
			*(dest32 + 3) = RGB_LUT[*(src16 + 3)];
			
			src16 += 4;
			dest32 += 4;
		}
		
		src16 += pitchSrcDiff;
		dest32 += pitchDestDiff;
	}
}

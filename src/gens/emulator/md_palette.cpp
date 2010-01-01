/***************************************************************************
 * Gens: Palette Handler.                                                  *
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

#include "md_palette.hpp"

// C includes.
#include <math.h>

#include "g_main.hpp"
#include "ui/gens_ui.hpp"

#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"
#include "gens_core/vdp/vdp_io.h"

int Contrast_Level;
int Brightness_Level;
int Greyscale;
int Invert_Color;
ColorScaleMethod_t ColorScaleMethod;


/**
 * Constrain_Color_Component(): Constrains a color component.
 * @param mask Color component mask. (max value)
 * @param c Color component to constrain.
 */
template<int mask>
static inline void T_constrainColorComponent(int& c)
{
	if (c < 0)
		c = 0;
	else if (c > mask)
		c = mask;
}


/**
 * calculateGrayScale(): Calculates grayscale color values.
 * @param r Red component.
 * @param g Green component.
 * @param b Blue component.
 * @return Grayscale value.
 */
static inline int calculateGrayScale(int r, int g, int b)
{
	// Standard grayscale computation: Y = R*0.30 + G*0.59 + B*0.11
	r = lrint((double)r * 0.30);
	g = lrint((double)g * 0.59);
	b = lrint((double)b * 0.11);
	return (r + g + b);
}


static inline void adjustContrast(int& r, int& g, int& b, int contrast)
{
	if (contrast == 100)
		return;
	
	r = (r * contrast) / 100;
	g = (g * contrast) / 100;
	b = (b * contrast) / 100;
}


/**
 * T_Recalculate_Palette_MD(): Recalculates the MD palette for brightness, contrast, and various effects.
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
static inline void T_Recalculate_Palette_MD(pixel *palMD)
{
	int r, g, b;
	
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness_Level == 100)
	// Normal contrast:   (  Contrast_Level == 100)

	const int brightness = (Brightness_Level - 100);
	const int contrast = Contrast_Level;
	
	int mdComponentScale;
	switch (ColorScaleMethod)
	{
		case COLSCALE_RAW:
			mdComponentScale = 0;
			break;
		case COLSCALE_FULL:
			mdComponentScale = 0xE0;
			break;
		case COLSCALE_FULL_HS:
		default:
			mdComponentScale = 0xF0;
			break;
	}
	
	// Calculate the MD palette.
	for (unsigned int i = 0x0000; i < 0x1000; i++)
	{
		// Process using 8-bit color components.
		r = (i & 0x000F) << 4;
		g = (i & 0x00F0);
		b = (i >> 4) & 0xF0;
		
		// Scale the colors to full RGB.
		if (ColorScaleMethod != COLSCALE_RAW)
		{
			r = (r * 0xFF) / mdComponentScale;
			g = (g * 0xFF) / mdComponentScale;
			b = (b * 0xFF) / mdComponentScale;
		}
		
		// Adjust brightness.
		if (brightness != 0)
		{
			r += brightness;
			g += brightness;
			b += brightness;
		}
		
		// Adjust contrast.
		adjustContrast(r, g, b, contrast);
		
		if (Greyscale)
		{
			// Convert the color to grayscale.
			r = g = b = calculateGrayScale(r, g, b);
		}
		
		if (Invert_Color)
		{
			// Invert the color components.
			r ^= 0xFF;
			g ^= 0xFF;
			b ^= 0xFF;
		}
		
		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Constrain the color components.
		T_constrainColorComponent<RMask>(r);
		T_constrainColorComponent<GMask>(g);
		T_constrainColorComponent<BMask>(b);
		
		if (GMask == 0x3F)
		{
			// 16-bit color. (RGB565)
			// Mask off the LSB of the green component.
			g &= ~1;
		}
		
		// Create the color.
		palMD[i] = (r << (BBits + GBits)) |
			   (g << (BBits)) |
			   (b);
	}
}


/**
 * T_Recalculate_Palette_32X(): Recalculates the 32X palette for brightness, contrast, and various effects.
 */
template<typename pixel,
	int RBits, int GBits, int BBits,
	int RMask, int GMask, int BMask>
static inline void T_Recalculate_Palette_32X(pixel *pal32X, pixel *cramAdjusted32X)
{
	int r, g, b;
	
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness_Level == 100)
	// Normal contrast:   (  Contrast_Level == 100)

	const int brightness = (Brightness_Level - 100);
	const int contrast = Contrast_Level;
	
	// Calculate the 32X palette.
	for (unsigned int i = 0; i < 0x10000; i++)
	{
		// Process using 8-bit color components.
		r = (i & 0x1F) << 3;
		g = (i >> 2) & 0xF8;
		b = (i >> 7) & 0xF8;
		
		// Scale the colors to full RGB.
		if (ColorScaleMethod != COLSCALE_RAW)
		{
			r = (r * 0xFF) / 0xF8;
			g = (g * 0xFF) / 0xF8;
			b = (b * 0xFF) / 0xF8;
		}
		
		// Adjust brightness.
		if (brightness != 0)
		{
			r += brightness;
			g += brightness;
			b += brightness;
		}
		
		// Adjust contrast.
		adjustContrast(r, g, b, contrast);
		
		if (Greyscale)
		{
			// Convert the color to grayscale.
			r = g = b = calculateGrayScale(r, g, b);
		}
		
		if (Invert_Color)
		{
			// Invert the color components.
			r ^= 0xFF;
			g ^= 0xFF;
			b ^= 0xFF;
		}
		
		// Reduce color components to original color depth.
		r >>= (8 - RBits);
		g >>= (8 - GBits);
		b >>= (8 - BBits);
		
		// Constrain the color components.
		T_constrainColorComponent<RMask>(r);
		T_constrainColorComponent<GMask>(g);
		T_constrainColorComponent<BMask>(b);
		
		if (GMask == 0x3F)
		{
			// 16-bit color. (RGB565)
			// Mask off the LSB of the green component.
			g &= ~1;
		}
		
		// Create the color.
		pal32X[i] = (r << (GBits + BBits)) |
			    (g << (BBits)) |
			    (b);
	}
	
	// Adjust the 32X VDP CRAM.
	for (unsigned int i = 0; i < 0x100; i++)
	{
		cramAdjusted32X[i] = pal32X[_32X_VDP_CRam[i]];
	}
}


void Recalculate_Palettes(void)
{
	// Always recalculate both the 16-bit and 32-bit MD palettes.
	if (bppMD == 15)
		T_Recalculate_Palette_MD<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>(Palette);
	else
		T_Recalculate_Palette_MD<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>(Palette);
	
	T_Recalculate_Palette_MD<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>(Palette32);
	
	// Recalculate only one 32X palette.
	switch (bppMD)
	{
		case 15:
			T_Recalculate_Palette_32X<uint16_t, 5, 5, 5, 0x1F, 0x1F, 0x1F>
					(_32X_Palette_16B, _32X_VDP_CRam_Adjusted);
			break;
		
		case 16:
			T_Recalculate_Palette_32X<uint16_t, 5, 6, 5, 0x1F, 0x3F, 0x1F>
					(_32X_Palette_16B, _32X_VDP_CRam_Adjusted);
			break;
		
		case 32:
			T_Recalculate_Palette_32X<uint32_t, 8, 8, 8, 0xFF, 0xFF, 0xFF>
					(_32X_Palette_32B, _32X_VDP_CRam_Adjusted32);
			break;
	}
	
	// Set the CRam flag to force a palette update.
	VDP_Flags.CRam = 1;
	
	// TODO: Do_VDP_Only() / Do_32X_VDP_Only() if paused.
	
	// Force a wakeup.
	GensUI::wakeup();
}

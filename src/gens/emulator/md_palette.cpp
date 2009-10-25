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
 * Recalculate_Palettes(): Recalculates the MD and 32X palettes for brightness, contrast, and various effects.
 */
template<typename pixel, pixel *palMD, pixel *pal32X, pixel *cramAdjusted32X,
	 int RBits, int GBits, int BBits,
	 int RMask, int GMask, int BMask>
static inline void T_Recalculate_Palettes(void)
{
	int r, g, b;
	
	// Brightness / Contrast
	// These values are scaled to positive numbers.
	// Normal brightness: (Brightness_Level == 100)
	// Normal contrast:   (  Contrast_Level == 100)
	const int brightness = (Brightness_Level - 100);
	const int brightness_R = (brightness * (RMask+1)) / 100;
	const int brightness_G = (brightness * (GMask+1)) / 100;
	const int brightness_B = (brightness * (BMask+1)) / 100;
	
	const int contrast = Contrast_Level;
	
	int mdComponentScale;
	switch (ColorScaleMethod)
	{
		case COLSCALE_RAW:
			mdComponentScale = 0;
			break;
		case COLSCALE_FULL:
			mdComponentScale = 0xE;
			break;
		case COLSCALE_FULL_HS:
		default:
			mdComponentScale = 0xF;
			break;
	}
	
	// Calculate the MD palette.
	for (unsigned int i = 0x0000; i < 0x1000; i++)
	{
		// Mask off the LSB of each MD color component.
		unsigned int color = (i & 0x0FFF);
		
		r = (color & 0xF) << (RBits - 4);
		g = ((color >> 4) & 0x0F) << (GBits - 4);
		b = ((color >> 8) & 0x0F) << (BBits - 4);
		
		// Scale the colors to full RGB.
		if (ColorScaleMethod != COLSCALE_RAW)
		{
			r = (r * RMask) / (mdComponentScale << (RBits - 4));
			g = (g * GMask) / (mdComponentScale << (GBits - 4));
			b = (b * BMask) / (mdComponentScale << (BBits - 4));
		}
		
		// Adjust brightness.
		if (brightness != 0)
		{
			r += brightness_R;
			g += brightness_G;
			b += brightness_B;
		}
		
		// Adjust contrast.
		adjustContrast(r, g, b, contrast);
		
		// Constrain the color components.
		T_constrainColorComponent<RMask>(r);
		T_constrainColorComponent<GMask>(g);
		T_constrainColorComponent<BMask>(b);
		
		// Create the color.
		palMD[i] = (r << (GBits + BBits)) |
			   (g << (BBits)) |
			   (b);
	}
	
	// Calculate the 32X palette.
	for (unsigned int i = 0; i < 0x10000; i++)
	{
		r = (i & 0x1F) << (RBits - 5);
		g = ((i >> 5) & 0x1F) << (GBits - 5);
		b = ((i >> 10) & 0x1F) << (BBits - 5);
		
		// Scale the colors to full RGB.
		if (ColorScaleMethod != COLSCALE_RAW)
		{
			r = (r * RMask) / (0x1F << (RBits - 5));
			g = (g * GMask) / (0x1F << (GBits - 5));
			b = (b * BMask) / (0x1F << (BBits - 5));
		}
		
		// Adjust brightness.
		if (brightness != 0)
		{
			r += brightness_R;
			g += brightness_G;
			b += brightness_B;
		}
		
		// Adjust contrast.
		adjustContrast(r, g, b, contrast);
		
		// Constrain the color components.
		T_constrainColorComponent<RMask>(r);
		T_constrainColorComponent<GMask>(g);
		T_constrainColorComponent<BMask>(b);
		
		// Create the color.
		pal32X[i] = (r << (GBits + BBits)) |
			    (g << (BBits)) |
			    (b);
	}
	
	// Convert colors to grayscale, if necessary.
	if (Greyscale)
	{
		int gray;
		
		// MD palette.
		for (unsigned int i = 0; i < 0x1000; i++)
		{
			r = (palMD[i] >> (GBits + BBits)) & RMask;
			g = (palMD[i] >> (BBits)) & GMask;
			b = palMD[i] & BMask;
			
			// Convert to 24-bit color.
			r <<= (8 - RBits);
			g <<= (8 - GBits);
			b <<= (8 - BBits);
			
			// Calculate the grayscale value.
			gray = calculateGrayScale(r, g, b);
			
			// Convert back to the original color depth.
			r = (gray >> (8 - RBits)) & RMask;
			g = (gray >> (8 - GBits)) & GMask;
			b = (gray >> (8 - BBits)) & BMask;
			
			// Create the color.
			palMD[i] = (r << (GBits + BBits)) |
				   (g << (BBits)) |
				   (b);
		}
		
		// 32X palette.
		for (unsigned i = 0; i < 0x10000; i++)
		{
			r = (pal32X[i] >> (GBits + BBits)) & RMask;
			g = (pal32X[i] >> (BBits)) & GMask;
			b = pal32X[i] & BMask;
			
			// Convert to 24-bit color.
			r <<= (8 - RBits);
			g <<= (8 - GBits);
			b <<= (8 - BBits);
			
			// Calculate the grayscale value.
			gray = calculateGrayScale(r, g, b);
			
			// Convert back to the original color depth.
			r = (gray >> (8 - RBits)) & RMask;
			g = (gray >> (8 - GBits)) & GMask;
			b = (gray >> (8 - BBits)) & BMask;
			
			// Create the color.
			pal32X[i] = (r << (GBits + BBits)) |
				    (g << (BBits)) |
				    (b);
		}
	}
	
	// Invert colors, if necessary.
	if (Invert_Color)
	{
		// MD palette.
		for (unsigned int i = 0; i < 0x1000; i++)
		{
			palMD[i] = ~palMD[i];
		}
		
		// 32X palette.
		for (unsigned int i = 0; i < 0x10000; i++)
		{
			pal32X[i] = ~pal32X[i];
		}
	}
	
	// Adjust the 32X VDP CRAM.
	for (unsigned int i = 0; i < 0x100; i++)
	{
		cramAdjusted32X[i] = pal32X[_32X_VDP_CRam[i]];
	}
}


void Recalculate_Palettes(void)
{
	if (bppMD == 15)
		T_Recalculate_Palettes<uint16_t, Palette, _32X_Palette_16B, _32X_VDP_CRam_Adjusted, 5, 5, 5, 0x1F, 0x1F, 0x1F>();
	else //if (bppMD == 16)
		T_Recalculate_Palettes<uint16_t, Palette, _32X_Palette_16B, _32X_VDP_CRam_Adjusted, 5, 6, 5, 0x1F, 0x3F, 0x1F>();
	
	// 32-bit color.
	T_Recalculate_Palettes<uint32_t, Palette32, _32X_Palette_32B, _32X_VDP_CRam_Adjusted32, 8, 8, 8, 0xFF, 0xFF, 0xFF>();
	
	// Set CRam_Flag.
	CRam_Flag = 1;
	
	// TODO: Do_VDP_Only() / Do_32X_VDP_Only() if paused.
	
	// Force a wakeup.
	GensUI::wakeup();
}

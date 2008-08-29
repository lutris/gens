/**
 * GENS: Palette handler.
 */


#include "g_palette.h"
#include "g_main.h"
#include "vdp_rend.h"
#include "vdp_32x.h"


int RMax_Level;
int GMax_Level;
int BMax_Level;
int Contrast_Level;
int Brightness_Level;
int Greyscale;
int Invert_Color;


/**
 * Constrain_Color_Component(): Constrains a color component.
 * @param c Color component.
 * @return Constrained color component.
 */
inline int Constrain_Color_Component(int c)
{
	if (c < 0)
		return 0;
	else if (c > 0x3F)
		return 0x3F;
	return c;
}


/**
 * CalculateGrayScale_16B(): Calculate the grayscale color values for 16-bit colors.
 * @param r Red component.
 * @param g Green component.
 * @param b Blue component.
 * @return Grayscale value.
 */
inline int CalculateGrayScale_16B(int r, int g, int b)
{
	// Standard grayscale computation: Y = R*0.30 + G*0.59 + B*0.11
	r = (r * (unsigned int) (0.30 * 65536.0)) >> 16;
	g = (g * (unsigned int) (0.59 * 65536.0)) >> 16;
	b = (b * (unsigned int) (0.11 * 65536.0)) >> 16;
	return (r + g + b);
}


/**
 * Recalculate_Palettes(): Recalculates the MD and 32X palettes for brightness, contrast, and various effects.
 */
void Recalculate_Palettes(void)
{
	int i;
	int r, g, b;
	int rf, gf, bf;
	int bright, cont;
	unsigned short color;
	
	// Calculate the MD palette.
	for (r = 0; r < 0x10; r++)
	{
		for (g = 0; g < 0x10; g++)
		{
			for (b = 0; b < 0x10; b++)
			{
				color = (b << 8) | (g << 4) | r;
				
				rf = (r & 0xE) << 2;
				gf = (g & 0xE) << 2;
				bf = (b & 0xE) << 2;
				
				rf = (int) (double) (rf) * (int) ((double) (RMax_Level) / 224.0);
				gf = (int) (double) (gf) * (int) ((double) (GMax_Level) / 224.0);
				bf = (int) (double) (bf) * (int) ((double) (BMax_Level) / 224.0);
				
				// Compute colors here (64 levels)
				
				bright = Brightness_Level;
				bright -= 100;
				bright *= 32;
				bright /= 100;
				
				rf += bright;
				gf += bright;
				bf += bright;
				
				rf = Constrain_Color_Component(rf);
				gf = Constrain_Color_Component(gf);
				bf = Constrain_Color_Component(bf);
				
				cont = Contrast_Level;
				
				rf = (rf * cont) / 100;
				gf = (gf * cont) / 100;
				bf = (bf * cont) / 100;
				
				rf = Constrain_Color_Component(rf);
				gf = Constrain_Color_Component(gf);
				bf = Constrain_Color_Component(bf);
				
				// 32-bit palette
				Palette32[color] = (rf << 18) | (gf << 10) | (bf << 2);
				
				// 16-bit palette
				if (bpp == 15)
				{
					rf = (rf >> 1) << 10;
					gf = (gf >> 1) << 5;
				}
				else
				{
					rf = (rf >> 1) << 11;
					gf = (gf >> 0) << 5;
				}
				bf = (bf >> 1) << 0;
				
				Palette[color] = rf | gf | bf;
			}
		}
	}
	
	// Calculate the 32X palette.
	for (i = 0; i < 0x10000; i++)
	{
		bf = ((i >> 10) & 0x1F) << 1;
		gf = ((i >> 5) & 0x1F) << 1;
		rf = ((i >> 0) & 0x1F) << 1;
		
		rf = (int) (double) (rf) * (int) ((double) (RMax_Level) / 248.0);
		gf = (int) (double) (gf) * (int) ((double) (GMax_Level) / 248.0);
		bf = (int) (double) (bf) * (int) ((double) (BMax_Level) / 248.0);
		
		// Compute colors here (64 levels)
		
		bright = Brightness_Level;
		bright -= 100;
		bright *= 32;
		bright /= 100;
		
		rf += bright;
		gf += bright;
		bf += bright;
		
		rf = Constrain_Color_Component(rf);
		gf = Constrain_Color_Component(gf);
		bf = Constrain_Color_Component(bf);
		
		cont = Contrast_Level;
		
		rf = (rf * cont) / 100;
		gf = (gf * cont) / 100;
		bf = (bf * cont) / 100;
		
		rf = Constrain_Color_Component(rf);
		gf = Constrain_Color_Component(gf);
		bf = Constrain_Color_Component(bf);
		
		// 32-bit palette
		_32X_Palette_32B[i] = (rf << 18) | (gf << 10) | (bf << 2);
		
		// 16-bit palette
		if (bpp == 15)
		{
			rf = (rf >> 1) << 10;
			gf = (gf >> 1) << 5;
		}
		else
		{
			rf = (rf >> 1) << 11;
			gf = (gf >> 0) << 5;
		}
		bf = (bf >> 1) << 0;
		
		_32X_Palette_16B[i] = rf | gf | bf;
	}
	
	// Convert colors to grayscale, if necessary.
	if (Greyscale)
	{
		// MD palette.
		for (i = 0; i < 0x1000; i++)
		{
			if (bpp == 15)
			{
				r = ((Palette[i] >> 10) & 0x1F) << 1;
				g = ((Palette[i] >> 5) & 0x1F) << 1;
			}
			else
			{
				r = ((Palette[i] >> 11) & 0x1F) << 1;
				g = (Palette[i] >> 5) & 0x3F;
			}
			
			b = ((Palette[i] >> 0) & 0x1F) << 1;
			
			r = g = b = CalculateGrayScale_16B(r, g, b);
			
			if (bpp == 15)
			{
				r = (r >> 1) << 10;
				g = (g >> 1) << 5;
			}
			else
			{
				r = (r >> 1) << 11;
				g = (g >> 0) << 5;
			}
			
			b = (b >> 1) << 0;
			
			Palette[i] = r | g | b;
		}
		
		// 32X palette.
		for (i = 0; i < 0x10000; i++)
		{
			if (bpp == 15)
			{
				r = ((_32X_Palette_16B[i] >> 10) & 0x1F) << 1;
				g = ((_32X_Palette_16B[i] >> 5) & 0x1F) << 1;
			}
			else
			{
				r = ((_32X_Palette_16B[i] >> 11) & 0x1F) << 1;
				g = (_32X_Palette_16B[i] >> 5) & 0x3F;
			}
			
			b = ((_32X_Palette_16B[i] >> 0) & 0x1F) << 1;
			
			r = g = b = CalculateGrayScale_16B(r, g, b);
			
			if (bpp == 15)
			{
				r = (r >> 1) << 10;
				g = (g >> 1) << 5;
			}
			else
			{
				r = (r >> 1) << 11;
				g = (g >> 0) << 5;
			}
			
			b = (b >> 1) << 0;
			
			_32X_Palette_16B[i] = r | g | b;
		}
	}
	
	// Invert colors, if necessary.
	if (Invert_Color)
	{
		// MD palette.
		for (i = 0; i < 0x1000; i++)
		{
			Palette[i] ^= 0xFFFF;
			Palette32[i] ^= 0xFFFFFF;
		}
		
		// 32X palette.
		for (i = 0; i < 0x10000; i++)
		{
			_32X_Palette_16B[i] ^= 0xFFFF;
			_32X_Palette_32B[i] ^= 0xFFFFFF;
		}
	}
	
	// Adjust 32X VDP CRAM.
	for (i = 0; i < 0x100; i++)
	{
		_32X_VDP_CRam_Ajusted[i] = _32X_Palette_16B[_32X_VDP_CRam[i]];
		_32X_VDP_CRam_Ajusted32[i] = _32X_Palette_32B[_32X_VDP_CRam[i]];
	}
}

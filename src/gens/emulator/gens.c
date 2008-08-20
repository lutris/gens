#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "gens.h"
#include "g_main.h"
#include "g_sdldraw.h"
#include "g_sdlsound.h"
#include "g_sdlinput.h"
#include "rom.h"
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "mem_sh2.h"
#include "ym2612.h"
#include "psg.h"
#include "cpu_68k.h"
#include "cpu_z80.h"
#include "cpu_sh2.h"
#include "z80.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "vdp_32x.h"
#include "io.h"
#include "misc.h"
#include "save.h"
#include "ggenie.h"
#include "cd_sys.h"
#include "lc89510.h"
#include "gfx_cd.h"
#include "wave.h"
#include "pcm.h"
#include "pwm.h"
#include "cd_sys.h"
#include "cd_file.h"
#include "gym.h"
#include "support.h"

int Debug;
int Frame_Skip;
int Frame_Number;
int DAC_Improv;
int RMax_Level;
int GMax_Level;
int BMax_Level;
int Contrast_Level;
int Brightness_Level;
int Greyscale;
int Invert_Color;

int Round_Double (double val)
{
	if ((val - (double) (int) val) > 0.5)
		return (int) (val + 1);
	else
		return (int) val;
}

void Init_Tab (void)
{
	int x, y, dep;
	
	for (x = 0; x < 1024; x++)
	{
		for (y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0x3FC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 9)] = (unsigned short) dep;
		}
	}
	
	for (x = 0; x < 512; x++)
	{
		for (y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0x1FC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 8) + 0x8000] =
				(unsigned short) (dep + 0x8000);
		}
	}
	
	for (x = 0; x < 256; x++)
	{
		for (y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0xFC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xC000] =
				(unsigned short) (dep + 0xC000);
		}
	}
	
	for (x = 0; x < 256; x++)
	{
		for (y = 0; y < 32; y++)
		{
			dep = (x & 3) + (((x & 0xFC) >> 2) << 7);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xE000] =
				(unsigned short) (dep + 0xE000);
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xF000] =
				(unsigned short) (dep + 0xF000);
		}
	}
	
	for (x = 0; x < 512; x++)
		Z80_M68K_Cycle_Tab[x] = (int) ((double) x * 7.0 / 15.0);
}


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
 * Recalculate_Palettes(): Recalculates the MD and 32X palettes for brightness, contrast, and various effects.
 */
void Recalculate_Palettes (void)
{
	int i;
	int r, g, b;
	int rf, gf, bf;
	int bright, cont;
	
	// Calculate the MD palette.
	for (r = 0; r < 0x10; r++)
	{
		for (g = 0; g < 0x10; g++)
		{
			for (b = 0; b < 0x10; b++)
			{
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
				
				if (Mode_555 & 1)
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
				
				Palette[(b << 8) | (g << 4) | r] = rf | gf | bf;
			}
		}
	}
	
	// Calculate the 32X palette.
	for (i = 0; i < 0x10000; i++)
	{
		b = ((i >> 10) & 0x1F) << 1;
		g = ((i >> 5) & 0x1F) << 1;
		r = ((i >> 0) & 0x1F) << 1;
		
		r = (int) (double) (r) * (int) ((double) (RMax_Level) / 248.0);
		g = (int) (double) (g) * (int) ((double) (GMax_Level) / 248.0);
		b = (int) (double) (b) * (int) ((double) (BMax_Level) / 248.0);
		
		// Compute colors here (64 levels)
		
		bright = Brightness_Level;
		bright -= 100;
		bright *= 32;
		bright /= 100;
		
		r += bright;
		g += bright;
		b += bright;
		
		r = Constrain_Color_Component(r);
		g = Constrain_Color_Component(g);
		b = Constrain_Color_Component(b);
		
		cont = Contrast_Level;
		
		r = (r * cont) / 100;
		g = (g * cont) / 100;
		b = (b * cont) / 100;
		
		r = Constrain_Color_Component(r);
		g = Constrain_Color_Component(g);
		b = Constrain_Color_Component(b);
		
		if (Mode_555 & 1)
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
	
	// Convert colors to grayscale, if necessary.
	if (Greyscale)
	{
		// MD palette.
		for (i = 0; i < 0x1000; i++)
		{
			if (Mode_555 & 1)
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
			
			r = (r * (unsigned int) (0.30 * 65536.0)) >> 16;
			g = (g * (unsigned int) (0.59 * 65536.0)) >> 16;
			b = (b * (unsigned int) (0.11 * 65536.0)) >> 16;
			
			r = g = b = r + g + b;
			
			if (Mode_555 & 1)
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
			if (Mode_555 & 1)
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
			
			r = (r * (unsigned int) (0.30 * 65536.0)) >> 16;
			g = (g * (unsigned int) (0.59 * 65536.0)) >> 16;
			b = (b * (unsigned int) (0.11 * 65536.0)) >> 16;
			
			r = g = b = (r + g + b);
			
			if (Mode_555 & 1)
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
		}
		
		// 32X palette.
		for (i = 0; i < 0x10000; i++)
		{
			_32X_Palette_16B[i] ^= 0xFFFF;
		}
	}
	
	// Adjust 32X VDP CRAM.
	for (i = 0; i < 0x100; i++)
	{
		_32X_VDP_CRam_Ajusted[i] = _32X_Palette_16B[_32X_VDP_CRam[i]];
	
	}
}


void Check_Country_Order (void)
{
	if ((Country_Order[0] == Country_Order[1])
	    || (Country_Order[0] == Country_Order[2])
	    || (Country_Order[1] == Country_Order[2])
	    || (Country_Order[0] == Country_Order[2]) || (Country_Order[0] > 2)
	    || (Country_Order[0] < 0) || (Country_Order[1] > 2)
	    || (Country_Order[1] < 0) || (Country_Order[2] > 2)
	    || (Country_Order[2] < 0))
	{
		Country_Order[0] = 0;
		Country_Order[1] = 1;
		Country_Order[2] = 2;
	}
}

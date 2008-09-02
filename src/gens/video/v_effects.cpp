/***************************************************************************
 * Gens: Miscellaneous Effects.                                            *
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

#include <math.h>

#include "v_effects.hpp"
#include "gens_core/misc/misc.h"
#include "gens_core/misc/fastblur.h"

#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"


/**
 * Update_Gens_Logo(): Update the Gens logo.
 * @return 1 on success.
 */
int Update_Gens_Logo(void)
{
	// TODO: 32-bit support.
	
	int i, j, m, n;
	static short tab[64000], Init = 0;
	static float renv = 0, /*ang = 0,*/ zoom_x = 0, zoom_y = 0, pas;
	unsigned short c;
	
	if (!Init)
	{
		// TODO: Don't use SDL for image loading.
		SDL_Surface* Logo;
		
		Logo = SDL_LoadBMP(DATADIR "/gens_big.bmp");
		
		SDL_LockSurface(Logo);
		memcpy(tab, Logo->pixels, 64000);
		SDL_UnlockSurface(Logo);
		
		pas = 0.05;
		Init = 1;
	}
	
	renv += pas;
	zoom_x = sin(renv);
	if (zoom_x == 0.0)
			zoom_x = 0.0000001;
	zoom_x = (1 / zoom_x) * 1;
	zoom_y = 1;
	
	if (VDP_Reg.Set4 & 0x1)
	{
		for(j = 0; j < 240; j++)
		{
			for(i = 0; i < 320; i++)
			{
				m = (float)(i - 160) * zoom_x;
				n = (float)(j - 120) * zoom_y;
				
				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5))
						MD_Screen[TAB336[j] + i + 8] = c;
				}
			}
		}
	}
	else
	{
		for(j = 0; j < 240; j++)
		{
			for(i = 0; i < 256; i++)
			{
				m = (float)(i - 128) * zoom_x;
				n = (float)(j - 120) * zoom_y;
				
				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5)) MD_Screen[TAB336[j] + i + 8] = c;
				}
			}
		}
	}
	
	Fast_Blur();
	//draw->Flip();
	
	return 1;
}


/**
 * Update_Crazy_Effect(): Update the "Crazy" Effect.
 * @return 1 on success.
 */
int Update_Crazy_Effect(unsigned char introEffectColor)
{
	int i, j, offset;
	int r = 0, v = 0, b = 0, prev_l, prev_p;
	int RB, G;
	
	for(offset = 336 * 240, j = 0; j < 240; j++)
	{
		for(i = 0; i < 336; i++, offset--)
		{
			prev_l = MD_Screen[offset - 336];
			prev_p = MD_Screen[offset - 1];
			
			if (bpp == 15)
			{
				RB = ((prev_l & 0x7C1F) + (prev_p & 0x7C1F)) >> 1;
				G = ((prev_l & 0x03E0) + (prev_p & 0x03E0)) >> 1;
				
				if (introEffectColor & 0x4)
				{
					r = RB & 0x7C00;
					if (rand() > 0x2C00) r += 0x0400;
					else r -= 0x0400;
					if (r > 0x7C00) r = 0x7C00;
					else if (r < 0x0400) r = 0;
				}
				
				if (introEffectColor & 0x2)
				{
					v = G & 0x03E0;
					if (rand() > 0x2C00) v += 0x0020;
					else v -= 0x0020;
					if (v > 0x03E0) v = 0x03E0;
					else if (v < 0x0020) v = 0;
				}
				
				if (introEffectColor & 0x1)
				{
					b = RB & 0x001F;
					if (rand() > 0x2C00) b++;
					else b--;
					if (b > 0x1F) b = 0x1F;
					else if (b < 0) b = 0;
				}
			}
			else
			{
				RB = ((prev_l & 0xF81F) + (prev_p & 0xF81F)) >> 1;
				G = ((prev_l & 0x07C0) + (prev_p & 0x07C0)) >> 1;
				
				if (introEffectColor & 0x4)
				{
					r = RB & 0xF800;
					if (rand() > 0x2C00) r += 0x0800;
					else r -= 0x0800;
					if (r > 0xF800) r = 0xF800;
					else if (r < 0x0800) r = 0;
				}
				
				if (introEffectColor & 0x2)
				{
					v = G & 0x07C0;
					if (rand() > 0x2C00) v += 0x0040;
					else v -= 0x0040;
					if (v > 0x07C0) v = 0x07C0;
					else if (v < 0x0040) v = 0;
				}
				
				if (introEffectColor & 0x1)
				{
					b = RB & 0x001F;
					if (rand() > 0x2C00) b++;
					else b--;
					if (b > 0x1F) b = 0x1F;
					else if (b < 0) b = 0;
				}
			}
			
			MD_Screen[offset] = r + v + b;
		}
	}
	
	//draw->Flip();
	
	return 1;
}


/**
 * Pause_Screen(): Tint the screen a purple hue to indicate that emulation is paused.
 */
void Pause_Screen(void)
{
	int i, j, offset;
	unsigned char r, g, b, nr, ng, nb;
	int sum;
	
	r = g = b = nr = ng = nb = 0;
	
	for (offset = j = 0; j < 240; j++)
	{
		for (i = 0; i < 336; i++, offset++)
		{
			if (bpp == 15)
			{
				r = (unsigned char)((MD_Screen[offset] & 0x7C00) >> 10);
				g = (unsigned char)((MD_Screen[offset] & 0x03E0) >> 5);
				b = (unsigned char)((MD_Screen[offset] & 0x001F));
			}
			else if (bpp == 16)
			{
				r = (unsigned char)((MD_Screen[offset] & 0xF800) >> 11);
				g = (unsigned char)((MD_Screen[offset] & 0x07C0) >> 6);
				b = (unsigned char)((MD_Screen[offset] & 0x001F));
			}
			else //if (bpp == 32)
			{
				r = (unsigned char)(((MD_Screen32[offset] >> 16) & 0xFF) >> 3);
				g = (unsigned char)(((MD_Screen32[offset] >> 8) & 0xFF) >> 3);
				b = (unsigned char)(((MD_Screen32[offset] >> 0) & 0xFF) >> 3);
			}
			
			sum = r + g + b;
			sum /= 3;
			nr = ng = nb = (unsigned char)sum;
			
			// L-shift the blue component to tint the image.
			nb <<= 1;
			if (nb > 0x1F)
				nb = 0x1F;
			
			nr &= 0x1E;
			ng &= 0x1E;
			nb &= 0x1E;
			
			if (bpp == 15)
				MD_Screen[offset] = (nr << 10) | (ng << 5) | nb;
			else if (bpp == 16)
				MD_Screen[offset] = (nr << 11) | (ng << 6) | nb;
			else //if (bpp == 32)
				MD_Screen32[offset] = (nr << 19) | (ng << 11) | (nb << 3);
		}
	}
	
	if (ice == 2)
		ice = 3;
}

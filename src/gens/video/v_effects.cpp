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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// C includes.
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef GENS_OS_UNIX
#include <SDL/SDL.h>
#endif

#ifdef GENS_OS_WIN32
#include "ui/win32/resource.h"
#endif

#include "v_effects.hpp"
#include "v_inline.h"
#include "gens_core/misc/misc.h"
#include "gens_core/gfx/fastblur.hpp"

#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? a : b)
#endif


/**
 * T_Update_Gens_Logo(): Update the Gens logo.
 */
template<typename pixel, int trans_min, int trans_max>
static inline void T_Update_Gens_Logo(pixel *screen, const pixel *logo)
{
	// The Gens logo is 260x180.
	// Thus, in 16-bit color, it's 93,600 bytes,
	// and in 32-bit color, it's 187,200 bytes.
	
	// TODO: Figure out why there's extra blurring in 32-bit color.
	
	static float renv = 0, /*ang = 0,*/ zoom_x = 0, zoom_y = 0, pas = 0.05f;
	int i, j, m, n;
	pixel c;
	
	// Zoom factor.
	renv += pas;
	zoom_x = sin(renv);
	if (zoom_x == 0.0)
		zoom_x = 0.0000001;
	zoom_x = (1 / zoom_x) * 1;
	zoom_y = 1;
	
	const int w = (isFullXRes() ? 320 : 256);
	const int w_div2 = (w / 2);
	const int h = VDP_Num_Vis_Lines;
	const int h_div2 = (h / 2);
	
	for (j = h; j != 0; j--)
	{
		for (i = w; i != 0; i--)
		{
			m = (float)(i - w_div2) * zoom_x;
			n = (float)(j - h_div2) * zoom_y;
			
			if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
			{
				c = logo[m + 130 + (n + 90) * 260];
				if ((c > trans_max) || (c < trans_min))
					screen[TAB336[j] + i + 8] = c;
			}
		}
	}
	
	// Blur the image.
	Fast_Blur();
}


/**
 * Update_Gens_Logo(): Update the Gens logo.
 * @return 1 on success.
 */
int Update_Gens_Logo(void)
{
	// TODO: 32-bit support.
	// TODO: Only allocate one at a time.
	static uint16_t logo_data16[65536];
	static uint32_t logo_data32[65536];
	int lastBpp = 0;
	
	if (lastBpp != bppMD)
	{
		// Load the Gens logo.
		
		// TODO: Don't use SDL for image loading.
		// TODO: Fix this for Win32.
#ifndef GENS_OS_WIN32
		SDL_Surface *logo_load = SDL_LoadBMP(GENS_DATADIR "/gens_big.bmp");
		
		// Make sure the logo has the correct color depth.
		static SDL_PixelFormat fmt15 =
		{
			NULL, 15, 2,
			3, 3, 3, 0,			// Loss compared to 32-bit.
			10, 5, 0, 0,			// Shift.
			0x7C00, 0x03E0, 0x001F, 0,	// Mask.
			0, 0
		};
		static SDL_PixelFormat fmt16 =
		{
			NULL, 16, 2,
			3, 2, 3, 0,			// Loss compared to 32-bit.
			11, 5, 0, 0,			// Shift.
			0xF800, 0x07E0, 0x001F, 0,	// Mask.
			0, 0
		};
		static SDL_PixelFormat fmt32 =
		{
			NULL, 32, 4,
			0, 0, 0, 0,				// Loss compared to 32-bit.
			16, 8, 0, 0,				// Shift.
			0xFF0000, 0x00FF00, 0x0000FF, 0,	// Mask.
			0, 0
		};
		
		SDL_Surface *logo_use = NULL;
		switch (bppMD)
		{
			case 15:
				logo_use = SDL_ConvertSurface(logo_load, &fmt15, SDL_SWSURFACE);
				SDL_FreeSurface(logo_load);
				logo_load = NULL;
				break;
			
			case 16:
				logo_use = SDL_ConvertSurface(logo_load, &fmt16, SDL_SWSURFACE);
				SDL_FreeSurface(logo_load);
				logo_load = NULL;
				break;
			
			case 32:
				logo_use = SDL_ConvertSurface(logo_load, &fmt32, SDL_SWSURFACE);
				SDL_FreeSurface(logo_load);
				logo_load = NULL;
				break;
		}
		
		if (!logo_use)
		{
			if (logo_load)
				SDL_FreeSurface(logo_load);
			return 0;
		}
		
		SDL_LockSurface(logo_use);
		int logo_size = (logo_use->pitch * logo_use->h);
		if (bppMD != 32)
			memcpy(logo_data16, logo_use->pixels, MIN(logo_size, sizeof(logo_data16)));
		else
			memcpy(logo_data32, logo_use->pixels, MIN(logo_size, sizeof(logo_data32)));
		SDL_UnlockSurface(logo_use);
		
		// Free the logo.
		SDL_FreeSurface(logo_use);
		
		lastBpp = bppMD;
#else
		// TODO: LoadBitmap() uses the desktop color depth,
		// which may not be the same as bppMD.
		HBITMAP hbmpLogo = LoadBitmap(ghInstance, MAKEINTRESOURCE(IDB_GENS_LOGO_BIG));
		if (bppMD != 32)
			GetBitmapBits(hbmpLogo, sizeof(logo_data16), logo_data16);
		else
			GetBitmapBits(hbmpLogo, sizeof(logo_data32), logo_data32);
		lastBpp = bppMD;
#endif
	}
	
	if (bppMD != 32)
		T_Update_Gens_Logo<uint16_t, 5, 31>(MD_Screen, logo_data16);
	else
		T_Update_Gens_Logo<uint32_t, 40, 248>(MD_Screen32, logo_data32);
	
	return 1;
}


/**
 * T_Update_Crazy_Effect(): Update the "Crazy" Effect.
 * @param introEffectColor Intro effect color.
 */
template<typename pixel, pixel Rmask, pixel Gmask, pixel Bmask,
		  pixel Radd, pixel Gadd, pixel Badd>
static inline void T_Update_Crazy_Effect(int introEffectColor, pixel *screen)
{
	const pixel RBmask = (Rmask | Bmask);
	int r = 0, g = 0, b = 0;
	pixel RB, G;
	
	pixel *pix = &screen[336*240 - 1];
	pixel *prev_l = pix - 336;
	pixel *prev_p = pix - 1;
	
	for (unsigned int i = 336*240; i != 0; i--)
	{
		pixel pl, pp;
		pl = (prev_l >= screen ? *prev_l : 0);
		pp = (prev_p >= screen ? *prev_p : 0);
		
		RB = ((pl & RBmask) + (pp & RBmask)) >> 1;
		G = ((pl & Gmask) + (pp & Gmask)) >> 1;
		
		if (introEffectColor & 0x4)
		{
			// Red channel.
			r = RB & Rmask;
			r += (((rand() & 0x7FFF) > 0x2C00) ? Radd : -Radd);
			
			if (r > (int)Rmask)
				r = Rmask;
			else if (r < (int)Radd)
				r = 0;
		}
		
		if (introEffectColor & 0x2)
		{
			// Green channel.
			g = G & Gmask;
			g += (((rand() & 0x7FFF) > 0x2C00) ? Gadd : -Gadd);
			
			if (g > (int)Gmask)
				g = Gmask;
			else if (g < (int)Gadd)
				g = 0;
		}
		
		if (introEffectColor & 0x1)
		{
			// Blue channel.
			b = RB & Bmask;
			b += (((rand() & 0x7FFF) > 0x2C00) ? Badd : -Badd);
			
			if (b > (int)Bmask)
				b = Bmask;
			else if (b < (int)Badd)
				b = 0;
		}
		
		*pix = r | g | b;
		
		// Next pixels.
		prev_l--;
		prev_p--;
		pix--;
	}
}


/**
 * Update_Crazy_Effect(): Update the "Crazy" Effect.
 * @param introEffectColor Intro effect color.
 */
void Update_Crazy_Effect(int introEffectColor)
{
	switch (bppMD)
	{
		case 15:
			T_Update_Crazy_Effect<uint16_t, 0x7C00, 0x03E0, 0x001F,
					      0x0400, 0x0020, 0x0001>(introEffectColor, MD_Screen);
			break;
		
		case 16:
			T_Update_Crazy_Effect<uint16_t, 0xF800, 0x07C0, 0x001F,
					      0x0800, 0x0040, 0x0001>(introEffectColor, MD_Screen);
			break;
		
		case 32:
			T_Update_Crazy_Effect<uint32_t, (uint32_t)0xF80000, (uint32_t)0x00F800, (uint32_t)0x0000F8,
					      (uint32_t)0x080000, (uint32_t)0x000800, (uint32_t)0x000008>(introEffectColor, MD_Screen32);
			break;
	}
}


#if 0
/**
 * Update_Crazy_Effect(): Update the "Crazy" Effect.
 * @param introEffectColor Intro effect color.
 */
void Update_Crazy_Effect(int introEffectColor)
{
	int r = 0, g = 0, b = 0;
	int RB, G;
	
	uint16_t *pix = &MD_Screen[336*240 - 1];
	uint16_t *prev_l = pix - 336;
	uint16_t *prev_p = pix - 1;
	
	for (unsigned int i = 336*240; i != 0; i--)
	{
		uint16_t pl, pp;
		pl = (prev_l >= MD_Screen ? *prev_l : 0);
		pp = (prev_p >= MD_Screen ? *prev_p : 0);
		
		if (bppMD == 15)
		{
			RB = ((pl & 0x7C1F) + (pp & 0x7C1F)) >> 1;
			G = ((pl & 0x03E0) + (pp & 0x03E0)) >> 1;
			
			if (introEffectColor & 0x4)
			{
				r = RB & 0x7C00;
				if ((rand() & 0x7FFF) > 0x2C00) r += 0x0400;
				else r -= 0x0400;
				if (r > 0x7C00) r = 0x7C00;
				else if (r < 0x0400) r = 0;
			}
			
			if (introEffectColor & 0x2)
			{
				g = G & 0x03E0;
				if ((rand() & 0x7FFF) > 0x2C00) g += 0x0020;
				else g -= 0x0020;
				if (g > 0x03E0) g = 0x03E0;
				else if (g < 0x0020) g = 0;
			}
			
			if (introEffectColor & 0x1)
			{
				b = RB & 0x001F;
				if ((rand() & 0x7FFF) > 0x2C00) b++;
				else b--;
				if (b > 0x1F) b = 0x1F;
				else if (b < 0) b = 0;
			}
		}
		else
		{
			RB = ((pl & 0xF81F) + (pp & 0xF81F)) >> 1;
			G = ((pl & 0x07C0) + (pp & 0x07C0)) >> 1;
			
			if (introEffectColor & 0x4)
			{
				r = RB & 0xF800;
				if ((rand() & 0x7FFF) > 0x2C00) r += 0x0800;
				else r -= 0x0800;
				if (r > 0xF800) r = 0xF800;
				else if (r < 0x0800) r = 0;
			}
			
			if (introEffectColor & 0x2)
			{
				g = G & 0x07C0;
				if ((rand() & 0x7FFF) > 0x2C00) g += 0x0040;
				else g -= 0x0040;
				if (g > 0x07C0) g = 0x07C0;
				else if (g < 0x0040) g = 0;
			}
			
			if (introEffectColor & 0x1)
			{
				b = RB & 0x001F;
				if ((rand() & 0x7FFF) > 0x2C00) b++;
				else b--;
				if (b > 0x1F) b = 0x1F;
				else if (b < 0) b = 0;
			}
		}
		
		*pix = r | g | b;
		
		// Next pixels.
		prev_l--;
		prev_p--;
		pix--;
	}
}
#endif


/**
 * T_veffect_pause_tint(): Tint the screen a purple hue to indicate that emulation is paused.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param RMask Red component mask.
 * @param GMask Green component mask.
 * @param BMask Blue component mask.
 * @param RShift Red component shift.
 * @param GShift Green component shift.
 * @param BShift Blue component shift.
 * @param rInfo Rendering information.
 * @param scale Scaling value.
 */
template<typename pixel, pixel *mdScreen, pixel RMask, pixel GMask, pixel BMask, uint8_t RShift, uint8_t GShift, uint8_t BShift>
static void T_veffect_pause_tint(void)
{
	uint8_t r, g, b, nr, ng, nb;
	uint16_t sum;
	
	pixel *pos = mdScreen;
	
	for (unsigned int y = 240; y != 0; y--)
	{
		for (unsigned int x = 336; x != 0; x--)
		{
			// Get the color components.
			r = (uint8_t)((*pos & RMask) >> RShift);
			g = (uint8_t)((*pos & GMask) >> GShift);
			b = (uint8_t)((*pos & BMask) >> BShift);
			
			// Add the components together.
			sum = r + g + b;
			sum /= 3;
			nr = ng = nb = (uint8_t)sum;
			
			// L-shift the blue component to tint the image.
			nb <<= 1;
			if (nb > 0x1F)
				nb = 0x1F;
			
			// Mask off the LSB.
			nr &= 0x1E;
			ng &= 0x1E;
			nb &= 0x1E;
			
			// Put the new pixel.
			*pos++ = (nr << RShift) | (ng << GShift) | (nb << BShift);
		}
	}
}


/**
 * veffect_pause_tint(): Tint the screen a purple hue to indicate that emulation is paused.
 */
void veffect_pause_tint(void)
{
	switch (bppMD)
	{
		case 32:
			// 32-bit color.
			T_veffect_pause_tint<uint32_t, MD_Screen32,
					     (uint32_t)0xFF0000, (uint32_t)0x00FF00, (uint32_t)0x0000FF,
					     16+3, 8+3, 0+3>();
			break;
		case 16:
			// 16-bit color.
			T_veffect_pause_tint<uint16_t, MD_Screen,
					     (uint16_t)0xF800, (uint16_t)0x07C0, (uint32_t)0x001F,
					     11, 6, 0>();
			break;
		case 15:
			// 15-bit color.
			T_veffect_pause_tint<uint16_t, MD_Screen,
					     (uint16_t)0x7C00, (uint16_t)0x03E0, (uint32_t)0x001F,
					     10, 5, 0>();
	}
	
	if (ice == 2)
		ice = 3;
}

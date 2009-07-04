/***************************************************************************
 * Gens: Video Drawing - Hailu Backend.                                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * Copyright (c) 2009 by Phil Costin                                       *
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

#include "vdraw.h"
#include "vdraw_haiku.hpp"

// Message logging.
#include "macros/log_msg.h"

// Unused Parameter macro.
#include "macros/unused.h"

#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"

// VDraw C++ functions.
#include "vdraw_cpp.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// libgsft includes.
#include "libgsft/gsft_win32.h"

// Gens window.
#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"
#include "ui/gens_ui.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// RGB color conversion functions.
#include "vdraw_RGB.h"

// Inline video functions.
#include "v_inline.h"

// Audio Handler.
#include "audio/audio.h"


// X and Y resolutions.
static int Res_X;
static int Res_Y;


// Function prototypes.
int	vdraw_haiku_init(void);
int	vdraw_haiku_end(void);

void vdraw_haiku_clear_screen(void);

int	vdraw_haiku_flip(void);
void vdraw_haiku_update_renderer(void);


/**
 * vdraw_haiku_init(): Initialize the Haiku video subsystem.
 * @return 0 on success; non-zero on error.
 */
int vdraw_haiku_init(void)
{
	mdp_render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return 0;
	const int w = 320 * scale;
	const int h = 240 * scale;

	// TODO: Haiku vdraw init code

	// Reset the border color to make sure it's redrawn.
	vdraw_border_color_16 = ~MD_Palette[0];
	vdraw_border_color_32 = ~MD_Palette32[0];
	
	// SDL initialized successfully.
	return 0;
}


/**
 * vdraw_haiku_end(): Close the Haiku renderer.
 * @return 0 on success; non-zero on error.
 */
int vdraw_haiku_end(void)
{
	return 0;
}


/**
 * vdraw_haiku_clear_screen(): Clear the screen.
 */
void vdraw_haiku_clear_screen(void)
{
	//SDL_LockSurface(vdraw_sdl_screen);
	//memset(vdraw_sdl_screen->pixels, 0x00,
	       //vdraw_sdl_screen->pitch * vdraw_sdl_screen->h);
	//SDL_UnlockSurface(vdraw_sdl_screen);
	
	// Reset the border color to make sure it's redrawn.
	vdraw_border_color_16 = ~MD_Palette[0];
	vdraw_border_color_32 = ~MD_Palette32[0];
}


/**
 * vdraw_haiku_flip(): Flip the screen buffer. [Called by vdraw_flip().]
 * @return 0 on success; non-zero on error.
 */
int vdraw_haiku_flip(void)
{
	//SDL_LockSurface(vdraw_sdl_screen);
	
	// Draw the border.
	//vdraw_sdl_draw_border();
	
	//const int bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	// Start of the SDL framebuffer.
	//const int pitch = vdraw_sdl_screen->pitch;
	//const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
	//const int HBorder = vdraw_border_h * (bytespp / 2);	// Left border width, in pixels.
	
	//const int startPos = ((pitch * VBorder) + HBorder) * vdraw_scale;	// Starting position from within the screen.
	
	// Start of the SDL framebuffer.
	//unsigned char *start = &(((unsigned char*)(vdraw_sdl_screen->pixels))[startPos]);
	
	// Set up the render information.
	//vdraw_rInfo.destScreen = (void*)start;
	//vdraw_rInfo.width = 320 - vdraw_border_h;
	//vdraw_rInfo.height = VDP_Num_Vis_Lines;
	//vdraw_rInfo.destPitch = pitch;
/*	
	if (vdraw_needs_conversion)
	{
		// Color depth conversion is required.
		vdraw_rgb_convert(&vdraw_rInfo);
	}
	else
	{
		// Color conversion is not required.
		if (vdraw_get_fullscreen())
			vdraw_blitFS(&vdraw_rInfo);
		else
			vdraw_blitW(&vdraw_rInfo);
	}
*/	
	// Draw the message and/or FPS counter.
/*
	if (vdraw_msg_visible)
	{
		// Message is visible.
		draw_text(start, vdraw_sdl_screen->w,
			  vdraw_rInfo.width * vdraw_scale,
			  vdraw_rInfo.height * vdraw_scale,
			  vdraw_msg_text, &vdraw_msg_style, FALSE);
	}
	else if (vdraw_fps_enabled && (Game != NULL) && Active && !Paused && !Debug)
	{
		// FPS is enabled.
		draw_text(start, vdraw_sdl_screen->w,
			  vdraw_rInfo.width * vdraw_scale,
			  vdraw_rInfo.height * vdraw_scale,
			  vdraw_msg_text, &vdraw_fps_style, FALSE);
	}
	
	SDL_UnlockSurface(vdraw_sdl_screen);
	
	SDL_Flip(vdraw_sdl_screen);
*/	
	// TODO: Return appropriate error code.
	return 0;
}


/**
 * vdraw_haiku_draw_border(): Draw the border color.
 * Called from vdraw_haiku_flip().
 */
void vdraw_haiku_draw_border(void)
{
	// TODO: Consolidate this function by using a macro.
/*	
	SDL_Rect border;
	
	if (!Video.borderColorEmulation)
	{
		// Border color emulation is disabled.
		// Don't do anything if the border color is currently black.
		if (vdraw_border_color_16 == 0 && vdraw_border_color_32 == 0)
			return;
	}
	
	unsigned short bc16 = MD_Palette[0];
	unsigned int bc32 = MD_Palette32[0];
	
	if (!Video.borderColorEmulation || (Game == NULL) || (Debug > 0))
	{
		// Either no game is loaded or the debugger is enabled.
		// Make sure the border color is black.
		bc16 = 0;
		bc32 = 0;
	}
	
	if ((bppOut == 15 || bppOut == 16) && (vdraw_border_color_16 != bc16))
	{
		vdraw_border_color_16 = bc16;
		
		// Check if the border color needs to be converted.
		if (bppMD == 15 && bppOut == 16)
		{
			// MD palette is 15-bit; output is 16-bit.
			// MD:  0RRRRRGG GGGBBBBB
			// Out: RRRRRGGG GGGBBBBB
			bc16 = ((bc16 & 0x7C00) << 1) | ((bc16 & 0x03E0) << 1) | (bc16 & 0x1F);
		}
		else if (bppMD == 16 && bppOut == 15)
		{
			// MD palette is 16-bit; output is 15-bit.
			// MD:  RRRRRGGG GGGBBBBB
			// Out: 0RRRRRGG GGGBBBBB
			bc16 = ((bc16 & 0xF800) >> 1) | ((bc16 & 0x07C0) >> 1) | (bc16 & 0x1F);
		}
		
		if (VDP_Num_Vis_Lines < 240)
		{
			// Top/Bottom borders.
			border.x = 0; border.w = vdraw_sdl_screen->w;
			border.h = ((240 - VDP_Num_Vis_Lines) >> 1) * vdraw_scale;
			border.y = 0;
			SDL_FillRect(vdraw_sdl_screen, &border, bc16);
			border.y = vdraw_sdl_screen->h - border.h;
			SDL_FillRect(vdraw_sdl_screen, &border, bc16);
		}
		if (vdraw_border_h > 0)
		{
			// Left/Right borders.
			if (border.h != 0)
			{
				border.y = 0;
				border.h = 240 - border.h;
			}
			
			border.x = 0; border.h = vdraw_sdl_screen->h;
			border.w = (vdraw_border_h >> 1) * vdraw_scale;
			border.y = 0;
			SDL_FillRect(vdraw_sdl_screen, &border, bc16);
			border.x = vdraw_sdl_screen->w - border.w;
			SDL_FillRect(vdraw_sdl_screen, &border, bc16);
		}
	}
	else if ((bppOut == 32) && (vdraw_border_color_32 != bc32))
	{
		vdraw_border_color_32 = bc32;
		
		if (VDP_Num_Vis_Lines < 240)
		{
			// Top/Bottom borders.
			border.x = 0; border.w = vdraw_sdl_screen->w;
			border.h = ((240 - VDP_Num_Vis_Lines) >> 1) * vdraw_scale;
			border.y = 0;
			SDL_FillRect(vdraw_sdl_screen, &border, bc32);
			border.y = vdraw_sdl_screen->h - border.h;
			SDL_FillRect(vdraw_sdl_screen, &border, bc32);
		}
		if (vdraw_border_h > 0)
		{
			// Left/Right borders.
			if (border.h != 0)
			{
				border.y = 0;
				border.h = 240 - border.h;
			}
			
			border.x = 0; border.h = vdraw_sdl_screen->h;
			border.w = (vdraw_border_h >> 1) * vdraw_scale;
			border.y = 0;
			SDL_FillRect(vdraw_sdl_screen, &border, bc32);
			border.x = vdraw_sdl_screen->w - border.w;
			SDL_FillRect(vdraw_sdl_screen, &border, bc32);
		}
	}
*/
}


/**
 * vdraw_haiku_update_renderer(): Update the renderer.
 */
void vdraw_haiku_update_renderer(void)
{
/*
	// Check if a resolution switch is needed.
	mdp_render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return;
	const int w = 320 * scale;
	const int h = 240 * scale;
	
	if (vdraw_haiku_screen->w == w && vdraw_haiku_screen->h == h)
	{
		// No resolution switch is necessary. Simply clear the screen.
		vdraw_haiku_clear_screen();
		return;
	}
	
	// Resolution switch is needed.
	vdraw_haiku_end();
	vdraw_haiku_init();
	
	// Clear the screen.
	vdraw_haiku_clear_screen();
*/
}

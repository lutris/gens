/***************************************************************************
 * Gens: Video Drawing - SDL Backend.                                      *
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

#include "vdraw_sdl.h"
#include "vdraw_sdl_common.h"

// Message logging.
#include "macros/log_msg.h"

#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"
#include "ui/gens_ui.hpp"
#include "debugger/debugger.hpp"

// C includes.
#include <stdio.h>
#include <string.h>

// SDL includes.
#include <SDL/SDL.h>
#include "input/input_sdl_events.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// RGB color conversion functions.
#include "vdraw_RGB.h"


// Function prototypes.
static int	vdraw_sdl_init(void);
static int	vdraw_sdl_end(void);

static void	vdraw_sdl_clear_screen(void);

static int	vdraw_sdl_flip(void);
static void	vdraw_sdl_update_renderer(void);
static int	vdraw_sdl_reinit_gens_window(void);

// Used internally. (Not used in vdraw_backend_t.)
static void	vdraw_sdl_draw_border(void);

// Miscellaneous.
#define VDRAW_SDL_FLAGS (SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_ASYNCBLIT | SDL_HWACCEL)
static SDL_Surface *vdraw_sdl_screen;


// VDraw Backend struct.
const vdraw_backend_t vdraw_backend_sdl =
{
	.name			= "SDL",
	.flags			= VDRAW_BACKEND_FLAG_FULLSCREEN,
	
	.init			= vdraw_sdl_init,
	.end			= vdraw_sdl_end,
	
	.init_subsystem		= vdraw_sdl_common_init_subsystem,
	.shutdown		= vdraw_sdl_common_shutdown,
	
	.clear_screen		= vdraw_sdl_clear_screen,
	.update_vsync		= NULL,
	
	.flip			= vdraw_sdl_flip,
	.stretch_adjust		= NULL,
	.update_renderer	= vdraw_sdl_update_renderer,
	.reinit_gens_window	= vdraw_sdl_reinit_gens_window
};


/**
 * vdraw_sdl_init(): Initialize the SDL video subsystem.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_init(void)
{
	mdp_render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return 0;
	const int w = 320 * scale;
	const int h = 240 * scale;
	
	// Set up SDL embedding.
	vdraw_sdl_common_embed(w, h);
	
	// Return value.
	int rval;
	
	// Initialize SDL.
	rval = SDL_InitSubSystem(SDL_INIT_VIDEO);
	if (rval < 0)
	{
		// Error initializing SDL.
		LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
			"SDL_InitSubSystem(SDL_INIT_VIDEO) failed: 0x%08X", rval);
		return -1;
	}
	
	// Initialize the SDL backend.
	vdraw_sdl_screen = SDL_SetVideoMode(w, h, bppOut, VDRAW_SDL_FLAGS | (vdraw_get_fullscreen() ? SDL_FULLSCREEN : 0));
	if (!vdraw_sdl_screen)
	{
		// Error setting the SDL video mode.
		const char *sErr = SDL_GetError();
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
			"SDL_SetVideoMode() failed: %s", sErr);
		return -2;
	}
	
	// Disable the cursor in fullscreen mode.
	SDL_ShowCursor(vdraw_get_fullscreen() ? SDL_DISABLE : SDL_ENABLE);
	
	// Reset the border color to make sure it's redrawn.
	vdraw_border_color_16 = ~MD_Palette[0];
	vdraw_border_color_32 = ~MD_Palette32[0];
	
	// SDL initialized successfully.
	return 0;
}


/**
 * vdraw_sdl_end(): Close the SDL renderer.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_end(void)
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return 0;
}


/**
 * vdraw_sdl_clear_screen(): Clear the screen.
 */
static void vdraw_sdl_clear_screen(void)
{
	SDL_LockSurface(vdraw_sdl_screen);
	memset(vdraw_sdl_screen->pixels, 0x00,
	       vdraw_sdl_screen->pitch * vdraw_sdl_screen->h);
	SDL_UnlockSurface(vdraw_sdl_screen);
	
	// Reset the border color to make sure it's redrawn.
	vdraw_border_color_16 = ~MD_Palette[0];
	vdraw_border_color_32 = ~MD_Palette32[0];
}


/**
 * vdraw_sdl_flip(): Flip the screen buffer. [Called by vdraw_flip().]
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_flip(void)
{
	SDL_LockSurface(vdraw_sdl_screen);
	
	// Draw the border.
	vdraw_sdl_draw_border();
	
	const int bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	// Start of the SDL framebuffer.
	const int pitch = vdraw_sdl_screen->pitch;
	const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
	const int HBorder = vdraw_border_h * (bytespp / 2);	// Left border width, in pixels.
	
	const int startPos = ((pitch * VBorder) + HBorder) * vdraw_scale;	// Starting position from within the screen.
	
	// Start of the SDL framebuffer.
	unsigned char *start = &(((unsigned char*)(vdraw_sdl_screen->pixels))[startPos]);
	
	// Set up the render information.
	vdraw_rInfo.destScreen = (void*)start;
	vdraw_rInfo.width = 320 - vdraw_border_h;
	vdraw_rInfo.height = VDP_Num_Vis_Lines;
	vdraw_rInfo.destPitch = pitch;
	
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
	
	// Draw the message and/or FPS counter.
	if (vdraw_msg_visible)
	{
		// Message is visible.
		draw_text(start, vdraw_sdl_screen->w,
			  vdraw_rInfo.width * vdraw_scale,
			  vdraw_rInfo.height * vdraw_scale,
			  vdraw_msg_text, &vdraw_msg_style, FALSE);
	}
	else if (vdraw_fps_enabled && (Game != NULL) && Settings.Active && !Settings.Paused && !IS_DEBUGGING())
	{
		// FPS is enabled.
		draw_text(start, vdraw_sdl_screen->w,
			  vdraw_rInfo.width * vdraw_scale,
			  vdraw_rInfo.height * vdraw_scale,
			  vdraw_msg_text, &vdraw_fps_style, FALSE);
	}
	
	SDL_UnlockSurface(vdraw_sdl_screen);
	
	SDL_Flip(vdraw_sdl_screen);
	
	// TODO: Return appropriate error code.
	return 0;
}


/**
 * vdraw_sdl_draw_border(): Draw the border color.
 * Called from vdraw_sdl_flip().
 */
static void vdraw_sdl_draw_border(void)
{
	// TODO: Consolidate this function by using a macro.
	
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
	
	if (!Video.borderColorEmulation || (Game == NULL) || IS_DEBUGGING())
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
}


/**
 * vdraw_sdl_update_renderer(): Update the renderer.
 */
static void vdraw_sdl_update_renderer(void)
{
	// Check if a resolution switch is needed.
	mdp_render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return;
	const int w = 320 * scale;
	const int h = 240 * scale;
	
	if (vdraw_sdl_screen->w == w && vdraw_sdl_screen->h == h)
	{
		// No resolution switch is necessary. Simply clear the screen.
		vdraw_sdl_clear_screen();
		return;
	}
	
	// Resolution switch is needed.
	vdraw_sdl_end();
	vdraw_sdl_init();
	
	// Clear the screen.
	vdraw_sdl_clear_screen();
}


/**
 * vdraw_sdl_reinit_gens_window(): Reinitialize the Gens window.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_reinit_gens_window(void)
{
	// Hide the Gens window if fullscreen.
	// Show the Gens window if windowed.
	GensUI_setWindowVisibility(!vdraw_get_fullscreen());
	
	// Reset the SDL modifier keys.
	input_sdl_reset_modifiers();
	
	// TODO: Make the Gens window non-resizable.
	return 0;
}

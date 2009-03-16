/***************************************************************************
 * Gens: Video Drawing - SDL Backend.                                      *
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

#include "vdraw_sdl.h"

// Message logging.
#include "macros/log_msg.h"

#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// SDL includes.
#include <SDL/SDL.h>

// GTK+ includes.
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "ui/gtk/gtk-compat.h"

// Gens window.
#include "gens/gens_window.h"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Text drawing functions.
#include "vdraw_text.hpp"


// Function prototypes.
static int	vdraw_sdl_init(void);
static int	vdraw_sdl_end(void);

static int	vdraw_sdl_init_subsystem(void);
static int	vdraw_sdl_shutdown(void);

static void	vdraw_sdl_clear_screen(void);

static int	vdraw_sdl_flip(void);
static void	vdraw_sdl_draw_border(void); // Not used in vdraw_backend_t.
static void	vdraw_sdl_update_renderer(void);


// Miscellaneous.
#define VDRAW_SDL_FLAGS (SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_ASYNCBLIT | SDL_HWACCEL)
static SDL_Surface *vdraw_sdl_screen;


// VDraw Backend struct.
const vdraw_backend_t vdraw_backend_sdl =
{
	.name = "SDL",
	.flags = 0,
	
	.init = vdraw_sdl_init,
	.end = vdraw_sdl_end,
	
	.init_subsystem = vdraw_sdl_init_subsystem,
	.shutdown = vdraw_sdl_shutdown,
	
	.clear_screen = vdraw_sdl_clear_screen,
	.update_vsync = NULL,
	
	.flip = vdraw_sdl_flip,
	.stretch_adjust = NULL,
	.update_renderer = vdraw_sdl_update_renderer
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
	
	if (vdraw_get_fullscreen())
	{
		// Hide the embedded SDL window.
		gtk_widget_hide(gens_window_sdlsock);
		unsetenv("SDL_WINDOWID");
	}
	else
	{
		// Show the embedded SDL window.
		gtk_widget_set_size_request(gens_window_sdlsock, w, h);
		gtk_widget_realize(gens_window_sdlsock);
		gtk_widget_show(gens_window_sdlsock);
		
		// Wait for GTK+ to catch up.
		// TODO: If gtk_main_iteration_do() returns TRUE, exit the program.
		while (gtk_events_pending())
			gtk_main_iteration_do(FALSE);
		
		// Get the Window ID of the SDL socket.
		char SDL_WindowID[24];
		sprintf(SDL_WindowID, "%d", (int)(GDK_WINDOW_XWINDOW(gtk_widget_get_window(gens_window_sdlsock))));
		setenv("SDL_WINDOWID", SDL_WindowID, 1);
	}
	
	// Initialize SDL.
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		// Error initializing SDL.
		vdraw_init_fail("Couldn't initialize embedded SDL.");
		return 0;
	}
	
	// Initialize the SDL backend.
	vdraw_sdl_screen = SDL_SetVideoMode(w, h, bppOut, VDRAW_SDL_FLAGS | (vdraw_get_fullscreen() ? SDL_FULLSCREEN : 0));
	if (!vdraw_sdl_screen)
	{
		// Error initializing SDL.
		LOG_MSG(video, LOG_MSG_LEVEL_CRITICAL,
			"Error creating SDL primary surface: %s", SDL_GetError());
		exit(1);
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
 * vdraw_sdl_init_subsystem(): Initialize the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_init_subsystem(void)
{
	if (SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
	{
		LOG_MSG(video, LOG_MSG_LEVEL_CRITICAL,
			"Error initializing SDL timers: %s", SDL_GetError());
		return -1;
	}
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		LOG_MSG(video, LOG_MSG_LEVEL_CRITICAL,
			"Error initializing SDL video: %s", SDL_GetError());
		return -1;
	}
	
	// Take it back down now that we know it works.
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	
	// Initialized successfully.
	return 0;
}


/**
 * vdraw_sdl_shutdown(): Shut down the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_shutdown(void)
{
	SDL_Quit();
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
	
	if (bppMD == 16 && bppOut != 16)
	{
		// MDP_RENDER_FLAG_SRC16DST32.
		// Render as 16-bit to an internal surface.
		
		// Make sure the internal surface is initialized.
		if (vdraw_16to32_scale != vdraw_scale)
		{
			if (vdraw_16to32_surface)
				free(vdraw_16to32_surface);
			
			vdraw_16to32_scale = vdraw_scale;
			vdraw_16to32_pitch = 320 * vdraw_scale * 2;
			vdraw_16to32_surface = (uint16_t*)(malloc(vdraw_16to32_pitch * 240 * vdraw_scale));
		}
		
		vdraw_rInfo.destScreen = (void*)vdraw_16to32_surface;
		vdraw_rInfo.destPitch = vdraw_16to32_pitch;
		if (vdraw_get_fullscreen())
			vdraw_blitFS(&vdraw_rInfo);
		else
			vdraw_blitW(&vdraw_rInfo);
		
		vdraw_render_16to32((uint32_t*)start, vdraw_16to32_surface,
				    vdraw_rInfo.width * vdraw_scale, vdraw_rInfo.height * vdraw_scale,
				    pitch, vdraw_16to32_pitch);
	}
	else
	{
		if (vdraw_get_fullscreen())
			vdraw_blitFS(&vdraw_rInfo);
		else
			vdraw_blitW(&vdraw_rInfo);
	}
	
	// Draw the message and/or FPS counter.
	if (vdraw_msg_visible)
	{
		// Message is visible.
		draw_text(vdraw_sdl_screen->pixels, vdraw_sdl_screen->w,
			  vdraw_sdl_screen->w, vdraw_sdl_screen->h,
			  vdraw_msg_text, &vdraw_msg_style, TRUE);
	}
	else if (vdraw_fps_enabled && (Game != NULL) && Active && !Paused && !Debug)
	{
		// FPS is enabled.
		draw_text(vdraw_sdl_screen->pixels, vdraw_sdl_screen->w,
			  vdraw_sdl_screen->w, vdraw_sdl_screen->h,
			  vdraw_msg_text, &vdraw_fps_style, TRUE);
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
	
	unsigned short new_border_color_16 = MD_Palette[0];
	unsigned int new_border_color_32 = MD_Palette32[0];
	
	if (!Video.borderColorEmulation || (Game == NULL) || (Debug > 0))
	{
		// Either no game is loaded or the debugger is enabled.
		// Make sure the border color is black.
		new_border_color_16 = 0;
		new_border_color_32 = 0;
	}
	
	if ((bppOut == 15 || bppOut == 16) && (vdraw_border_color_16 != new_border_color_16))
	{
		vdraw_border_color_16 = new_border_color_16;
		if (VDP_Num_Vis_Lines < 240)
		{
			// Top/Bottom borders.
			border.x = 0; border.w = vdraw_sdl_screen->w;
			border.h = ((240 - VDP_Num_Vis_Lines) >> 1) * vdraw_scale;
			border.y = 0;
			SDL_FillRect(vdraw_sdl_screen, &border, vdraw_border_color_16);
			border.y = vdraw_sdl_screen->h - border.h;
			SDL_FillRect(vdraw_sdl_screen, &border, vdraw_border_color_16);
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
			SDL_FillRect(vdraw_sdl_screen, &border, vdraw_border_color_16);
			border.x = vdraw_sdl_screen->w - border.w;
			SDL_FillRect(vdraw_sdl_screen, &border, vdraw_border_color_16);
		}
	}
	else if ((bppOut == 32) && (vdraw_border_color_32 != new_border_color_32))
	{
		vdraw_border_color_32 = new_border_color_32;
		if (VDP_Num_Vis_Lines < 240)
		{
			// Top/Bottom borders.
			border.x = 0; border.w = vdraw_sdl_screen->w;
			border.h = ((240 - VDP_Num_Vis_Lines) >> 1) * vdraw_scale;
			border.y = 0;
			SDL_FillRect(vdraw_sdl_screen, &border, vdraw_border_color_32);
			border.y = vdraw_sdl_screen->h - border.h;
			SDL_FillRect(vdraw_sdl_screen, &border, vdraw_border_color_32);
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
			SDL_FillRect(vdraw_sdl_screen, &border, vdraw_border_color_32);
			border.x = vdraw_sdl_screen->w - border.w;
			SDL_FillRect(vdraw_sdl_screen, &border, vdraw_border_color_32);
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

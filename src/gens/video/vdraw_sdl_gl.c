/***************************************************************************
 * Gens: Video Drawing - SDL+OpenGL Backend.                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#include "vdraw_sdl_gl.h"
#include "vdraw_sdl_common.h"

// Message logging.
#include "macros/log_msg.h"

#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"
#include "ui/gens_ui.hpp"
#include "debugger/debugger.hpp"

// C includes.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// SDL includes.
#include <SDL/SDL.h>
#include "input/input_sdl_events.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// Common OpenGL functions.
#include "vdraw_gl.h"

// Function prototypes.
static int	vdraw_sdl_gl_init(void);
static int	vdraw_sdl_gl_end(void);

static int	vdraw_sdl_gl_flip(void);
static void	vdraw_sdl_gl_update_renderer(void);
static int	vdraw_sdl_gl_reinit_gens_window(void);

// Used internally. (Not used in vdraw_backend_t.)
static int	vdraw_sdl_gl_init_opengl(const int w, const int h, const BOOL reinitSDL);

// Miscellaneous.
#define VDRAW_SDL_GL_FLAGS (SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_ASYNCBLIT | SDL_HWACCEL | SDL_OPENGL)
static SDL_Surface *vdraw_sdl_gl_screen;


// VDraw Backend struct.
const vdraw_backend_t vdraw_backend_sdl_gl =
{
	.name			= "SDL+OpenGL",
	.flags			= VDRAW_BACKEND_FLAG_VSYNC |
				  VDRAW_BACKEND_FLAG_STRETCH |
				  VDRAW_BACKEND_FLAG_FULLSCREEN,
	
	.init			= vdraw_sdl_gl_init,
	.end			= vdraw_sdl_gl_end,
	
	.init_subsystem		= vdraw_sdl_common_init_subsystem,
	.shutdown		= vdraw_sdl_common_shutdown,
	
	.clear_screen		= vdraw_gl_clear_screen,
	.update_vsync		= vdraw_gl_update_vsync,
	
	.flip			= vdraw_sdl_gl_flip,
	.update_renderer	= vdraw_sdl_gl_update_renderer,
	.reinit_gens_window	= vdraw_sdl_gl_reinit_gens_window
};


/**
 * vdraw_sdl_gl_init(): Initialize the SDL+OpenGL video subsystem.
 * @return 0 on success; non-zero on error.
 */
int vdraw_sdl_gl_init(void)
{
	// Make sure OpenGL is supported by the current system.
	if (!vdraw_gl_is_supported())
		return -1;
	
	vdraw_gl_init_vsync();
	
	// Set up SDL embedding.
	vdraw_sdl_common_embed(Video.GL.width, Video.GL.height);
	
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
	
	// Initialize the SDL+OpenGL backend.
	int x = vdraw_sdl_gl_init_opengl(Video.GL.width, Video.GL.height, TRUE);
	
	if (x == 0)
	{
		// Disable the cursor in fullscreen mode.
		SDL_ShowCursor(vdraw_get_fullscreen() ? SDL_DISABLE : SDL_ENABLE);
	}
	
	// Return the status code from vdraw_sdl_gl_init_opengl().
	return x;
}


/**
 * vdraw_sdl_gl_set_visual(): Set OpenGL visual attributes.
 * @param depth Color depth.
 * @param r Red bits.
 * @param g Green bits.
 * @param b Blue bits.
 * @param a Alpha bits.
 * @param format Pixel format.
 * @param type Data type.
 */
static inline void vdraw_sdl_gl_set_visual(unsigned int depth,
					   unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depth);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   r);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, g);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  b);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, a);
}


/**
 * vdraw_sdl_gl_init_opengl(): Initialize the OpenGL backend.
 * @param w Width.
 * @param h Height.
 * @param reinitSDL If TRUE, reinitializes SDL.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_gl_init_opengl(const int w, const int h, const BOOL reinitSDL)
{
	if (reinitSDL)
	{
		// Set various OpenGL attributes.
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		
		// Color depth values.
		if (bppOut == 15)
		{
			// 15-bit color. (Mode 555)
			vdraw_sdl_gl_set_visual(15, 5, 5, 5, 0);
			vdraw_gl_set_visual(GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV);
		}
		else if (bppOut == 16)
		{
			// 16-bit color. (Mode 565)
			vdraw_sdl_gl_set_visual(16, 5, 6, 5, 0);
			vdraw_gl_set_visual(GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
		}
		else //if (bppOut == 32)
		{
			// 32-bit color.
			vdraw_sdl_gl_set_visual(24, 8, 8, 8, 0);
			vdraw_gl_set_visual(GL_BGRA, GL_UNSIGNED_BYTE);
		}
		
		vdraw_sdl_gl_screen = SDL_SetVideoMode(w, h, 0, VDRAW_SDL_GL_FLAGS | (vdraw_get_fullscreen() ? SDL_FULLSCREEN : 0));
		if (!vdraw_sdl_gl_screen)
		{
			// Error setting the SDL video mode.
			const char *sErr = SDL_GetError();
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"SDL_SetVideoMode() failed: %s", sErr);
			
			SDL_QuitSubSystem(SDL_INIT_VIDEO);
			return -1;
		}
	}
	
	// Update VSync.
	vdraw_gl_update_vsync(TRUE);
	
	// Initialize OpenGL.
	return vdraw_gl_init(w, h);
}


/**
 * vdraw_sdl_gl_end(): Close the SDL+OpenGL renderer.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_gl_end(void)
{
	// Shut down OpenGL.
	vdraw_gl_end();
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return 0;
}


/**
 * vdraw_sdl_gl_flip(): Flip the screen buffer. [Called by vdraw_flip().]
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_gl_flip(void)
{
	vdraw_gl_flip();
	
	// Swap the SDL GL buffers.
	SDL_GL_SwapBuffers();
	
	// TODO: Return appropriate error code.
	return 0;
}


/**
 * vdraw_gl_update_renderer(): Update the renderer.
 */
static void vdraw_sdl_gl_update_renderer(void)
{
	int ret = vdraw_gl_update_renderer(vdraw_sdl_gl_screen->w, vdraw_sdl_gl_screen->h);
	if (ret == 0)
		return;
	
	// Reinitialize the GL buffers.
	if (vdraw_sdl_gl_screen->w == Video.GL.width &&
	    vdraw_sdl_gl_screen->h == Video.GL.height)
	{
		// Output resolution is the same. Don't reinitialize SDL.
		vdraw_sdl_gl_init_opengl(Video.GL.width, Video.GL.height, FALSE);
	}
	else
	{
		// Output resolution has changed. Reinitialize SDL.
		vdraw_sdl_gl_end();
		vdraw_sdl_gl_init();
	}
	
	// Clear the screen.
	vdraw_gl_clear_screen();
}


/**
 * vdraw_sdl_gl_reinit_gens_window(): Reinitialize the Gens window.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_gl_reinit_gens_window(void)
{
	// Hide the Gens window if fullscreen.
	// Show the Gens window if windowed.
	GensUI_setWindowVisibility(!vdraw_get_fullscreen());
	
	// Reset the SDL modifier keys.
	input_sdl_reset_modifiers();
	
	// TODO: Make the Gens window resizable.
	return 0;
}

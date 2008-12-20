/***************************************************************************
 * Gens: Video Drawing - SDL+OpenGL Backend.                               *
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

#include "vdraw_sdl_gl.h"

#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Other includes.
#include <SDL/SDL.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "ui/gtk/gtk-misc.h"

// Gens window.
#include "gens/gens_window.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// CPU flags.
#include "gens_core/misc/cpuflags.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// Aligned malloc().
#include "macros/malloc_align.h"


// OpenGL includes
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>

// Needed for VSync on Linux.
// Code copied from Mesa's glxswapcontrol.c
// TODO: Make sure this works correctly on all drivers.

#ifndef GLX_MESA_swap_control
typedef GLint (*PFNGLXSWAPINTERVALMESAPROC)(unsigned interval);
typedef GLint (*PFNGLXGETSWAPINTERVALMESAPROC)( void );
#endif

#if !defined(GLX_OML_sync_control) && defined(_STDINT_H)
#define GLX_OML_sync_control 1
typedef Bool (*PFNGLXGETMSCRATEOMLPROC)(Display *dpy, GLXDrawable drawable, int32_t *numerator, int32_t *denominator);
#endif

#ifndef GLX_MESA_swap_frame_usage
#define GLX_MESA_swap_frame_usage 1
typedef int (*PFNGLXGETFRAMEUSAGEMESAPROC)(Display *dpy, GLXDrawable drawable, float * usage);
#endif


// Function prototypes.
static int	vdraw_sdl_gl_init(void);
static int	vdraw_sdl_gl_end(void);

static int	vdraw_sdl_gl_init_subsystem(void);
static int	vdraw_sdl_gl_shutdown(void);

static void	vdraw_sdl_gl_clear_screen(void);
static void	vdraw_sdl_gl_update_vsync(const BOOL fromInitSDLGL);

static int	vdraw_sdl_gl_flip(void);
static void	vdraw_sdl_gl_draw_border(void); // Not used in vdraw_backend_t.
static void	vdraw_sdl_gl_stretch_adjust(void);
static void	vdraw_sdl_gl_update_renderer(void);

// Used internally. (Not used in vdraw_backend_t.)
static int	vdraw_sdl_gl_init_opengl(const int w, const int h, const BOOL reinitSDL);

// Miscellaneous.
#define VDRAW_SDL_GL_FLAGS (SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_ASYNCBLIT | SDL_HWACCEL | SDL_OPENGL)
static SDL_Surface *vdraw_sdl_gl_screen;

// OpenGL variables.
GLuint textures[1] = {0};
int rowLength = 0;
int textureSize = 0;
unsigned char *filterBuffer = NULL;
int filterBufferSize = 0;
unsigned int m_pixelFormat = 0;
unsigned int m_pixelType = 0;

// Render parameters
double m_HRender = 0.0, m_VRender = 0.0;

// Stretch parameters
double m_HStretch = 0.0, m_VStretch = 0.0;

// OpenGL VSync stuff
// Copied from Mesa's glxswapcontrol.c
PFNGLXSWAPINTERVALMESAPROC set_swap_interval = NULL;
PFNGLXGETSWAPINTERVALMESAPROC get_swap_interval = NULL;


/**
 * vdraw_sdl_gl_calc_texture_size(): Calculate the texture size for the given scale value.
 * @param scale Scale value.
 * @return Texture size.
 */
static inline int vdraw_sdl_gl_calc_texture_size(int scale)
{
	const int tex_size = 256 * scale;
	
	// Check that the texture size is a power of two.
	// TODO: Optimize this code.
	
	if (tex_size <= 256)
		return 256;
	else if (tex_size <= 512)
		return 512;
	else if (tex_size <= 1024)
		return 1024;
	else if (tex_size <= 2048)
		return 2048;
	else if (tex_size <= 4096)
		return 4096;
	
	// Texture size is larger than 4096.
	return tex_size;
}


// VDraw Backend struct.
vdraw_backend_t vdraw_backend_sdl_gl =
{
	.init = vdraw_sdl_gl_init,
	.end = vdraw_sdl_gl_end,
	
	.init_subsystem = vdraw_sdl_gl_init_subsystem,
	.shutdown = vdraw_sdl_gl_shutdown,
	
	.clear_screen = vdraw_sdl_gl_clear_screen,
	.update_vsync = vdraw_sdl_gl_update_vsync,
	
	.flip = vdraw_sdl_gl_flip,
	.stretch_adjust = vdraw_sdl_gl_stretch_adjust,
	.update_renderer = vdraw_sdl_gl_update_renderer,
	
	.reinit_gens_window = NULL
};


/**
 * vdraw_sdl_gl_init(): Initialize the SDL+OpenGL video subsystem.
 * @return 0 on success; non-zero on error.
 */
int vdraw_sdl_gl_init(void)
{
	// OpenGL width/height.
	// TODO: Move these values here or something.
	const int w = Video.Width_GL;
	const int h = Video.Height_GL;
	
	if (vdraw_get_fullscreen())
	{
		// Hide the embedded SDL window.
		gtk_widget_hide(lookup_widget(gens_window, "sdlsock"));
		
		unsetenv("SDL_WINDOWID");
	}
	else
	{
		// Show the embedded SDL window.
		GtkWidget *sdlsock = lookup_widget(gens_window, "sdlsock");
		gtk_widget_set_size_request(sdlsock, w, h);
		gtk_widget_realize(sdlsock);
		gtk_widget_show(sdlsock);
		
		// Wait for GTK+ to catch up.
		// TODO: If gtk_main_iteration_do() returns TRUE, exit the program.
		while (gtk_events_pending())
			gtk_main_iteration_do(FALSE);
		
		// Get the Window ID of the SDL socket.
		char SDL_WindowID[24];
		sprintf(SDL_WindowID, "%d", (int)(GDK_WINDOW_XWINDOW(sdlsock->window)));
		setenv("SDL_WINDOWID", SDL_WindowID, 1);
	}
	
	// Initialize SDL.
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		// Error initializing SDL.
		vdraw_init_fail("Couldn't initialize embedded SDL.");
		return 0;
	}
	
	// Initialize the SDL+OpenGL backend.
	int x = vdraw_sdl_gl_init_opengl(w, h, TRUE);
	
	// Disable the cursor in fullscreen mode.
	SDL_ShowCursor(vdraw_get_fullscreen() ? SDL_DISABLE : SDL_ENABLE);
	
	// Return the status code from vdraw_sdl_gl_init_opengl().
	return x;
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
		vdraw_sdl_gl_screen = SDL_SetVideoMode(w, h, bppOut, VDRAW_SDL_GL_FLAGS | (vdraw_get_fullscreen() ? SDL_FULLSCREEN : 0));
		if (!vdraw_sdl_gl_screen)
		{
			// Error initializing SDL.
			fprintf(stderr, "%s(): Error creating SDL primary surface: %s\n",
				__func__, SDL_GetError());
			exit(1);
		}
	}
	
	// Update VSync.
	vdraw_sdl_gl_update_vsync(TRUE);
	
	MDP_Render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
        // Determine the texture size using the scaling factor.
	if (scale <= 0)
		return 1;
	
	rowLength = 320 * scale;
	textureSize = vdraw_sdl_gl_calc_texture_size(scale);
	
	// Calculate the rendering parameters.
	m_HRender = (double)(rowLength) / (double)(textureSize*2);
	m_VRender = (double)(240 * scale) / (double)(textureSize);
	
	// Allocate the filter buffer.
	int bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	filterBufferSize = rowLength * textureSize * bytespp;
	filterBuffer = (unsigned char*)(gens_malloc_align(filterBufferSize, 16));
	
	glViewport(0, 0, vdraw_sdl_gl_screen->w, vdraw_sdl_gl_screen->h);
	
	// GL Orthographic Projection code imported from Gens/Linux 2.15.4.
	// TODO: Is this stuff really necessary?
	// NOTE: Disabled for now due to garbage problems.
#if 0
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if ((Video.Width_GL * 3 > Video.Height_GL * 4) && Video.Height_GL != 0)
	{
		glOrtho(-((float)Video.Width_GL * 3) / ((float)Video.Height_GL * 4),
			((float)Video.Width_GL * 3) / ((float)Video.Height_GL * 4),
			-1, 1, -1, 1);
	}
	else if ((Video.Width_GL * 3 < Video.Height_GL * 4) && Video.Width_GL != 0)
	{
		glOrtho(-1, 1,
			-((float)Video.Height_GL * 4) / ((float)Video.Width_GL * 3),
			((float)Video.Height_GL * 3) / ((float)Video.Width_GL * 3),
			-1, 1);
	}
	else
	{
		glOrtho(-1, 1, -1, 1, -1, 1);
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#endif
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, textures);
	
	// Enable double buffering.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	// Color depth values.
	if (bppOut == 15)
	{
		// 15-bit color. (Mode 555)
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 15);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   5);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  0);
		m_pixelType = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		m_pixelFormat = GL_BGRA;
	}
	else if (bppOut == 16)
	{
		// 16-bit color. (Mode 565)
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   5);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  0);
		m_pixelType = GL_UNSIGNED_SHORT_5_6_5;
		m_pixelFormat = GL_RGB;
	}
	else //if (bppOut == 32)
	{
		// 32-bit color.
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  0);
		m_pixelType = GL_UNSIGNED_BYTE;
		m_pixelFormat = GL_BGRA;
	}
	
	// Initialize the texture.
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	vdraw_sdl_gl_clear_screen();
	
	// Set GL clamping parameters.
	// TODO: GL_CLAMP_TO_EDGE or GL_CLAMP?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// GL filters.
	// TODO: Set MAG filter when the linear filter setting is changed.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	// Set the texture format.
	glTexImage2D(GL_TEXTURE_2D, 0, 3, textureSize * 2, textureSize, 0,
		     m_pixelFormat, m_pixelType, NULL);
	
	// Adjust stretch parameters.
	vdraw_sdl_gl_stretch_adjust();
	
	return 0;
}

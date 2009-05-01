/***************************************************************************
 * Gens: Video Drawing - SDL+OpenGL Backend.                               *
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

#include "vdraw_sdl_gl.h"
#include "vdraw_sdl_common.h"

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

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// Video effects.
#include "v_effects.hpp"

// Aligned malloc().
#include "macros/malloc_align.h"


// OpenGL includes
#ifndef __APPLE__

// Linux/UNIX.
#include <GL/gl.h>

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glext.h>

// Needed for VSync on Linux.
// Code copied from Mesa's glxswapcontrol.c
// TODO: Make sure this works correctly on all drivers.

#ifndef GLX_MESA_swap_control
typedef GLint (*PFNGLXSWAPINTERVALMESAPROC)(unsigned interval);
typedef GLint (*PFNGLXGETSWAPINTERVALMESAPROC)(void);
#endif

#if !defined(GLX_OML_sync_control) && defined(_STDINT_H)
#define GLX_OML_sync_control 1
typedef Bool (*PFNGLXGETMSCRATEOMLPROC)(Display *dpy, GLXDrawable drawable, int32_t *numerator, int32_t *denominator);
#endif

#ifndef GLX_MESA_swap_frame_usage
#define GLX_MESA_swap_frame_usage 1
typedef int (*PFNGLXGETFRAMEUSAGEMESAPROC)(Display *dpy, GLXDrawable drawable, float *usage);
#endif

// OpenGL VSync stuff
// Copied from Mesa's glxswapcontrol.c
PFNGLXSWAPINTERVALMESAPROC set_swap_interval = NULL;
PFNGLXGETSWAPINTERVALMESAPROC get_swap_interval = NULL;

#else /* __APPLE__ */

// MacOS X.
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>

#endif /* __APPLE __ */

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


// VDraw Backend struct.
const vdraw_backend_t vdraw_backend_sdl_gl =
{
	.name = "SDL+OpenGL",
	.flags = VDRAW_BACKEND_FLAG_VSYNC | VDRAW_BACKEND_FLAG_STRETCH,
	
	.init = vdraw_sdl_gl_init,
	.end = vdraw_sdl_gl_end,
	
	.init_subsystem = vdraw_sdl_gl_init_subsystem,
	.shutdown = vdraw_sdl_gl_shutdown,
	
	.clear_screen = vdraw_sdl_gl_clear_screen,
	.update_vsync = vdraw_sdl_gl_update_vsync,
	
	.flip = vdraw_sdl_gl_flip,
	.stretch_adjust = vdraw_sdl_gl_stretch_adjust,
	.update_renderer = vdraw_sdl_gl_update_renderer,
};


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


/**
 * vdraw_sdl_gl_init(): Initialize the SDL+OpenGL video subsystem.
 * @return 0 on success; non-zero on error.
 */
int vdraw_sdl_gl_init(void)
{
#ifndef __APPLE__
	// Linux/UNIX: Get the VSync functions.
	// TODO: Copy functions to test for VSync from Mesa's glxswapcontrol.c
	/*
	has_OML_sync_control = is_extension_supported("GLX_OML_sync_control");
	has_SGI_swap_control = is_extension_supported("GLX_SGI_swap_control");
	has_MESA_swap_control = is_extension_supported("GLX_MESA_swap_control");
	*/
	set_swap_interval = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI");
	get_swap_interval = (PFNGLXGETSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXGetSwapIntervalSGI");
#endif
	
	// OpenGL width/height.
	// TODO: Move these values here or something.
	const int w = Video.Width_GL;
	const int h = Video.Height_GL;
	
	// Set up SDL embedding.
	vdraw_sdl_common_embed(w, h);
	
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
			LOG_MSG(video, LOG_MSG_LEVEL_CRITICAL,
				"Error creating SDL primary surface: %s", SDL_GetError());
			exit(1);
		}
	}
	
	// Update VSync.
	vdraw_sdl_gl_update_vsync(TRUE);
	
	mdp_render_t *rendMode = get_mdp_render_t();
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


/**
 * vdraw_sdl_gl_end(): Close the SDL+OpenGL renderer.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_gl_end(void)
{
	if (filterBuffer)
	{
		// Delete the GL textures and filter buffer.
		glDeleteTextures(1, textures);
		free(filterBuffer);
		filterBuffer = NULL;
	}
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	return 0;
}


/**
 * vdraw_sdl_gl_init_subsystem(): Initialize the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_gl_init_subsystem(void)
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
 * vdraw_sdl_gl_shutdown(): Shut down the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_gl_shutdown(void)
{
	SDL_Quit();
	return 0;
}


/**
 * vdraw_sdl_gl_stretch_adjust(): Adjust stretch parameters.
 * Called by either vdraw or another function in vdraw_sdl_gl.
 */
static void vdraw_sdl_gl_stretch_adjust(void)
{
	uint8_t stretch = vdraw_get_stretch();
	
	if (stretch & STRETCH_H)
		m_HStretch = (((double)vdraw_border_h * (((double)rowLength / (double)textureSize)) / 20.0) / 64.0);
	else
		m_HStretch = 0;
	
	if (stretch & STRETCH_V)
	{
		// TODO: Fix this ugly hack.
		if (vdraw_scale == 3)
			m_VStretch = (((240 - VDP_Num_Vis_Lines) / 240.0f) / (2.0 + (2.0/3.0)));
		else
			m_VStretch = (((240 - VDP_Num_Vis_Lines) / 240.0f) / 2.0);
	}
	else
		m_VStretch = 0;
}


/**
 * vdraw_sdl_gl_clear_screen(): Clear the screen.
 */
static void vdraw_sdl_gl_clear_screen(void)
{
	// Clear the screen.
	glClear(GL_COLOR_BUFFER_BIT);
	memset(filterBuffer, 0x00, filterBufferSize);
	
	// Reset the border color to make sure it's redrawn.
	vdraw_border_color_16 = ~MD_Palette[0];
	vdraw_border_color_32 = ~MD_Palette32[0];
}


/**
 * vdraw_sdl_gl_flip(): Flip the screen buffer. [Called by vdraw_flip().]
 * @return 0 on success; non-zero on error.
 */
static int vdraw_sdl_gl_flip(void)
{
	const unsigned char bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	// Start of the SDL framebuffer.
	const int pitch = rowLength * bytespp;
	const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
	const int HBorder = vdraw_border_h * (bytespp / 2);	// Left border width, in pixels.
	
	const int startPos = ((pitch * VBorder) + HBorder) * vdraw_scale;	// Starting position from within the screen.
	
	// Start of the SDL framebuffer.
	unsigned char *start = &(((unsigned char*)(filterBuffer))[startPos]);
	
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
	
	// Apply the pause tint, if necessary.
	if (Paused && Video.pauseTint)
	{
		veffect_pause_tint(&vdraw_rInfo, vdraw_scale);
	}
	
	// Draw the message and/or FPS counter.
	if (vdraw_msg_visible)
	{
		// Message is visible.
		draw_text(filterBuffer, rowLength, rowLength, (rowLength / 4) * 3,
			  vdraw_msg_text, &vdraw_msg_style, TRUE);
	}
	else if (vdraw_fps_enabled && (Game != NULL) && Active && !Paused && !Debug)
	{
		// FPS is enabled.
		draw_text(filterBuffer, rowLength, rowLength, (rowLength / 4) * 3,
			  vdraw_msg_text, &vdraw_fps_style, TRUE);
	}
	
	// Set the GL MAG filter.
	// TODO: Only do this when the linear filter setting is changed.
	if (Video.glLinearFilter)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// OpenGL needs to know the width of the texture data.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
	
	// Set the texture data.
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			0,						// x offset
			((240 - VDP_Num_Vis_Lines) >> 1) * vdraw_scale,	// y offsets
			rowLength,					// width
			((rowLength * 3) / 4) - ((240 - VDP_Num_Vis_Lines) * vdraw_scale),	// height
			m_pixelFormat, m_pixelType,
			filterBuffer + (bytespp * rowLength * ((240 - VDP_Num_Vis_Lines) >> 1) * vdraw_scale));
	
	// Corners of the rectangle.
	glBegin(GL_QUADS);
	
	glTexCoord2d(0.0 + m_HStretch, m_VStretch);	// Upper-left corner of the texture.
	glVertex2i(-1,  1);				// Upper-left vertex of the quad.
	
	glTexCoord2d(m_HRender - m_HStretch, m_VStretch);	// Upper-right corner of the texture.
	glVertex2i( 1,  1);				// Upper-right vertex of the quad.
	
	// 0.9375 = 240/256; 0.9375 = 480/512
	glTexCoord2d(m_HRender - m_HStretch, m_VRender - m_VStretch);	// Lower-right corner of the texture.
	glVertex2i( 1, -1);						// Lower-right vertex of the quad.
	
	glTexCoord2d(0.0 + m_HStretch, m_VRender - m_VStretch);	// Lower-left corner of the texture.
	glVertex2i(-1, -1);					// Lower-left corner of the quad.
	
	glEnd();
	
	// Draw the border.
	if (Video.borderColorEmulation)
		vdraw_sdl_gl_draw_border();
	
	// Swap the SDL GL buffers.
	SDL_GL_SwapBuffers();
	
	// TODO: Return appropriate error code.
	return 0;
}


/**
 * vdraw_sdl_gl_draw_border(): Draw the border color.
 * Called from vdraw_sdl_gl_flip().
 */
static void vdraw_sdl_gl_draw_border(void)
{
	if (!Video.borderColorEmulation)
		return;
	
	if ((Game == NULL) || (Debug > 0))
	{
		// Either no system is active or the debugger is enabled.
		// Make sure the border color is black.
		vdraw_border_color_16 = 0;
		vdraw_border_color_32 = 0;
	}
	else
	{
		// Set the border color to the first palette entry.
		vdraw_border_color_16 = MD_Palette[0];
		vdraw_border_color_32 = MD_Palette32[0];
	}
	
	uint8_t stretch = vdraw_get_stretch();
	
	if (stretch < STRETCH_FULL)
	{
		glDisable(GL_TEXTURE_2D);
		
		// TODO: This may not work properly on big-endian systems.
		unsigned char* bcolor = (unsigned char*)(&vdraw_border_color_32);
		glColor3ub(bcolor[2], bcolor[1], bcolor[0]);
		
		if (!(stretch & STRETCH_V) && (VDP_Num_Vis_Lines < 240))
		{
			// Top/Bottom borders.
			float borderSize = ((float)((240 - VDP_Num_Vis_Lines) / 2)) / 240.0f;
			glRectf(-1,  1, 1, ( 1.0f - (borderSize * 2.0)));
			glRectf(-1, -1, 1, (-1.0f + (borderSize * 2.0)));
		}
		
		if (!(stretch & STRETCH_H) && (vdraw_border_h > 0))
		{
			// Left/Right borders.
			float borderSize = (float)(vdraw_border_h / 2) / 320.0f;
			glRectf(-1, 1, (-1.0f + (borderSize * 2.0)), -1);
			glRectf( 1, 1, ( 1.0f - (borderSize * 2.0)), -1);
		}
		
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
	}
}


/**
 * vdraw_sdl_gl_update_renderer(): Update the renderer.
 */
static void vdraw_sdl_gl_update_renderer(void)
{
	// Check if a resolution switch is needed.
	mdp_render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return;
	const int w = 320 * scale;
	
	if (vdraw_sdl_gl_screen->w == Video.Width_GL &&
	    vdraw_sdl_gl_screen->h == Video.Height_GL &&
	    rowLength == w && textureSize == vdraw_sdl_gl_calc_texture_size(scale))
	{
		// No resolution switch is necessary. Simply clear the screen.
		vdraw_sdl_gl_clear_screen();
		return;
	}
	
	// Resolution switch is needed.
	
	// Clear the GL buffers.
	// TODO: Make this a separate function that is also called by End_Video().
	if (filterBuffer)
	{
		// Delete the GL textures and filter buffer.
		glDeleteTextures(1, textures);
		free(filterBuffer);
		filterBuffer = NULL;
	}
	
	// Reinitialize the GL buffers.
	if (vdraw_sdl_gl_screen->w == Video.Width_GL &&
	    vdraw_sdl_gl_screen->h == Video.Height_GL)
	{
		// Output resolution is the same. Don't reinitialize SDL.
		vdraw_sdl_gl_init_opengl(Video.Width_GL, Video.Height_GL, FALSE);
	}
	else
	{
		// Output resolution has changed. Reinitialize SDL.
		vdraw_sdl_gl_end();
		vdraw_sdl_gl_init();
	}
	
	// Clear the screen.
	vdraw_sdl_gl_clear_screen();
}


/**
 * vdraw_sdl_gl_update_vsync(): Update the VSync value.
 * @param fromInitSDLGL If TRUE, this function is being called from vdraw_sdl_gl_init_opengl().
 */
static void vdraw_sdl_gl_update_vsync(const BOOL fromInitSDLGL)
{
	// Set the VSync value.
	// TODO: Turning VSync off seems to require a refresh...
	
	GLint vsync = (vdraw_get_fullscreen() ? Video.VSync_FS : Video.VSync_W);
	
#ifndef __APPLE__
	// Linux/UNIX.
	if (set_swap_interval != NULL)
	{
		if (vsync)
			set_swap_interval(1);
		else if (!fromInitSDLGL)
			vdraw_refresh_video();
	}
#else /* __APPLE__ */
	// MacOS X.
	if (vsync)
		CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &vsync);
	else
	{
		CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &vsync);
		if (!fromInitSDLGL)
			vdraw_refresh_video();
	}
#endif /* __APPLE __ */
}

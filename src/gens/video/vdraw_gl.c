/***************************************************************************
 * Gens: Video Drawing - Common OpenGL Functions.                          *
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

#include "vdraw_gl.h"
#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_io.h"

// RGB color conversion functions.
#include "vdraw_RGB.h"

// libgsft includes.
#include "libgsft/gsft_malloc_align.h"

// OpenGL variables.
static GLuint textures[1] = {0};
static int rowLength = 0;
static int textureSize = 0;
static unsigned char *filterBuffer = NULL;
static int filterBufferSize = 0;
static unsigned int m_pixelFormat = 0;
static unsigned int m_pixelType = 0;


/**
 * vdraw_gl_calc_texture_size(): Calculate the texture size for the given scale value.
 * @param scale Scale value.
 * @return Texture size.
 */
static inline int vdraw_gl_calc_texture_size(int scale)
{
	const int tex_size = (256 * scale);
	
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
 * vdraw_gl_init(): Initialize OpenGL.
 * @param vp_w Viewport width.
 * @param vp_h Viewport height.
 * @return 0 on success; non-zero on error.
 */
int vdraw_gl_init(const int vp_w, const int vp_h)
{
	mdp_render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
        // Determine the texture size using the scaling factor.
	if (scale <= 0)
		return -2;
	
	rowLength = 320 * scale;
	textureSize = vdraw_gl_calc_texture_size(scale);
	
	// Allocate the filter buffer.
	int bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	filterBufferSize = rowLength * (240 * scale) * bytespp;
	filterBuffer = (unsigned char*)(gsft_malloc_align(filterBufferSize, 16));
	
	glViewport(0, 0, vp_w, vp_h);
	
	// Disable depth testing.
	glDisable(GL_DEPTH_TEST);
	
	// Initialize the orthographic projection.
	vdraw_gl_init_orthographic_projection();
	
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, textures);
	
	// Initialize the texture.
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	vdraw_gl_clear_screen();
	
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
	
	// OpenGL initialized.
	return 0;
}


/**
 * vdraw_gl_end(): Shut down OpenGL.
 */
void vdraw_gl_end(void)
{
	if (filterBuffer)
	{
		// Delete the GL textures and filter buffer.
		glDeleteTextures(1, textures);
		free(filterBuffer);
		filterBuffer = NULL;
	}
}


/**
 * vdraw_gl_set_visual(): Set the OpenGL visual.
 * @param format GL format.
 * @param type Data type.
 */
void vdraw_gl_set_visual(GLenum format, GLenum type)
{
	m_pixelFormat = format;
	m_pixelType = type;
}


/**
 * vdraw_gl_clear_screen(): Clear the screen.
 */
void vdraw_gl_clear_screen(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	memset(filterBuffer, 0x00, filterBufferSize);
}


void vdraw_gl_init_orthographic_projection(void)
{
	// GL Orthographic Projection code imported from Gens/Linux 2.15.4.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if (!Video.GL.glOrthographicProjection)
	{
		// Orthographic projection is disabled.
		glOrtho(-1, 1, -1, 1, -1, 1);
	}
	else
	{
		if ((Video.GL.width * 3 > Video.GL.height * 4) && Video.GL.height != 0)
		{
			// Window is wider than 4:3.
			glOrtho(-((float)Video.GL.width * 3) / ((float)Video.GL.height * 4),
				((float)Video.GL.width * 3) / ((float)Video.GL.height * 4),
				-1, 1, -1, 1);
		}
		else if ((Video.GL.width * 3 < Video.GL.height * 4) && Video.GL.width != 0)
		{
			// Window is taller than 4:3.
			glOrtho(-1, 1,
				-((float)Video.GL.height * 4) / ((float)Video.GL.width * 3),
				((float)Video.GL.height * 4) / ((float)Video.GL.width * 3),
				-1, 1);
		}
		else
		{
			// Window is 4:3.
			glOrtho(-1, 1, -1, 1, -1, 1);
		}
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void vdraw_gl_flip(void)
{
	// Clear the GL surface.
	// This is needed in order to make sure that we have the
	// correct video mode for some reason.
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	const unsigned char bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	// OpenGL framebuffer pitch.
	const int pitch = rowLength * bytespp;
	
	// Start of the OpenGL texture buffer.
	uint8_t *start = filterBuffer;
	
	// Position in the texture to start rendering from.
	// This is modified if STRETCH_V is enabled.
	uint8_t *glStart = start;
	
	// Set up the render information.
	// TODO: If STRETCH_V is set, only render the visible area.
	vdraw_rInfo.destScreen = (void*)start;
	vdraw_rInfo.width = 320;
	vdraw_rInfo.height = 240;
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
	
	const uint8_t stretch_flags = vdraw_get_stretch();
	
	// Calculate the texture size.
	int texHeight = 240 * vdraw_scale;
	if (VDP_Lines.Visible.Total < 240)
	{
		// Check for vertical stretch.
		const unsigned int start_offset = (pitch * VDP_Lines.Visible.Border_Size * vdraw_scale);
		start += start_offset;	// Text starting position.
		
		if (stretch_flags & STRETCH_V)
		{
			// Vertical stretch is enabled.
			texHeight = VDP_Lines.Visible.Total * vdraw_scale;
			glStart += start_offset;
		}
	}
	
	int texWidth = 320 * vdraw_scale;
	const int HPix = vdp_getHPix();
	if (HPix < 320)
	{
		// Check for horizontal stretch.
		const unsigned int start_offset = (((320 - HPix) / 2) * bytespp * vdraw_scale);
		start += start_offset;	// Text starting position.
		
		if (stretch_flags & STRETCH_H)
		{
			// Horizontal stretch is enabled.
			texWidth = HPix * vdraw_scale;
			glStart += start_offset;
		}
	}
	
	if (vdraw_msg_visible)
	{
		// Message is visible.
		draw_text(start, rowLength,
				HPix * vdraw_scale,
				VDP_Lines.Visible.Total * vdraw_scale,
				vdraw_msg_text, &vdraw_msg_style);
	}
	else if (vdraw_fps_enabled && (Game != NULL) && Settings.Active && !Settings.Paused && !IS_DEBUGGING())
	{
		// FPS is enabled.
		draw_text(start, rowLength,
				HPix * vdraw_scale,
				VDP_Lines.Visible.Total * vdraw_scale,
				vdraw_msg_text, &vdraw_fps_style);
	}
	
	// Set the GL MAG filter.
	// TODO: Only do this when the linear filter setting is changed.
	if (Video.GL.glLinearFilter)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// OpenGL needs to know the width of the texture data.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
	
	// Set the texture data.
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			0,			// x offset
			0,			// y offset
			texWidth,		// width
			texHeight,		// height
			m_pixelFormat, m_pixelType,
			glStart);
	
	// Corners of the rectangle.
	glBegin(GL_QUADS);
	
	double imgWidth = (double)(texWidth) / (double)(textureSize * 2);
	double imgHeight = (double)(texHeight) / (double)(textureSize);
	
	glTexCoord2d(0.0, 0.0);			// Upper-left corner of the texture.
	glVertex2d(-1.0,  1.0);			// Upper-left vertex of the quad.
	
	glTexCoord2d(imgWidth, 0.0);		// Upper-right corner of the texture.
	glVertex2d( 1.0,  1.0);			// Upper-right vertex of the quad.
	
	glTexCoord2d(imgWidth, imgHeight);	// Lower-right corner of the texture.
	glVertex2d( 1.0, -1.0);			// Lower-right vertex of the quad.
	
	glTexCoord2d(0.0, imgHeight);		// Lower-left corner of the texture.
	glVertex2d(-1.0, -1.0);			// Lower-left corner of the quad.
	
	glEnd();
}


/**
 * vdraw_gl_update_renderer(): Update the renderer.
 * @param vp_w Viewport width.
 * @param vp_h Viewport height.
 * @return 0 if no resolution switch is necessary; non-zero if reinit is required.
 */
int vdraw_gl_update_renderer(const int vp_w, const int vp_h)
{
	// Reinitialize the orthographic projection.
	vdraw_gl_init_orthographic_projection();
	
	// Check if a resolution switch is needed.
	mdp_render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
	{
		// TODO: Error handling.
		return 0;
	}
	
	// Calculate the row length.
	const int w = 320 * scale;
	
	if (vp_w == Video.GL.width &&
	    vp_h == Video.GL.height &&
	    rowLength == w &&
	    textureSize == vdraw_gl_calc_texture_size(scale))
	{
		// No resolution switch is necessary. Simply clear the screen.
		vdraw_gl_clear_screen();
		return 0;
	}
	
	// Resolution switch is needed.
	vdraw_gl_end();
	return 1;
}

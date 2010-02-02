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

#ifndef GENS_VDRAW_GL_H
#define GENS_VDRAW_GL_H

#include "vdraw.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Checks if OpenGL is supported by the current system.
int vdraw_gl_is_supported(void);

void vdraw_gl_init_vsync(void);
void vdraw_gl_update_vsync(const int fromInitSDLGL);

// Initialize OpenGL.
int vdraw_gl_init(const int vp_w, const int vp_h);
void vdraw_gl_end(void);

// GL visual information.
void vdraw_gl_set_visual(GLenum format, GLenum type);

void vdraw_gl_clear_screen(void);

// GL Orthographic Projection.
void vdraw_gl_init_orthographic_projection(void);

void vdraw_gl_flip(void);
int vdraw_gl_update_renderer(const int vp_w, const int vp_h);

#ifdef __cplusplus
}
#endif

#endif

/***************************************************************************
 * Gens: Video Drawing - OS-Specific OpenGL Functions. (MacOS X)           *
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

#include "vdraw_gl.h"
#include "emulator/g_main.hpp"

// OpenGL includes.
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>


/**
 * vdraw_gl_is_supported(): Checks if OpenGL is supported by the current system.
 * @return 0 if not supported; non-zero if supported.
 */
int vdraw_gl_is_supported(void)
{
	// OpenGL is always supported on MacOS X.
	return 1;
}


/**
 * vdraw_gl_init_vsync(): Initialize VSync functionality.
 */
void vdraw_gl_init_vsync(void)
{
	// Does nothing on MacOS X.
}


/**
 * vdraw_gl_update_vsync(): Update the VSync setting.
 * @param fromInitSDLGL If nonzero, this function is being called from vdraw_sdl_gl_init_opengl().
 */
void vdraw_gl_update_vsync(const int fromInitSDLGL)
{
	// Set the VSync value.
	// TODO: Turning VSync off seems to require a refresh...
	
	GLint vsync = (vdraw_get_fullscreen() ? Video.VSync_FS : Video.VSync_W);
	
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &vsync);
	if (!vsync && !fromInitSDLGL)
		vdraw_refresh_video();
}

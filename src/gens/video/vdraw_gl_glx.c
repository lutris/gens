/***************************************************************************
 * Gens: Video Drawing - OS-Specific OpenGL Functions. (X11)               *
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
#include <GL/gl.h>

// GLX includes.
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
static PFNGLXSWAPINTERVALMESAPROC set_swap_interval = NULL;
static PFNGLXGETSWAPINTERVALMESAPROC get_swap_interval = NULL;


/**
 * vdraw_gl_init_vsync(): Initialize VSync functionality.
 */
void vdraw_gl_init_vsync(void)
{
	// Linux/UNIX: Get the VSync functions.
	// TODO: Copy functions to test for VSync from Mesa's glxswapcontrol.c
	/*
	has_OML_sync_control = is_extension_supported("GLX_OML_sync_control");
	has_SGI_swap_control = is_extension_supported("GLX_SGI_swap_control");
	has_MESA_swap_control = is_extension_supported("GLX_MESA_swap_control");
	*/
	set_swap_interval = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI");
	get_swap_interval = (PFNGLXGETSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXGetSwapIntervalSGI");
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
	
	// Linux/UNIX.
	if (set_swap_interval != NULL)
	{
		if (vsync)
			set_swap_interval(1);
		if (!fromInitSDLGL)
			vdraw_refresh_video();
	}
}

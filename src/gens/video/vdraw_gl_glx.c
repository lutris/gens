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
#include "macros/log_msg.h"

#include <stdlib.h>

// X11 includes.
#include <X11/Xlib.h>

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
#endif

// OpenGL VSync stuff
// Copied from Mesa's glxswapcontrol.c
static PFNGLXSWAPINTERVALMESAPROC set_swap_interval_sgi = NULL;
static PFNGLXSWAPINTERVALMESAPROC set_swap_interval_mesa = NULL;

// make_extension_table() and is_extension_supported() were
// copied from Mesa's glxswapcontrol.c

#define NUL '\0'
static char **extension_table = NULL;
static unsigned int num_extensions;


/**
 * Fill in the table of extension strings from a supplied extensions string
 * (as returned by glXQueryExtensionsString).
 *
 * \param string   String of GLX extensions.
 * \sa is_extension_supported
 */
static void make_extension_table(const char* string)
{
	char **string_tab;
	unsigned int num_strings;
	unsigned int base;
	unsigned int idx;
	unsigned int i;
	
	if (!string)
		return;
	
	/* Count the number of spaces in the string.  That gives a base-line
	 * figure for the number of extension in the string.
	 */
	
	num_strings = 1;
	for (i = 0; string[i] != NUL; i++)
	{
		if (string[i] == ' ')
		{
			num_strings++;
		}
	}
	
	string_tab = (char**)malloc(sizeof(char*) * num_strings);
	if (string_tab == NULL)
		return;
	
	base = 0;
	idx = 0;
	
	while (string[base] != NUL)
	{
		/* Determine the length of the next extension string.
		 */
		
		for (i = 0; (string[base + i] != NUL) && (string[base + i] != ' '); i++)
		{
			/* empty */ ;
		}
		
		if (i > 0)
		{
			/* If the string was non-zero length, add it to the table.  We
			 * can get zero length strings if there is a space at the end of
			 * the string or if there are two (or more) spaces next to each
			 * other in the string.
			 */

			string_tab[idx] = (char*)malloc(sizeof(char) * (i + 1));
			if (string_tab[idx] == NULL)
				return;
			
			memcpy(string_tab[idx], &string[base], i);
			string_tab[idx][i] = NUL;
			idx++;
		}
		
		/* Skip to the start of the next extension string.
		 */

		for (base += i; (string[ base ] == ' ') && (string[ base ] != NUL); base++ )
		{
			/* empty */ ;
		}
	}
	
	extension_table = string_tab;
	num_extensions = idx;
}


/**
 * Determine if an extension is supported.  The extension string table
 * must have already be initialized by calling \c make_extension_table.
 *
 * \praram ext  Extension to be tested.
 * \return GL_TRUE of the extension is supported, GL_FALSE otherwise.
 * \sa make_extension_table
 */
static GLboolean is_extension_supported(const char* ext)
{
	unsigned int i;
	
	for (i = 0; i < num_extensions; i++)
	{
		if (strcmp(ext, extension_table[i]) == 0)
		{
			return GL_TRUE;
		}
	}
	
	return GL_FALSE;
}


static void clear_extension_table(void)
{
	if (!extension_table)
		return;
	
	for (unsigned int i = 0; i < num_extensions; i++)
	{
		free(extension_table[i]);
	}
	
	free(extension_table);
}


/**
 * vdraw_gl_is_supported(): Checks if OpenGL is supported by the current system.
 * @return 0 if not supported; non-zero if supported.
 */
int vdraw_gl_is_supported(void)
{
	// Mesa 7.4.4/7.5-rc4 and earlier have a bug that causes the program
	// to crash in GetGLXScreenConfigs() if GLX isn't suppported by the
	// X server due to a null pointer dereference. So, we have to check
	// if the X server supports GLX manually.
	
	// Open the X11 display.
	Display *dpy;
	char *dpyname = getenv("DISPLAY");
	dpy = XOpenDisplay(dpyname);
	if (!dpy)
	{
		LOG_MSG(video, LOG_MSG_LEVEL_WARNING,
			"Could not open X11 display. OpenGL will be disabled.");
		return 0;
	}
	
	// Query the GLX version.
	Bool rval = glXQueryVersion(dpy, NULL, NULL);
	
	// Close the X11 display.
	XCloseDisplay(dpy);
	
	if (!rval)
	{
		LOG_MSG(video, LOG_MSG_LEVEL_WARNING,
			"X server doesn't support the GLX extension. OpenGL will be disabled.");
	}
	
	// If GLX was queried successfully, rval is TRUE; otherwise, it's FALSE.
	return rval;
}


/**
 * vdraw_gl_init_vsync(): Initialize VSync functionality.
 */
void vdraw_gl_init_vsync(void)
{
	// Get the X11 display.
	Display *dpy;
	char *dpyname = getenv("DISPLAY");
	dpy = XOpenDisplay(dpyname);
	if (!dpy)
	{
		LOG_MSG(video, LOG_MSG_LEVEL_WARNING,
			"Could not open X11 display. VSync will not be enabled.");
		return;
	}
	
	// Create the extension table.
	make_extension_table((const char*)glXQueryExtensionsString(dpy, DefaultScreen(dpy)));
	
	// Close the X11 display.
	XCloseDisplay(dpy);
	
	// TODO: Allow the user to select a VSync method.
	// Currently, MESA is preferred, followed by SGI.
	
	// Check for MESA swap control.
	if (is_extension_supported("GLX_MESA_swap_control"))
	{
		set_swap_interval_mesa = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalMESA");
	}
	
	// Check for SGI swap control.
	if (is_extension_supported("GLX_SGI_swap_control"))
	{
		set_swap_interval_sgi = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI");
	}
	
	// Clear the extension tbale.
	clear_extension_table();
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
	if (set_swap_interval_mesa)
	{
		set_swap_interval_mesa(vsync);
	}
	else if (set_swap_interval_sgi)
	{
		set_swap_interval_sgi(vsync);
	}
	
	if (!fromInitSDLGL)
		vdraw_refresh_video();
}

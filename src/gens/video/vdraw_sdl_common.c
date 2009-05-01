/***************************************************************************
 * Gens: Video Drawing - SDL Common Functions.                             *
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

#include "vdraw_sdl_common.h"
#include "vdraw.h"

#include "gens/gens_window.h"

// C includes.
#include <stdint.h>
#include <stdlib.h>

// GTK+ includes.
#include <gtk/gtk.h>
#include "ui/gtk/gtk-compat.h"

// Hack to disable SDL window embedding on OS X.
#include <gdkconfig.h>
#if defined(GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#elif defined(GDK_WINDOWING_QUARTZ)
// Quartz (MacOS X)
#else
#error Unsupported GTK+ windowing system.
#endif


/**
 * vdraw_sdl_common_embed(): Embed the SDL window in the main window.
 * @param w Width.
 * @param h Height.
 */
void vdraw_sdl_common_embed(const int w, const int h)
{
#ifdef GDK_WINDOWING_X11
	// X11. If windowed, embed the SDL window inside of the GTK+ window.
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
		char SDL_WindowID[16];
		snprintf(SDL_WindowID, sizeof(SDL_WindowID), "%u",
			 (uint32_t)(GDK_WINDOW_XWINDOW(gtk_widget_get_window(gens_window_sdlsock))));
		SDL_WindowID[sizeof(SDL_WindowID)-1] = 0x00;
		
		setenv("SDL_WINDOWID", SDL_WindowID, 1);
	}
#elif defined(GDK_WINDOWING_QUARTZ)
	// Quartz (MacOS X).
	// Hide the embedded SDL window regardless of anything else.
	gtk_widget_hide(gens_window_sdlsock);
	unsetenv("SDL_WINDOWID");
#endif
}

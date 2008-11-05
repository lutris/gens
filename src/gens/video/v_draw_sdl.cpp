/**
 * Gens: Video Drawing class - SDL
 */ 

#include "v_draw_sdl.hpp"

#include <string.h>
#include <gdk/gdkx.h>

#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "ui/gtk/gtk-misc.h"
#include "gens/gens_window.hpp"

int VDraw_SDL::SDL_RefCount = 0;

VDraw_SDL::VDraw_SDL()
{
	SDL_RefCount++;
	if (SDL_RefCount == 1)
	{
		// First VDraw_SDL instance. Initialize SDL.
		if (SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
		{
			fprintf(stderr, "VDraw_SDL::VDraw_SDL(): Couldn't initialize SDL's timer subsystem: %s\n", SDL_GetError());
			exit(1);
		}
		
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		{
			fprintf(stderr, "VDraw_SDL::VDraw_SDL(): Couldn't initialize SDL's video subsystem: %s\n", SDL_GetError());
			exit(1);
		}
	}
}

VDraw_SDL::VDraw_SDL(VDraw *oldDraw)
	: VDraw(oldDraw)
{
}

VDraw_SDL::~VDraw_SDL()
{
	SDL_RefCount--;
	if (SDL_RefCount == 0)
	{
		// Last VDraw_SDL instance. Shut down the video and timer subsystems.
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		SDL_QuitSubSystem(SDL_INIT_TIMER);
	}
}


/**
 * setupSDLWindow(): Set up the SDL window.
 * @param w Width
 * @param h Height
 */
void VDraw_SDL::setupSDLWindow(const int w, const int h)
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	if (m_FullScreen)
	{
		// Hide the embedded SDL window.
		gtk_widget_hide(lookup_widget(gens_window, "sdlsock"));
		
		unsetenv("SDL_WINDOWID");
		strcpy(SDL_WindowID, "");
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
		sprintf(SDL_WindowID, "%d", (int)(GDK_WINDOW_XWINDOW(sdlsock->window)));
		setenv("SDL_WINDOWID", SDL_WindowID, 1);
	}
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	
	// Disable the cursor in fullscreen mode.
	SDL_ShowCursor(m_FullScreen ? SDL_DISABLE : SDL_ENABLE);
}


/**
 * Shut_Down(): Shut down the graphics subsystem.
 */
void VDraw_SDL::Shut_Down(void)
{
	SDL_Quit();
}

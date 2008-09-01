/**
 * Gens: Video Drawing class - SDL
 */ 

#include "v_draw_sdl.hpp"

#include <string.h>
#include <gdk/gdkx.h>

#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

#include "emulator/g_main.hpp"
#include "ui/ui-common.h"
#include "ui/gtk/gtk-misc.h"
#include "gens/gens_window.h"

VDraw_SDL::VDraw_SDL()
{
}

VDraw_SDL::~VDraw_SDL()
{
}

/**
 * Init_Video(): Initialize the video subsystem.
 * @return 1 on success.
 */
int VDraw_SDL::Init_Video(void)
{
	int x;
	int w, h;
	
	if (Video.Render_Mode == 0)
	{
		// Normal render mode. 320x240
		w = 320; h = 240;
	}
	else
	{
		// 2x render mode. 640x480
		w = 640; h = 480;
	}
	
	if (Video.Full_Screen)
	{
		// Hide the embedded SDL window.
		gtk_widget_hide(lookup_widget(gens_window, "sdlsock"));
		
		if (strlen(SDL_WindowID) == 0)
		{
			unsetenv("SDL_WINDOWID");
			strcpy(SDL_WindowID, "");
		}
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
		
		if (strlen(SDL_WindowID) == 0)
		{
			// Get the Window ID of the SDL socket.
			sprintf(SDL_WindowID, "%d", (int)(GDK_WINDOW_XWINDOW(sdlsock->window)));
			setenv("SDL_WINDOWID", SDL_WindowID, 1);
		}
	}
	
	// Initialize SDL.
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		// Error initializing SDL.
		Init_Fail("Couldn't initialize embedded SDL.");
		return 0;
	}
	
	// Initialize the renderer.
	x = Init_SDL_Renderer(w, h);
	
	// Disable the cursor in fullscreen mode.
	SDL_ShowCursor(Video.Full_Screen ? SDL_DISABLE : SDL_ENABLE);
	
	// Adjust stretch parameters.
	Adjust_Stretch();
	
	// If normal rendering mode is set, disable the video shift.
	m_shift = (Video.Render_Mode != 0);
	
	// Return the status code from Init_SDL_Renderer().
	return x;
}


/**
 * Init_SDL_Renderer(): Initialize the SDL renderer.
 * @param w Width of the screen.
 * @param h Height of the screen.
 * @return 1 on success; 0 on error.
 */
int VDraw_SDL::Init_SDL_Renderer(int w, int h)
{
	screen = SDL_SetVideoMode(w, h, bpp, SDL_Flags | (Video.Full_Screen ? SDL_FULLSCREEN : 0));
	
	if (!screen)
	{
		// Error initializing SDL.
		fprintf(stderr, "Error creating SDL primary surface: %s\n", SDL_GetError());
		exit(0);
	}
	
	return 1;
}


/**
 * End_Video(): Closes the SDL renderer.
 */
void VDraw_SDL::End_Video(void)
{
	// Placeholder function in case something needs to be added later.
}


/**
 * Adjust_Stretch(): Adjust stretch parameters.
 */
void VDraw_SDL::Adjust_Stretch(void)
{
	// TODO
}


/**
 * Clear_Screen(): Clears the screen.
 */
void VDraw_SDL::Clear_Screen(void)
{
	SDL_LockSurface(screen);
	SDL_FillRect(screen, NULL, 0);
	SDL_UnlockSurface(screen);
}


/**
 * Flip_internal(): Flip the screen buffer. (Called by v_draw.)
 * @return 1 on success; 0 on error.
 */
int VDraw_SDL::flipInternal(void)
{
	SDL_Rect border;
	
	SDL_LockSurface(screen);
	
	unsigned short newBorderColor_16B = MD_Palette[0];
	unsigned int newBorderColor_32B = MD_Palette32[0];
	
	if ((!Genesis_Started && !SegaCD_Started && !_32X_Started) || (Debug > 0))
	{
		// Either no system is active or the debugger is enabled.
		// Make sure the border color is black.
		newBorderColor_16B = 0;
		newBorderColor_32B = 0;
	}
	
	// Draw the border.
	// TODO: Make this more accurate and/or more efficient.
	// In particular, it only works for 1x and 2x rendering.
	if ((bpp == 15 || bpp == 16) && (m_BorderColor_16B != newBorderColor_16B))
	{
		m_BorderColor_16B = newBorderColor_16B;
		if (VDP_Num_Vis_Lines < 240)
		{
			// Top/Bottom borders.
			border.x = 0; border.w = screen->w;
			border.h = 240 - VDP_Num_Vis_Lines;
			if (screen->h == 240)
				border.h >>= 1;
			border.y = 0;
			SDL_FillRect(screen, &border, m_BorderColor_16B);
			border.y = screen->h - border.h;
			SDL_FillRect(screen, &border, m_BorderColor_16B);
		}
		if (m_HBorder > 0)
		{
			// Left/Right borders.
			if (border.h != 0)
			{
				border.y = 0;
				border.h = 240 - border.h;
			}
			
			border.x = 0; border.h = screen->h;
			border.w = m_HBorder;
			if (screen->w == 320)
				border.w >>= 1;
			border.y = 0;
			SDL_FillRect(screen, &border, m_BorderColor_16B);
			border.x = screen->w - border.w;
			SDL_FillRect(screen, &border, m_BorderColor_16B);
		}
	}
	else if ((bpp == 32) && (m_BorderColor_32B != newBorderColor_32B))
	{
		m_BorderColor_32B = newBorderColor_32B;
		if (VDP_Num_Vis_Lines < 240)
		{
			// Top/Bottom borders.
			border.x = 0; border.w = screen->w;
			border.h = 240 - VDP_Num_Vis_Lines;
			if (screen->h == 240)
				border.h >>= 1;
			border.y = 0;
			SDL_FillRect(screen, &border, m_BorderColor_32B);
			border.y = screen->h - border.h;
			SDL_FillRect(screen, &border, m_BorderColor_32B);
		}
		if (m_HBorder > 0)
		{
			// Left/Right borders.
			if (border.h != 0)
			{
				border.y = 0;
				border.h = 240 - border.h;
			}
			
			border.x = 0; border.h = screen->h;
			border.w = m_HBorder;
			if (screen->w == 320)
				border.w >>= 1;
			border.y = 0;
			SDL_FillRect(screen, &border, m_BorderColor_32B);
			border.x = screen->w - border.w;
			SDL_FillRect(screen, &border, m_BorderColor_32B);
		}
	}
	
	unsigned char bytespp = (bpp == 15 ? 2 : bpp / 8);
	
	// Start of the SDL framebuffer.
	int pitch = screen->w * bytespp;
	int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
	int HBorder = m_HBorder * (bytespp / 2);	// Left border width, in pixels.
	
	int startPos = ((pitch * VBorder) + HBorder) << m_shift;  // Starting position from within the screen.
	
	// Start of the SDL framebuffer.
	unsigned char *start = &(((unsigned char*)(screen->pixels))[startPos]);
	
	if (Video.Full_Screen)
	{
		Blit_FS(start, pitch, 320 - m_HBorder, VDP_Num_Vis_Lines, 32 + m_HBorder*2);
	}
	else
	{
		Blit_W(start, pitch, 320 - m_HBorder, VDP_Num_Vis_Lines, 32 + m_HBorder*2);
	}
	
	SDL_UnlockSurface(screen);
	
	SDL_Flip(screen);
	
	// TODO: Return appropriate error code.
	return 1;
}


/**
 * Init_Subsystem(): Initialize the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
int VDraw_SDL::Init_Subsystem(void)
{
	if (SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
	{
		fprintf (stderr, SDL_GetError ());
		return -1;
	}
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, SDL_GetError ());
		return -1;
	}
	
	/* Take it back down now that we know it works. */
	SDL_QuitSubSystem (SDL_INIT_VIDEO);
	
	// Initialize joysticks.
	// TODO: If there's an error here, merely disable joystick functionality.
	if (SDL_InitSubSystem (SDL_INIT_JOYSTICK) < 0)
	{
		fprintf (stderr, SDL_GetError ());
		return -1;
	}
	
	// Initialize CD-ROM.
	// TODO: If there's an error here, merely disable CD-ROM functionality.
	if (SDL_InitSubSystem(SDL_INIT_CDROM) < 0)
	{
		fprintf(stderr, SDL_GetError ());
		return -1;
	}
	
	return 0;
}


/**
 * Shut_Down(): Shut down the graphics subsystem.
 */
int VDraw_SDL::Shut_Down(void)
{
	SDL_Quit();
	return 1;
}

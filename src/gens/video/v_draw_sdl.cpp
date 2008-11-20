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

// CPU flags
#include "gens_core/misc/cpuflags.h"


VDraw_SDL::VDraw_SDL()
{
}

VDraw_SDL::VDraw_SDL(VDraw *oldDraw)
	: VDraw(oldDraw)
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
	
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	MDP_Render_t *rendPlugin = (MDP_Render_t*)(PluginMgr::vRenderPlugins.at(rendMode)->plugin_t);
	
	// Determine the window size using the scaling factor.
	if (rendPlugin->scale <= 0)
		return 0;
	int w = 320 * rendPlugin->scale;
	int h = 240 * rendPlugin->scale;
	
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
	SDL_ShowCursor(m_FullScreen ? SDL_DISABLE : SDL_ENABLE);
	
	// If normal rendering mode is set, disable the video shift.
	m_shift = (rendMode == 0) ? 0 : 1;
	
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
	screen = SDL_SetVideoMode(w, h, bpp, SDL_Flags | (m_FullScreen ? SDL_FULLSCREEN : 0));
	
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
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


/**
 * clearScreen(): Clears the screen.
 */
void VDraw_SDL::clearScreen(void)
{
	// Clear the screen.
	SDL_LockSurface(screen);
	memset(screen->pixels, 0x00, screen->pitch * screen->h);
	SDL_UnlockSurface(screen);
	
	// Reset the border color to make sure it's redrawn.
	m_BorderColor_16B = ~MD_Palette[0];
	m_BorderColor_32B = ~MD_Palette32[0];
}


/**
 * Flip_internal(): Flip the screen buffer. (Called by v_draw.)
 * @return 1 on success; 0 on error.
 */
int VDraw_SDL::flipInternal(void)
{
	SDL_LockSurface(screen);
	
	// Draw the border.
	drawBorder();
	
	unsigned char bytespp = (bpp == 15 ? 2 : bpp / 8);
	
	// Start of the SDL framebuffer.
	int pitch = screen->w * bytespp;
	int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
	int HBorder = m_HBorder * (bytespp / 2);	// Left border width, in pixels.
	
	int startPos = ((pitch * VBorder) + HBorder) << m_shift;  // Starting position from within the screen.
	
	// Start of the SDL framebuffer.
	unsigned char *start = &(((unsigned char*)(screen->pixels))[startPos]);
	
	// Set up the render information.
	if (m_rInfo.bpp != bpp)
	{
		// bpp has changed. Reinitialize the screen pointers.
		m_rInfo.bpp = bpp;
		m_rInfo.cpuFlags = 0;//CPU_Flags;
		if (bpp == 15 || bpp == 16)
			m_rInfo.mdScreen = (void*)(&MD_Screen[8]);
		else
			m_rInfo.mdScreen = (void*)(&MD_Screen32[8]);
	}
	
	m_rInfo.destScreen = (void*)start;
	m_rInfo.width = 320 - m_HBorder;
	m_rInfo.height = VDP_Num_Vis_Lines;
	m_rInfo.pitch = pitch;
	m_rInfo.offset = 16 + m_HBorder;
	
	if (m_FullScreen)
		m_BlitFS(&m_rInfo);
	else
		m_BlitW(&m_rInfo);
	
	// Draw the message and/or FPS.
	if (m_MsgVisible)
	{
		// Message is visible.
		drawText(screen->pixels, screen->w, screen->w, screen->h, m_MsgText.c_str(), m_MsgStyle);
	}
	else if (m_FPSEnabled && (Game != NULL) && Active && !Paused && !Debug)
	{
		// FPS is enabled.
		drawText(screen->pixels, screen->w, screen->w, screen->h, m_MsgText.c_str(), m_FPSStyle);
	}
	
	SDL_UnlockSurface(screen);
	
	SDL_Flip(screen);
	
	// TODO: Return appropriate error code.
	return 1;
}


/**
 * drawBorder(): Draw the border color.
 * Called from flipInternal().
 */
void VDraw_SDL::drawBorder(void)
{
	// TODO: Make this more accurate and/or more efficient.
	// In particular, it only works for 1x and 2x rendering.
	
	SDL_Rect border;
	
	if (!Video.borderColorEmulation)
	{
		// Border color emulation is disabled.
		// Don't do anything if the border color is currently black.
		if (m_BorderColor_16B == 0 && m_BorderColor_32B == 0)
			return;
	}
	
	unsigned short newBorderColor_16B = MD_Palette[0];
	unsigned int newBorderColor_32B = MD_Palette32[0];
	
	if (!Video.borderColorEmulation || (Game == NULL) || (Debug > 0))
	{
		// Either no game is loaded or the debugger is enabled.
		// Make sure the border color is black.
		newBorderColor_16B = 0;
		newBorderColor_32B = 0;
	}
	
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
}


/**
 * Init_Subsystem(): Initialize the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
int VDraw_SDL::Init_Subsystem(void)
{
	if (SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
	{
		fprintf (stderr, SDL_GetError());
		return -1;
	}
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, SDL_GetError());
		return -1;
	}
	
	/* Take it back down now that we know it works. */
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	
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


/**
 * updateRenderer(): Update the renderer.
 */
void VDraw_SDL::updateRenderer(void)
{
	// Check if a resolution switch is needed.
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	MDP_Render_t *rendPlugin = (MDP_Render_t*)(PluginMgr::vRenderPlugins.at(rendMode)->plugin_t);
	
	// Determine the window size using the scaling factor.
	if (rendPlugin->scale <= 0)
		return;
	int w = 320 * rendPlugin->scale;
	int h = 240 * rendPlugin->scale;
	
	if (screen->w == w && screen->h == h)
	{
		// No resolution switch is necessary. Simply clear the screen.
		clearScreen();
		return;
	}
	
	// Resolution switch is needed.
	End_Video();
	Init_Video();
	
	// Clear the screen.
	clearScreen();
	
	// Adjust stretch parameters.
	stretchAdjustInternal();
}


/**
 * updateVSync(): Update VSync value.
 * @param unused Unused in this function.
 */
void VDraw_SDL::updateVSync(bool unused)
{
	GENS_UNUSED_PARAMETER(unused);
	
	// Unfortunately, plain old SDL doesn't support VSync. :(
	return;
}

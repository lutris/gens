/**
 * Gens: Video Drawing class - SDL (Software Rendering)
 */

#include "v_draw_sdl_sw.hpp"

#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

#include "emulator/g_main.hpp"


VDraw_SDL_SW::VDraw_SDL_SW()
{
}

VDraw_SDL_SW::VDraw_SDL_SW(VDraw *oldDraw)
	: VDraw_SDL(oldDraw)
{
}

VDraw_SDL_SW::~VDraw_SDL_SW()
{
}


/**
 * Init_Video(): Initialize the video subsystem.
 * @return 1 on success.
 */
int VDraw_SDL_SW::Init_Video(void)
{
	int x;
	int w, h;
	
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	if (rendMode == 0)
	{
		// Normal render mode. 320x240
		w = 320; h = 240;
	}
	else
	{
		// 2x render mode. 640x480
		w = 640; h = 480;
	}
	
	// Set up the SDL window.
	setupSDLWindow(w, h);
	
	// If normal rendering mode is set, disable the video shift.
	m_shift = (rendMode == 0) ? 0 : 1;
	
	// Initialize the renderer.
	return initRenderer(w, h);
}


/**
 * initRenderer(): Initialize the SDL renderer.
 * @param w Width of the screen.
 * @param h Height of the screen.
 * @param reinitSDL Unused in this VDraw_SDL_SW.
 * @return 1 on success; 0 on error.
 */
int VDraw_SDL_SW::initRenderer(const int w, const int h, const bool reinitSDL)
{
	GENS_UNUSED_PARAMETER(reinitSDL);
	
	screen = SDL_SetVideoMode(w, h, bpp, SDL_Flags | (m_FullScreen ? SDL_FULLSCREEN : 0));
	
	if (!screen)
	{
		// Error initializing SDL.
		fprintf(stderr, "VDraw_SDL_SW::initRenderer(): Error creating SDL primary surface: %s\n", SDL_GetError());
		return 0;
	}
	
	return 1;
}


/**
 * clearScreen(): Clears the screen.
 */
void VDraw_SDL_SW::clearScreen(void)
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
 * flipInternal(): Flip the screen buffer. (Called by v_draw.)
 */
void VDraw_SDL_SW::flipInternal(void)
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
	
	if (m_FullScreen)
	{
		Blit_FS(start, pitch, 320 - m_HBorder, VDP_Num_Vis_Lines, 32 + (m_HBorder * 2));
	}
	else
	{
		Blit_W(start, pitch, 320 - m_HBorder, VDP_Num_Vis_Lines, 32 + (m_HBorder * 2));
	}
	
	// Draw the message and/or FPS.
	if (m_MsgVisible)
	{
		// Message is visible.
		drawText(screen->pixels, screen->w, screen->w, screen->h, m_MsgText.c_str(), m_MsgStyle);
	}
	else if (m_FPSEnabled && (Genesis_Started || _32X_Started || SegaCD_Started) && !Paused)
	{
		// FPS is enabled.
		drawText(screen->pixels, screen->w, screen->w, screen->h, m_MsgText.c_str(), m_FPSStyle);
	}
	
	SDL_UnlockSurface(screen);
	
	SDL_Flip(screen);
}


/**
 * updateRenderer(): Update the renderer.
 */
void VDraw_SDL_SW::updateRenderer(void)
{
	// Check if a resolution switch is needed.
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	if (rendMode == 0)
	{
		// 1x rendering.
		if (screen->w == 320 && screen->h == 240)
		{
			// Already 1x rendering. Simply clear the screen.
			clearScreen();
			return;
		}
	}
	else
	{
		// 2x rendering.
		if (screen->w == 640 && screen->h == 480)
		{
			// Already 2x rendering. Simply clear the screen.
			clearScreen();
			return;
		}
	}
	
	// Resolution switch is needed.
	End_Video();
	Init_Video();
	
	// Clear the screen.
	clearScreen();
	
	// Adjust stretch parameters.
	stretchAdjustInternal();
}

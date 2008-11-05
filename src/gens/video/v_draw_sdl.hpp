/**
 * Gens: Video Drawing class - SDL (Base Class)
 */

#ifndef GENS_V_DRAW_SDL_HPP
#define GENS_V_DRAW_SDL_HPP

#include <SDL/SDL.h>
#include "v_draw.hpp"

class VDraw_SDL : public VDraw
{
	public:
		VDraw_SDL();
		VDraw_SDL(VDraw *oldDraw);
		~VDraw_SDL();
		
		virtual int Init_Video(void) = 0;
		virtual void End_Video(void) { }
		
		// Shut down the graphics subsystem.
		void Shut_Down(void);
		
		// Update VSync value.
		virtual void updateVSync(bool unused = false) { }
		
	protected:
		static int SDL_RefCount;
		
		void setupSDLWindow(const int w, const int h);
		
		virtual int initRenderer(const int w, const int h, const bool reinitSDL = true) = 0;
		
		// Flip the screen buffer. (Called by v_draw.)
		virtual void flipInternal(void) = 0;
		
		// Update the renderer.
		virtual void updateRenderer(void) = 0;
		
		// SDL flags
		static const int SDL_Flags =
				SDL_DOUBLEBUF |
				SDL_HWSURFACE |
				SDL_HWPALETTE |
				SDL_ASYNCBLIT |
				SDL_HWACCEL;
		
		SDL_Surface *screen;
		char SDL_WindowID[24];
};

#endif /* GENS_V_DRAW_SDL_HPP */

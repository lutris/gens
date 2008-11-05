/**
 * Gens: Video Drawing class - SDL (Software Rendering)
 */

#ifndef GENS_V_DRAW_SDL_SW_HPP
#define GENS_V_DRAW_SDL_SW_HPP

#include "v_draw_sdl.hpp"

class VDraw_SDL_SW : public VDraw_SDL
{
	public:
		VDraw_SDL_SW();
		VDraw_SDL_SW(VDraw *oldDraw);
		~VDraw_SDL_SW();
		
		int Init_Video(void);
		
		// Clear the screen.
		void clearScreen(void);
		//void Clear_Primary_Screen(void);
		//void Clear_Back_Screen(void);
		
	protected:
		int initRenderer(const int w, const int h, const bool reinitSDL = true);
		
		// Flip the screen buffer. (Called by v_draw.)
		void flipInternal(void);
		
		// Update the renderer.
		void updateRenderer(void);
		
		// SDL flags
		static const int SDL_Flags =
				SDL_DOUBLEBUF |
				SDL_HWSURFACE |
				SDL_HWPALETTE |
				SDL_ASYNCBLIT |
				SDL_HWACCEL;
};

#endif /* GENS_V_DRAW_SDL_SW_HPP */

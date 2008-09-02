/**
 * Gens: Video Drawing class - SDL + OpenGL
 */

#ifndef GENS_V_DRAW_SDL_GL_HPP
#define GENS_V_DRAW_SDL_GL_HPP

#include <SDL/SDL.h>
#include "v_draw.hpp"

// OpenGL includes
#include <GL/gl.h>
#include <GL/glext.h>

class VDraw_SDL_GL : public VDraw
{
	public:
		VDraw_SDL_GL();
		~VDraw_SDL_GL();
		
		int Init_Video(void);
		void End_Video(void);
		
		// Initialize the graphics subsystem.
		int Init_Subsystem(void);
		
		// Shut down the graphics subsystem.
		int Shut_Down(void);
		
		// Clear the screen.
		void Clear_Screen(void);
		//void Clear_Primary_Screen(void);
		//void Clear_Back_Screen(void);
		
		// Adjust stretch parameters.
		void Adjust_Stretch(void);
		
	protected:
		int Init_SDL_GL_Renderer(int w, int h);
		
		// Flip the screen buffer. (Called by v_draw.)
		int flipInternal(void);
		
		// SDL flags
		static const int SDL_GL_Flags =
				SDL_DOUBLEBUF |
				SDL_HWSURFACE |
				SDL_HWPALETTE |
				SDL_ASYNCBLIT |
				SDL_HWACCEL |
				SDL_OPENGL;
		
		SDL_Surface *screen;
		char SDL_WindowID[24];
		
		// GL variables.
		GLuint textures[2];
		int rowLength;
		int textureSize;
		int nonpow2tex;
		int glLinearFilter;
		unsigned short *filterBuffer;
		
		// Stretch parameters.
		float m_HStretch, m_VStretch;
};

#endif

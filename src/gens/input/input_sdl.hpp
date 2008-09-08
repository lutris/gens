/**
 * Gens: Input class - SDL
 */

#ifndef GENS_INPUT_SDL_HPP
#define GENS_INPUT_SDL_HPP

#include "input.hpp"

#include <SDL/SDL.h>

// GTK stuff
#include <gtk/gtk.h>

class Input_SDL : public Input
{
	public:
		Input_SDL();
		~Input_SDL();
		
		// Get a key. (Used for controller configuration.)
		unsigned int getKey(void);
		
	protected:
		static gint GDK_KeySnoop(GtkWidget *grab, GdkEventKey *event, gpointer user_data);
		
		// Functions required by the Input class.
		bool joyExists(int joyNum);
		
		// Number of joysticks connected
		int m_numJoysticks;
		
		// SDL joystick structs
		SDL_Joystick *m_joy[6];
};

#endif

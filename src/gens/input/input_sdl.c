/***************************************************************************
 * Gens: Input Handler - SDL Backend.                                      *
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

#include "input_sdl.h"
#include "input_sdl_keys.h"
#include "input_sdl_events.hpp"
#include "input_sdl_joystate.h"

// Message logging.
#include "macros/log_msg.h"

#include <unistd.h>

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "gens/gens_window.h"

// Needed to handle controller input configuration.
#include "controller_config/cc_window.h"

#include <SDL/SDL.h>

// GTK stuff
#include <gtk/gtk.h>

// SDL key names.
#include "input_sdl_key_names.h"


// Function prototypes.
static int		input_sdl_init(void);
static int		input_sdl_end(void);

static int		input_sdl_update(void);
static BOOL		input_sdl_check_key_pressed(uint16_t key);
static unsigned int	input_sdl_get_key(void);
static BOOL		input_sdl_joy_exists(int joy_num);

// Miscellaneous.
static gint input_sdl_gdk_keysnoop(GtkWidget *grab, GdkEventKey *event, gpointer user_data);

// Maximum number of keys and joysticks.
#define INPUT_SDL_MAX_KEYS 512
#define INPUT_SDL_MAX_JOYSTICKS 6

// Check an SDL joystick axis.
static inline void input_sdl_check_joystick_axis(SDL_Event *event);

// Internal variables.
static int input_sdl_num_joysticks;	// Number of joysticks connected
static SDL_Joystick *input_sdl_joys[INPUT_SDL_MAX_JOYSTICKS];	// SDL joystick structs

// Key state.
static uint8_t input_sdl_keys[INPUT_SDL_MAX_KEYS];

// Joystick state.
static input_joy_state_t input_sdl_joy_state[INPUT_SDL_MAX_JOYSTICKS];

// Default keymap.
static const input_keymap_t input_sdl_keymap_default[8] =
{
	// Player 1
	{{GENS_KEY_UP, GENS_KEY_DOWN, GENS_KEY_LEFT, GENS_KEY_RIGHT,
	GENS_KEY_RETURN, GENS_KEY_a, GENS_KEY_s, GENS_KEY_d,
	GENS_KEY_RSHIFT, GENS_KEY_q, GENS_KEY_w, GENS_KEY_e}},
	
	// Player 2
	{{GENS_KEY_y, GENS_KEY_h, GENS_KEY_g, GENS_KEY_j,
	GENS_KEY_u, GENS_KEY_k, GENS_KEY_l, GENS_KEY_m,
	GENS_KEY_t, GENS_KEY_i, GENS_KEY_o, GENS_KEY_p}},
	
	// Players 1B, 1C, 1D
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	
	// Players 2B, 2C, 2D
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
};

// Axis values.
static const unsigned char input_sdl_joy_axis_values[2][6] =
{
	// axis value < -10,000
	{0x03, 0x01, 0x07, 0x05, 0x0B, 0x09},
	
	// axis value > 10,000
	{0x04, 0x02, 0x08, 0x06, 0x0C, 0x0A},
};


// Input Backend struct.
const input_backend_t input_backend_sdl =
{
	.init = input_sdl_init,
	.end = input_sdl_end,
	
	.keymap_default = &input_sdl_keymap_default[0],
	
	.update			= input_sdl_update,
	.check_key_pressed	= input_sdl_check_key_pressed,
	.get_key		= input_sdl_get_key,
	.joy_exists		= input_sdl_joy_exists,
	.get_key_name		= input_sdl_get_key_name
};


/**
 * input_sdl_init(): Initialize the SDL input subsystem.
 * @return 0 on success; non-zero on error.
 */
int input_sdl_init(void)
{
	// Initialize the keys and joystick state arrays.
	memset(input_sdl_keys, 0x00, sizeof(input_sdl_keys));
	memset(input_sdl_joy_state, 0x00, sizeof(input_sdl_joy_state));
	
	// Install the GTK+ key snooper.
	gtk_key_snooper_install(input_sdl_gdk_keysnoop, NULL);
	
	// Initialize joysticks.
	input_sdl_num_joysticks = 0;
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
	{
		// Error initializing the SDL joystick handler.
		// Disable joysticks.
		LOG_MSG(input, LOG_MSG_LEVEL_CRITICAL,
			"SDL joystick initialization failed: %s. Joysticks will be unavailable.",
			SDL_GetError());
		return 0;
	}
	
	// If any joysticks are connected, set them up.
	// TODO: Increase number of joysticks from 6?
	if (SDL_NumJoysticks() > 0)
	{
		SDL_JoystickEventState(SDL_ENABLE);
		
		for (int i = 0; i < INPUT_SDL_MAX_JOYSTICKS; i++)
		{
			input_sdl_joys[i] = SDL_JoystickOpen(i);
			if (input_sdl_joys[i])
				input_sdl_num_joysticks++;
		}
	}
	
	// Joysticks initialized.
	return 0;
}


/**
 * input_sdl_end(): Shut down the SDL input subsystem.
 * @return 0 on success; non-zero on error.
 */
int input_sdl_end(void)
{
	// If any joysticks were opened, close them.
	for (unsigned int i = 0; i < INPUT_SDL_MAX_JOYSTICKS; i++)
	{
		if (SDL_JoystickOpened(i))
		{
			SDL_JoystickClose(input_sdl_joys[i]);
			input_sdl_joys[i] = NULL;
		}
	}
	
	return 0;
}


/**
 * input_sdl_gdk_keysnoop(): Keysnooping callback event for GTK+/GDK.
 * @param grab_widget Widget this key was snooped from.
 * @param event Event information.
 * @param func_data User data.
 * @return TRUE to stop processing this event; FALSE to allow GTK+ to process this event.
 */
static gint input_sdl_gdk_keysnoop(GtkWidget *grab, GdkEventKey *event, gpointer user_data)
{
	SDL_Event sdlev;
	
	// Only grab keys from the Gens window. (or controller config window)
	if (grab != gens_window)
	{
		// Not the Gens window.
		if (grab == cc_window && cc_window_is_configuring)
		{
			// Configuring controllers.
			// Don't allow GTK+ to handle this keypress.
			return TRUE;
		}
		
		// Don't push this key onto the SDL event stack.
		return FALSE;
	}
	
	switch (event->type)
	{
		case GDK_KEY_PRESS:
			sdlev.type = SDL_KEYDOWN;
			sdlev.key.state = SDL_PRESSED;
			break;
		
		case GDK_KEY_RELEASE:
			sdlev.type = SDL_KEYUP;
			sdlev.key.state = SDL_RELEASED;
			break;
		
		default:
			LOG_MSG(input, LOG_MSG_LEVEL_ERROR,
				"Unhandled GDK event type: %d", event->type);
			return FALSE;
	}
	
	// Convert this keypress from GDK to SDL.
	// TODO: Use GENS key defines instead.
	sdlev.key.keysym.sym = (SDLKey)input_sdl_gdk_to_gens_keyval(event->keyval);
	if (sdlev.key.keysym.sym != -1)
		SDL_PushEvent(&sdlev);
	
	// Allow GTK+ to process this key.
	return FALSE;
}


/**
 * input_sdl_joy_exists(): Check if the specified joystick exists.
 * @param joyNum Joystick number.
 * @return TRUE if the joystick exists; FALSE if it does not exist.
 */
BOOL input_sdl_joy_exists(int joyNum)
{
	if (joyNum < 0 || joyNum >= INPUT_SDL_MAX_JOYSTICKS)
		return FALSE;
	
	if (input_sdl_joys[joyNum])
		return TRUE;
	
	// Joystick does not exist.
	return FALSE;
}


/**
 * input_sdl_get_key(): Get a key. (Used for controller configuration.)
 * @return Key value.
 */
unsigned int input_sdl_get_key(void)
{
	// TODO: Optimize this function.
	GdkEvent *event;
	SDL_Event sdl_event;
	
	// Update the UI.
	GensUI_update();
	
	while (TRUE)
	{
		while (SDL_PollEvent(&sdl_event))
		{
			switch (sdl_event.type)
			{
				case SDL_JOYAXISMOTION:
					if (/*sdl_event.jaxis.axis < 0 ||*/ sdl_event.jaxis.axis >= 128)
						break;
					
					if (sdl_event.jaxis.value < -10000)
					{
						return INPUT_GETKEY_AXIS(sdl_event.jaxis.which,
									 sdl_event.jaxis.axis,
									 INPUT_JOYSTICK_AXIS_NEGATIVE);
					}
					else if (sdl_event.jaxis.value > 10000)
					{
						return INPUT_GETKEY_AXIS(sdl_event.jaxis.which,
									 sdl_event.jaxis.axis,
									 INPUT_JOYSTICK_AXIS_POSITIVE);
					}
					else
					{
						// FIXME: WTF is this for?!
						return input_sdl_get_key();
					}
					break;
				
				case SDL_JOYBUTTONUP:
					return INPUT_GETKEY_BUTTON(sdl_event.jbutton.which,
								   sdl_event.jbutton.button);
					break;
				
				case SDL_JOYHATMOTION:
				{
					uint8_t povHatDirection;
					if (sdl_event.jhat.value & SDL_HAT_UP)
						povHatDirection = INPUT_JOYSTICK_POVHAT_UP;
					else if (sdl_event.jhat.value & SDL_HAT_RIGHT)
						povHatDirection = INPUT_JOYSTICK_POVHAT_RIGHT;
					else if (sdl_event.jhat.value & SDL_HAT_DOWN)
						povHatDirection = INPUT_JOYSTICK_POVHAT_DOWN;
					else if (sdl_event.jhat.value & SDL_HAT_LEFT)
						povHatDirection = INPUT_JOYSTICK_POVHAT_LEFT;
					else
						break;
					
					return INPUT_GETKEY_POVHAT_DIRECTION(sdl_event.jhat.which,
									     sdl_event.jhat.hat,
									     povHatDirection);
					break;
				}
			}
		}
		
		// Check if a GDK key press occurred.
		event = gdk_event_get();
		if (event && event->type == GDK_KEY_PRESS)
			return input_sdl_gdk_to_gens_keyval(event->key.keyval);
		
		// Sleep for 1 ms.
		usleep(1000);
		
		// Make sure the "Controller Configuration" window is still configuring.
		if (!cc_window_is_configuring)
			break;
	}
}


/**
 * input_sdl_update(): Update the input subsystem.
 * @return 0 on success; non-zero on error.
 */
int input_sdl_update(void)
{
	// Check for SDL events
	SDL_Event event;
	
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				close_gens();
				return 0;
			
			/* TODO: SDL_VIDEORESIZE should work in GL mode.
			case SDL_VIDEORESIZE:
				surface = SDL_SetVideoMode(event.resize.w, event.resize.h, 16, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
				break;
			*/
			
			case SDL_ACTIVEEVENT:
				if (event.active.state & SDL_APPINPUTFOCUS)
					input_sdl_reset_modifiers();
				break;
			
			case SDL_KEYDOWN:
				input_sdl_keys[event.key.keysym.sym] = TRUE;
				input_sdl_event_key_down(event.key.keysym.sym);
				break;
				
			case SDL_KEYUP:
				input_sdl_keys[event.key.keysym.sym] = FALSE;
				input_sdl_event_key_up(event.key.keysym.sym);
				break;
			
			case SDL_JOYAXISMOTION:
				input_sdl_check_joystick_axis(&event);
				break;
			
			case SDL_JOYBUTTONDOWN:
				INPUT_SDL_JOYSTICK_SET_BUTTON(input_sdl_joy_state,
							      event.jbutton.which,
							      event.jbutton.button, TRUE);
				break;
			
			case SDL_JOYBUTTONUP:
				INPUT_SDL_JOYSTICK_SET_BUTTON(input_sdl_joy_state,
							      event.jbutton.which,
							      event.jbutton.button, FALSE);
				break;
			
			case SDL_JOYHATMOTION:
				INPUT_SDL_JOYSTICK_SET_POVHAT_DIRECTION(input_sdl_joy_state,
									event.jhat.which,
									event.jhat.hat,
									event.jhat.value);
				break;
			
			default:
				break;
		}
	}
	
	return 0;
}


/**
 * input_sdl_check_joystick_axis: Check the SDL_Event for a joystick axis event.
 * @param event Pointer to SDL_Event.
 */
static inline void input_sdl_check_joystick_axis(SDL_Event *event)
{
	if (event->jaxis.axis >= 128)
	{
		// Gens doesn't support more than 128 axes.
		return;
	}
	
	if (event->jaxis.value < -10000)
	{
		INPUT_SDL_JOYSTICK_SET_AXIS_NEGATIVE(input_sdl_joy_state,
						     event->jaxis.which,
						     event->jaxis.axis);
	}
	else if (event->jaxis.value > 10000)
	{
		INPUT_SDL_JOYSTICK_SET_AXIS_POSITIVE(input_sdl_joy_state,
						     event->jaxis.which,
						     event->jaxis.axis);
	}
	else
	{
		INPUT_SDL_JOYSTICK_SET_AXIS_CENTER(input_sdl_joy_state,
						   event->jaxis.which,
						   event->jaxis.axis);
	}
}


/**
 * input_sdl_check_key_pressed(): Checks if the specified key is pressed.
 * @param key Key to check.
 * @return TRUE if the key is pressed; FALSE if the key is not pressed.
 */
BOOL input_sdl_check_key_pressed(uint16_t key)
{
	// If the key value is <INPUT_SDL_MAX_KEYS, it's a keyboard key.
	if (key < INPUT_SDL_MAX_KEYS)
		return input_sdl_keys[key];
	
	// If this isn't a joystick input, don't check anything else.
	if (!INPUT_IS_JOYSTICK(key))
		return FALSE;
	
	// Joystick "key" check.
	
	// Determine which joystick we're looking for.
	int joyNum = INPUT_JOYSTICK_GET_NUMBER(key);
	
	// Check that this joystick exists.
	if (!input_sdl_joy_exists(joyNum))
		return FALSE;
	
	// Joystick exists. Check the state.
	switch (INPUT_JOYSTICK_GET_TYPE(key))
	{
		case INPUT_JOYSTICK_TYPE_AXIS:
			// Joystick axis.
			if (INPUT_SDL_JOYSTICK_CHECK_AXIS(input_sdl_joy_state, joyNum, key))
				return TRUE;
			break;
		
		case INPUT_JOYSTICK_TYPE_BUTTON:
			// Joystick button.
			if (INPUT_SDL_JOYSTICK_CHECK_BUTTON(input_sdl_joy_state, joyNum, key))
				return TRUE;
			break;
		
		case INPUT_JOYSTICK_TYPE_POVHAT:
		{
			// Joystick POV hat.
			static const uint8_t povKeyToBit[4] = {SDL_HAT_UP, SDL_HAT_RIGHT, SDL_HAT_DOWN, SDL_HAT_LEFT};
			uint8_t povBit = povKeyToBit[INPUT_JOYSTICK_GET_POVHAT_DIRECTION(key)];
			
			if (INPUT_SDL_JOYSTICK_CHECK_POVHAT_DIRECTION(input_sdl_joy_state, joyNum, key, povBit))
				return TRUE;
			break;
		}
	}
	
	// Key is not pressed.
	return FALSE;
}

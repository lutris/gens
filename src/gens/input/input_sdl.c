/***************************************************************************
 * Gens: Input Handler - SDL Backend.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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
#include "gdk/gdkkeysyms.h"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "gens/gens_window.hpp"

// Needed to handle controller input configuration
#include "controller_config/controller_config_window.hpp"
#include "controller_config/controller_config_window_misc.hpp"


#include <SDL/SDL.h>

// GTK stuff
#include <gtk/gtk.h>

// Function prototypes.
static int		input_sdl_init(void);
static int		input_sdl_end(void);

static int		input_sdl_update(void);
static BOOL		input_sdl_check_key_pressed(unsigned int key);
static unsigned int	input_sdl_get_key(void);
static BOOL		input_sdl_joy_exists(int joy_num);

// Miscellaneous.
static gint input_sdl_gdk_keysnoop(GtkWidget *grab, GdkEventKey *event, gpointer user_data);
static int input_sdl_gdk_to_sdl_keyval(int gdk_key);

// Maximum number of keys and joysticks.
#define INPUT_SDL_MAX_KEYS 1024
#define INPUT_SDL_MAX_JOYSTICKS 6

// Check an SDL joystick axis.
static void input_sdl_check_joystick_axis(SDL_Event *event);

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
	{GENS_KEY_UP, GENS_KEY_DOWN, GENS_KEY_LEFT, GENS_KEY_RIGHT,
	GENS_KEY_RETURN, GENS_KEY_a, GENS_KEY_s, GENS_KEY_d,
	GENS_KEY_RSHIFT, GENS_KEY_z, GENS_KEY_x, GENS_KEY_c},
	
	// Players 1B, 1C, 1D
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	
	// Player 2
	{GENS_KEY_y, GENS_KEY_h, GENS_KEY_g, GENS_KEY_j,
	GENS_KEY_u, GENS_KEY_k, GENS_KEY_l, GENS_KEY_m,
	GENS_KEY_t, GENS_KEY_i, GENS_KEY_o, GENS_KEY_p},
	
	// Players 2B, 2C, 2D
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
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
	
	.update = input_sdl_update,
	.check_key_pressed = input_sdl_check_key_pressed,
	.get_key = input_sdl_get_key,
	.joy_exists = input_sdl_joy_exists
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
		// Error initializing SDL.
		fprintf(stderr, "%s(): Error initializing SDL's joystick handler: %s\n", __func__, SDL_GetError());
		return -1;
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
	if (grab != gens_window && grab != controller_config_window)
		return FALSE;
	
	switch(event->type)
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
			fputs("Can't happen: keysnoop got a bad type\n", stderr);
			return 0;
	}
	
	// Convert this keypress from GDK to SDL.
	// TODO: Use GENS key defines instead.
	sdlev.key.keysym.sym = (SDLKey)input_sdl_gdk_to_sdl_keyval(event->keyval);
	if (sdlev.key.keysym.sym != -1)
		SDL_PushEvent(&sdlev);
	
	if (grab == controller_config_window && CC_Configuring)
	{
		// Configuring controllers. Don't allow GTK+ to handle this keypress.
		return TRUE;
	}
	
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
	SDL_Joystick *js[6];
	SDL_JoystickEventState(SDL_ENABLE);
	
	// Open all 6 joysticks.
	for (int i = 0; i < INPUT_SDL_MAX_JOYSTICKS; i++)
	{
		js[i] = SDL_JoystickOpen(i);
	}
	
	// Update the UI.
	GensUI_update();
	
	while (TRUE)
	{
		while (SDL_PollEvent(&sdl_event))
		{
			switch (sdl_event.type)
			{
				case SDL_JOYAXISMOTION:
					if (/*sdl_event.jaxis.axis < 0 ||*/ sdl_event.jaxis.axis >= 6)
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
					// TODO: Joystick POV hat.
					//return (0xdeadbeef + (0x100 * sdl_event.jhat.which) + sdl_event.jhat.hat + sdl_event.jhat.value);
					break;
			}
		}
		
		// Check if a GDK key press occurred.
		event = gdk_event_get();
		if (event && event->type == GDK_KEY_PRESS)
			return input_sdl_gdk_to_sdl_keyval(event->key.keyval);
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
				INPUT_SDL_JOYSTICK_SET_POVHAT(input_sdl_joy_state,
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
static void input_sdl_check_joystick_axis(SDL_Event *event)
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
BOOL input_sdl_check_key_pressed(unsigned int key)
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
		
		case INPUT_JOYSTICK_TYPE_POV:
			// TODO
			break;
	}
	
	// Key is not pressed.
	return FALSE;
}


/**
 * input_sdl_gdk_to_sdl_keyval(): Converts a GDK key value to a Gens key value.
 * @param gdk_key GDK key value.
 * @return Gens key value.
 */
static int input_sdl_gdk_to_sdl_keyval(int gdk_key)
{
	if (!(gdk_key & 0xFF00))
	{
		// ASCII symbol.
		// SDL and GDK use the same values for these keys.
		
		// Make sure the key value is lowercase.
		gdk_key = tolower(gdk_key);
		
		// Return the key value.
		return gdk_key;
	}
	
	// Non-ASCII symbol.
	switch (gdk_key)
	{
		case GDK_BackSpace:
			return GENS_KEY_BACKSPACE;
		case GDK_Tab:
			return GENS_KEY_TAB;
		case GDK_Clear:
			return GENS_KEY_CLEAR;
		case GDK_Return:
			return GENS_KEY_RETURN;
		case GDK_Pause:
			return GENS_KEY_PAUSE;
		case GDK_Escape:
			return GENS_KEY_ESCAPE;
		case GDK_KP_Space:
			return GENS_KEY_SPACE;
		case GDK_Delete:
			return GENS_KEY_DELETE;
		case GDK_KP_0:
			return GENS_KEY_NUM_0;
		case GDK_KP_1:
			return GENS_KEY_NUM_1;
		case GDK_KP_2:
			return GENS_KEY_NUM_2;
		case GDK_KP_3:
			return GENS_KEY_NUM_3;
		case GDK_KP_4:
			return GENS_KEY_NUM_4;
		case GDK_KP_5:
			return GENS_KEY_NUM_5;
		case GDK_KP_6:
			return GENS_KEY_NUM_6;
		case GDK_KP_7:
			return GENS_KEY_NUM_7;
		case GDK_KP_8:
			return GENS_KEY_NUM_8;
		case GDK_KP_9:
			return GENS_KEY_NUM_9;
		case GDK_KP_Decimal:
			return GENS_KEY_NUM_PERIOD;
		case GDK_KP_Divide:
			return GENS_KEY_NUM_DIVIDE;
		case GDK_KP_Multiply:
			return GENS_KEY_NUM_MULTIPLY;
		case GDK_KP_Subtract:
			return GENS_KEY_NUM_MINUS;
		case GDK_KP_Add:
			return GENS_KEY_NUM_PLUS;
		case GDK_KP_Enter:
			return GENS_KEY_NUM_ENTER;
		case GDK_KP_Equal:
			return GENS_KEY_NUM_EQUALS;
		case GDK_Up:
			return GENS_KEY_UP;
		case GDK_Down:
			return GENS_KEY_DOWN;
		case GDK_Right:
			return GENS_KEY_RIGHT;
		case GDK_Left:
			return GENS_KEY_LEFT;
		case GDK_Insert:
			return GENS_KEY_INSERT;
		case GDK_Home:
			return GENS_KEY_HOME;
		case GDK_End:
			return GENS_KEY_END;
		case GDK_Page_Up:
			return GENS_KEY_PAGEUP;
		case GDK_Page_Down:
			return GENS_KEY_PAGEDOWN;
		case GDK_F1:
			return GENS_KEY_F1;
		case GDK_F2:
			return GENS_KEY_F2;
		case GDK_F3:
			return GENS_KEY_F3;
		case GDK_F4:
			return GENS_KEY_F4;
		case GDK_F5:
			return GENS_KEY_F5;
		case GDK_F6:
			return GENS_KEY_F6;
		case GDK_F7:
			return GENS_KEY_F7;
		case GDK_F8:
			return GENS_KEY_F8;
		case GDK_F9:
			return GENS_KEY_F9;
		case GDK_F10:
			return GENS_KEY_F10;
		case GDK_F11:
			return GENS_KEY_F11;
		case GDK_F12:
			return GENS_KEY_F12;
		case GDK_F13:
			return GENS_KEY_F13;
		case GDK_F14:
			return GENS_KEY_F14;
		case GDK_F15:
			return GENS_KEY_F15;
		case GDK_Num_Lock:
			return GENS_KEY_NUMLOCK;
		case GDK_Caps_Lock:
			return GENS_KEY_CAPSLOCK;
		case GDK_Scroll_Lock:
			return GENS_KEY_SCROLLOCK;
		case GDK_Shift_R:
			return GENS_KEY_RSHIFT;
		case GDK_Shift_L:
			return GENS_KEY_LSHIFT;
		case GDK_Control_R:
			return GENS_KEY_RCTRL;
		case GDK_Control_L:
			return GENS_KEY_LCTRL;
		case GDK_Alt_R:
			return GENS_KEY_RALT;
		case GDK_Alt_L:
			return GENS_KEY_LALT;
		case GDK_Meta_R:
			return GENS_KEY_RMETA;
		case GDK_Meta_L:
			return GENS_KEY_LMETA;
		case GDK_Super_L:
			return GENS_KEY_LSUPER;
		case GDK_Super_R:
			return GENS_KEY_RSUPER;
		case GDK_Mode_switch:
			return GENS_KEY_MODE;
		//case GDK_:
		//	return GENS_KEY_COMPOSE;
		case GDK_Help:
			return GENS_KEY_HELP;
		case GDK_Print:
			return GENS_KEY_PRINT;
		case GDK_Sys_Req:
			return GENS_KEY_SYSREQ;
		case GDK_Break:
			return GENS_KEY_BREAK;
		case GDK_Menu:
			return GENS_KEY_MENU;
		//case GDK_:
		//	return GENS_KEY_POWER;
		case GDK_EuroSign:
			return GENS_KEY_EURO;
		case GDK_Undo:
			return GENS_KEY_UNDO;
		default:
			fprintf(stderr, "%s(): Unknown GDK key: 0x%04X\n", __func__, gdk_key);
			return -1;
	}
}

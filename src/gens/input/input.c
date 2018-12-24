/***************************************************************************
 * Gens: Input Handler - Base Code.                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * Copyright (c) 2009 by Phil Costin                                       *
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

#include "input.h"
#include "input_update.h"

// C includes.
#include <stdio.h>
#include <string.h>

// libgsft includes.
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"

// Array of key names.
const char input_key_names[12][8] =
{
	"Up", "Down", "Left", "Right",
	"Start", "A", "B", "C",
	"Mode", "X", "Y", "Z"
};


// Array of player names.
const char input_player_names[8][4] =
{
	"P1", "P2",
	"P1B", "P1C", "P1D",
	"P2B", "P2C", "P2D"
};


// Input backends.
#if defined(GENS_OS_WIN32)
	#include "input_dinput_t.h"
#elif defined(GENS_OS_HAIKU)
	#include "input_haiku_t.h"
#elif defined(GENS_OS_UNIX)
	#include "input_sdl.h"
#endif

static const input_backend_t * const input_backends[] =
{
	#if defined(GENS_OS_WIN32)
		&input_backend_dinput,
	#elif defined(GENS_OS_HAIKU)
		&input_backend_haiku,
	#elif defined(GENS_OS_UNIX)
		&input_backend_sdl,
	#endif
};

// Current backend.
const input_backend_t *input_cur_backend = NULL;
INPUT_BACKEND input_cur_backend_id;

// Function and array pointers.
int			(*input_update)(void) = NULL;
BOOL			(*input_check_key_pressed)(uint16_t key) = NULL;
uint16_t		(*input_get_key)(void) = NULL;
const input_keymap_t	*input_keymap_default = NULL;
#ifdef GENS_OS_WIN32
int			(*input_set_cooperative_level)(HWND hWnd);
#endif /* GENS_OS_WIN32 */

// Current keymap.
input_keymap_t	input_keymap[8];


/**
 * input_init(): Initialize an Input backend.
 * @param backend Input backend to initialize.
 * @return 0 on success; non-zero on error.
 */
int input_init(INPUT_BACKEND backend)
{
	if (backend < 0 || backend >= INPUT_BACKEND_MAX)
	{
		// Invalid backend.
		return -1;
	}
	
	if (input_cur_backend)
	{
		// Backend is currently initialized.
		return -2;
	}
	
	// Set up the variables.
	input_cur_backend = input_backends[backend];
	input_cur_backend_id = backend;
	
	// Copy the function and array pointers.
	input_update		= input_cur_backend->update;
	input_check_key_pressed	= input_cur_backend->check_key_pressed;
	input_get_key		= input_cur_backend->get_key;
	input_keymap_default	= input_cur_backend->keymap_default;
#ifdef GENS_OS_WIN32
	input_set_cooperative_level = input_cur_backend->set_cooperative_level;
#endif /* GENS_OS_WIN32 */
	
	// Initialize the backend.
	if (input_cur_backend->init)
		return input_cur_backend->init();
	
	// Initialized successfully.
	return 0;
}


/**
 * input_end(): Shut down the Input backend.
 * @return 0 on success; non-zero on error.
 */
int input_end(void)
{
	if (!input_cur_backend)
	{
		// No Input backend is initialized.
		return -1;
	}
	
	// Shut down the Input backend.
	if (input_cur_backend->end)
		input_cur_backend->end();
	
	input_cur_backend = NULL;
	return 0;
}


/**
 * input_update_joykey_format(): Update a joystick "key" from the old format to the new format.
 * @param key Joystick "key".
 * @return Updated joystick "key", or original value if no update was necessary.
 */
uint16_t input_update_joykey_format(uint16_t key)
{
	// The old joystick key mapping has a "1" in the most-significant nybble.
	if ((key & 0xF000) != 0x1000)
	{
		// Not in the old format.
		return key;
	}
	
	// Joystick "key" is in the old format.
	// Check what type of "key" it is.
	if (key & 0x70)
	{
		// Button.
		return (0x8000 | (INPUT_JOYSTICK_TYPE_BUTTON << 12) | (key & 0x0F00) | ((key & 0x7F) - 0x10));
	}
	else if (key & 0x80)
	{
		// POV Hat.
		// TODO
		return key;
	}
	else
	{
		// Axis.
		static const uint8_t mapOldJoyAxisToNew[12] =
		{
			0x02, 0x03, 0x00, 0x01, 0x06, 0x07,
			0x04, 0x05, 0x0A, 0x0B, 0x08, 0x09
		};
		
		uint8_t axis = (key & 0x0F);
		if (axis == 0 || axis > 12)
		{
			// Invalid axis.
			return key;
		}
		
		return (0x8000 | (INPUT_JOYSTICK_TYPE_AXIS << 12) | (key & 0x0F00) | (mapOldJoyAxisToNew[(key & 0x0F) - 1]));
	}
}


/**
 * input_get_key_name(): Get a key name.
 * @param key Key.
 * @param buf Buffer to store the key name in. ("Unknown Key" is stored on error.)
 * @param size Size of the buffer.
 * @return 0 on success; non-zero on error.
 */
int input_get_key_name(uint16_t key, char* buf, int size)
{
	if (size == 0)
		return -1;
	
	if (key == 0)
	{
		// 0 == not configured.
		strlcpy(buf, "Not Configured", size);
		return 0;
	}
	
	if (!INPUT_IS_JOYSTICK(key))
	{
		// Not a joystick input.
		// Defer the key name lookup to the current input handler.
		if (input_cur_backend)
		{
			int rval = input_cur_backend->get_key_name(key, buf, size);
			if (!rval)
				return 0;
			else
			{
				// Unknown key.
				strlcpy(buf, "Unknown Key", size);
				return rval;
			}
		}
		
		// No backend available. Return an error.
		strlcpy(buf, "Unknown Key", size);
		return -1;
	}
	
	// Joystick input.
	static const char pov_directions[4][8]	= {"Up", "Right", "Down", "Left"};
	static const char axis_names[6][4]	= {"X", "Y", "Z", "Rx", "Ry", "Rz"};
	
	// Joystick number.
	const int joy_num = INPUT_JOYSTICK_GET_NUMBER(key);
	
	switch (INPUT_JOYSTICK_GET_TYPE(key))
	{
		case INPUT_JOYSTICK_TYPE_AXIS:
		{
			int axis = INPUT_JOYSTICK_GET_AXIS(key);
			char dir = (INPUT_JOYSTICK_GET_AXIS_DIRECTION(key) == INPUT_JOYSTICK_AXIS_NEGATIVE ? '-' : '+');
			
			if (axis < 6)
				szprintf(buf, size, "Joy %d, Axis %s%c", joy_num, axis_names[axis], dir);
			else
				szprintf(buf, size, "Joy %d, Axis %d%c", joy_num, axis, dir);
			break;
		}
		
		case INPUT_JOYSTICK_TYPE_BUTTON:
			szprintf(buf, size, "Joy %d, Button %d", joy_num,
				 INPUT_JOYSTICK_GET_BUTTON(key));
			break;
		
		case INPUT_JOYSTICK_TYPE_POVHAT:
			szprintf(buf, size, "Joy %d, POV %d %s", joy_num,
				 INPUT_JOYSTICK_GET_POVHAT_NUMBER(key),
				 pov_directions[INPUT_JOYSTICK_GET_POVHAT_DIRECTION(key)]);
			break;
		
		default:
			strlcpy(buf, "Unknown Joy Key", size);
			return -1;
	}
	
	return 0;
}

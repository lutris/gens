/***************************************************************************
 * Gens: Input Handler - SDL Backend. (Joystick State Macros)              *
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

#ifndef GENS_INPUT_SDL_JOYSTATE_H
#define GENS_INPUT_SDL_JOYSTATE_H

// Joystick state struct.
typedef struct
{
	uint8_t buttons[256];
	uint8_t axes[128];
	uint8_t povhats[64];	// See SDL_HAT_* constants in SDL_joystick.h.
	
	// Reserved for future use.
	// Also, this aligns input_joy_state_t to 512 bytes.
	uint8_t reserved[64];
} input_sdl_joystate_t;

// Axis values.
#define INPUT_SDL_JOYSTATE_AXIS_CENTER		0
#define INPUT_SDL_JOYSTATE_AXIS_NEGATIVE	(1 << 0)
#define INPUT_SDL_JOYSTATE_AXIS_POSITIVE	(1 << 1)
#define INPUT_SDL_JOYSTATE_AXIS_MASK		0x03

// Axis manipulation macros.
#define INPUT_SDL_JOYSTICK_SET_AXIS_NEGATIVE(joystate, joystick, axis)		\
	joystate[joystick].axes[axis & 0x7F] = INPUT_SDL_JOYSTATE_AXIS_NEGATIVE;
#define INPUT_SDL_JOYSTICK_SET_AXIS_POSITIVE(joystate, joystick, axis)		\
	joystate[joystick].axes[axis & 0x7F] = INPUT_SDL_JOYSTATE_AXIS_POSITIVE;
#define INPUT_SDL_JOYSTICK_SET_AXIS_CENTER(joystate, joystick, axis)		\
	joystate[joystick].axes[axis & 0x7F] = INPUT_SDL_JOYSTATE_AXIS_CENTER;

// Button manipulation macros.
#define INPUT_SDL_JOYSTICK_SET_BUTTON(joystate, joystick, button, value) \
	joystate[joystick].buttons[button] = value

// POV Hat manipulation macros.
#define INPUT_SDL_JOYSTICK_SET_POVHAT_DIRECTION(joystate, joystick, povhat, direction)	\
	joystate[joystick].povhats[povhat & 0x3F] = direction

// Macros to manipulate joystick button "key" values.
#define INPUT_SDL_JOYSTICK_CHECK_AXIS(joystate, joystick, key)		\
	(joystate[joystick].axes[(key >> 1) & 0x7F] == ((key & 0x01) + 1))
#define INPUT_SDL_JOYSTICK_CHECK_BUTTON(joystate, joystick, key)	\
	((joystate[joystick].buttons[key & 0xFF]) & 0x01)
#define INPUT_SDL_JOYSTICK_CHECK_POVHAT_DIRECTION(joystate, joystick, key, direction)	\
	(joystate[joystick].povhats[(key >> 2) & 0x3F] & (direction))

#endif /* GENS_INPUT_SDL_JOYSTATE_H */

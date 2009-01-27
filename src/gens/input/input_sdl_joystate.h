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
	uint8_t axes[128][2];	// index 0 == negative; index 1 == positive
	uint8_t buttons[256];
} input_joy_state_t;

// Macros to manipulate input_joy_state_t.
#define INPUT_SDL_JOYSTICK_SET_AXIS_NEGATIVE(joystate, joystick, axis)	\
	joystate[joystick].axes[axis][0] = TRUE;			\
	joystate[joystick].axes[axis][1] = FALSE
#define INPUT_SDL_JOYSTICK_SET_AXIS_POSITIVE(joystate, joystick, axis)	\
	joystate[joystick].axes[axis][0] = FALSE;			\
	joystate[joystick].axes[axis][1] = TRUE
#define INPUT_SDL_JOYSTICK_SET_AXIS_NONE(joystate, joystick, axis)	\
	joystate[joystick].axes[axis][0] = FALSE;			\
	joystate[joystick].axes[axis][1] = FALSE
#define INPUT_SDL_JOYSTICK_SET_BUTTON(joystate, joystick, button, value) \
	joystate[joystick].buttons[button] = value

// Macros to manipulate joystick button "key" values.
#define INPUT_SDL_JOYSTICK_CHECK_AXIS(joystate, joystick, key)		\
	joystate[joystick].axes[(key >> 1) & 0x7F][key & 0x01]
#define INPUT_SDL_JOYSTICK_CHECK_BUTTON(joystate, joystick, key)	\
	joystate[joystick].buttons[key]

#endif /* GENS_INPUT_SDL_JOYSTATE_H */

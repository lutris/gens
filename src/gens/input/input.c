/***************************************************************************
 * Gens: Input Handler - Base Code.                                        *
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

#include "input.h"

// C includes.
#include <string.h>

#include "gens_core/io/io.h"


// Array of key names.
const char input_key_names[12][8] =
{
	"Up", "Down", "Left", "Right",
	"Start", "A", "B", "C",
	"Mode", "X", "Y", "Z"
};


// Input backends.
#ifdef GENS_OS_WIN32
	#include "input_dinput_t.h"
#else /* !GENS_OS_WIN32 */
	#include "input_sdl.h"
#endif /* GENS_OS_WIN32 */

static const input_backend_t * const input_backends[] =
{
	#ifdef GENS_OS_WIN32
		&input_backend_dinput,
	#else /* !GENS_OS_WIN32 */
		&input_backend_sdl,
	#endif /* GENS_OS_WIN32 */
};

// Current backend.
const input_backend_t *input_cur_backend = NULL;
INPUT_BACKEND input_cur_backend_id;

// Function and array pointers.
int			(*input_update)(void) = NULL;
static BOOL		(*input_check_key_pressed)(unsigned int key) = NULL;
unsigned int		(*input_get_key)(void) = NULL;
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


#define CHECK_BUTTON(player, ctrl, button, mask)			\
	if (input_check_key_pressed(input_keymap[player].keys.button))	\
		Controller_ ## ctrl ## _Buttons &= ~mask;		\
	else Controller_ ## ctrl ## _Buttons |= mask;

#define CHECK_DIR(player, ctrl)						\
	if (input_check_key_pressed(input_keymap[player].keys.Up))	\
	{					   			\
		Controller_ ## ctrl ## _Buttons &= ~CONTROLLER_UP;	\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_DOWN;	\
	}								\
	else								\
	{								\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_UP;	\
		CHECK_BUTTON(player, ctrl, Down, CONTROLLER_DOWN)	\
	}								\
	if (input_check_key_pressed(input_keymap[player].keys.Left))	\
	{					   			\
		Controller_ ## ctrl ## _Buttons &= ~CONTROLLER_LEFT;	\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_RIGHT;	\
	}								\
	else								\
	{								\
		Controller_ ## ctrl ## _Buttons |= CONTROLLER_LEFT;	\
		CHECK_BUTTON(player, ctrl, Right, CONTROLLER_RIGHT)	\
	}

#define CHECK_ALL_BUTTONS(player, ctrl)					\
	CHECK_BUTTON(player, ctrl, Start, CONTROLLER_START)		\
	CHECK_BUTTON(player, ctrl, A, CONTROLLER_A)			\
	CHECK_BUTTON(player, ctrl, B, CONTROLLER_B)			\
	CHECK_BUTTON(player, ctrl, C, CONTROLLER_C)			\
									\
	if (Controller_ ## ctrl ## _Type & 1)				\
	{								\
		CHECK_BUTTON(player, ctrl, Mode, CONTROLLER_MODE)	\
		CHECK_BUTTON(player, ctrl, X, CONTROLLER_X)		\
		CHECK_BUTTON(player, ctrl, Y, CONTROLLER_Y)		\
		CHECK_BUTTON(player, ctrl, Z, CONTROLLER_Z)		\
	}

#define CHECK_PLAYER_PAD(player, ctrl)					\
	CHECK_DIR(player, ctrl)						\
	CHECK_ALL_BUTTONS(player, ctrl)

/**
 * input_update_controllers(): Update the controller bitfields.
 */
void input_update_controllers(void)
{
	if (!input_cur_backend)
		return;
	
	CHECK_PLAYER_PAD(0, 1);
	CHECK_PLAYER_PAD(1, 2);
	
	if (Controller_1_Type & 0x10)
	{
		// TEAMPLAYER PORT 1
		CHECK_PLAYER_PAD(2, 1B);
		CHECK_PLAYER_PAD(3, 1C);
		CHECK_PLAYER_PAD(4, 1D);
	}
	
	if (Controller_2_Type & 0x10)
	{
		// TEAMPLAYER PORT 2
		CHECK_PLAYER_PAD(5, 2B);
		CHECK_PLAYER_PAD(6, 2C);
		CHECK_PLAYER_PAD(7, 2D);
	}
}

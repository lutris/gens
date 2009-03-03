/***************************************************************************
 * Gens: Input Handler - Update Controllers.                               *
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

#include "input_update.h"
#include "input.h"

#include "emulator/g_main.hpp"
#include "gens_core/io/io.h"

// C includes.
#include <stdint.h>


/**
 * CHECK_BUTTON(): Check if a button is pressed.
 * @param player Player number.
 * @param ctrl Controller ID. (Player 0 == ID 1)
 * @param button Button.
 * @param mask Button mask.
 */
#define CHECK_BUTTON(player, ctrl, button, mask)				\
	do									\
	{									\
		if (input_check_key_pressed(input_keymap[player].keys.button))	\
			Controller_ ## ctrl ## _Buttons &= ~mask;		\
		else Controller_ ## ctrl ## _Buttons |= mask;			\
	} while (0)


/**
 * CHECK_DIR_RESTRICT(): Check the player's direction, with input restrictions.
 * @param player Player number.
 * @param ctrl Controller ID. (Player 0 == ID 1)
 */
#define CHECK_DIR_RESTRICT(player, ctrl)					\
	do									\
	{									\
		/* Up / Down */							\
		if (input_check_key_pressed(input_keymap[player].keys.Up) &&	\
		    Controller_ ## ctrl ## _Buttons & CONTROLLER_DOWN)		\
		{					   			\
			Controller_ ## ctrl ## _Buttons &= ~CONTROLLER_UP;	\
		}								\
		else								\
		{								\
			Controller_ ## ctrl ## _Buttons |= CONTROLLER_UP;	\
		}								\
		if (input_check_key_pressed(input_keymap[player].keys.Down) &&	\
		    Controller_ ## ctrl ## _Buttons & CONTROLLER_UP)		\
		{					   			\
			Controller_ ## ctrl ## _Buttons &= ~CONTROLLER_DOWN;	\
		}								\
		else								\
		{								\
			Controller_ ## ctrl ## _Buttons |= CONTROLLER_DOWN;	\
		}								\
										\
		/* Left / Right */						\
		if (input_check_key_pressed(input_keymap[player].keys.Left) &&	\
		    Controller_ ## ctrl ## _Buttons & CONTROLLER_RIGHT)		\
		{					   			\
			Controller_ ## ctrl ## _Buttons &= ~CONTROLLER_LEFT;	\
		}								\
		else								\
		{								\
			Controller_ ## ctrl ## _Buttons |= CONTROLLER_LEFT;	\
		}								\
		if (input_check_key_pressed(input_keymap[player].keys.Right) &&	\
		    Controller_ ## ctrl ## _Buttons & CONTROLLER_LEFT)		\
		{					   			\
			Controller_ ## ctrl ## _Buttons &= ~CONTROLLER_RIGHT;	\
		}								\
		else								\
		{								\
			Controller_ ## ctrl ## _Buttons |= CONTROLLER_RIGHT;	\
		}								\
	} while (0)


/**
 * CHECK_DIR_NO_RESTRICT(): Check the player's direction, without input restrictions.
 * @param player Player number.
 * @param ctrl Controller ID. (Player 0 == ID 1)
 */
#define CHECK_DIR_NO_RESTRICT(player, ctrl)					\
	do									\
	{									\
		CHECK_BUTTON(player, ctrl, Up, CONTROLLER_UP);			\
		CHECK_BUTTON(player, ctrl, Down, CONTROLLER_DOWN);		\
		CHECK_BUTTON(player, ctrl, Left, CONTROLLER_LEFT);		\
		CHECK_BUTTON(player, ctrl, Right, CONTROLLER_RIGHT);		\
	} while (0)


/**
 * CHECK_ALL_BUTTONS(): Check all buttons on a controller (excluding D-pad).
 * @param player Player number.
 * @param ctrl Controller ID. (Player 0 == ID 1)
 */
#define CHECK_ALL_BUTTONS(player, ctrl)						\
	do									\
	{									\
		CHECK_BUTTON(player, ctrl, Start, CONTROLLER_START);		\
		CHECK_BUTTON(player, ctrl, A, CONTROLLER_A);			\
		CHECK_BUTTON(player, ctrl, B, CONTROLLER_B);			\
		CHECK_BUTTON(player, ctrl, C, CONTROLLER_C);			\
										\
		if (Controller_ ## ctrl ## _Type & 1)				\
		{								\
			CHECK_BUTTON(player, ctrl, Mode, CONTROLLER_MODE);	\
			CHECK_BUTTON(player, ctrl, X, CONTROLLER_X);		\
			CHECK_BUTTON(player, ctrl, Y, CONTROLLER_Y);		\
			CHECK_BUTTON(player, ctrl, Z, CONTROLLER_Z);		\
		}								\
	} while (0)


/**
 * CHECK_PLAYER_PAD(): Check a controller.
 * @param player Player number.
 * @param ctrl Controller ID. (Player 0 == ID 1)
 */
#define CHECK_PLAYER_PAD(player, ctrl)				\
	do							\
	{							\
		if (Settings.restrict_input)			\
			CHECK_DIR_RESTRICT(player, ctrl);	\
		else						\
			CHECK_DIR_NO_RESTRICT(player, ctrl);	\
		CHECK_ALL_BUTTONS(player, ctrl);		\
	} while (0)


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

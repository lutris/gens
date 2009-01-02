/***************************************************************************
 * Gens: Input Handler - DirectInput 5 Backend. (input_backend_t struct)   *
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

#include "input_dinput_t.h"
#include "input_dinput.hpp"

// Input Backend struct.
const input_backend_t input_backend_dinput =
{
	.init = input_dinput_init,
	.end = input_dinput_end,
	
	.keymap_default = &input_dinput_keymap_default[0],
	
	.update = input_dinput_update,
	.check_key_pressed = input_dinput_check_key_pressed,
	.get_key = input_dinput_get_key,
	.joy_exists = input_dinput_joy_exists,
	
	// Win32-specific functions.
	.set_cooperative_level = input_dinput_set_cooperative_level
};

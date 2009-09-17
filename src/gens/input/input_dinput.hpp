/***************************************************************************
 * Gens: Input Handler - DirectInput 5 Backend.                            *
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

#ifndef GENS_INPUT_DINPUT_HPP
#define GENS_INPUT_DINPUT_HPP

#include <stdint.h>
#include "libgsft/gsft_bool.h"

#include "input.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes.
int		input_dinput_init(void);
int		input_dinput_end(void);

int		input_dinput_update(void);
BOOL		input_dinput_check_key_pressed(uint16_t key);
uint16_t	input_dinput_get_key(void);
BOOL		input_dinput_joy_exists(int joy_num);

// Win32-specific functions.
int		input_dinput_set_cooperative_level(HWND hWnd);

// Win32-specific functions not used in input_backend_t.
int		input_dinput_init_joysticks(HWND hWnd);

// Used for the Controller Configuration window.
void		input_dinput_add_joysticks_to_listbox(HWND lstBox);

// Default keymap.
extern const input_keymap_t input_dinput_keymap_default[8];

#ifdef __cplusplus
}
#endif

#endif

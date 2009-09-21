/***************************************************************************
 * Gens: Input Handler - Haiku Backend.                                    *
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

#ifndef GENS_INPUT_HAIKU_HPP
#define GENS_INPUT_HAIKU_HPP

#include "libgsft/gsft_bool.h"
#include <stdint.h>

#include "input.h"

#ifdef __cplusplus
extern "C" {
#endif

// Haiku-specific input callback structure
extern const input_backend_t input_backend_haiku;

// Function prototypes.
int		input_haiku_init(void);
int		input_haiku_end(void);

int		input_haiku_update(void);
BOOL		input_haiku_check_key_pressed(uint16_t key);
uint16_t	input_haiku_get_key(void);
BOOL		input_haiku_joy_exists(int joy_num);

// Haiku-specific key handler function declarations
void input_haiku_event_key_down(int key);
void input_haiku_event_key_up(int key);

// Keymap variables and functions.
extern const input_keymap_t input_haiku_keymap_default[8];
int input_haiku_get_key_name(uint16_t key, char* buf, int size);

#ifdef __cplusplus
}
#endif

#endif /* GENS_INPUT_HAIKU_HPP */

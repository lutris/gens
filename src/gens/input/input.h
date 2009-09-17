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

#ifndef GENS_INPUT_H
#define GENS_INPUT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// OS-specific includes.
#if defined(GENS_OS_UNIX)
#include "input_sdl_keys.h"
#elif defined(GENS_OS_WIN32)
#include "input_win32_keys.h"
#elif defined(GENS_OS_HAIKU)
// Nothing else to include
#else
#error Unsupported operating system.
#endif

// Needed for HWND in input_set_cooperative_level.
#ifdef GENS_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "libgsft/gsft_bool.h"

/************************************************************************
 * Controller key mapping.
 * If the high bit of the key value is 0, it is a keyboard key.
 * If the high bit of the key value is 1, it is a joystick input.
 * Joystick inputs have the following format:
 *
 * 1TTT JJJJ WWWW WWWW
 *
 * TTT       == type of input. (0 == axis, 1 == button, 2 == POV hat)
 * JJJJ      == joystick number.
 * WWWW WWWW == which axis/button.
 *
 * Axis format: NNNN NNND
 * - NNNN NNN == the axis number. (0 to 127)
 * - D == the axis direction. 0 == negative; 1 == positive.
 *
 * POV Hat format: NNNN NNDD
 * - NNNN NN: POV hat number.
 * - DD: POV hat direction. (0 = up; 1 = right; 2 = down; 3 = left)
 ************************************************************************/

typedef union
{
	struct
	{
		uint16_t Up, Down, Left, Right;
		uint16_t Start, A, B, C;
		uint16_t Mode, X, Y, Z;
	} keys;
	uint16_t data[12];
} input_keymap_t;

// Key names.
extern const char input_key_names[12][8];

// Player names.
extern const char input_player_names[8][4];

// Joystick maximums.
#define INPUT_JOYSTICK_MAX_AXES		128
#define INPUT_JOYSTICK_MAX_BUTTONS	256
#define INPUT_JOYSTICK_MAX_POVHATS	64

// Joystick input types.
typedef enum
{
	INPUT_JOYSTICK_TYPE_AXIS = 0,
	INPUT_JOYSTICK_TYPE_BUTTON = 1,
	INPUT_JOYSTICK_TYPE_POVHAT = 2
} INPUT_JOYSTICK_TYPE;

// Joystick axis sign values.
#define INPUT_JOYSTICK_AXIS_NEGATIVE	0
#define INPUT_JOYSTICK_AXIS_POSITIVE	1

// POV hat directions.
#define INPUT_JOYSTICK_POVHAT_UP	0
#define INPUT_JOYSTICK_POVHAT_RIGHT	1
#define INPUT_JOYSTICK_POVHAT_DOWN	2
#define INPUT_JOYSTICK_POVHAT_LEFT	3

// Macros to manipulate joystick button "key" values.
#define INPUT_IS_JOYSTICK(key)		\
	((key) & 0x8000)
#define INPUT_JOYSTICK_GET_NUMBER(key)	\
	((key >> 8) & 0xF)
#define INPUT_JOYSTICK_GET_TYPE(key)	\
	((key >> 12) & 0x7)
#define INPUT_JOYSTICK_GET_AXIS(key)	\
	((key >> 1) & 0x7F)
#define INPUT_JOYSTICK_GET_AXIS_DIRECTION(key)	\
	(key & 0x01)
#define INPUT_JOYSTICK_GET_BUTTON(key)	\
	(key & 0xFF)
#define INPUT_JOYSTICK_GET_POVHAT_NUMBER(key)	\
	((key >> 2) & 0x3F)
#define INPUT_JOYSTICK_GET_POVHAT_DIRECTION(key)	\
	(key & 0x03)

// Macros for get_key().
#define INPUT_GETKEY_AXIS(joystick, axis, positive)			\
	(0x8000 | (((uint16_t)INPUT_JOYSTICK_TYPE_AXIS << 12)) |	\
		((joystick & 0xF) << 8) | ((axis & 0x7F) << 1) | (positive & 0x01))

#define INPUT_GETKEY_BUTTON(joystick, button)				\
	(0x8000 | (((uint16_t)INPUT_JOYSTICK_TYPE_BUTTON) << 12) |	\
		((joystick & 0xF) << 8) | (button & 0xFF))

#define INPUT_GETKEY_POVHAT_DIRECTION(joystick, povhat, direction)	\
	(0x8000 | (((uint16_t)INPUT_JOYSTICK_TYPE_POVHAT) << 12) |	\
		((joystick & 0xF) << 8) | ((povhat & 0x3F) << 2) | (direction & 0x03))

// TODO: INPUT_GETKEY_POV()

// Input backends.
typedef enum
{
	#if defined(GENS_OS_WIN32)
		INPUT_BACKEND_DINPUT,
	#elif defined(GENS_OS_HAIKU)
		INPUT_BACKEND_HAIKU,
	#else /* !GENS_OS_WIN32 */
		INPUT_BACKEND_SDL,
	#endif /* GENS_OS_WIN32 */
	INPUT_BACKEND_MAX
} INPUT_BACKEND;

// Input backend function pointers.
typedef struct
{
	int	(*init)(void);
	int	(*end)(void);
	
	// Default keymap. (Must have 8 elements.)
	const input_keymap_t *keymap_default;
	
	// Update the input subsystem.
	int	(*update)(void);
	
	// Check if the specified key is pressed.
	BOOL	(*check_key_pressed)(uint16_t key);
	
	// Get a key. (Used for controller configuration.)
	uint16_t (*get_key)(void);
	
	// Check if a joystick exists.
	BOOL	(*joy_exists)(int joy_num);
	
	// Get a key name.
	int	(*get_key_name)(uint16_t key, char* buf, int size);
	
#ifdef GENS_OS_WIN32
	// Win32-specific functions.
	int	(*set_cooperative_level)(HWND hWnd);
#endif /* GENS_OS_WIN32 */
} input_backend_t;

int	input_init(INPUT_BACKEND backend);
int	input_end(void);

// Current backend.
extern const input_backend_t *input_cur_backend;
extern INPUT_BACKEND input_cur_backend_id;

// Function and array pointers.
extern int			(*input_update)(void);
extern BOOL			(*input_check_key_pressed)(uint16_t key);
extern uint16_t			(*input_get_key)(void);
extern const input_keymap_t	*input_keymap_default;
#ifdef GENS_OS_WIN32
extern int			(*input_set_cooperative_level)(HWND hWnd);
#endif /* GENS_OS_WIN32 */

// Current keymap.
extern input_keymap_t	input_keymap[8];

// Update a joystick "key" from the old format to the new format.
uint16_t input_update_joykey_format(uint16_t key);

// Get a key name.
int input_get_key_name(uint16_t key, char* buf, int size);

#ifdef __cplusplus
}
#endif

#endif

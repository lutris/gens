/***************************************************************************
 * Gens: Input Handler - DirectInput 5 Backend. (Key Names)                *
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

#include "input_dinput_key_names.h"

#include <string.h>

// libgsft includes.
#include "libgsft/gsft_strlcpy.h"

/**
 * input_dinput_get_key_name(): Get a DirectInput key name.
 * @param key Key.
 * @param buf Buffer to store the key name in.
 * @param size Size of the buffer.
 * @return 0 on success; non-zero on error.
 */
int input_dinput_get_key_name(uint16_t key, char* buf, int size)
{
	static const char* const dinput_keys[0x100] =
	{
		// 0x00 - 0x0F
		NULL, "Escape", "1", "2",
		"3", "4", "5", "6",
		"7", "8", "9", "0",
		"-", "=", "Backspace", "Tab",
		
		// 0x10 - 0x1F
		"Q", "W", "E", "R",
		"T", "Y", "U", "I",
		"O", "P", "[", "]",
		"Enter", "Left Ctrl", "A", "S",
		
		// 0x20 - 0x2F
		"D", "F", "G", "H",
		"J", "K", "L", ";",
		"'", "`", "Left Shift", "\\",
		"Z", "X", "C", "V",
		
		// 0x30 - 0x3F
		"B", "N", "M", ",",
		".", "/", "Right Shift", "Numpad *",
		"Left Alt", "Space", "Caps Lock", "F1",
		"F2", "F3", "F4", "F5",
		
		// 0x40 - 0x4F
		"F6", "F7", "F8", "F9",
		"F10", "Num Lock", "Scroll Lock", "Numpad 7",
		"Numpad 8", "Numpad 9", "Numpad -", "Numpad 4",
		"Numpad 5", "Numpad 6", "Numpad +", "Numpad 1",
		
		// 0x50 - 0x5F
		"Numpad 2", "Numpad 3", "Numpad 0", "Numpad .",
		NULL, NULL, NULL, "F11",
		"F12", NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		
		// 0x60 - 0x6F [NEC PC98]
		NULL, NULL, NULL, NULL,
		"F13", "F14", NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		
		// 0x70 - 0x7F [Japanese]
		"Kana", NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, "Convert", NULL, "No Convert",
		NULL, "Yen", NULL, NULL,
		
		// 0x80 - 0x8F [NEC PC98]
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, "Numpad =", NULL, NULL,
		
		// 0x90 - 0x9F [Japanese for 0x90; NEC PC98, and others]
		"Circumflex", "@", ":", "Underline",
		"Kanfi", "Stop", "AX", "Unlabeled",
		NULL, NULL, NULL, NULL,
		"Numpad Enter", "Right Ctrl", NULL, NULL,
		
		// 0xA0 - 0xAF [unused]
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		
		// 0xB0 - 0xBF
		NULL, NULL, NULL, "Numpad ,",
		NULL, "Numpad /", NULL, "SysRq",
		"Right Alt", NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		
		// 0xC0 - 0xCF
		NULL, NULL, NULL, NULL,
		NULL, "Pause", NULL, "Home",
		"Up", "Page Up", NULL, "Left",
		NULL, "Right", NULL, "End",
		
		// 0xD0 - 0xDF
		"Down", "Page Down", "Insert", "Delete",
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "Left Windows",
		"Right Windows", "Menu", "Power", "Sleep",
		
		// 0xE0 - 0xEF [unused]
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		
		// 0xF0 - 0xFF [unused]
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL,
	};
	
	if (key > (sizeof(dinput_keys) / sizeof(const char*)))
	{
		// Unknown key.
		return -1;
	}

	const char* key_name = dinput_keys[key];
	if (!key_name)
	{
		// Unknown key.
		return -1;
	}
	
	// Known key.
	strlcpy(buf, key_name, size);
	return 0;
}

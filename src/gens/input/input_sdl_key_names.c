/***************************************************************************
 * Gens: Input Handler - SDL Backend. (Key Names)                          *
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

#include <SDL/SDL.h>

#include "gdk/gdkkeysyms.h"
#include "gdk/gdkkeys.h"

#include <gdkconfig.h>
#ifdef GDK_WINDOWING_X11
#include "X11/Xlib.h"
#endif

#include "input_sdl_key_names.h"
#include "input_sdl_keys.h"

// Message logging.
#include "macros/log_msg.h"

#include <string.h>

// libgsft includes.
#include "libgsft/gsft_strlcpy.h"

/**
 * input_sdl_gdk_to_gens_keyval(): Converts a GDK key value to a Gens key value.
 * @param gdk_key GDK key value.
 * @return Gens key value.
 */
uint16_t input_sdl_gdk_to_gens_keyval(int gdk_key)
{
	if (!(gdk_key & 0xFF00))
	{
		// ASCII symbol.
		// SDL and GDK use the same values for these keys.
		
		// Make sure the key value is lowercase.
		gdk_key = tolower(gdk_key);
		
		// Return the key value.
		return gdk_key;
	}
	
	if (gdk_key & 0xFFFF0000)
	{
		// Extended X11 key. Not supported by SDL.
#ifdef GDK_WINDOWING_X11
		LOG_MSG(input, LOG_MSG_LEVEL_WARNING,
			"Unhandled extended X11 key: 0x%08X (%s)",
			gdk_key, XKeysymToString(gdk_key));
#else
		LOG_MSG(input, LOG_MSG_LEVEL_WARNING,
			"Unhandled extended key: 0x%08X\n",
			gdk_key);
#endif
		return 0;
	}
	
	// Non-ASCII symbol.
	static const uint16_t gdk_to_sdl_table[0x100] =
	{
		// 0x00 - 0x0F
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		GENS_KEY_BACKSPACE, GENS_KEY_TAB, GENS_KEY_RETURN, GENS_KEY_CLEAR,
		0x0000, GENS_KEY_RETURN, 0x0000, 0x0000,
		
		// 0x10 - 0x1F
		0x0000, 0x0000, 0x0000, GENS_KEY_PAUSE,
		GENS_KEY_SCROLLLOCK, GENS_KEY_SYSREQ, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, GENS_KEY_ESCAPE,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x20 - 0x2F
		GENS_KEY_COMPOSE, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x30 - 0x3F [Japanese keys]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x40 - 0x4F [unused]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x50 - 0x5F
		GENS_KEY_HOME, GENS_KEY_LEFT, GENS_KEY_UP, GENS_KEY_RIGHT,
		GENS_KEY_DOWN, GENS_KEY_PAGEUP, GENS_KEY_PAGEDOWN, GENS_KEY_END,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x60 - 0x6F
		0x0000, GENS_KEY_PRINT, 0x0000, GENS_KEY_INSERT,
		GENS_KEY_UNDO, 0x0000, 0x0000, GENS_KEY_MENU,
		0x0000, GENS_KEY_HELP, GENS_KEY_BREAK, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x70 - 0x7F [mostly unused, except for Alt Gr and Num Lock]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, GENS_KEY_MODE, GENS_KEY_NUMLOCK,
		
		// 0x80 - 0x8F [mostly unused, except for some numeric keypad keys]
		GENS_KEY_NUM_5, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, GENS_KEY_NUM_ENTER, 0x0000, 0x0000,
		
		// 0x90 - 0x9F
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, GENS_KEY_NUM_7, GENS_KEY_NUM_4, GENS_KEY_NUM_8,
		GENS_KEY_NUM_6, GENS_KEY_NUM_2, GENS_KEY_NUM_9, GENS_KEY_NUM_3,
		GENS_KEY_NUM_1, GENS_KEY_NUM_5, GENS_KEY_NUM_0, GENS_KEY_NUM_PERIOD,
		
		// 0xA0 - 0xAF
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, GENS_KEY_NUM_MULTIPLY, GENS_KEY_NUM_PLUS,
		0x0000, GENS_KEY_NUM_MINUS, GENS_KEY_NUM_PERIOD, GENS_KEY_NUM_DIVIDE,
		
		// 0xB0 - 0xBF
		GENS_KEY_NUM_0, GENS_KEY_NUM_1, GENS_KEY_NUM_2, GENS_KEY_NUM_3,
		GENS_KEY_NUM_4, GENS_KEY_NUM_5, GENS_KEY_NUM_6, GENS_KEY_NUM_7,
		GENS_KEY_NUM_8, GENS_KEY_NUM_9, 0x0000, 0x0000,
		0x0000, GENS_KEY_NUM_EQUALS, GENS_KEY_F1, GENS_KEY_F2,
		
		// 0xC0 - 0xCF
		GENS_KEY_F3, GENS_KEY_F4, GENS_KEY_F5, GENS_KEY_F6,
		GENS_KEY_F7, GENS_KEY_F8, GENS_KEY_F9, GENS_KEY_F10,
		GENS_KEY_F11, GENS_KEY_F12, GENS_KEY_F13, GENS_KEY_F14,
		GENS_KEY_F15, 0x0000, 0x0000, 0x0000,
		
		// 0xD0 - 0xDF [L* and R* function keys]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0xE0 - 0xEF
		0x0000, GENS_KEY_LSHIFT, GENS_KEY_RSHIFT, GENS_KEY_LCTRL,
		GENS_KEY_RCTRL, GENS_KEY_CAPSLOCK, 0x0000, GENS_KEY_LMETA,
		GENS_KEY_RMETA, GENS_KEY_LALT, GENS_KEY_RALT, GENS_KEY_LSUPER,
		GENS_KEY_RSUPER, 0x0000, 0x0000, 0x0000,
		
		// 0xF0 - 0xFF [mostly unused, except for Delete]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, GENS_KEY_DELETE,		
	};
	
	uint16_t sdl_key = gdk_to_sdl_table[gdk_key & 0xFF];
	if (sdl_key == 0)
	{
		// Unhandled GDK key.
		LOG_MSG(input, LOG_MSG_LEVEL_WARNING,
			"Unhandled GDK key: 0x%04X (%s)",
			gdk_key, gdk_keyval_name(gdk_key));
		return 0;
	}
	
	return sdl_key;
}


/**
 * input_sdl_get_key_name(): Get an SDL key name.
 * @param key Key.
 * @param buf Buffer to store the key name in.
 * @param size Size of the buffer.
 * @return 0 on success; non-zero on error.
 */
int input_sdl_get_key_name(uint16_t key, char* buf, int size)
{
	static const char* const sdl_keys[] =
	{
		// 0x00 - 0x1F
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		"Backspace", "Tab", NULL, NULL, "Clear", "Enter", NULL, NULL,
		NULL, NULL, NULL, "Pause", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "Escape", NULL, NULL, NULL, NULL,
		
		// 0x20 - 0x3F
		"Space", "!", "\"", "#", "$", NULL, "&", "'",
		"(", ")", "*", "+", ",", "-", ".", "/",
		"0", "1", "2", "3", "4", "5", "6", "7",
		"8", "9", ":", ";", "<", "=", ">", "?",
		
		// 0x40 - 0x5F
		// Uppercase letters are ignored by SDL.
		"@", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "[", "\\", "]", "^", "_",
		
		// 0x60 - 0x7F
		"`", "A", "B", "C", "D", "E", "F", "G",
		"H", "I", "J", "K", "L", "M", "N", "O",
		"P", "Q", "R", "S", "T", "U", "V", "W",
		"X", "Y", "Z", "{", "|", "}", "~", "Delete",
		
		// 0x80 - 0x9F
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		
		// 0xA0 - 0xAF
		// International keyboard symbols.
		"World 0x00", "World 0x01", "World 0x02", "World 0x03",
		"World 0x04", "World 0x05", "World 0x06", "World 0x07",
		"World 0x08", "World 0x09", "World 0x0A", "World 0x0B",
		"World 0x0C", "World 0x0D", "World 0x0E", "World 0x0F",
		
		// 0xB0 - 0xBF
		// International keyboard symbols.
		"World 0x10", "World 0x11", "World 0x12", "World 0x13",
		"World 0x14", "World 0x15", "World 0x16", "World 0x17",
		"World 0x18", "World 0x19", "World 0x1A", "World 0x1B",
		"World 0x1C", "World 0x1D", "World 0x1E", "World 0x1F",
		
		// 0xC0 - 0xCF
		// International keyboard symbols.
		"World 0x20", "World 0x21", "World 0x22", "World 0x23",
		"World 0x24", "World 0x25", "World 0x26", "World 0x27",
		"World 0x28", "World 0x29", "World 0x2A", "World 0x2B",
		"World 0x2C", "World 0x2D", "World 0x2E", "World 0x2F",
		
		// 0xD0 - 0xDF
		// International keyboard symbols.
		"World 0x30", "World 0x31", "World 0x32", "World 0x33",
		"World 0x34", "World 0x35", "World 0x36", "World 0x37",
		"World 0x38", "World 0x39", "World 0x3A", "World 0x3B",
		"World 0x3C", "World 0x3D", "World 0x3E", "World 0x3F",
		
		// 0xE0 - 0xEF
		// International keyboard symbols.
		"World 0x40", "World 0x41", "World 0x42", "World 0x43",
		"World 0x44", "World 0x45", "World 0x46", "World 0x47",
		"World 0x48", "World 0x49", "World 0x4A", "World 0x4B",
		"World 0x4C", "World 0x4D", "World 0x4E", "World 0x4F",
		
		// 0xF0 - 0xFF
		// International keyboard symbols.
		"World 0x50", "World 0x51", "World 0x52", "World 0x53",
		"World 0x54", "World 0x55", "World 0x56", "World 0x57",
		"World 0x58", "World 0x59", "World 0x5A", "World 0x5B",
		"World 0x5C", "World 0x5D", "World 0x5E", "World 0x5F",

		// 0x100 - 0x10F
		// Numeric Keypad
		"Numpad 0", "Numpad 1", "Numpad 2", "Numpad 3",
		"Numpad 4", "Numpad 5", "Numpad 6", "Numpad 7",
		"Numpad 8", "Numpad 9", "Numpad .", "Numpad /",
		"Numpad *", "Numpad -", "Numpad +", "Numpad Enter",
		
		// 0x110 - 0x12F
		"Numpad =", "Up", "Down", "Right", "Left", "Insert", "Home", "End",
		"Page Up", "PageDown", "F1", "F2", "F3", "F4", "F5", "F6",
		"F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14",
		"F15", NULL, NULL, NULL, "Num Lock", "Caps Lock", "Scroll Lock", "Right Shift",
		
		// 0x130 - 0x13F
		"Left Shift", "Right Ctrl", "Left Ctrl", "Right Alt",
		"Left Alt", "Right Meta", "Left Meta", "Left Super",
		"Right Super", "Alt Gr", "Compose", "Help",
		"Print Screen", "SysRq", "Break", "Menu",
		
		// 0x140 - 0x142
		"Power", "Euro", "Undo"
	};

	if (key > (sizeof(sdl_keys) / sizeof(const char*)))
	{
		// Unknown key.
		return -1;
	}

	const char* key_name = sdl_keys[key];
	if (!key_name)
	{
		// Unknown key.
		return -1;
	}
	
	// Known key.
	strlcpy(buf, key_name, size);
	return 0;
}

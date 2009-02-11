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

#include "input_sdl_key_names.h"
#include "input_sdl_keys.h"

/**
 * input_sdl_gdk_to_sdl_keyval(): Converts a GDK key value to a Gens key value.
 * @param gdk_key GDK key value.
 * @return Gens key value.
 */
int input_sdl_gdk_to_sdl_keyval(int gdk_key)
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
	
	// Non-ASCII symbol.
	switch (gdk_key)
	{
		case GDK_BackSpace:
			return GENS_KEY_BACKSPACE;
		case GDK_Tab:
			return GENS_KEY_TAB;
		case GDK_Clear:
			return GENS_KEY_CLEAR;
		case GDK_Return:
			return GENS_KEY_RETURN;
		case GDK_Pause:
			return GENS_KEY_PAUSE;
		case GDK_Escape:
			return GENS_KEY_ESCAPE;
		case GDK_KP_Space:
			return GENS_KEY_SPACE;
		case GDK_Delete:
			return GENS_KEY_DELETE;
		case GDK_KP_0:
			return GENS_KEY_NUM_0;
		case GDK_KP_1:
			return GENS_KEY_NUM_1;
		case GDK_KP_2:
			return GENS_KEY_NUM_2;
		case GDK_KP_3:
			return GENS_KEY_NUM_3;
		case GDK_KP_4:
			return GENS_KEY_NUM_4;
		case GDK_KP_5:
			return GENS_KEY_NUM_5;
		case GDK_KP_6:
			return GENS_KEY_NUM_6;
		case GDK_KP_7:
			return GENS_KEY_NUM_7;
		case GDK_KP_8:
			return GENS_KEY_NUM_8;
		case GDK_KP_9:
			return GENS_KEY_NUM_9;
		case GDK_KP_Decimal:
			return GENS_KEY_NUM_PERIOD;
		case GDK_KP_Divide:
			return GENS_KEY_NUM_DIVIDE;
		case GDK_KP_Multiply:
			return GENS_KEY_NUM_MULTIPLY;
		case GDK_KP_Subtract:
			return GENS_KEY_NUM_MINUS;
		case GDK_KP_Add:
			return GENS_KEY_NUM_PLUS;
		case GDK_KP_Enter:
			return GENS_KEY_NUM_ENTER;
		case GDK_KP_Equal:
			return GENS_KEY_NUM_EQUALS;
		case GDK_Up:
			return GENS_KEY_UP;
		case GDK_Down:
			return GENS_KEY_DOWN;
		case GDK_Right:
			return GENS_KEY_RIGHT;
		case GDK_Left:
			return GENS_KEY_LEFT;
		case GDK_Insert:
			return GENS_KEY_INSERT;
		case GDK_Home:
			return GENS_KEY_HOME;
		case GDK_End:
			return GENS_KEY_END;
		case GDK_Page_Up:
			return GENS_KEY_PAGEUP;
		case GDK_Page_Down:
			return GENS_KEY_PAGEDOWN;
		case GDK_F1:
			return GENS_KEY_F1;
		case GDK_F2:
			return GENS_KEY_F2;
		case GDK_F3:
			return GENS_KEY_F3;
		case GDK_F4:
			return GENS_KEY_F4;
		case GDK_F5:
			return GENS_KEY_F5;
		case GDK_F6:
			return GENS_KEY_F6;
		case GDK_F7:
			return GENS_KEY_F7;
		case GDK_F8:
			return GENS_KEY_F8;
		case GDK_F9:
			return GENS_KEY_F9;
		case GDK_F10:
			return GENS_KEY_F10;
		case GDK_F11:
			return GENS_KEY_F11;
		case GDK_F12:
			return GENS_KEY_F12;
		case GDK_F13:
			return GENS_KEY_F13;
		case GDK_F14:
			return GENS_KEY_F14;
		case GDK_F15:
			return GENS_KEY_F15;
		case GDK_Num_Lock:
			return GENS_KEY_NUMLOCK;
		case GDK_Caps_Lock:
			return GENS_KEY_CAPSLOCK;
		case GDK_Scroll_Lock:
			return GENS_KEY_SCROLLOCK;
		case GDK_Shift_R:
			return GENS_KEY_RSHIFT;
		case GDK_Shift_L:
			return GENS_KEY_LSHIFT;
		case GDK_Control_R:
			return GENS_KEY_RCTRL;
		case GDK_Control_L:
			return GENS_KEY_LCTRL;
		case GDK_Alt_R:
			return GENS_KEY_RALT;
		case GDK_Alt_L:
			return GENS_KEY_LALT;
		case GDK_Meta_R:
			return GENS_KEY_RMETA;
		case GDK_Meta_L:
			return GENS_KEY_LMETA;
		case GDK_Super_L:
			return GENS_KEY_LSUPER;
		case GDK_Super_R:
			return GENS_KEY_RSUPER;
		case GDK_Mode_switch:
			return GENS_KEY_MODE;
		//case GDK_:
		//	return GENS_KEY_COMPOSE;
		case GDK_Help:
			return GENS_KEY_HELP;
		case GDK_Print:
			return GENS_KEY_PRINT;
		case GDK_Sys_Req:
			return GENS_KEY_SYSREQ;
		case GDK_Break:
			return GENS_KEY_BREAK;
		case GDK_Menu:
			return GENS_KEY_MENU;
		//case GDK_:
		//	return GENS_KEY_POWER;
		case GDK_EuroSign:
			return GENS_KEY_EURO;
		case GDK_Undo:
			return GENS_KEY_UNDO;
		default:
			fprintf(stderr, "%s(): Unknown GDK key: 0x%04X\n", __func__, gdk_key);
			return -1;
	}
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
		"X", "Y", "Z", NULL, NULL, NULL, NULL, "Delete",
		
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
		"NumPad 0", "NumPad 1", "NumPad 2", "NumPad 3",
		"NumPad 4", "NumPad 5", "NumPad 6", "NumPad 7",
		"NumPad 8", "NumPad 9", "NumPad .", "NumPad /",
		"NumPad *", "NumPad -", "NumPad +", "NumPad Enter",
		
		// 0x110 - 0x12F
		"NumPad =", "Up", "Down", "Right", "Left", "Insert", "Home", "End",
		"Page Up", "PageDown", "F1", "F2", "F3", "F4", "F5", "F6",
		"F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14",
		"F15", NULL, NULL, NULL, "Num Lock", "Caps Lock", "Scroll Lock", "Right Shift",
		
		// 0x130 - 0x13F
		"Left Shift", "Right Ctrl", "Left Ctrl", "Right Alt",
		"Left Alt", "Right Meta", "Left Meta", "Right Super",
		"Left Super", "Alt Gr", "Compose", "Help",
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
	strncpy(buf, key_name, size);
	buf[size - 1] = 0x00;
	return 0;
}

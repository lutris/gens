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

#include "input_haiku.hpp"

// Message logging.
#include "macros/log_msg.h"

#include <unistd.h>

#include "emulator/g_main.hpp"
#include "emulator/g_md.hpp"
#include "emulator/g_mcd.hpp"
#include "emulator/options.hpp"
#include "util/file/save.hpp"
#include "util/sound/gym.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/gfx/imageutil.hpp"

#include "ui/gens_ui.hpp"
#include "gens/gens_window.h"

//#ifdef GENS_DEBUGGER
//#include "debugger/debugger.hpp"
//#endif

//#ifdef GENS_CDROM
//#include "segacd/cd_aspi.hpp"
//#endif

#include "gens/gens_window_sync.hpp"
#include "gens_ui.hpp"

// Plugin Manager and Render Manager.
#include "plugins/pluginmgr.hpp"
#include "plugins/rendermgr.hpp"

// Video, Audio, Input.
#include "video/vdraw.h"
#include "video/vdraw_cpp.hpp"
#include "audio/audio.h"
#include "input/input.h"

// C++ includes.
#include <list>
using std::list;

// Haiku Key Constants
#include <InterfaceKit.h>

// Needed to handle controller input configuration.
#include "controller_config/cc_window.h"


// Default keymap.
const input_keymap_t input_haiku_keymap_default[8] =
{
	// Player 1
	{{B_UP_ARROW, B_DOWN_ARROW, B_LEFT_ARROW, B_RIGHT_ARROW,
	B_RETURN, 'a', 's', 'd',
	B_RIGHT_SHIFT_KEY, 'q', 'w', 'e'}},
	
	// Player 2
	{{'y', 'h', 'g', 'j',
	'u', 'k', 'l', 'm',
	't', 'i', 'o', 'p'}},
	
	// Players 1B, 1C, 1D
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	
	// Players 2B, 2C, 2D
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
};


// Haiku-specific handler function definitions
int input_haiku_init(void)
{
	return 0;
}

int input_haiku_end(void)
{
	return 0;
}

int input_haiku_update(void)
{
	return 0;
}

BOOL input_haiku_check_key_pressed(uint16_t key)
{
	return true;
}

uint16_t input_haiku_get_key(void)
{
	return (unsigned int)0;
}

BOOL input_haiku_joy_exists(int joy_num)
{
	// We don't have any joystick support in Haiku just yet AFAIK (2009-06-29)
	return false;
}

void input_haiku_event_key_down(int key)
{
	// TODO
	STUB();
}

void input_haiku_event_key_up(int key)
{
	// TODO
	STUB();
}


/**
 * input_haiku_get_key_name(): Get a Haiku key name.
 * @param key Key.
 * @param buf Buffer to store the key name in.
 * @param size Size of the buffer.
 * @return 0 on success; non-zero on error.
 */
int input_haiku_get_key_name(uint16_t key, char* buf, int size)
{
	static const char* const haiku_keys[] =
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

	if (key > (sizeof(haiku_keys) / sizeof(const char*)))
	{
		// Unknown key.
		return -1;
	}

	const char* key_name = haiku_keys[key];
	if (!key_name)
	{
		// Unknown key.
		return -1;
	}
	
	// Known key.
	strlcpy(buf, key_name, size);
	return 0;
}

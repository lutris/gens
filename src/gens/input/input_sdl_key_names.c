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
	const char* key_name;
	
	// TODO: Add more comprehensive SDL key mapping.
	switch (key)
	{
		case SDLK_UP:
			key_name = "Up";
			break;
		case SDLK_DOWN:
			key_name = "Down";
			break;
		case SDLK_LEFT:
			key_name = "Left";
			break;
		case SDLK_RIGHT:
			key_name = "Right";
			break;
		case SDLK_RETURN:
			key_name = "Enter";
			break;
		case SDLK_LSHIFT:
			key_name = "Left Shift";
			break;
		case SDLK_RSHIFT:
			key_name = "Right Shift";
			break;
		default:
			if (key >= 'A' && key <= 'Z')
			{
				buf[0] = key;
				buf[1] = 0x00;
				return 0;
			}
			else if (key >= 'a' && key <= 'z')
			{
				buf[0] = key - ('a' - 'A');
				buf[1] = 0x00;
				return 0;
			}
			
			key_name = NULL;
			break;
	}
	
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

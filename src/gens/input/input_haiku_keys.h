/***************************************************************************
 * Gens: Input Handler - Haiku Key Mappings.                               *
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

#ifndef GENS_INPUT_HAIKU_KEYS_H
#define GENS_INPUT_HAIKU_KEYS_H

#include <InterfaceKit.h>

// Maps key symbols from Haiku to OS-independent key symbols.
#define GENS_KEY_BACKSPACE	B_BACKSPACE
#define GENS_KEY_TAB		B_TAB
#define GENS_KEY_CLEAR		B_FUNCTION_KEY
#define GENS_KEY_RETURN		B_RETURN
#define GENS_KEY_PAUSE		B_FUNCTION_KEY
#define GENS_KEY_ESCAPE		B_ESCAPE
#define GENS_KEY_SPACE		B_SPACE
#define GENS_KEY_EXCLAIM	'!'
#define GENS_KEY_QUOTEDBL	'"'
#define GENS_KEY_HASH		'#'
#define GENS_KEY_DOLLAR		'$'
#define GENS_KEY_AMPERSAND	'&'
#define GENS_KEY_QUOTE		'\''
#define GENS_KEY_LEFTPAREN	'('
#define GENS_KEY_RIGHTPAREN	')'
#define GENS_KEY_ASTERISK	'*'
#define GENS_KEY_PLUS		'+'
#define GENS_KEY_COMMA		','
#define GENS_KEY_MINUS		'-'
#define GENS_KEY_PERIOD		'.'
#define GENS_KEY_SLASH		'/'
#define GENS_KEY_COLON		':'
#define GENS_KEY_SEMICOLON	';'
#define GENS_KEY_LESS		'<'
#define GENS_KEY_EQUALS		'='
#define GENS_KEY_GREATER	'>'
#define GENS_KEY_QUESTION	'?'
#define GENS_KEY_AT		'@'
#define GENS_KEY_LEFTBRACKET	'['
#define GENS_KEY_BACKSLASH	'\\'
#define GENS_KEY_RIGHTBRACKET	']'
#define GENS_KEY_CARET		'^'
#define GENS_KEY_UNDERSCORE	'_'
#define GENS_KEY_BACKQUOTE	'`'

// Numbers
#define GENS_KEY_0		'0'
#define GENS_KEY_1		'1'
#define GENS_KEY_2		'2'
#define GENS_KEY_3		'3'
#define GENS_KEY_4		'4'
#define GENS_KEY_5		'5'
#define GENS_KEY_6		'6'
#define GENS_KEY_7		'7'
#define GENS_KEY_8		'8'
#define GENS_KEY_9		'9'

// Letters
#define	GENS_KEY_a		'A'
#define	GENS_KEY_b		'B'
#define	GENS_KEY_c		'C'
#define	GENS_KEY_d		'D'
#define	GENS_KEY_e		'E'
#define	GENS_KEY_f		'F'
#define	GENS_KEY_g		'G'
#define	GENS_KEY_h		'H'
#define	GENS_KEY_i		'I'
#define	GENS_KEY_j		'J'
#define	GENS_KEY_k		'K'
#define	GENS_KEY_l		'L'
#define	GENS_KEY_m		'M'
#define	GENS_KEY_n		'N'
#define	GENS_KEY_o		'O'
#define	GENS_KEY_p		'P'
#define	GENS_KEY_q		'Q'
#define	GENS_KEY_r		'R'
#define	GENS_KEY_s		'S'
#define	GENS_KEY_t		'T'
#define	GENS_KEY_u		'U'
#define	GENS_KEY_v		'V'
#define	GENS_KEY_w		'W'
#define	GENS_KEY_x		'X'
#define	GENS_KEY_y		'Y'
#define	GENS_KEY_z		'Z'

// Numeric Keypad
#define GENS_KEY_NUM_0		B_INSERT
#define GENS_KEY_NUM_1		B_END
#define GENS_KEY_NUM_2		B_DOWN_ARROW
#define GENS_KEY_NUM_3		B_PAGE_DOWN
#define GENS_KEY_NUM_4		B_LEFT_ARROW
#define GENS_KEY_NUM_5		'5'
#define GENS_KEY_NUM_6		B_RIGHT_ARROW
#define GENS_KEY_NUM_7		B_HOME
#define GENS_KEY_NUM_8		B_UP_ARROW
#define GENS_KEY_NUM_9		B_END
#define GENS_KEY_NUM_PERIOD	B_DELETE
#define GENS_KEY_NUM_DIVIDE	'/'
#define GENS_KEY_NUM_MULTIPLY	'*'
#define GENS_KEY_NUM_MINUS	'-'
#define GENS_KEY_NUM_PLUS	'+'
#define GENS_KEY_NUM_ENTER	B_ENTER
#define GENS_KEY_NUM_EQUALS	'='

// Arrow Keys
#define GENS_KEY_UP		B_UP_ARROW
#define GENS_KEY_DOWN	B_DOWN_ARROW
#define GENS_KEY_LEFT	B_LEFT_ARROW
#define GENS_KEY_RIGHT	B_RIGHT_ARROW

// Home/End section
#define GENS_KEY_INSERT		B_INSERT
#define GENS_KEY_DELETE		B_DELETE
#define GENS_KEY_HOME		B_HOME
#define GENS_KEY_END		B_END
#define GENS_KEY_PAGEUP		B_PAGE_UP
#define GENS_KEY_PAGEDOWN	B_PAGE_DOWN

// Function Keys
#define GENS_KEY_F1		B_F1_KEY
#define GENS_KEY_F2		B_F2_KEY
#define GENS_KEY_F3		B_F3_KEY
#define GENS_KEY_F4		B_F4_KEY
#define GENS_KEY_F5		B_F5_KEY
#define GENS_KEY_F6		B_F6_KEY
#define GENS_KEY_F7		B_F7_KEY
#define GENS_KEY_F8		B_F8_KEY
#define GENS_KEY_F9		B_F9_KEY
#define GENS_KEY_F10	B_F10_KEY
#define GENS_KEY_F11	B_F11_KEY
#define GENS_KEY_F12	B_F12_KEY
#define GENS_KEY_F13	B_F12_KEY
#define GENS_KEY_F14	B_F12_KEY
#define GENS_KEY_F15	B_F12_KEY

// Modifier Keys (not the same as modifiers, which are included with regular keypresses)
#define GENS_KEY_NUMLOCK	B_NUM_LOCK
#define GENS_KEY_CAPSLOCK	B_CAPS_LOCK
#define GENS_KEY_SCROLLLOCK	B_SCROLL_LOCK
#define GENS_KEY_LSHIFT		B_LEFT_SHIFT_KEY
#define GENS_KEY_RSHIFT		B_RIGHT_SHIFT_KEY
#define GENS_KEY_LCTRL		B_LEFT_CONTROL_KEY
#define GENS_KEY_RCTRL		B_RIGHT_CONTROL_KEY
#define GENS_KEY_LALT		B_LEFT_COMMAND_KEY
#define GENS_KEY_RALT		B_RIGHT_COMMAND_KEY
#define GENS_KEY_LMETA		B_LEFT_OPTION_KEY
#define GENS_KEY_RMETA		B_RIGHT_OPTION_KEY
#define GENS_KEY_LSUPER		B_LEFT_OPTION_KEY
#define GENS_KEY_RSUPER		B_RIGHT_OPTION_KEY

// Miscellaneous function keys
#define GENS_KEY_HELP		NULL
#define GENS_KEY_PRINT		B_PRINT_KEY
#define GENS_KEY_MENU		B_MENU_KEY
#define GENS_KEY_POWER		NULL

// Maps key modifiers from SDL to OS-independent key modifiers.
#define GENS_KMOD_NONE		NULL
#define GENS_KMOD_LSHIFT	B_LEFT_SHIFT_KEY
#define GENS_KMOD_RSHIFT	B_RIGHT_SHIFT_KEY
#define GENS_KMOD_LCTRL		B_LEFT_CONTROL_KEY
#define GENS_KMOD_RCTRL		B_RIGHT_CONTROL_KEY
#define GENS_KMOD_LALT		B_LEFT_COMMAND_KEY
#define GENS_KMOD_RALT		B_RIGHT_COMMAND_KEY
#define GENS_KMOD_LMETA		B_LEFT_OPTION_KEY
#define GENS_KMOD_RMETA		B_RIGHT_OPTION_KEY
#define GENS_KMOD_NUM		B_NUM_LOCK
#define GENS_KMOD_CAPS		B_CAPS_LOCK
#define GENS_KMOD_MODE		NULL

// Composite modifiers.
#define GENS_KMOD_CTRL		B_CONTROL_KEY
#define GENS_KMOD_SHIFT		B_SHIFT_KEY
#define GENS_KMOD_ALT		B_COMMAND_KEY
#define GENS_KMOD_META		B_OPTION_KEY

#endif /* GENS_INPUT_HAIKU_KEYS_H */

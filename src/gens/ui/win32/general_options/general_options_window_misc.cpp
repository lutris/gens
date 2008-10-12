/***************************************************************************
 * Gens: (Win32) General Options Window - Miscellaneous Functions.         *
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

#include <string.h>

#include "general_options_window.h"
#include "general_options_window_callbacks.h"
#include "general_options_window_misc.hpp"
#include "gens/gens_window.h"

#include "emulator/g_main.hpp"

#include <windows.h>
#include <windowsx.h>


/**
 * Open_General_Options(): Opens the General Options window.
 */
void Open_General_Options(void)
{
	HWND go = create_general_options_window();
	if (!go)
	{
		// Either an error occurred while creating the General Options window,
		// or the General Options window is already created.
		return;
	}
	
	// TODO: Make the window modal.
	//gtk_window_set_transient_for(GTK_WINDOW(go), GTK_WINDOW(gens_window));
	
	// Get the current options.
	unsigned char curFPSStyle, curMsgStyle;
	
	// System
	Button_SetCheck(go_chkAutoFixChecksum, (Auto_Fix_CS ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkAutoPause, (Auto_Pause ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkFastBlur, (draw->fastBlur() ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkSegaCDLEDs, (Show_LED ? BST_CHECKED : BST_UNCHECKED));
	
	// FPS counter
	curFPSStyle = draw->fpsStyle();
	Button_SetCheck(go_chkMsgEnable[0], (draw->fpsEnabled() ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkMsgDoubleSized[0], ((curFPSStyle & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkMsgTransparency[0], ((curFPSStyle & 0x08) ? BST_CHECKED : BST_UNCHECKED));
	go_stcColor_State[0] = (curFPSStyle & 0x06) >> 1;
	
	// Message
	curMsgStyle = draw->msgStyle();
	Button_SetCheck(go_chkMsgEnable[1], (draw->msgEnabled() ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkMsgDoubleSized[1], ((curMsgStyle & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkMsgTransparency[1], ((curMsgStyle & 0x08) ? BST_CHECKED : BST_UNCHECKED));
	go_stcColor_State[1] = (curMsgStyle & 0x06) >> 1;
	
	// Intro effect color
	go_stcColor_State[2] = draw->introEffectColor();
	
	// Show the General Options window.
	ShowWindow(go, 1);
}


/**
 * General_Options_Save(): Save the General Options.
 */
void General_Options_Save(void)
{
	// Save the current options.
	unsigned char curFPSStyle, curMsgStyle;
	
	// System
	Auto_Fix_CS = (Button_GetCheck(go_chkAutoFixChecksum) == BST_CHECKED);
	Auto_Pause = (Button_GetCheck(go_chkAutoPause) == BST_CHECKED);
	draw->setFastBlur(Button_GetCheck(go_chkFastBlur) == BST_CHECKED);
	Show_LED = (Button_GetCheck(go_chkSegaCDLEDs) == BST_CHECKED);
	
	// FPS counter
	draw->setFPSEnabled(Button_GetCheck(go_chkMsgEnable[0]) == BST_CHECKED);
	curFPSStyle = draw->fpsStyle() & ~0x18;
	curFPSStyle |= ((Button_GetCheck(go_chkMsgDoubleSized[0]) == BST_CHECKED) ? 0x10 : 0x00);
	curFPSStyle |= ((Button_GetCheck(go_chkMsgTransparency[0]) == BST_CHECKED) ? 0x08 : 0x00);
	curFPSStyle &= ~0x06;
	curFPSStyle |= go_stcColor_State[0] << 1;
	draw->setFPSStyle(curFPSStyle);
	
	// Message
	draw->setMsgEnabled(Button_GetCheck(go_chkMsgEnable[1]) == BST_CHECKED);
	curMsgStyle = draw->msgStyle() & ~0x18;
	curMsgStyle |= ((Button_GetCheck(go_chkMsgDoubleSized[1]) == BST_CHECKED) ? 0x10 : 0x00);
	curMsgStyle |= ((Button_GetCheck(go_chkMsgTransparency[1]) == BST_CHECKED) ? 0x08 : 0x00);
	curMsgStyle &= ~0x06;
	curMsgStyle |= go_stcColor_State[1] << 1;
	draw->setMsgStyle(curMsgStyle);
	
	// Intro effect color
	draw->setIntroEffectColor(static_cast<unsigned char>(go_stcColor_State[2]));
}

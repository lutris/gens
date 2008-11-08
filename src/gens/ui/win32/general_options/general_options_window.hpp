/***************************************************************************
 * Gens: (Win32) General Options Window.                                   *
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

#ifndef GENS_WIN32_GENERAL_OPTIONS_WINDOW_H
#define GENS_WIN32_GENERAL_OPTIONS_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

HWND create_general_options_window(void); 
extern HWND general_options_window;
void General_Options_Window_CreateChildWindows(HWND hWnd);

extern HBRUSH go_MsgColors_brushes[4][2];
extern HPEN go_MsgColors_pens[4][2];
extern HBRUSH go_IntroEffectColors_brushes[8][2];
extern HPEN go_IntroEffectColors_pens[8][2];

// Controls
extern HWND go_chkAutoFixChecksum;
extern HWND go_chkAutoPause;
extern HWND go_chkFastBlur;
extern HWND go_chkSegaCDLEDs;

// Index 0: FPS; Index 1: msg
extern HWND go_chkMsgEnable[2];
extern HWND go_chkMsgDoubleSized[2];
extern HWND go_chkMsgTransparency[2];
extern HWND go_stcMsgColor[2][4];
extern HWND go_stcIntroEffectColor[8];
extern unsigned short go_stcColor_State[3];

#ifdef __cplusplus
}
#endif

#endif

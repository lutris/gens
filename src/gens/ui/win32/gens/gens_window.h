/***************************************************************************
 * Gens: (Win32) Main Window.                                              *
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

#ifndef GTK_WIN32_GENS_WINDOW_H
#define GTK_WIN32_GENS_WINDOW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

HWND initGens_hWnd(void);
HWND create_gens_window(void);
extern HWND Gens_hWnd;

extern HMENU MainMenu;
extern HMENU FileMenu;
extern HMENU FileMenu_ROMHistory;
extern HMENU FileMenu_ChangeState;
extern HMENU GraphicsMenu;
extern HMENU GraphicsMenu_Render;
extern HMENU GraphicsMenu_FrameSkip;
extern HMENU CPUMenu;
extern HMENU CPUMenu_Debug;
extern HMENU CPUMenu_Country;
extern HMENU SoundMenu;
extern HMENU SoundMenu_Rate;
extern HMENU OptionsMenu;
extern HMENU OptionsMenu_SegaCDSRAMSize;
extern HMENU HelpMenu;

#ifdef GENS_DEBUGGER
// Debug menu items
//extern GtkWidget *debugMenuItems[9];
//extern GtkWidget *debugSeparators[2];
#endif /* GENS_DEBUGGER */

#ifdef __cplusplus
}
#endif

#endif

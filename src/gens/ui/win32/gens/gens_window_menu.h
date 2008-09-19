/***************************************************************************
 * Gens: (Win32) Main Window - Menu Definitions.                           *
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

#ifndef GTK_WIN32_GENS_WINDOW_MENU_H
#define GTK_WIN32_GENS_WINDOW_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

// Menu identifier & 0xF000 == the menu it's from

// File Menu
#define ID_FILE_MENU			0x1000
#define ID_FILE_OPENROM			0x1001
#define ID_FILE_BOOTCD			0x1002
#define ID_FILE_NETPLAY			0x1003
#define ID_FILE_ROMHISTORY		0x1004
#define ID_FILE_CLOSEROM		0x1005
#define ID_FILE_GAMEGENIE		0x1006
#define ID_FILE_LOADSTATE		0x1007
#define ID_FILE_SAVESTATE		0x1008
#define ID_FILE_QUICKLOAD		0x1009
#define ID_FILE_QUICKSAVE		0x100A
#define ID_FILE_CHANGESTATE		0x100B
#define ID_FILE_QUIT			0x1FFF

// Graphics Menu
#define ID_GRAPHICS_MENU		0x2000
#define ID_GRAPHICS_FULLSCREEN		0x2001
#define ID_GRAPHICS_VSYNC		0x2002
#define ID_GRAPHICS_STRETCH		0x2003
#define ID_GRAPHICS_COLORADJUST		0x2004
#define ID_GRAPHICS_RENDER		0x2005
#define ID_GRAPHICS_SPRITELIMIT		0x2006
#define ID_GRAPHICS_FRAMESKIP		0x2007
#define ID_GRAPHICS_SCREENSHOT		0x2008

#ifdef __cplusplus
}
#endif

#endif

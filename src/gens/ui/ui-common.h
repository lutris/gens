/***************************************************************************
 * Gens: Common UI functions.                                              *
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
 
#ifndef UI_COMMON_H
#define UI_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	AnyFile		= 0,
	ROMFile		= 1,
	SavestateFile	= 2,
	CDImage		= 3,
	ConfigFile	= 4,
	GYMFile		= 5,
} FileFilterType;

// The following functions are implemented by the OS-specific ui-common_*.c file.
void UI_Set_Window_Title(const char* title);
void UI_Set_Window_Visibility (int visibility);

void UI_Hide_Embedded_Window(void);
void UI_Show_Embedded_Window(int w, int h);
int UI_Get_Embedded_WindowID(void);

void UI_MsgBox(const char* msg, const char* title);
int UI_OpenFile(const char* title, const char* initFile, FileFilterType filterType, char* retSelectedFile);
int UI_SaveFile(const char* title, const char* initFile, FileFilterType filterType, char* retSelectedFile);
int UI_SelectDir(const char* title, const char* initDir, char* retSelectedDir);

// The following functions are implemented by ui-common.c.
void UI_Set_Window_Title_Idle(void);
void UI_Set_Window_Title_Game(const char* system, const char* game);
void UI_Set_Window_Title_Init(const char* system, int reinit);


#ifdef __cplusplus
}
#endif

#endif

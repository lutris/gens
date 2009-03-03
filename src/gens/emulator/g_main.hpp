/***************************************************************************
 * Gens: Main Loop.                                                        *
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

#ifndef G_MAIN_HPP
#define G_MAIN_HPP

#include "gens.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// OS-specific includes.
#if defined(GENS_OS_UNIX)
#include "g_main_unix.hpp"
#elif defined(GENS_OS_WIN32)
#include "g_main_win32.hpp"
#else
#error Unsupported operating system.
#endif


#ifdef __cplusplus
extern "C" {
#endif


// Provides uint8_t, uint16_t, uint32_t, etc.
#include <stdint.h>


// Gens settings
typedef struct _Gens_Settings_t
{
	int restrict_input;	// If 1, restructs U+D/L+R.
} Gens_Settings_t;


// Gens Pathnames
typedef struct _Gens_PathNames_t
{
	char Gens_Path[GENS_PATH_MAX];	// TODO: Rename to Gens_Save_Path.
#ifdef GENS_OS_WIN32
	char Gens_EXE_Path[GENS_PATH_MAX];
#endif
	char Start_Rom[GENS_PATH_MAX];
	char Language_Path[GENS_PATH_MAX];
	char Dump_WAV_Dir[GENS_PATH_MAX];
	char Dump_GYM_Dir[GENS_PATH_MAX];
	char Screenshot_Dir[GENS_PATH_MAX];
	
	// TODO: Implement a way to allow plugins to add directories.
	char Patch_Dir[GENS_PATH_MAX];
} Gens_PathNames_t;


// BIOS filenames
typedef struct _Gens_BIOS_Filenames_t
{
	char MD_TMSS[GENS_PATH_MAX];
	char SegaCD_US[GENS_PATH_MAX];
	char MegaCD_EU[GENS_PATH_MAX];
	char MegaCD_JP[GENS_PATH_MAX];
	char _32X_MC68000[GENS_PATH_MAX];
	char _32X_MSH2[GENS_PATH_MAX];
	char _32X_SSH2[GENS_PATH_MAX];
} Gens_BIOS_Filenames_t;


// Miscellaneous filenames
typedef struct _Gens_Misc_Filenames_t
{
	char RAR_Binary[GENS_PATH_MAX];
} Gens_Misc_Filenames_t;


// Video settings
typedef struct _Gens_VideoSettings_t
{
	int VSync_FS;
	int VSync_W;
	int borderColorEmulation;
	int pauseTint;
#ifdef GENS_OPENGL
	int Width_GL;
	int Height_GL;
	int glLinearFilter;
#endif
} Gens_VideoSettings_t;

// bpp settings.
extern uint8_t bppMD;	// MD bpp
extern uint8_t bppOut;	// Output bpp.

extern Gens_Settings_t Settings;
extern Gens_PathNames_t PathNames;
extern Gens_BIOS_Filenames_t BIOS_Filenames;
extern Gens_Misc_Filenames_t Misc_Filenames;
extern Gens_VideoSettings_t Video;

// TODO: Only used for DirectDraw.
extern int Flag_Clr_Scr;

extern int Paused;
extern int Net_Play;
extern int Current_State;
//extern int gl_linear_filter;
extern int Show_LED;
extern int Auto_Pause;
extern int Auto_Fix_CS;
extern int Country;
extern int Country_Order[3];
extern int Kaillera_Client_Running;
extern int Intro_Style;
extern int SegaCD_Accurate;
extern int Active;
extern int Quick_Exit;
extern int FS_Minimised;
extern int fast_forward;

#ifndef GENS_OS_WIN32
typedef struct _POINT
{
	int x;
	int y;
} POINT;
#endif /* GENS_OS_WIN32 */
extern POINT Window_Pos;

extern char **language_name;

extern unsigned char Keys[];
extern unsigned char joystate[];

// Stupid temporary string needed for some stuff.
extern char Str_Tmp[GENS_PATH_MAX];

int Change_68K_Type(int hWnd, int Num, int Reset_SND);
//extern void Read_To_68K_Space(int adr);

void End_All(void);
int is_gens_running(void);
void close_gens(void);

// Gens Rerecording
int IsAsyncAllowed(void);

// Initialization functions.
int Init(void);
int Init_Settings(void);

extern int ice;

// Update Frame function pointers
extern int (*Update_Frame)(void);
extern int (*Update_Frame_Fast)(void);

// Miscellaneous.
void Clear_Screen_MD(void);

// The main program loop.
void GensMainLoop(void);

// Get the current mdp_render_t*.
#include "mdp/mdp_render.h"
mdp_render_t* get_mdp_render_t(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// C++ classes

// Renderers
#include "plugins/pluginmgr.hpp"
extern std::list<mdp_render_t*>::iterator rendMode_FS;
extern std::list<mdp_render_t*>::iterator rendMode_W;

#endif

#endif /* G_MAIN_HPP */

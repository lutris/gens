/***************************************************************************
 * Gens: Main Menu definitions.                                            *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "gens_menu.h"

#include <unistd.h>
#ifndef NULL
#define NULL ((void*)0)
#endif


/** Main Menu **/


static struct GensMenuItem_t gmiFile[];
static struct GensMenuItem_t gmiGraphics[];
static struct GensMenuItem_t gmiCPU[];
static struct GensMenuItem_t gmiSound[];
static struct GensMenuItem_t gmiOptions[];
static struct GensMenuItem_t gmiHelp[];

struct GensMenuItem_t gmiMain[] =
{
	{IDM_FILE_MENU,		GMF_ITEM_SUBMENU,	"&File",		&gmiFile[0],		0, 0, NULL},
	{IDM_GRAPHICS_MENU,	GMF_ITEM_SUBMENU,	"&Graphics",		&gmiGraphics[0],	0, 0, NULL},
	{IDM_CPU_MENU,		GMF_ITEM_SUBMENU,	"&CPU",			&gmiCPU[0],		0, 0, NULL},
	{IDM_SOUND_MENU,	GMF_ITEM_SUBMENU,	"&Sound",		&gmiSound[0],		0, 0, NULL},
	{IDM_OPTIONS_MENU,	GMF_ITEM_SUBMENU,	"&Options",		&gmiOptions[0],		0, 0, NULL},
	{IDM_HELP_MENU,		GMF_ITEM_SUBMENU,	"&Help",		&gmiHelp[0],		0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};


/** File Menu **/


static struct GensMenuItem_t gmiFile_ChgState[];

static struct GensMenuItem_t gmiFile[] =
{
	{IDM_FILE_OPENROM,	GMF_ICON_STOCK,		"&Open ROM...",		NULL,	GMAM_CTRL, 'O', "gtk-open"},
	{IDM_FILE_BOOTCD,	GMF_ICON_STOCK,		"&Boot CD",		NULL,	GMAM_CTRL, 'B', "gtk-cdrom"},
	{IDM_FILE_NETPLAY,	GMF_ICON_FILE,		"&Netplay",		NULL,	0, 0, "modem.png"},
	{IDM_FILE_ROMHISTORY,	GMF_ICON_FILE,		"ROM &History",		NULL,	0, 0, "history.png"},
	{IDM_FILE_CLOSEROM,	GMF_ICON_STOCK,		"&Close ROM",		NULL,	GMAM_CTRL, 'W', "gtk-close"},
	{IDM_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL,	0, 0, NULL},
	{IDM_FILE_GAMEGENIE,	GMF_ICON_FILE,		"&Game Genie",		NULL,	GMAM_CTRL, 'G', "password.png"},
	{IDM_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL,	0, 0, NULL},
	{IDM_FILE_LOADSTATE,	GMF_ICON_STOCK,		"&Load State...",	NULL,	GMAM_SHIFT, GMAK_F8, "gtk-open"},
	{IDM_FILE_SAVESTATE,	GMF_ICON_STOCK,		"&Save State As...",	NULL,	GMAM_SHIFT, GMAK_F5, "gtk-save-as"},
	{IDM_FILE_QUICKLOAD,	GMF_ICON_STOCK,		"Quick Load",		NULL,	0, GMAK_F8, "gtk-refresh"},
	{IDM_FILE_QUICKSAVE,	GMF_ICON_STOCK,		"Quick Save",		NULL,	0, GMAK_F5, "gtk-save"},
	{IDM_FILE_CHANGESTATE,	GMF_ICON_STOCK | GMF_ITEM_SUBMENU, "Change State", &gmiFile_ChgState[0], 0, 0, "gtk-revert-to-saved"},
	{IDM_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL,	0, 0, NULL},
#ifdef GENS_OS_WIN32
	{IDM_FILE_EXIT,		GMF_ICON_STOCK,		"E&xit",		NULL,	GMAM_CTRL, 'Q', "gtk-quit"},
#else /* !GENS_OS_WIN32 */
 	{IDM_FILE_QUIT,		GMF_ICON_STOCK,		"&Quit",		NULL,	GMAM_CTRL, 'Q', "gtk-quit"},
#endif /* GENS_OS_WIN32 */
	{0, 0, NULL, NULL, NULL}
};

static struct GensMenuItem_t gmiFile_ChgState[] =
{
	{IDM_FILE_CHANGESTATE_0, GMF_ITEM_RADIO, "0", NULL, 0, 0, NULL},
	{IDM_FILE_CHANGESTATE_1, GMF_ITEM_RADIO, "1", NULL, 0, 0, NULL},
	{IDM_FILE_CHANGESTATE_2, GMF_ITEM_RADIO, "2", NULL, 0, 0, NULL},
	{IDM_FILE_CHANGESTATE_3, GMF_ITEM_RADIO, "3", NULL, 0, 0, NULL},
	{IDM_FILE_CHANGESTATE_4, GMF_ITEM_RADIO, "4", NULL, 0, 0, NULL},
	{IDM_FILE_CHANGESTATE_5, GMF_ITEM_RADIO, "5", NULL, 0, 0, NULL},
	{IDM_FILE_CHANGESTATE_6, GMF_ITEM_RADIO, "6", NULL, 0, 0, NULL},
	{IDM_FILE_CHANGESTATE_7, GMF_ITEM_RADIO, "7", NULL, 0, 0, NULL},
	{IDM_FILE_CHANGESTATE_8, GMF_ITEM_RADIO, "8", NULL, 0, 0, NULL},
	{IDM_FILE_CHANGESTATE_9, GMF_ITEM_RADIO, "9", NULL, 0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};


/** Graphics Menu **/


#ifdef GENS_OPENGL
static struct GensMenuItem_t gmiGraphics_GLRes[];
#endif
#ifdef GENS_OS_UNIX
static struct GensMenuItem_t gmiGraphics_bpp[];
#endif
static struct GensMenuItem_t gmiGraphics_FrameSkip[];

static struct GensMenuItem_t gmiGraphics[] =
{
	{IDM_GRAPHICS_FULLSCREEN,	GMF_ICON_STOCK,		"&Full Screen",			NULL,	GMAM_ALT, GMAK_ENTER, "gtk-fullscreen"},
	{IDM_GRAPHICS_VSYNC,		GMF_ITEM_CHECK,		"&VSync",			NULL,	GMAM_SHIFT, GMAK_F3, NULL},
	{IDM_GRAPHICS_STRETCH,		GMF_ITEM_CHECK,		"&Stretch",			NULL,	GMAM_SHIFT, GMAK_F2, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, NULL},
#ifdef GENS_OPENGL
	{IDM_GRAPHICS_OPENGL,		GMF_ITEM_CHECK,		"Open&GL",			NULL,	GMAM_SHIFT, 'R', NULL},
	{IDM_GRAPHICS_OPENGL_FILTER,	GMF_ITEM_CHECK,		"OpenGL &Linear Filter",	NULL,	0, 0, NULL},
	{IDM_GRAPHICS_OPENGL_RES,	GMF_ITEM_SUBMENU,	"OpenGL Resolution",		&gmiGraphics_GLRes[0],	0, 0, NULL},
#ifndef GENS_OS_UNIX
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, NULL},
#endif /* !GENS_OS_UNIX */
#endif /* GENS_OPENGL */
#ifdef GENS_OS_UNIX
	{IDM_GRAPHICS_BPP,		GMF_ITEM_SUBMENU,	"Bits per pixel",		&gmiGraphics_bpp[0],	0, 0, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, NULL},
#endif /* GENS_OS_UNIX */
	{IDM_GRAPHICS_COLORADJUST,	GMF_ICON_STOCK,		"Color Adjust...",		NULL,	0, 0, "gtk-select-color"},
	{IDM_GRAPHICS_RENDER,		GMF_ICON_FILE,		"&Render",			NULL,	0, 0, "viewmag.png"},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, NULL},
	{IDM_GRAPHICS_SPRITELIMIT,	GMF_ITEM_CHECK,		"Sprite Limit",			NULL,	0, 0, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, NULL},
	{IDM_GRAPHICS_FRAMESKIP,	GMF_ICON_FILE | GMF_ITEM_SUBMENU, "Frame Skip",		&gmiGraphics_FrameSkip[0], 0, 0, "2rightarrow.png"},
	{IDM_GRAPHICS_SCREENSHOT,	GMF_ICON_STOCK,		"Screen Shot",			NULL,	GMAM_SHIFT, GMAK_BACKSPACE, "gtk-copy"},
	{0, 0, NULL, NULL, NULL}
};

#ifdef GENS_OPENGL
static struct GensMenuItem_t gmiGraphics_GLRes[] =
{
	{IDM_GRAPHICS_OPENGL_RES_320,		GMF_ITEM_RADIO,		"320x240",	NULL,	0, 0, NULL},
	{IDM_GRAPHICS_OPENGL_RES_640,		GMF_ITEM_RADIO,		"640x480",	NULL,	0, 0, NULL},
	{IDM_GRAPHICS_OPENGL_RES_800,		GMF_ITEM_RADIO,		"800x600",	NULL,	0, 0, NULL},
	{IDM_GRAPHICS_OPENGL_RES_1024,		GMF_ITEM_RADIO,		"1024x768",	NULL,	0, 0, NULL},
	{IDM_GRAPHICS_OPENGL_RES_CUSTOM,	GMF_ITEM_RADIO,		"Custom",	NULL,	0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};
#endif

#ifdef GENS_OS_UNIX
static struct GensMenuItem_t gmiGraphics_bpp[] =
{
	{IDM_GRAPHICS_BPP_15,	GMF_ITEM_RADIO,		"15 (555)",	NULL,	0, 0, NULL},
	{IDM_GRAPHICS_BPP_16,	GMF_ITEM_RADIO,		"16 (555)",	NULL,	0, 0, NULL},
	{IDM_GRAPHICS_BPP_32,	GMF_ITEM_RADIO,		"32",		NULL,	0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};
#endif

static struct GensMenuItem_t gmiGraphics_FrameSkip[] =
{
	{IDM_GRAPHICS_FRAMESKIP_AUTO,	GMF_ITEM_RADIO,		"Auto",		NULL,	0, 0, NULL},
	{IDM_GRAPHICS_FRAMESKIP_0,	GMF_ITEM_RADIO,		"0",		NULL,	0, 0, NULL},
	{IDM_GRAPHICS_FRAMESKIP_1,	GMF_ITEM_RADIO,		"1",		NULL,	0, 0, NULL},
	{IDM_GRAPHICS_FRAMESKIP_2,	GMF_ITEM_RADIO,		"2",		NULL,	0, 0, NULL},
	{IDM_GRAPHICS_FRAMESKIP_3,	GMF_ITEM_RADIO,		"3",		NULL,	0, 0, NULL},
	{IDM_GRAPHICS_FRAMESKIP_4,	GMF_ITEM_RADIO,		"4",		NULL,	0, 0, NULL},
	{IDM_GRAPHICS_FRAMESKIP_5,	GMF_ITEM_RADIO,		"5",		NULL,	0, 0, NULL},
	{IDM_GRAPHICS_FRAMESKIP_6,	GMF_ITEM_RADIO,		"6",		NULL,	0, 0, NULL},
	{IDM_GRAPHICS_FRAMESKIP_7,	GMF_ITEM_RADIO,		"7",		NULL,	0, 0, NULL},
	{IDM_GRAPHICS_FRAMESKIP_8,	GMF_ITEM_RADIO,		"8",		NULL,	0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};


/** CPU Menu **/


#ifdef GENS_DEBUGGER
static struct GensMenuItem_t gmiCPU_Debug[];
#endif /* GENS_DEBUGGER */
static struct GensMenuItem_t gmiCPU_Country[];

static struct GensMenuItem_t gmiCPU[] =
{
#ifdef GENS_DEBUGGER
	{IDM_CPU_DEBUG,			GMF_ITEM_SUBMENU,	"&Debug",			&gmiCPU_Debug[0], 0, 0, NULL},
#endif /* GENS_DEBUGGER */
	{IDM_CPU_COUNTRY,		GMF_ITEM_SUBMENU,	"&Country",			&gmiCPU_Country[0], 0, 0, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, NULL},
	{IDM_CPU_HARDRESET,		GMF_ICON_STOCK,		"Hard &Reset",			NULL,	0, GMAK_TAB, "gtk-refresh"},
	{IDM_CPU_RESET68K,		GMF_ITEM_NORMAL,	"Reset 68000",			NULL,	0, 0, NULL},
	{IDM_CPU_RESETMAIN68K,		GMF_ITEM_NORMAL,	"Reset Main 68000",		NULL,	0, 0, NULL},
	{IDM_CPU_RESETSUB68K,		GMF_ITEM_NORMAL,	"Reset Sub 68000",		NULL,	0, 0, NULL},
	{IDM_CPU_RESETMAINSH2,		GMF_ITEM_NORMAL,	"Reset Main SH2",		NULL,	0, 0, NULL},
	{IDM_CPU_RESETSUBSH2,		GMF_ITEM_NORMAL,	"Reset Sub SH2",		NULL,	0, 0, NULL},
	{IDM_CPU_RESETZ80,		GMF_ITEM_NORMAL,	"Reset Z80",			NULL,	0, 0, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	0, 0, NULL},
	{IDM_CPU_SEGACDPERFECTSYNC,	GMF_ITEM_CHECK,		"SegaCD Perfect Sync (SLOW)",	NULL,	0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};

#ifdef GENS_DEBUGGER
static struct GensMenuItem_t gmiCPU_Debug[] =
{
	{IDM_CPU_DEBUG_MC68000,			GMF_ITEM_CHECK,		"&Genesis - 680000",	NULL, 0, 0, NULL},
	{IDM_CPU_DEBUG_Z80,			GMF_ITEM_CHECK,		"Genesis - &Z80",	NULL, 0, 0, NULL},
	{IDM_CPU_DEBUG_VDP,			GMF_ITEM_CHECK,		"Genesis - &VDP",	NULL, 0, 0, NULL},
	{IDM_CPU_DEBUG_SEGACD_SEPARATOR,	GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, NULL},
	{IDM_CPU_DEBUG_SEGACD_MC68000,		GMF_ITEM_CHECK,		"&SegaCD - 68000",	NULL, 0, 0, NULL},
	{IDM_CPU_DEBUG_SEGACD_CDC,		GMF_ITEM_CHECK,		"SegaCD - &CDC",	NULL, 0, 0, NULL},
	{IDM_CPU_DEBUG_SEGACD_GFX,		GMF_ITEM_CHECK,		"SegaCD - GF&X",	NULL, 0, 0, NULL},
	{IDM_CPU_DEBUG_32X_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, NULL},
	{IDM_CPU_DEBUG_32X_MSH2,		GMF_ITEM_CHECK,		"32X - Main SH2",	NULL, 0, 0, NULL},
	{IDM_CPU_DEBUG_32X_SSH2,		GMF_ITEM_CHECK,		"32X - Sub SH2",	NULL, 0, 0, NULL},
	{IDM_CPU_DEBUG_32X_VDP,			GMF_ITEM_CHECK,		"32X - VDP",		NULL, 0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};
#endif /* GENS_DEBUGGER */

static struct GensMenuItem_t gmiCPU_Country[] =
{
	{IDM_CPU_COUNTRY_AUTO,		GMF_ITEM_RADIO,		"Auto Detect",		NULL, 0, 0, NULL},
	{IDM_CPU_COUNTRY_JAPAN_NTSC,	GMF_ITEM_RADIO,		"Japan (NTSC)",		NULL, 0, 0, NULL},
	{IDM_CPU_COUNTRY_USA,		GMF_ITEM_RADIO,		"USA (NTSC)",		NULL, 0, 0, NULL},
	{IDM_CPU_COUNTRY_EUROPE,	GMF_ITEM_RADIO,		"Europe (PAL)",		NULL, 0, 0, NULL},
	{IDM_CPU_COUNTRY_JAPAN_PAL,	GMF_ITEM_RADIO,		"Japan (PAL)",		NULL, 0, 0, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, NULL},
	{IDM_CPU_COUNTRY_ORDER,		GMF_ITEM_NORMAL,	"Auto-Detection Order...", NULL, 0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};


/** Sound Menu **/


static struct GensMenuItem_t gmiSound_Rate[];

static struct GensMenuItem_t gmiSound[] =
{
	{IDM_SOUND_ENABLE,		GMF_ITEM_CHECK,		"&Enable",		NULL, 0, 0, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, NULL},
	{IDM_SOUND_RATE,		GMF_ITEM_SUBMENU,	"&Rate",		&gmiSound_Rate[0], 0, 0, NULL},
	{IDM_SOUND_STEREO,		GMF_ITEM_CHECK,		"&Stereo",		NULL, 0, 0, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, NULL},
	{IDM_SOUND_Z80,			GMF_ITEM_CHECK,		"&Z80",			NULL, 0, 0, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, NULL},
	{IDM_SOUND_YM2612,		GMF_ITEM_CHECK,		"&YM2612",		NULL, 0, 0, NULL},
	{IDM_SOUND_YM2612_IMPROVED,	GMF_ITEM_CHECK,		"YM2612 Improved",	NULL, 0, 0, NULL},
	{IDM_SOUND_DAC,			GMF_ITEM_CHECK,		"&DAC",			NULL, 0, 0, NULL},
	{IDM_SOUND_DAC_IMPROVED,	GMF_ITEM_CHECK,		"DAC Improved",		NULL, 0, 0, NULL},
	{IDM_SOUND_PSG,			GMF_ITEM_CHECK,		"&PSG",			NULL, 0, 0, NULL},
	{IDM_SOUND_PSG_IMPROVED,	GMF_ITEM_CHECK,		"PSG Improved",		NULL, 0, 0, NULL},
	{IDM_SOUND_PCM,			GMF_ITEM_CHECK,		"P&CM",			NULL, 0, 0, NULL},
	{IDM_SOUND_PWM,			GMF_ITEM_CHECK,		"P&WM",			NULL, 0, 0, NULL},
	{IDM_SOUND_CDDA,		GMF_ITEM_CHECK,		"CDDA (CD Audio)",	NULL, 0, 0, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, NULL},
	{IDM_SOUND_WAVDUMP,		GMF_ITEM_NORMAL,	"Start WAV Dump",	NULL, 0, 0, NULL},
	{IDM_SOUND_GYMDUMP,		GMF_ITEM_NORMAL,	"Start GYM Dump",	NULL, 0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};

static struct GensMenuItem_t gmiSound_Rate[] =
{
	{IDM_SOUND_RATE_11025,		GMF_ITEM_RADIO,		"11,025 Hz",		NULL, 0, 0, NULL},
	{IDM_SOUND_RATE_16000,		GMF_ITEM_RADIO,		"16,000 Hz",		NULL, 0, 0, NULL},
	{IDM_SOUND_RATE_22050,		GMF_ITEM_RADIO,		"22,050 Hz",		NULL, 0, 0, NULL},
	{IDM_SOUND_RATE_24000,		GMF_ITEM_RADIO,		"24,000 Hz",		NULL, 0, 0, NULL},
	{IDM_SOUND_RATE_44100,		GMF_ITEM_RADIO,		"44,100 Hz",		NULL, 0, 0, NULL},
	{IDM_SOUND_RATE_48000,		GMF_ITEM_RADIO,		"48,000 Hz",		NULL, 0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};


/** Options Menu **/


static struct GensMenuItem_t gmiOptions_SegaCDSRAMSize[];

static struct GensMenuItem_t gmiOptions[] =
{
	{IDM_OPTIONS_GENERAL,		GMF_ICON_FILE,		"&General Options...",	NULL, 0, 0, "ksysguard.png"},
	{IDM_OPTIONS_JOYPADS,		GMF_ICON_FILE,		"&Joypads...",		NULL, 0, 0, "package_games.png"},
	{IDM_OPTIONS_DIRECTORIES,	GMF_ICON_FILE,		"&Directories...",	NULL, 0, 0, "folder_slin_open.png"},
	{IDM_OPTIONS_BIOSMISCFILES,	GMF_ICON_FILE,		"&BIOS/Misc Files...",	NULL, 0, 0, "binary.png"},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, NULL},
#ifdef GENS_CDROM
	{IDM_OPTIONS_CURRENT_CD_DRIVE,	GMF_ICON_STOCK,		"Current &CD Drive...",	NULL, 0, 0, "gtk-cdrom"},
#endif /* GENS_CDROM */
	{IDM_OPTIONS_SEGACDSRAMSIZE,	GMF_ICON_FILE | GMF_ITEM_SUBMENU, "Sega CD S&RAM Size", &gmiOptions_SegaCDSRAMSize[0], 0, 0, "memory.png"},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, 0, 0, NULL},
	{IDM_OPTIONS_LOADCONFIG,	GMF_ICON_STOCK,		"&Load Config...",	NULL, 0, 0, "gtk-open"},
	{IDM_OPTIONS_SAVECONFIGAS,	GMF_ICON_STOCK,		"&Save Config As...",	NULL, 0, 0, "gtk-save-as"},
	{0, 0, NULL, NULL, NULL}
};

static struct GensMenuItem_t gmiOptions_SegaCDSRAMSize[] =
{
	{IDM_OPTIONS_SEGACDSRAMSIZE_NONE,	GMF_ITEM_RADIO, "None",  NULL, 0, 0, NULL},
	{IDM_OPTIONS_SEGACDSRAMSIZE_8KB,	GMF_ITEM_RADIO, "8 KB",  NULL, 0, 0, NULL},
	{IDM_OPTIONS_SEGACDSRAMSIZE_16KB,	GMF_ITEM_RADIO, "16 KB", NULL, 0, 0, NULL},
	{IDM_OPTIONS_SEGACDSRAMSIZE_32KB,	GMF_ITEM_RADIO, "32 KB", NULL, 0, 0, NULL},
	{IDM_OPTIONS_SEGACDSRAMSIZE_64KB,	GMF_ITEM_RADIO, "64 KB", NULL, 0, 0, NULL},
	{0, 0, NULL, NULL, NULL}
};


/** Help Menu **/


static struct GensMenuItem_t gmiHelp[] =
{
	{IDM_HELP_ABOUT,	GMF_ICON_STOCK,		"&About",	NULL,	0, 0, "gtk-help"},
	{0, 0, NULL, NULL, NULL}
};

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
	{IDM_FILE_MENU,		GMF_ITEM_SUBMENU,	"&File",		&gmiFile[0],		NULL},
	{IDM_GRAPHICS_MENU,	GMF_ITEM_SUBMENU,	"&Graphics",		&gmiGraphics[0],	NULL},
	{IDM_CPU_MENU,		GMF_ITEM_SUBMENU,	"&CPU",			&gmiCPU[0],		NULL},
	{IDM_SOUND_MENU,	GMF_ITEM_SUBMENU,	"&Sound",		&gmiSound[0],		NULL},
	{IDM_OPTIONS_MENU,	GMF_ITEM_SUBMENU,	"&Options",		&gmiOptions[0],		NULL},
	{IDM_HELP_MENU,		GMF_ITEM_SUBMENU,	"&Help",		&gmiHelp[0],		NULL},
	{0, 0, NULL, NULL, NULL}
};


/** File Menu **/


static struct GensMenuItem_t gmiFile_ChgState[];

static struct GensMenuItem_t gmiFile[] =
{
	{IDM_FILE_OPENROM,	GMF_ICON_STOCK,		"&Open ROM...",		NULL,			"gtk-open"},
	{IDM_FILE_BOOTCD,	GMF_ICON_STOCK,		"&Boot CD",		NULL,			"gtk-cdrom"},
	{IDM_FILE_NETPLAY,	GMF_ICON_FILE,		"&Netplay",		NULL,			"modem.png"},
	{IDM_FILE_ROMHISTORY,	GMF_ICON_FILE,		"ROM &History",		NULL,			"history.png"},
	{IDM_FILE_CLOSEROM,	GMF_ICON_STOCK,		"&Close ROM",		NULL,			"gtk-close"},
	{IDM_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL,			NULL},
	{IDM_FILE_GAMEGENIE,	GMF_ICON_FILE,		"&Game Genie",		NULL,			"password.png"},
	{IDM_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL,			NULL},
	{IDM_FILE_LOADSTATE,	GMF_ICON_STOCK,		"&Load State...",	NULL,			"gtk-open"},
	{IDM_FILE_SAVESTATE,	GMF_ICON_STOCK,		"&Save State As...",	NULL,			"gtk-save-as"},
	{IDM_FILE_QUICKLOAD,	GMF_ICON_STOCK,		"Quick Load",		NULL,			"gtk-refresh"},
	{IDM_FILE_QUICKSAVE,	GMF_ICON_STOCK,		"Quick Save",		NULL,			"gtk-save"},
	{IDM_FILE_CHANGESTATE,	GMF_ICON_STOCK,		"Change State",		&gmiFile_ChgState[0],	"gtk-revert-to-saved"},
	{IDM_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL,			NULL},
#ifdef GENS_OS_WIN32
	{IDM_FILE_EXIT,		GMF_ICON_STOCK,		"E&xit",		NULL,			"gtk-quit"},
#else /* !GENS_OS_WIN32 */
	{IDM_FILE_QUIT,		GMF_ICON_STOCK,		"&Quit",		NULL,			"gtk-quit"},
#endif /* GENS_OS_WIN32 */
	{0, 0, NULL, NULL, NULL}
};

static struct GensMenuItem_t gmiFile_ChgState[] =
{
	{IDM_FILE_CHANGESTATE_0, 0, "0", NULL, NULL},
	{IDM_FILE_CHANGESTATE_1, 0, "1", NULL, NULL},
	{IDM_FILE_CHANGESTATE_2, 0, "2", NULL, NULL},
	{IDM_FILE_CHANGESTATE_3, 0, "3", NULL, NULL},
	{IDM_FILE_CHANGESTATE_4, 0, "4", NULL, NULL},
	{IDM_FILE_CHANGESTATE_5, 0, "5", NULL, NULL},
	{IDM_FILE_CHANGESTATE_6, 0, "6", NULL, NULL},
	{IDM_FILE_CHANGESTATE_7, 0, "7", NULL, NULL},
	{IDM_FILE_CHANGESTATE_8, 0, "8", NULL, NULL},
	{IDM_FILE_CHANGESTATE_9, 0, "9", NULL, NULL},
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
	{IDM_GRAPHICS_FULLSCREEN,	GMF_ICON_STOCK,		"&Full Screen",			NULL,	"gtk-fullscreen"},
	{IDM_GRAPHICS_VSYNC,		GMF_ITEM_CHECK,		"&VSync",			NULL,	NULL},
	{IDM_GRAPHICS_STRETCH,		GMF_ITEM_CHECK,		"&Stretch",			NULL,	NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	NULL},
#ifdef GENS_OPENGL
	{IDM_GRAPHICS_OPENGL,		GMF_ITEM_CHECK,		"Open&GL",			NULL,	NULL},
	{IDM_GRAPHICS_OPENGL_FILTER,	GMF_ITEM_CHECK,		"OpenGL &Linear Filter",	NULL,	NULL},
	{IDM_GRAPHICS_OPENGL_RES,	GMF_ITEM_SUBMENU,	"OpenGL Resolution",		&gmiGraphics_GLRes[0],	NULL},
#ifndef GENS_OS_UNIX
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	NULL},
#endif /* !GENS_OS_UNIX */
#endif /* GENS_OPENGL */
#ifdef GENS_OS_UNIX
	{IDM_GRAPHICS_BPP,		GMF_ITEM_SUBMENU,	"Bits per pixel",		&gmiGraphics_bpp[0],	NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	NULL},
#endif /* GENS_OS_UNIX */
	{IDM_GRAPHICS_COLORADJUST,	GMF_ICON_STOCK,		"Color Adjust...",		NULL,	"gtk-select-color"},
	{IDM_GRAPHICS_RENDER,		GMF_ICON_FILE,		"&Render",			NULL,	"viewmag.png"},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	NULL},
	{IDM_GRAPHICS_SPRITELIMIT,	GMF_ITEM_CHECK,		"Sprite Limit",			NULL,	NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	NULL},
	{IDM_GRAPHICS_FRAMESKIP,	GMF_ICON_FILE | GMF_ITEM_SUBMENU, "Frame Skip",		&gmiGraphics_FrameSkip[0], "2rightarrow.png"},
	{IDM_GRAPHICS_SCREENSHOT,	GMF_ICON_STOCK,		"Screen Shot",			NULL,	"gtk-copy"},
	{0, 0, NULL, NULL, NULL}
};

#ifdef GENS_OPENGL
static struct GensMenuItem_t gmiGraphics_GLRes[] =
{
	{IDM_GRAPHICS_OPENGL_RES_320,		GMF_ITEM_RADIO,		"320x240",	NULL,	NULL},
	{IDM_GRAPHICS_OPENGL_RES_640,		GMF_ITEM_RADIO,		"640x240",	NULL,	NULL},
	{IDM_GRAPHICS_OPENGL_RES_800,		GMF_ITEM_RADIO,		"800x240",	NULL,	NULL},
	{IDM_GRAPHICS_OPENGL_RES_1024,		GMF_ITEM_RADIO,		"1024x240",	NULL,	NULL},
	{IDM_GRAPHICS_OPENGL_RES_CUSTOM,	GMF_ITEM_RADIO,		"Custom",	NULL,	NULL},
	{0, 0, NULL, NULL, NULL}
};
#endif

#ifdef GENS_OS_UNIX
static struct GensMenuItem_t gmiGraphics_bpp[] =
{
	{IDM_GRAPHICS_BPP_15,	GMF_ITEM_RADIO,		"15 (555)",	NULL,	NULL},
	{IDM_GRAPHICS_BPP_16,	GMF_ITEM_RADIO,		"16 (555)",	NULL,	NULL},
	{IDM_GRAPHICS_BPP_32,	GMF_ITEM_RADIO,		"32)",		NULL,	NULL},
	{0, 0, NULL, NULL, NULL}
};
#endif

static struct GensMenuItem_t gmiGraphics_FrameSkip[] =
{
	{IDM_GRAPHICS_FRAMESKIP_AUTO,	GMF_ITEM_RADIO,		"Auto",		NULL,	NULL},
	{IDM_GRAPHICS_FRAMESKIP_0,	GMF_ITEM_RADIO,		"0",		NULL,	NULL},
	{IDM_GRAPHICS_FRAMESKIP_1,	GMF_ITEM_RADIO,		"1",		NULL,	NULL},
	{IDM_GRAPHICS_FRAMESKIP_2,	GMF_ITEM_RADIO,		"2",		NULL,	NULL},
	{IDM_GRAPHICS_FRAMESKIP_3,	GMF_ITEM_RADIO,		"3",		NULL,	NULL},
	{IDM_GRAPHICS_FRAMESKIP_4,	GMF_ITEM_RADIO,		"4",		NULL,	NULL},
	{IDM_GRAPHICS_FRAMESKIP_5,	GMF_ITEM_RADIO,		"5",		NULL,	NULL},
	{IDM_GRAPHICS_FRAMESKIP_6,	GMF_ITEM_RADIO,		"6",		NULL,	NULL},
	{IDM_GRAPHICS_FRAMESKIP_7,	GMF_ITEM_RADIO,		"7",		NULL,	NULL},
	{IDM_GRAPHICS_FRAMESKIP_8,	GMF_ITEM_RADIO,		"8",		NULL,	NULL},
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
	{IDM_CPU_DEBUG,			GMF_ITEM_SUBMENU,	"&Debug",			&gmiCPU_Debug[0], NULL},
#endif /* GENS_DEBUGGER */
	{IDM_CPU_COUNTRY,		GMF_ITEM_SUBMENU,	"&Country",			&gmiCPU_Country[0], NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	NULL},
	{IDM_CPU_HARDRESET,		GMF_ICON_STOCK,		"Hard &Reset",			NULL,	"gtk-refresh"},
	{IDM_CPU_RESET68K,		GMF_ITEM_NORMAL,	"Reset 68000",			NULL,	NULL},
	{IDM_CPU_RESETMAIN68K,		GMF_ITEM_NORMAL,	"Reset Main 68000",		NULL,	NULL},
	{IDM_CPU_RESETSUB68K,		GMF_ITEM_NORMAL,	"Reset Sub 68000",		NULL,	NULL},
	{IDM_CPU_RESETMAINSH2,		GMF_ITEM_NORMAL,	"Reset Main SH2",		NULL,	NULL},
	{IDM_CPU_RESETSUBSH2,		GMF_ITEM_NORMAL,	"Reset Sub SH2",		NULL,	NULL},
	{IDM_CPU_RESETZ80,		GMF_ITEM_NORMAL,	"Reset Z80",			NULL,	NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,				NULL,	NULL},
	{IDM_CPU_SEGACDPERFECTSYNC,	GMF_ITEM_CHECK,		"SegaCD Perfect Sync (SLOW)",	NULL,	NULL},
	{0, 0, NULL, NULL, NULL}
};

#ifdef GENS_DEBUGGER
static struct GensMenuItem_t gmiCPU_Debug[] =
{
	{IDM_CPU_DEBUG_MC68000,			GMF_ITEM_CHECK,		"&Genesis - 680000",	NULL, NULL},
	{IDM_CPU_DEBUG_Z80,			GMF_ITEM_CHECK,		"Genesis - &Z80",	NULL, NULL},
	{IDM_CPU_DEBUG_VDP,			GMF_ITEM_CHECK,		"Genesis - &VDP",	NULL, NULL},
	{IDM_CPU_DEBUG_SEGACD_SEPARATOR,	GMF_ITEM_SEPARATOR,	NULL,			NULL, NULL},
	{IDM_CPU_DEBUG_SEGACD_MC68000,		GMF_ITEM_CHECK,		"&SegaCD - 68000",	NULL, NULL},
	{IDM_CPU_DEBUG_SEGACD_CDC,		GMF_ITEM_CHECK,		"SegaCD - &CDC",	NULL, NULL},
	{IDM_CPU_DEBUG_SEGACD_GFX,		GMF_ITEM_CHECK,		"SegaCD - GF&X",	NULL, NULL},
	{IDM_CPU_DEBUG_32X_SEPARATOR,		GMF_ITEM_SEPARATOR,	NULL,			NULL, NULL},
	{IDM_CPU_DEBUG_32X_MSH2,		GMF_ITEM_CHECK,		"32X - Main SH2",	NULL, NULL},
	{IDM_CPU_DEBUG_32X_SSH2,		GMF_ITEM_CHECK,		"32X - Sub SH2",	NULL, NULL},
	{IDM_CPU_DEBUG_32X_VDP,			GMF_ITEM_CHECK,		"32X - VDP",		NULL, NULL},
	{0, 0, NULL, NULL, NULL}
};
#endif /* GENS_DEBUGGER */

static struct GensMenuItem_t gmiCPU_Country[] =
{
	{IDM_CPU_COUNTRY_AUTO,		GMF_ITEM_RADIO,		"Auto Detect",		NULL, NULL},
	{IDM_CPU_COUNTRY_JAPAN_NTSC,	GMF_ITEM_RADIO,		"Japan (NTSC)",		NULL, NULL},
	{IDM_CPU_COUNTRY_USA,		GMF_ITEM_RADIO,		"USA (NTSC)",		NULL, NULL},
	{IDM_CPU_COUNTRY_EUROPE,	GMF_ITEM_RADIO,		"Europe (PAL)",		NULL, NULL},
	{IDM_CPU_COUNTRY_JAPAN_PAL,	GMF_ITEM_RADIO,		"Japan (PAL)",		NULL, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, NULL},
	{IDM_CPU_COUNTRY_ORDER,		GMF_ITEM_NORMAL,	"Auto-Detection Order...", NULL, NULL},
	{0, 0, NULL, NULL, NULL}
};


/** Sound Menu **/


static struct GensMenuItem_t gmiSound_Rate[];

static struct GensMenuItem_t gmiSound[] =
{
	{IDM_SOUND_ENABLE,		GMF_ITEM_CHECK,		"&Enable",		NULL, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, NULL},
	{IDM_SOUND_RATE,		GMF_ITEM_SUBMENU,	"&Rate",		&gmiSound_Rate[0], NULL},
	{IDM_SOUND_STEREO,		GMF_ITEM_CHECK,		"&Stereo",		NULL, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, NULL},
	{IDM_SOUND_Z80,			GMF_ITEM_CHECK,		"&Z80",			NULL, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, NULL},
	{IDM_SOUND_YM2612,		GMF_ITEM_CHECK,		"&YM2612",		NULL, NULL},
	{IDM_SOUND_YM2612_IMPROVED,	GMF_ITEM_CHECK,		"YM2612 Improved",	NULL, NULL},
	{IDM_SOUND_DAC,			GMF_ITEM_CHECK,		"&DAC",			NULL, NULL},
	{IDM_SOUND_DAC_IMPROVED,	GMF_ITEM_CHECK,		"DAC Improved",		NULL, NULL},
	{IDM_SOUND_PSG,			GMF_ITEM_CHECK,		"&PSG",			NULL, NULL},
	{IDM_SOUND_PSG_IMPROVED,	GMF_ITEM_CHECK,		"PSG Improved",		NULL, NULL},
	{IDM_SOUND_PCM,			GMF_ITEM_CHECK,		"P&CM",			NULL, NULL},
	{IDM_SOUND_PWM,			GMF_ITEM_CHECK,		"P&WM",			NULL, NULL},
	{IDM_SOUND_CDDA,		GMF_ITEM_CHECK,		"CDDA (CD Audio)",	NULL, NULL},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, NULL},
	{IDM_SOUND_WAVDUMP,		GMF_ITEM_NORMAL,	"Start WAV Dump",	NULL, NULL},
	{IDM_SOUND_GYMDUMP,		GMF_ITEM_NORMAL,	"Start GYM Dump",	NULL, NULL},
	{0, 0, NULL, NULL, NULL}
};

static struct GensMenuItem_t gmiSound_Rate[] =
{
	{IDM_SOUND_RATE_11025,		GMF_ITEM_RADIO,		"11,025 Hz",		NULL, NULL},
	{IDM_SOUND_RATE_16000,		GMF_ITEM_RADIO,		"16,000 Hz",		NULL, NULL},
	{IDM_SOUND_RATE_22050,		GMF_ITEM_RADIO,		"22,050 Hz",		NULL, NULL},
	{IDM_SOUND_RATE_24000,		GMF_ITEM_RADIO,		"24,000 Hz",		NULL, NULL},
	{IDM_SOUND_RATE_44100,		GMF_ITEM_RADIO,		"44,100 Hz",		NULL, NULL},
	{IDM_SOUND_RATE_48000,		GMF_ITEM_RADIO,		"48,000 Hz",		NULL, NULL},
	{0, 0, NULL, NULL, NULL}
};


/** Options Menu **/


static struct GensMenuItem_t gmiOptions_SegaCDSRAMSize[];

static struct GensMenuItem_t gmiOptions[] =
{
	{IDM_OPTIONS_GENERAL,		GMF_ICON_FILE,		"&General Options...",	NULL, "ksysguard.png"},
	{IDM_OPTIONS_JOYPADS,		GMF_ICON_FILE,		"&Joypads...",		NULL, "package_games.png"},
	{IDM_OPTIONS_DIRECTORIES,	GMF_ICON_FILE,		"&Directories...",	NULL, "folder_slin_open.png"},
	{IDM_OPTIONS_BIOSMISCFILES,	GMF_ICON_FILE,		"&BIOS/Misc Files...",	NULL, "binary.png"},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, NULL},
#ifdef GENS_CDROM
	{IDM_OPTIONS_CURRENT_CD_DRIVE,	GMF_ICON_STOCK,		"Current &CD Drive...",	NULL, "gtk-cdrom"},
#endif /* GENS_CDROM */
	{IDM_OPTIONS_SEGACDSRAMSIZE,	GMF_ITEM_SEPARATOR | GMF_ICON_FILE, "Sega CD S&RAM Size", &gmiOptions_SegaCDSRAMSize[0], "memory.png"},
	{IDM_SEPARATOR,			GMF_ITEM_SEPARATOR,	NULL,			NULL, NULL},
	{IDM_OPTIONS_LOADCONFIG,	GMF_ICON_STOCK,		"&Load Config...",	NULL, "gtk-open"},
	{IDM_OPTIONS_SAVECONFIGAS,	GMF_ICON_STOCK,		"&Save Config As...",	NULL, "gtk-save-as"},
	{0, 0, NULL, NULL, NULL}
};

static struct GensMenuItem_t gmiOptions_SegaCDSRAMSize[] =
{
	{IDM_OPTIONS_SEGACDSRAMSIZE_NONE,	GMF_ITEM_RADIO, "None",  NULL, NULL},
	{IDM_OPTIONS_SEGACDSRAMSIZE_8KB,	GMF_ITEM_RADIO, "8 KB",  NULL, NULL},
	{IDM_OPTIONS_SEGACDSRAMSIZE_16KB,	GMF_ITEM_RADIO, "16 KB", NULL, NULL},
	{IDM_OPTIONS_SEGACDSRAMSIZE_32KB,	GMF_ITEM_RADIO, "32 KB", NULL, NULL},
	{IDM_OPTIONS_SEGACDSRAMSIZE_64KB,	GMF_ITEM_RADIO, "64 KB", NULL, NULL},
	{0, 0, NULL, NULL, NULL}
};


/** Help Menu **/


static struct GensMenuItem_t gmiHelp[] =
{
	{IDM_HELP_ABOUT,	GMF_ICON_STOCK,		"&About",	NULL,	"gtk-help"},
	{0, 0, NULL, NULL, NULL}
};

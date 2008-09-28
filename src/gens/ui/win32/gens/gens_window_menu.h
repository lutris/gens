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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef __cplusplus
extern "C" {
#endif

// Menu identifier & 0xF000 == the menu it's from

// File Menu
#define ID_FILE_MENU			0x1000
#define ID_FILE_OPENROM			0x1001
#ifdef GENS_CDROM
#define ID_FILE_BOOTCD			0x1002
#endif /* GENS_CDROM */
#define ID_FILE_NETPLAY			0x1003
#define ID_FILE_CLOSEROM		0x1004
#define ID_FILE_GAMEGENIE		0x1005
#define ID_FILE_LOADSTATE		0x1006
#define ID_FILE_SAVESTATE		0x1007
#define ID_FILE_QUICKLOAD		0x1008
#define ID_FILE_QUICKSAVE		0x1009
#define ID_FILE_ROMHISTORY		0x1100 // Same as ID_FILE_ROMHISTORY_0
#define ID_FILE_ROMHISTORY_0		0x1100
#define ID_FILE_ROMHISTORY_1		0x1101
#define ID_FILE_ROMHISTORY_2		0x1102
#define ID_FILE_ROMHISTORY_3		0x1103
#define ID_FILE_ROMHISTORY_4		0x1104
#define ID_FILE_ROMHISTORY_5		0x1105
#define ID_FILE_ROMHISTORY_6		0x1106
#define ID_FILE_ROMHISTORY_7		0x1107
#define ID_FILE_ROMHISTORY_8		0x1108
#define ID_FILE_ROMHISTORY_9		0x1109
#define ID_FILE_CHANGESTATE		0x1200 // Same as ID_FILE_CHANGESTATE_0
#define ID_FILE_CHANGESTATE_0		0x1200
#define ID_FILE_CHANGESTATE_1		0x1201
#define ID_FILE_CHANGESTATE_2		0x1202
#define ID_FILE_CHANGESTATE_3		0x1203
#define ID_FILE_CHANGESTATE_4		0x1204
#define ID_FILE_CHANGESTATE_5		0x1205
#define ID_FILE_CHANGESTATE_6		0x1206
#define ID_FILE_CHANGESTATE_7		0x1207
#define ID_FILE_CHANGESTATE_8		0x1208
#define ID_FILE_CHANGESTATE_9		0x1209
#define ID_FILE_QUIT			0x1FFF

// Graphics Menu
#define ID_GRAPHICS_MENU		0x2000
#define ID_GRAPHICS_FULLSCREEN		0x2001
#define ID_GRAPHICS_VSYNC		0x2002
#define ID_GRAPHICS_STRETCH		0x2003
#define ID_GRAPHICS_COLORADJUST		0x2004
#define ID_GRAPHICS_SPRITELIMIT		0x2005
#define ID_GRAPHICS_SCREENSHOT		0x2006
#define ID_GRAPHICS_RENDER		0x2100 // Same as ID_GRAPHICS_RENDER_NORMAL
#define ID_GRAPHICS_RENDER_NORMAL	0x2100
#define ID_GRAPHICS_RENDER_DOUBLE	0x2101
#define ID_GRAPHICS_FRAMESKIP		0x2200 // Same as ID_GRAPHICS_FRAMESKIP_AUTO
#define ID_GRAPHICS_FRAMESKIP_AUTO	0x2200
#define ID_GRAPHICS_FRAMESKIP_0		0x2201
#define ID_GRAPHICS_FRAMESKIP_1		0x2202
#define ID_GRAPHICS_FRAMESKIP_2		0x2203
#define ID_GRAPHICS_FRAMESKIP_3		0x2204
#define ID_GRAPHICS_FRAMESKIP_4		0x2205
#define ID_GRAPHICS_FRAMESKIP_5		0x2206
#define ID_GRAPHICS_FRAMESKIP_6		0x2207
#define ID_GRAPHICS_FRAMESKIP_7		0x2208
#define ID_GRAPHICS_FRAMESKIP_8		0x2209

// CPU Menu
#define ID_CPU_MENU			0x3000
#define ID_CPU_HARDRESET		0x3001
#define ID_CPU_RESET68K			0x3002
#define ID_CPU_RESETMAIN68K		0x3003
#define ID_CPU_RESETSUB68K		0x3004
#define ID_CPU_RESETMAINSH2		0x3005
#define ID_CPU_RESETSUBSH2		0x3006
#define ID_CPU_RESETZ80			0x3007
#define ID_CPU_SEGACDPERFECTSYNC	0x3008
#ifdef GENS_DEBUGGER
#define ID_CPU_DEBUG			0x3100 // Same as ID_CPU_DEBUG_MC68000
#define ID_CPU_DEBUG_MC68000		0x3100
#define ID_CPU_DEBUG_Z80		0x3101
#define ID_CPU_DEBUG_VDP		0x3102
#define ID_CPU_DEBUG_SEGACD_MC68000	0x3103
#define ID_CPU_DEBUG_SEGACD_CDC		0x3104
#define ID_CPU_DEBUG_SEGACD_GFX		0x3105
#define ID_CPU_DEBUG_32X_MSH2		0x3106
#define ID_CPU_DEBUG_32X_SSH2		0x3107
#define ID_CPU_DEBUG_32X_VDP		0x3108
#endif /* GENS_DEBUGGER */
#define ID_CPU_COUNTRY			0x3200 // Same as ID_CPU_COUNTRY_AUTO
#define ID_CPU_COUNTRY_AUTO		0x3200
#define ID_CPU_COUNTRY_JAPAN_NTSC	0x3201
#define ID_CPU_COUNTRY_USA		0x3202
#define ID_CPU_COUNTRY_EUROPE		0x3203
#define ID_CPU_COUNTRY_JAPAN_PAL	0x3204
#define ID_CPU_COUNTRY_ORDER		0x3205

// Sound Menu
#define ID_SOUND_MENU			0x4000
#define ID_SOUND_ENABLE			0x4001
#define ID_SOUND_RATE			0x4002
#define ID_SOUND_STEREO			0x4003
#define ID_SOUND_Z80			0x4004
#define ID_SOUND_YM2612			0x4005
#define ID_SOUND_YM2612_IMPROVED	0x4006
#define ID_SOUND_DAC			0x4007
#define ID_SOUND_DAC_IMPROVED		0x4008
#define ID_SOUND_PSG			0x4009
#define ID_SOUND_PSG_IMPROVED		0x400A
#define ID_SOUND_PCM			0x400B
#define ID_SOUND_PWM			0x400C
#define ID_SOUND_CDDA			0x400D
#define ID_SOUND_WAVDUMP		0x400E
#define ID_SOUND_GYMDUMP		0x400F

// Options Menu
#define ID_OPTIONS_MENU			0x5000
#define ID_OPTIONS_GENERAL		0x5001
#define ID_OPTIONS_JOYPADS		0x5002
#define ID_OPTIONS_DIRECTORIES		0x5003
#define ID_OPTIONS_BIOS_MISC_FILES	0x5004
#ifdef GENS_CDROM
#define ID_OPTIONS_CURRENT_CD_DRIVE	0x5005
#endif /* GENS_CDROM */
#define ID_OPTIONS_SEGACD_SRAM_SIZE	0x5006
#define ID_OPTIONS_LOADCONFIG		0x5007
#define ID_OPTIONS_SAVECONFIGAS		0x5008

// Help Menu
#define ID_HELP_MENU			0x6000
#define ID_HELP_ABOUT			0x6001

#ifdef __cplusplus
}
#endif

#endif

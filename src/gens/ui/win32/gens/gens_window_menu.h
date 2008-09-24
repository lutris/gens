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

// CPU Menu
#define ID_CPU_MENU			0x3000
#ifdef GENS_DEBUGGER
#define ID_CPU_DEBUG			0x3001
#endif /* GENS_DEBUGGER */
#define ID_CPU_COUNTRY			0x3002
#define ID_CPU_HARDRESET		0x3003
#define ID_CPU_RESET68K			0x3004
#define ID_CPU_RESETMAIN68K		0x3005
#define ID_CPU_RESETSUB68K		0x3006
#define ID_CPU_RESETMAINSH2		0x3007
#define ID_CPU_RESETSUBSH2		0x3008
#define ID_CPU_RESETZ80			0x3009
#define ID_CPU_SEGACDPERFECTSYNC	0x300A

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

#ifdef __cplusplus
}
#endif

#endif

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

#ifndef GENS_MENU_H
#define GENS_MENU_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define GMF_ITEM_NORMAL		0x0000
#define GMF_ITEM_SEPARATOR	0x0001
#define GMF_ITEM_SUBMENU	0x0002
#define GMF_ITEM_CHECK		0x0003
#define GMF_ITEM_RADIO		0x0004
#define GMF_ITEM_MASK		0x000F

// Accelerator modifiers
#define GMAM_CTRL		0x0001
#define GMAM_ALT		0x0002
#define GMAM_SHIFT		0x0004

// Accelerator keys
#define GMAK_BACKSPACE		0x08
#define GMAK_TAB		0x09
#define GMAK_ENTER		0x0D

#define GMAK_F1			0x0201
#define GMAK_F2			0x0202
#define GMAK_F3			0x0203
#define GMAK_F4			0x0204
#define GMAK_F5			0x0205
#define GMAK_F6			0x0206
#define GMAK_F7			0x0207
#define GMAK_F8			0x0208
#define GMAK_F9			0x0209
#define GMAK_F10		0x020A
#define GMAK_F11		0x020B
#define GMAK_F12		0x020C

// Menu icon IDs.
typedef enum _MenuIconID
{
	IDIM_MENU_ICON = 0x0100,
	IDIM_GENS,
	IDIM_GENSGS,
	IDIM_OPEN,
	IDIM_SAVE,
	IDIM_SAVE_AS,
	IDIM_REFRESH,
	IDIM_REVERT,
	IDIM_CLOSE,
	IDIM_QUIT,
	IDIM_CDROM,
	IDIM_CDROM_DRIVE,
	IDIM_NETPLAY,
	IDIM_ROMHISTORY,
	IDIM_GAME_GENIE,
	IDIM_FULLSCREEN,
	IDIM_COLOR_ADJUST,
	IDIM_RENDER,
	IDIM_FRAMESKIP,
	IDIM_SCREENSHOT,
	IDIM_RESET,
	IDIM_GENERAL_OPTIONS,
	IDIM_JOYPADS,
	IDIM_DIRECTORIES,
	IDIM_BIOSMISCFILES,
	IDIM_MEMORY,
	IDIM_HELP,
	IDIM_REPORTABUG,
	
	IDIM_MENU_ICON_MAX
} MenuIconID;

typedef struct _GensMenuItem_t
{
	uint16_t id;				// Menu identifier. (0xFFFF == separator)
	uint16_t flags;				// Menu item flags.
	const char* text;			// Menu text.
	const struct _GensMenuItem_t* submenu;	// First element of submenu.
	
	// Accelerator. (Set both values to 0 for no accelerator.)
	uint16_t accelModifier;
	uint16_t accelKey;
	
	uint16_t icon;				// Icon ID.
} GensMenuItem_t;

extern const GensMenuItem_t gmiMain[];

// Menu #defines
// Menu identifier & 0xF000 == the menu it's from

#define IDM_SEPARATOR			0xFFFF

// File Menu
#define IDM_FILE_MENU			0x1000
#define IDM_FILE_OPENROM		(IDM_FILE_MENU + 1)
#ifdef GENS_CDROM
#define IDM_FILE_BOOTCD			(IDM_FILE_MENU + 2)
#endif /* GENS_CDROM */
#define IDM_FILE_NETPLAY		(IDM_FILE_MENU + 3)
#define IDM_FILE_CLOSEROM		(IDM_FILE_MENU + 4)
#define IDM_FILE_LOADSTATE		(IDM_FILE_MENU + 5)
#define IDM_FILE_SAVESTATE		(IDM_FILE_MENU + 6)
#define IDM_FILE_QUICKLOAD		(IDM_FILE_MENU + 7)
#define IDM_FILE_QUICKSAVE		(IDM_FILE_MENU + 8)

#define IDM_FILE_ROMHISTORY		0x1100
#define IDM_FILE_ROMHISTORY_1		(IDM_FILE_ROMHISTORY + 1)
#define IDM_FILE_ROMHISTORY_2		(IDM_FILE_ROMHISTORY + 2)
#define IDM_FILE_ROMHISTORY_3		(IDM_FILE_ROMHISTORY + 3)
#define IDM_FILE_ROMHISTORY_4		(IDM_FILE_ROMHISTORY + 4)
#define IDM_FILE_ROMHISTORY_5		(IDM_FILE_ROMHISTORY + 5)
#define IDM_FILE_ROMHISTORY_6		(IDM_FILE_ROMHISTORY + 6)
#define IDM_FILE_ROMHISTORY_7		(IDM_FILE_ROMHISTORY + 7)
#define IDM_FILE_ROMHISTORY_8		(IDM_FILE_ROMHISTORY + 8)
#define IDM_FILE_ROMHISTORY_9		(IDM_FILE_ROMHISTORY + 9)

#define IDM_FILE_CHANGESTATE		0x1200
#define IDM_FILE_CHANGESTATE_0		(IDM_FILE_CHANGESTATE + 1)
#define IDM_FILE_CHANGESTATE_1		(IDM_FILE_CHANGESTATE + 2)
#define IDM_FILE_CHANGESTATE_2		(IDM_FILE_CHANGESTATE + 3)
#define IDM_FILE_CHANGESTATE_3		(IDM_FILE_CHANGESTATE + 4)
#define IDM_FILE_CHANGESTATE_4		(IDM_FILE_CHANGESTATE + 5)
#define IDM_FILE_CHANGESTATE_5		(IDM_FILE_CHANGESTATE + 6)
#define IDM_FILE_CHANGESTATE_6		(IDM_FILE_CHANGESTATE + 7)
#define IDM_FILE_CHANGESTATE_7		(IDM_FILE_CHANGESTATE + 8)
#define IDM_FILE_CHANGESTATE_8		(IDM_FILE_CHANGESTATE + 9)
#define IDM_FILE_CHANGESTATE_9		(IDM_FILE_CHANGESTATE + 10)

#define IDM_FILE_QUIT			0x1FFF
#define IDM_FILE_EXIT			0x1FFF

// Graphics Menu
#define IDM_GRAPHICS_MENU		0x2000
#define IDM_GRAPHICS_FULLSCREEN		(IDM_GRAPHICS_MENU + 1)
#define IDM_GRAPHICS_MENUBAR		(IDM_GRAPHICS_MENU + 2)
#define IDM_GRAPHICS_VSYNC		(IDM_GRAPHICS_MENU + 3)
#define IDM_GRAPHICS_OPENGL_FILTER	(IDM_GRAPHICS_MENU + 4)
#define IDM_GRAPHICS_COLORADJUST	(IDM_GRAPHICS_MENU + 5)
#define IDM_GRAPHICS_SPRITELIMIT	(IDM_GRAPHICS_MENU + 6)
#define IDM_GRAPHICS_SCREENSHOT		(IDM_GRAPHICS_MENU + 7)

#define IDM_GRAPHICS_STRETCH		0x2100
#define IDM_GRAPHICS_STRETCH_NONE	(IDM_GRAPHICS_STRETCH + 1)
#define IDM_GRAPHICS_STRETCH_H		(IDM_GRAPHICS_STRETCH + 2)
#define IDM_GRAPHICS_STRETCH_V		(IDM_GRAPHICS_STRETCH + 3)
#define IDM_GRAPHICS_STRETCH_FULL	(IDM_GRAPHICS_STRETCH + 4)

#define IDM_GRAPHICS_BACKEND		0x2200

#define IDM_GRAPHICS_OPENGL_RES			0x2300
#define IDM_GRAPHICS_OPENGL_RES_320x240		(IDM_GRAPHICS_OPENGL_RES + 1)
#define IDM_GRAPHICS_OPENGL_RES_640x480		(IDM_GRAPHICS_OPENGL_RES + 2)
#define IDM_GRAPHICS_OPENGL_RES_800x600		(IDM_GRAPHICS_OPENGL_RES + 3)
#define IDM_GRAPHICS_OPENGL_RES_960x720		(IDM_GRAPHICS_OPENGL_RES + 4)
#define IDM_GRAPHICS_OPENGL_RES_1024x768	(IDM_GRAPHICS_OPENGL_RES + 5)
#define IDM_GRAPHICS_OPENGL_RES_1280x960	(IDM_GRAPHICS_OPENGL_RES + 6)
#define IDM_GRAPHICS_OPENGL_RES_1280x1024	(IDM_GRAPHICS_OPENGL_RES + 7)
#define IDM_GRAPHICS_OPENGL_RES_1400x1050	(IDM_GRAPHICS_OPENGL_RES + 8)
#define IDM_GRAPHICS_OPENGL_RES_1600x1200	(IDM_GRAPHICS_OPENGL_RES + 9)
#define IDM_GRAPHICS_OPENGL_RES_CUSTOM		(IDM_GRAPHICS_OPENGL_RES | 0xFF)

#define IDM_GRAPHICS_BPP		0x2400
#define IDM_GRAPHICS_BPP_15		(IDM_GRAPHICS_BPP + 1) // 15-bit color (555)
#define IDM_GRAPHICS_BPP_16		(IDM_GRAPHICS_BPP + 2) // 16-bit color (565)
#define IDM_GRAPHICS_BPP_32		(IDM_GRAPHICS_BPP + 3) // 32-bit color

#define IDM_GRAPHICS_FRAMESKIP		0x2500
#define IDM_GRAPHICS_FRAMESKIP_AUTO	(IDM_GRAPHICS_FRAMESKIP + 1)
#define IDM_GRAPHICS_FRAMESKIP_0	(IDM_GRAPHICS_FRAMESKIP + 2)
#define IDM_GRAPHICS_FRAMESKIP_1	(IDM_GRAPHICS_FRAMESKIP + 3)
#define IDM_GRAPHICS_FRAMESKIP_2	(IDM_GRAPHICS_FRAMESKIP + 4)
#define IDM_GRAPHICS_FRAMESKIP_3	(IDM_GRAPHICS_FRAMESKIP + 5)
#define IDM_GRAPHICS_FRAMESKIP_4	(IDM_GRAPHICS_FRAMESKIP + 6)
#define IDM_GRAPHICS_FRAMESKIP_5	(IDM_GRAPHICS_FRAMESKIP + 7)
#define IDM_GRAPHICS_FRAMESKIP_6	(IDM_GRAPHICS_FRAMESKIP + 8)
#define IDM_GRAPHICS_FRAMESKIP_7	(IDM_GRAPHICS_FRAMESKIP + 9)
#define IDM_GRAPHICS_FRAMESKIP_8	(IDM_GRAPHICS_FRAMESKIP + 10)

#define IDM_GRAPHICS_RENDER		0x2800
#define IDM_GRAPHICS_RENDER_NORMAL	(IDM_GRAPHICS_RENDER + 1)
#define IDM_GRAPHICS_RENDER_DOUBLE	(IDM_GRAPHICS_RENDER + 2)

// CPU Menu
#define IDM_CPU_MENU			0x3000
#define IDM_CPU_HARDRESET		(IDM_CPU_MENU + 1)
#define IDM_CPU_RESET68K		(IDM_CPU_MENU + 2)
#define IDM_CPU_RESETMAIN68K		(IDM_CPU_MENU + 3)
#define IDM_CPU_RESETSUB68K		(IDM_CPU_MENU + 4)
#define IDM_CPU_RESETMAINSH2		(IDM_CPU_MENU + 5)
#define IDM_CPU_RESETSUBSH2		(IDM_CPU_MENU + 6)
#define IDM_CPU_RESETZ80		(IDM_CPU_MENU + 7)
#define IDM_CPU_SEGACDPERFECTSYNC	(IDM_CPU_MENU + 8)

#ifdef GENS_DEBUGGER
#define IDM_CPU_DEBUG			0x3100
#define IDM_CPU_DEBUG_MC68000		(IDM_CPU_DEBUG + 1)
#define IDM_CPU_DEBUG_Z80		(IDM_CPU_DEBUG + 2)
#define IDM_CPU_DEBUG_VDP		(IDM_CPU_DEBUG + 3)
#define IDM_CPU_DEBUG_SEGACD_MC68000	(IDM_CPU_DEBUG + 4)
#define IDM_CPU_DEBUG_SEGACD_CDC	(IDM_CPU_DEBUG + 5)
#define IDM_CPU_DEBUG_SEGACD_GFX	(IDM_CPU_DEBUG + 6)
#define IDM_CPU_DEBUG_32X_MSH2		(IDM_CPU_DEBUG + 7)
#define IDM_CPU_DEBUG_32X_SSH2		(IDM_CPU_DEBUG + 8)
#define IDM_CPU_DEBUG_32X_VDP		(IDM_CPU_DEBUG + 9)
#define IDM_CPU_DEBUG_SEGACD_SEPARATOR	(IDM_CPU_DEBUG + 10)
#define IDM_CPU_DEBUG_32X_SEPARATOR	(IDM_CPU_DEBUG + 11)
#endif /* GENS_DEBUGGER */

#define IDM_CPU_COUNTRY			0x3200
#define IDM_CPU_COUNTRY_AUTO		(IDM_CPU_COUNTRY + 1)
#define IDM_CPU_COUNTRY_JAPAN_NTSC	(IDM_CPU_COUNTRY + 2)
#define IDM_CPU_COUNTRY_USA		(IDM_CPU_COUNTRY + 3)
#define IDM_CPU_COUNTRY_EUROPE		(IDM_CPU_COUNTRY + 4)
#define IDM_CPU_COUNTRY_JAPAN_PAL	(IDM_CPU_COUNTRY + 5)
#define IDM_CPU_COUNTRY_ORDER		(IDM_CPU_COUNTRY + 6)

// Sound Menu
#define IDM_SOUND_MENU			0x4000
#define IDM_SOUND_ENABLE		(IDM_SOUND_MENU + 1)
#define IDM_SOUND_STEREO		(IDM_SOUND_MENU + 2)
#define IDM_SOUND_Z80			(IDM_SOUND_MENU + 3)
#define IDM_SOUND_YM2612		(IDM_SOUND_MENU + 4)
#define IDM_SOUND_YM2612_IMPROVED	(IDM_SOUND_MENU + 5)
#define IDM_SOUND_DAC			(IDM_SOUND_MENU + 6)
#define IDM_SOUND_PSG			(IDM_SOUND_MENU + 7)
#define IDM_SOUND_PSG_SINE		(IDM_SOUND_MENU + 8)
#define IDM_SOUND_PCM			(IDM_SOUND_MENU + 9)
#define IDM_SOUND_PWM			(IDM_SOUND_MENU + 10)
#define IDM_SOUND_CDDA			(IDM_SOUND_MENU + 11)
#define IDM_SOUND_WAVDUMP		(IDM_SOUND_MENU + 12)
#define IDM_SOUND_GYMDUMP		(IDM_SOUND_MENU + 13)

#define IDM_SOUND_RATE			0x4100
#define IDM_SOUND_RATE_11025		(IDM_SOUND_RATE + 1)
#define IDM_SOUND_RATE_22050		(IDM_SOUND_RATE + 2)
#define IDM_SOUND_RATE_44100		(IDM_SOUND_RATE + 3)

// Options Menu
#define IDM_OPTIONS_MENU		0x5000
#define IDM_OPTIONS_GENERAL		(IDM_OPTIONS_MENU + 1)
#define IDM_OPTIONS_JOYPADS		(IDM_OPTIONS_MENU + 2)
#define IDM_OPTIONS_DIRECTORIES		(IDM_OPTIONS_MENU + 3)
#define IDM_OPTIONS_BIOSMISCFILES	(IDM_OPTIONS_MENU + 4)
#define IDM_OPTIONS_SRAM_ENABLED	(IDM_OPTIONS_MENU + 5)
#ifdef GENS_CDROM
#define IDM_OPTIONS_CURRENT_CD_DRIVE	(IDM_OPTIONS_MENU + 6)
#endif /* GENS_CDROM */
#define IDM_OPTIONS_LOADCONFIG		(IDM_OPTIONS_MENU + 7)
#define IDM_OPTIONS_SAVECONFIGAS	(IDM_OPTIONS_MENU + 8)

#define IDM_OPTIONS_SEGACDSRAMSIZE	0x5100
#define IDM_OPTIONS_SEGACDSRAMSIZE_NONE	(IDM_OPTIONS_SEGACDSRAMSIZE + 1)
#define IDM_OPTIONS_SEGACDSRAMSIZE_8KB	(IDM_OPTIONS_SEGACDSRAMSIZE + 2)
#define IDM_OPTIONS_SEGACDSRAMSIZE_16KB	(IDM_OPTIONS_SEGACDSRAMSIZE + 3)
#define IDM_OPTIONS_SEGACDSRAMSIZE_32KB	(IDM_OPTIONS_SEGACDSRAMSIZE + 4)
#define IDM_OPTIONS_SEGACDSRAMSIZE_64KB	(IDM_OPTIONS_SEGACDSRAMSIZE + 5)

// Plugins Menu
#define IDM_PLUGINS_MENU		0x6000
#define IDM_PLUGINS_MANAGER		0x6FFF

// Help Menu
#define IDM_HELP_MENU			0xF000
#ifndef GENS_OS_MACOSX
#define IDM_HELP_MANUAL			(IDM_HELP_MENU + 1)
#define IDM_HELP_REPORTABUG		(IDM_HELP_MENU + 2)
#endif
#define IDM_HELP_ABOUT			(IDM_HELP_MENU + 3)

#ifdef __cplusplus
}
#endif

#endif /* GENS_MENU_H */

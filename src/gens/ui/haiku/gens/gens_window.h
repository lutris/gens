/***************************************************************************
 * Gens: (Haiku) Main Window.                                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * Copyright (c) 2009 by Phil Costin                                       *
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

#ifndef GENS_HAIKU_GENS_WINDOW_H
#define GENS_HAIKU_GENS_WINDOW_H

#ifdef __cplusplus

#include <Window.h>

#define kMsgOpenROM 'orom'
#define kMsgBootCD 'bootcd'
#define kMsgNetplay 'netplay'
#define kMsgCloseROM 'crom'
#define kMsgGameGenie 'gg'
#define kMsgLoadState 'ls'
#define kMsgSaveStateAs 'ssa'
#define kMsgQuickLoad 'ql'
#define kMsgQuickSave 'qs'
#define kMsgFullScreen 'fs'
#define kMsgVSync 'vs'
#define kMsgStretch 'str'
#define kMsgOpenGL 'ogl'
#define kMsgColorAdjust 'ca'
#define kMsgSpriteLimit 'sl'
#define kMsgScreenShot 'ss'
#define kMsgHardReset 'hr'
#define kMsgReset68K 'r68k'
#define kMsgResetZ80 'rz80'
#define kMsgPerfectSync 'ps'
#define kMsgSound 'snd'
#define kMsgStereo 'st'
#define kMsgZ80 'z80'
#define kMsgYM2612 'ym'
#define kMsgYM2612Impr 'ymi'
#define kMsgDAC 'dac'
#define kMsgDACImpr 'daci'
#define kMsgPSG 'psg'
#define kMsgPSGImpr 'psgi'
#define kMsgPCM 'pcm'
#define kMsgPWM 'pwm'
#define kMsgCDDA 'cdda'
#define kMsgWAVDump 'wd'
#define kMsgGYMDump 'gd'
#define kMsgGeneralOptions 'go'
#define kMsgJoypads 'jp'
#define kMsgDirectories 'dr'
#define kMsgBIOSMisc 'bm'
#define kMsgSDLSoundTest 'sst'
#define kMsgCurCDDrive 'cd'
#define kMsgLoadConfig 'lc'
#define kMsgSaveConfigAs 'sc'
#define kMsgAbout 'ab'

#ifdef __cplusplus
extern "C" {
#endif

void gens_window_create_menubar(void);

#ifdef __cplusplus
}
#endif

class BMenu;
class BMenuItem;
class BMessage;

class GensWindow : public BWindow {

public:
	GensWindow();
	~GensWindow();
};

#else
	void *gens_window;
#endif /* __cplusplus */

#endif /* GENS_HAIKU_GENS_WINDOW_H */

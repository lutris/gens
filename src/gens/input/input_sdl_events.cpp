/***************************************************************************
 * Gens: Input Handler - SDL Backend. (Events)                             *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "input_sdl_events.hpp"

// C includes.
#include <string.h>

// C++ includes.
#include <string>
#include <list>
using std::string;
using std::list;

#include "emulator/g_main.hpp"
#include "emulator/g_md.hpp"
#include "emulator/g_mcd.hpp"
#include "emulator/options.hpp"
#include "util/file/save.hpp"
#include "util/sound/gym.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/gfx/imageutil.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif

#include "gens/gens_window_sync.hpp"
#include "gens_ui.hpp"

// Plugin Manager.
#include "plugins/pluginmgr.hpp"

// Video, Audio, Input.
#include "video/vdraw.h"
#include "video/vdraw_cpp.hpp"
#include "audio/audio.h"
#include "input/input.h"


// Due to bugs with SDL and GTK, modifier state has to be tracked manually.
// TODO: Shift-A works, but if shift is still held down and B is pressed, nothing shows up on SDL.
// TODO: This isn't actually a bug with SDL/GTK - it's an issue with keysnooping...
static int mod = 0;

#define IS_KMOD_NONE(mod) (mod == 0)
#define IS_KMOD_CTRL(mod) \
	(!(mod & ~GENS_KMOD_CTRL) && ((mod & GENS_KMOD_LCTRL) || (mod & GENS_KMOD_RCTRL)))
#define IS_KMOD_ALT(mod) \
	(!(mod & ~GENS_KMOD_ALT) && ((mod & GENS_KMOD_LALT) || (mod & GENS_KMOD_RALT)))
#define IS_KMOD_SHIFT(mod) \
	(!(mod & ~GENS_KMOD_SHIFT) && ((mod & GENS_KMOD_LSHIFT) || (mod & GENS_KMOD_RSHIFT)))


/**
 * input_sdl_reset_modifiers(): Reset the key modifier state.
 * @param event Pointer to SDL_Event struct.
 */
void input_sdl_reset_modifiers(void)
{
	mod = 0;
}


/**
 * input_sdl_event_key_down(): Check if a key is pressed.
 * @param key Keycode.
 */
void input_sdl_event_key_down(int key)
{
#ifndef GENS_OS_MACOSX
	const int allow_gtk_hotkeys = vdraw_get_fullscreen();
#else
	// GTK+ uses a separate window on MacOS X, and that window may
	// be removed later by using the standard MacOS X menu bar.
	const int allow_gtk_hotkeys = 1;
#endif
	
	switch (key)
	{
		case GENS_KEY_LCTRL:
			mod |= GENS_KMOD_LCTRL;
			break;
		case GENS_KEY_RCTRL:
			mod |= GENS_KMOD_RCTRL;
			break;
		case GENS_KEY_LALT:
			mod |= GENS_KMOD_LALT;
			break;
		case GENS_KEY_RALT:
			mod |= GENS_KMOD_RALT;
			break;
		case GENS_KEY_LSHIFT:
			mod |= GENS_KMOD_LSHIFT;
			break;
		case GENS_KEY_RSHIFT:
			mod |= GENS_KMOD_RSHIFT;
			break;
		case GENS_KEY_ESCAPE:
			if (Quick_Exit)
				close_gens();
			
#ifdef GENS_DEBUGGER
			if (Options::debugMode())
			{
				Options::setDebugMode(DEBUG_NONE);
				Settings.Paused = 0;
				Sync_Gens_Window_CPUMenu();
			}
			else
#endif /* GENS_DEBUGGER */
			if (Settings.Paused)
			{
				Settings.Paused = 0;
			}
			else
			{
				Settings.Paused = 1;
				if (ice == 2)
					ice = 3;
				//Pause_Screen();
				audio_clear_sound_buffer();
			}
			break;
		
		case GENS_KEY_PAUSE:
			if (Settings.Paused)
			{
				Settings.Paused = 0;
			}
			else
			{
				Settings.Paused = 1;
				if (ice == 2)
					ice = 3;
				//Pause_Screen();
				audio_clear_sound_buffer();
			}
			break;
		
		case GENS_KEY_BACKSPACE:
			if (allow_gtk_hotkeys && IS_KMOD_SHIFT(mod))
			{
				audio_clear_sound_buffer();
				ImageUtil::ScreenShot();
			}
			else if (IS_KMOD_CTRL(mod))
			{
				// Congratulations!
				if (congratulations == 0)
					congratulations = 1;
			}
			break;
		
		case GENS_KEY_TAB:
#ifndef GENS_OS_WIN32
			if (vdraw_get_fullscreen() && IS_KMOD_ALT(mod))
			{
				// Alt-Tab in fullscreen.
				GensUI::fsMinimize(GensUI::FSMINIMIZE_ALTTAB);
			}
			else
#endif
			{
				// System Reset.
				Options::systemReset();
			}
			break;
		
		case GENS_KEY_RETURN:
			if (IS_KMOD_ALT(mod))
			{
				if (allow_gtk_hotkeys)
				{
					vdraw_set_fullscreen(!vdraw_get_fullscreen());
					Sync_Gens_Window_GraphicsMenu();
				}
				
				// Reset the modifier key value to prevent Alt from getting "stuck".
				mod = 0;
			}
			break;
		
		case GENS_KEY_F1:
			fast_forward = 1;
			break;
		
		case GENS_KEY_F2:
			if (IS_KMOD_SHIFT(mod))
			{
				Options::setStretch((Options::stretch() + 1) % 4);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (IS_KMOD_NONE(mod))
			{
				Options::setFrameSkip(-1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case GENS_KEY_F3:
			if (allow_gtk_hotkeys && IS_KMOD_SHIFT(mod))
			{
				int newVSync = !(vdraw_get_fullscreen() ? Video.VSync_FS : Video.VSync_W);
				Options::setVSync(newVSync);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (IS_KMOD_NONE(mod))
			{
				if (Options::frameSkip() == -1)
				{
					Options::setFrameSkip(0);
					Sync_Gens_Window_GraphicsMenu();
				}
				else if (Options::frameSkip() > 0)
				{
					Options::setFrameSkip(Options::frameSkip() - 1);
					Sync_Gens_Window_GraphicsMenu();
				}
			}
			break;
		
		case GENS_KEY_F4:
			if (!IS_KMOD_NONE(mod))
				break;
			
			if (Options::frameSkip() == -1)
			{
				Options::setFrameSkip(1);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (Options::frameSkip() < 8)
			{
				Options::setFrameSkip(Options::frameSkip() + 1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case GENS_KEY_F5:
			if (!allow_gtk_hotkeys)
				break;
			
			//if (Check_If_Kaillera_Running()) return 0;
			
			if (IS_KMOD_SHIFT(mod))
			{
				string filename = Savestate::SelectFile(true, State_Dir);
				if (!filename.empty())
					Savestate::SaveState(filename);
			}
			else if (IS_KMOD_NONE(mod))
			{
				string filename = Savestate::GetStateFilename();
				Savestate::SaveState(filename);
			}
			break;
		
		case GENS_KEY_F6:
			if (IS_KMOD_NONE(mod))
			{
				Options::setSaveSlot((Options::saveSlot() + 9) % 10);
				Sync_Gens_Window_FileMenu();
			}
			break;
		
		case GENS_KEY_F7:
			if (IS_KMOD_NONE(mod))
			{
				Options::setSaveSlot((Options::saveSlot() + 1) % 10);
				Sync_Gens_Window_FileMenu();
			}
			break;
		
		case GENS_KEY_F8:
			if (!allow_gtk_hotkeys)
				break;
			
			//if (Check_If_Kaillera_Running()) return 0;
			
			if (IS_KMOD_SHIFT(mod))
			{
				string filename = Savestate::SelectFile(false, State_Dir);
				if (!filename.empty())
					Savestate::LoadState(filename);
			}
			else if (IS_KMOD_NONE(mod))
			{
				string filename = Savestate::GetStateFilename();
				Savestate::LoadState(filename);
			}
			break;
		
		case GENS_KEY_F9:
			#ifdef GENS_OS_WIN32 // TODO: Implement SW Render options on SDL?
			if (IS_KMOD_SHIFT(mod))
				Options::setSwRender(!Options::swRender());
			else //if (IS_KMOD_NONE(mod))
			#endif /* GENS_OS_WIN32 */
				Options::setFastBlur(!Options::fastBlur());
			break;
		
		case GENS_KEY_F10:
			if (allow_gtk_hotkeys && IS_KMOD_NONE(mod))
			{
				vdraw_set_fps_enabled(!vdraw_get_fps_enabled());
			}
			break;
		
		case GENS_KEY_F11:
			if (IS_KMOD_SHIFT(mod))
			{
				Options::setSoundPSG_Sine(!Options::soundPSG_Sine());
				Sync_Gens_Window_SoundMenu();
			}
			else if (IS_KMOD_NONE(mod))
			{
				Options::rendererPrev();
			}
			break;
		
		case GENS_KEY_F12:
			if (IS_KMOD_SHIFT(mod))
			{
				Options::setSoundYM2612_Improved(!Options::soundYM2612_Improved());
				Sync_Gens_Window_SoundMenu();
			}
			else if (IS_KMOD_NONE(mod))
			{
				Options::rendererNext();
			}
			break;
		
		case GENS_KEY_0:
		case GENS_KEY_1:
		case GENS_KEY_2:
		case GENS_KEY_3:
		case GENS_KEY_4:
		case GENS_KEY_5:
		case GENS_KEY_6:
		case GENS_KEY_7:
		case GENS_KEY_8:
		case GENS_KEY_9:
		{
			unsigned int value = (key - GENS_KEY_0);
			
			if (IS_KMOD_NONE(mod) && (/*value >= 0 &&*/ value <= 9))
			{
				// No modifier key. Select save slot.
				Options::setSaveSlot(value);
				Sync_Gens_Window_FileMenu();
			}
			else if (IS_KMOD_CTRL(mod) && (value >= 1 && value <= 9))
			{
				// Ctrl. Select ROM from ROM History.
				if (ROM::Recent_ROMs.size() < value)
					break;
				
				//if ((Check_If_Kaillera_Running())) return 0;
				if (audio_get_gym_playing())
					gym_play_stop();
				
				ROM::openROM(ROM::Recent_ROMs.at(value - 1).filename,
					     ROM::Recent_ROMs.at(value - 1).z_filename);
				
				Sync_Gens_Window();
			}
			break;
		}
		
#ifdef GENS_CDROM
		case GENS_KEY_b:
			if (allow_gtk_hotkeys && IS_KMOD_CTRL(mod))
			{
				// Ctrl-B: Boot CD
				if (Num_CD_Drive == 0)
					return;	// return 1;
				//if (Check_If_Kaillera_Running()) return 0;
				if (audio_get_gym_playing())
					gym_play_stop();
				ROM::freeROM(Game);	// Don't forget it !
				SegaCD_Started = Init_SegaCD(NULL);
				Options::setGameName();
			}
			break;
#endif /* GENS_CDROM */
		
		case GENS_KEY_w:
			if (allow_gtk_hotkeys && IS_KMOD_CTRL(mod))
			{
				ROM::freeROM(Game);
			}
			break;
		
#if 0	// TODO: Should this hotkey be implemented?
#ifdef GENS_OPENGL
		case GENS_KEY_f:
			// Toggle the OpenGL Linear Filter.
			if (IS_KMOD_CTRL(mod))
				Options::setOpenGL_LinearFilter(!Options::OpenGL_LinearFilter());
			break;
#endif
#endif

#if 0	// TODO: Fix the MINIMIZE() macro.
		case GENS_KEY_g:
			if (IS_KMOD_CTRL(mod))
			{
				//if (Check_If_Kaillera_Running()) return 0;                                            
				MINIMIZE;
				// TODO: Re-enable this when the GTK+ GUI is rewritten.
				//open_game_genie();
			}
			break;
				
		case GENS_KEY_o:
			if (IS_KMOD_CTRL(mod))
			{
				//if ((Check_If_Kaillera_Running())) return 0;
				if (GYM_Playing)
					Stop_Play_GYM ();
				MINIMIZE;
				Get_Rom();
			}
			break;
		
		/* TODO: Fix MINIMIZE.
		case GENS_KEY_p:
			if (IS_KMOD_CTRL(mod) || IS_KMOD_SHIFT(mod))
			{
				if (!Genesis_Started && !SegaCD_Started && !_32X_Started)
				{
					MINIMIZE;
					if (GYM_Playing)
						Stop_Play_GYM();
					else
						Start_Play_GYM();
					Sync_Gens_Window_SoundMenu();
				}
			}
			break;
		*/
#endif
		
		case GENS_KEY_q:
			if (allow_gtk_hotkeys && IS_KMOD_CTRL(mod))
				close_gens();
			break;
		
		case GENS_KEY_r:
		{
			if (!IS_KMOD_SHIFT(mod))
				break;
			
			int curBackend = vdraw_cur_backend_id;
			curBackend++;
			if (curBackend >= VDRAW_BACKEND_MAX)
				curBackend = 0;
			Options::setBackend((VDRAW_BACKEND)curBackend);
			break;
		}
		
#ifdef GENS_CDROM
		case GENS_KEY_v:
			if (IS_KMOD_CTRL(mod))
			{
				if (SegaCD_Started)
					Change_CD();
			}
			break;
#endif /* GENS_CDROM */
		
		default:
			break;
	}
	
#ifdef GENS_DEBUGGER
	// If debugging, pass the key to the debug handler.
	if (IS_DEBUGGING())
		Debug_Event(key, mod);
#endif /* GENS_DEBUGGER */
}


/**
 * input_sdl_event_key_up(): Check if a key is released.
 * @param key Keycode.
 */
void input_sdl_event_key_up(int key)
{
	switch (key)
	{
		case GENS_KEY_LCTRL:
			mod &= ~GENS_KMOD_LCTRL;
			break;
		case GENS_KEY_RCTRL:
			mod &= ~GENS_KMOD_RCTRL;
			break;
		case GENS_KEY_LALT:
			mod &= ~GENS_KMOD_LALT;
			break;
		case GENS_KEY_RALT:
			mod &= ~GENS_KMOD_RALT;
			break;
		case GENS_KEY_LSHIFT:
			mod &= ~GENS_KMOD_LSHIFT;
			break;
		case GENS_KEY_RSHIFT:
			mod &= ~GENS_KMOD_RSHIFT;
			break;
		case GENS_KEY_F1:
			fast_forward = 0;
			break;
		default:
			break;
	}
}

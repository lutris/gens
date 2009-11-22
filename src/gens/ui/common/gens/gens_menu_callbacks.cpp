/***************************************************************************
 * Gens: Main Menu callbacks.                                              *
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

#include "gens_menu_callbacks.hpp"
#include "gens_menu.h"

// C includes.
#include <unistd.h>

// Message logging.
#include "macros/log_msg.h"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"

#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"
#include "controller_config/cc_window.h"
#include "bios_misc_files/bmf_window.hpp"
#include "directory_config/dir_window.hpp"
#include "general_options/genopt_window.hpp"
#include "about/about_window.hpp"
#include "color_adjust/ca_window.h"
#include "country_code/ccode_window.h"
#include "plugin_manager/pmgr_window.hpp"

#ifdef GENS_OPENGL
#include "opengl_resolution/glres_window.hpp"
#endif /* GENS_OPENGL */

#ifdef GENS_CDROM
#include "select_cdrom/selcd_window.hpp"
#endif /* GENS_CDROM */

#include "emulator/options.hpp"
#include "util/file/config_file.hpp"

// Unused Parameter macro.
#include "libgsft/gsft_unused.h"

#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/file/save.hpp"
#include "mdZ80/mdZ80.h"
#include "util/gfx/imageutil.hpp"

// Sega CD
#include "emulator/g_mcd.hpp"

// 32X
#include "gens_core/cpu/sh2/sh2.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// Video, Audio.
#include "video/vdraw.h"
#include "video/vdraw_cpp.hpp"
#include "audio/audio.h"

// Win32 includes.
#ifdef GENS_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#endif

// For some reason, these aren't extern'd anywhere...
extern "C"
{
	void main68k_reset();
	void sub68k_reset();
}

// C includes
#include <string.h>

// C++ includes
#include <string>
#include <list>
using std::string;
using std::list;

// Plugin Manager.
#include "plugins/pluginmgr.hpp"
#include "plugins/rendermgr.hpp"


static int gens_menu_callback_FileMenu(uint16_t menuID, uint16_t state);
static int gens_menu_callback_GraphicsMenu(uint16_t menuID, uint16_t state);
static int gens_menu_callback_CPUMenu(uint16_t menuID, uint16_t state);
static int gens_menu_callback_SoundMenu(uint16_t menuID, uint16_t state);
static int gens_menu_callback_OptionsMenu(uint16_t menuID, uint16_t state);
static int gens_menu_callback_PluginsMenu(uint16_t menuID, uint16_t state);
static int gens_menu_callback_HelpMenu(uint16_t menuID, uint16_t state);


/**
 * gens_common_menu_callback(): Menu item callback handler.
 * @param menuID Menu ID.
 * @param state Menu state. (Used for check/radio menu items.)
 * @return Non-zero if the callback was handled; 0 if the callback wasn't handled.
 */
int gens_common_menu_callback(uint16_t menuID, uint16_t state)
{
	// Force a wakeup.
	GensUI::wakeup();
	
	// Determine which menu this menu item is from.
	switch (menuID & 0xF000)
	{
		case IDM_FILE_MENU:
			return gens_menu_callback_FileMenu(menuID, state);
		case IDM_GRAPHICS_MENU:
			return gens_menu_callback_GraphicsMenu(menuID, state);
		case IDM_CPU_MENU:
			return gens_menu_callback_CPUMenu(menuID, state);
		case IDM_SOUND_MENU:
			return gens_menu_callback_SoundMenu(menuID, state);
		case IDM_OPTIONS_MENU:
			return gens_menu_callback_OptionsMenu(menuID, state);
		case IDM_PLUGINS_MENU:
			return gens_menu_callback_PluginsMenu(menuID, state);
		case IDM_HELP_MENU:
			return gens_menu_callback_HelpMenu(menuID, state);
		default:
			// Menu item not handled.
			return 0;
	}
}


static int gens_menu_callback_FileMenu(uint16_t menuID, uint16_t state)
{
	GSFT_UNUSED_PARAMETER(state);
	
	string filename;
	
	switch (menuID)
	{
		case IDM_FILE_OPENROM:
			/*
			if ((Check_If_Kaillera_Running()))
				return 0;
			*/
			if (audio_get_gym_playing())
				gym_play_stop();
			if (ROM::getROM() != -1)
				Sync_Gens_Window();
			break;
		
#ifdef GENS_CDROM
		case IDM_FILE_BOOTCD:
			if (!ASPI_Initialized || !Num_CD_Drive)
			{
				fprintf(stderr, "%s: ASPI not initialized and/or no CD-ROM drive(s) detected.\n", __func__);
				break;
			}
			
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			if (audio_get_gym_playing())
				gym_play_stop();
			
			ROM::freeROM(Game); // Don't forget it !
			SegaCD_Started = Init_SegaCD(NULL);
			Options::setGameName();
			Sync_Gens_Window();
			break;
#endif /* GENS_CDROM */
		
#ifdef GENS_OS_WIN32
		case IDM_FILE_NETPLAY:
			// Netplay isn't supported yet in Gens/GS.
			// There are only two ways this message could be received:
			// 1. Someone sent the message using another program.
			// 2. Someone used a Win32 API tool to remove the MF_GRAYED style.
			//
			// So, let's give them what they really want: A Rick Roll! :)
			Settings.Paused = 1;
			//Pause_Screen();
			audio_clear_sound_buffer();
			ShellExecute(NULL, NULL, TEXT("http://www.youtube.com/watch?v=oHg5SJYRHA0"), NULL, NULL, SW_MAXIMIZE);
			break;
#endif /* GENS_OS_WIN32 */
		
		case IDM_FILE_CLOSEROM:
			/* TODO: NetPlay
			if (Net_Play)
			{
				if (Video.Full_Screen)
					Set_Render(0, -1, 1);
			}
			*/
			
			ROM::freeROM(Game);
			Sync_Gens_Window();
			break;
		
		case IDM_FILE_ROMHISTORY_1:
		case IDM_FILE_ROMHISTORY_2:
		case IDM_FILE_ROMHISTORY_3:
		case IDM_FILE_ROMHISTORY_4:
		case IDM_FILE_ROMHISTORY_5:
		case IDM_FILE_ROMHISTORY_6:
		case IDM_FILE_ROMHISTORY_7:
		case IDM_FILE_ROMHISTORY_8:
		case IDM_FILE_ROMHISTORY_9:
			// ROM History.
			if (audio_get_gym_playing())
				gym_play_stop();
			
			if (ROM::Recent_ROMs.size() > ((unsigned int)menuID - IDM_FILE_ROMHISTORY_1))
			{
				ROM::openROM(ROM::Recent_ROMs.at(menuID - IDM_FILE_ROMHISTORY_1).filename,
					     ROM::Recent_ROMs.at(menuID - IDM_FILE_ROMHISTORY_1).z_filename);
			}
			
			Sync_Gens_Window();
			break;
		
		case IDM_FILE_LOADSTATE:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			filename = Savestate::SelectFile(false, State_Dir);
			if (!filename.empty())
				Savestate::LoadState(filename.c_str());
			break;
		
		case IDM_FILE_SAVESTATE:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			filename = Savestate::SelectFile(true, State_Dir);
			if (!filename.empty())
				Savestate::SaveState(filename.c_str());
			break;
		
		case IDM_FILE_QUICKLOAD:
			/*
			if (Check_If_Kaillera_Running())
			return 0;
			*/
			filename = Savestate::GetStateFilename();
			Savestate::LoadState(filename);
			break;
		
		case IDM_FILE_QUICKSAVE:
			/*
			if (Check_If_Kaillera_Running())
			return 0;
			*/
			filename = Savestate::GetStateFilename();
			Savestate::SaveState(filename.c_str());
			break;
		
		case IDM_FILE_CHANGESTATE_0:
		case IDM_FILE_CHANGESTATE_1:
		case IDM_FILE_CHANGESTATE_2:
		case IDM_FILE_CHANGESTATE_3:
		case IDM_FILE_CHANGESTATE_4:
		case IDM_FILE_CHANGESTATE_5:
		case IDM_FILE_CHANGESTATE_6:
		case IDM_FILE_CHANGESTATE_7:
		case IDM_FILE_CHANGESTATE_8:
		case IDM_FILE_CHANGESTATE_9:
			// Change state.
			Options::setSaveSlot(menuID - IDM_FILE_CHANGESTATE_0);
			Sync_Gens_Window_FileMenu();
			break;
		
		case IDM_FILE_EXIT:
			close_gens();
			break;
		
		default:
			// Unknown menu item ID.
			return 0;
	}
	
	// Menu item handled.
	return 1;
}


static int gens_menu_callback_GraphicsMenu(uint16_t menuID, uint16_t state)
{
	switch (menuID)
	{
		case IDM_GRAPHICS_FULLSCREEN:
			/*
			if (Full_Screen)
				Set_Render(0, -1, 1);
			else
				Set_Render(1, Render_FS, 1);
			*/
			
			vdraw_set_fullscreen(!vdraw_get_fullscreen());
			// TODO: See if draw->setRender() is still needed.
			//draw->setRender(Video.Render_Mode);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDM_GRAPHICS_MENUBAR:
			if (vdraw_get_fullscreen())
				break;
			
			Settings.showMenuBar = !state;
			
			// Rebuild the Gens Menu Bar.
			gens_window_create_menubar();
			break;
		
		case IDM_GRAPHICS_VSYNC:
			Options::setVSync(!state);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDM_GRAPHICS_STRETCH_NONE:
		case IDM_GRAPHICS_STRETCH_H:
		case IDM_GRAPHICS_STRETCH_V:
		case IDM_GRAPHICS_STRETCH_FULL:
			Options::setStretch(menuID - IDM_GRAPHICS_STRETCH_NONE);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
#ifdef GENS_OS_UNIX
		case IDM_GRAPHICS_BPP_15:
			vdraw_set_bpp(15, true);
			Sync_Gens_Window_GraphicsMenu();
			vdraw_text_write("SDL color depth: 15-bit (555)", 1500);
			break;
		
		case IDM_GRAPHICS_BPP_16:
			vdraw_set_bpp(16, true);
			Sync_Gens_Window_GraphicsMenu();
			vdraw_text_write("SDL color depth: 16-bit (565)", 1500);
			break;
		
		case IDM_GRAPHICS_BPP_32:
			vdraw_set_bpp(32, true);
			Sync_Gens_Window_GraphicsMenu();
			vdraw_text_write("SDL color depth: 32-bit", 1500);
			break;
#endif /* GENS_OS_UNIX */
		
#ifdef GENS_OPENGL
		case IDM_GRAPHICS_OPENGL_FILTER:
			Options::setOpenGL_LinearFilter(!state);
			break;
		
		case IDM_GRAPHICS_OPENGL_RES_320x240:
			Options::setOpenGL_Resolution(320, 240);
			break;
		
		case IDM_GRAPHICS_OPENGL_RES_640x480:
			Options::setOpenGL_Resolution(640, 480);
			break;
		
		case IDM_GRAPHICS_OPENGL_RES_800x600:
			Options::setOpenGL_Resolution(800, 600);
			break;
		
		case IDM_GRAPHICS_OPENGL_RES_960x720:
			Options::setOpenGL_Resolution(960, 720);
			break;
		
		case IDM_GRAPHICS_OPENGL_RES_1024x768:
			Options::setOpenGL_Resolution(1024, 768);
			break;
		
		case IDM_GRAPHICS_OPENGL_RES_1280x960:
			Options::setOpenGL_Resolution(1280, 960);
			break;
		
		case IDM_GRAPHICS_OPENGL_RES_1280x1024:
			Options::setOpenGL_Resolution(1280, 1024);
			break;
		
		case IDM_GRAPHICS_OPENGL_RES_1400x1050:
			Options::setOpenGL_Resolution(1400, 1050);
			break;
		
		case IDM_GRAPHICS_OPENGL_RES_1600x1200:
			Options::setOpenGL_Resolution(1600, 1200);
			break;
		
		case IDM_GRAPHICS_OPENGL_RES_CUSTOM:
			glres_window_show();
			break;
#endif /* GENS_OPENGL */
		
		case IDM_GRAPHICS_COLORADJUST:
			ca_window_show();
			break;
		
		case IDM_GRAPHICS_SPRITELIMIT:
			// Sprite Limit
			Options::setSpriteLimit(!state);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDM_GRAPHICS_FRAMESKIP_AUTO:
		case IDM_GRAPHICS_FRAMESKIP_0:
		case IDM_GRAPHICS_FRAMESKIP_1:
		case IDM_GRAPHICS_FRAMESKIP_2:
		case IDM_GRAPHICS_FRAMESKIP_3:
		case IDM_GRAPHICS_FRAMESKIP_4:
		case IDM_GRAPHICS_FRAMESKIP_5:
		case IDM_GRAPHICS_FRAMESKIP_6:
		case IDM_GRAPHICS_FRAMESKIP_7:
		case IDM_GRAPHICS_FRAMESKIP_8:
			// Set the frame skip value.
			Options::setFrameSkip(menuID - IDM_GRAPHICS_FRAMESKIP_AUTO - 1);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDM_GRAPHICS_SCREENSHOT:
			audio_clear_sound_buffer();
			ImageUtil::ScreenShot();
			break;
		
		default:
			switch (menuID & 0xFF00)
			{
				case IDM_GRAPHICS_BACKEND:
				{
					// Backend change.
					Options::setBackend((VDRAW_BACKEND)((menuID & 0xFF) - 1));
					break;
				}
				case IDM_GRAPHICS_RENDER:
				{
					// Render mode change.
					// TODO: Improve performance here.
					list<mdp_render_t*>::iterator renderIter = RenderMgr::getIterFromIndex(menuID & 0x00FF);
					if (renderIter == RenderMgr::end())
						break;
					
					// Found the renderer.
					vdraw_set_renderer(renderIter);
					
					// Synchronize the Graphics Menu.
					Sync_Gens_Window_GraphicsMenu();
					
					break;
				}
				default:
					// Unknown menu item ID.
					return 0;
					break;
			}
	}
	
	// Menu item handled.
	return 1;
}


static int gens_menu_callback_CPUMenu(uint16_t menuID, uint16_t state)
{
	switch (menuID)
	{
		case IDM_CPU_COUNTRY_AUTO:
		case IDM_CPU_COUNTRY_JAPAN_NTSC:
		case IDM_CPU_COUNTRY_USA:
		case IDM_CPU_COUNTRY_EUROPE:
		case IDM_CPU_COUNTRY_JAPAN_PAL:
			Options::setCountry(menuID - IDM_CPU_COUNTRY_AUTO - 1);
			Sync_Gens_Window_CPUMenu();
			break;
		
		case IDM_CPU_COUNTRY_ORDER:
			ccode_window_show();
			break;
		
		case IDM_CPU_HARDRESET:
			Options::systemReset();
			break;
		
		case IDM_CPU_RESET68K:
		case IDM_CPU_RESETMAIN68K:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			
			if (!Game)
				break;
			
			Settings.Paused = 0;
			main68k_reset();
			
			if (Genesis_Started || _32X_Started)
				vdraw_text_write("68000 CPU reset", 1000);
			else if (SegaCD_Started)
				vdraw_text_write("Main 68000 CPU reset", 1000);
			
			break;
		
		case IDM_CPU_RESETSUB68K:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			
			if (!Game || !SegaCD_Started)
				break;
			
			Settings.Paused = 0;
			sub68k_reset();
			vdraw_text_write("Sub 68000 CPU reset", 1000);
			break;
		
		case IDM_CPU_RESETMAINSH2:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			
			if (!Game || !_32X_Started)
				break;
			
			Settings.Paused = 0;
			SH2_Reset(&M_SH2, 1);
			vdraw_text_write("Master SH2 reset", 1000);
			break;
		
		case IDM_CPU_RESETSUBSH2:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			
			if (!Game || !_32X_Started)
				break;
			
			Settings.Paused = 0;
			SH2_Reset(&S_SH2, 1);
			vdraw_text_write("Slave SH2 reset", 1000);
			break;
		
		case IDM_CPU_RESETZ80:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			
			if (!Game)
				break;
			
			mdZ80_reset(&M_Z80);
			vdraw_text_write("Z80 reset", 1000);
			break;
		
		case IDM_CPU_SEGACDPERFECTSYNC:
			Options::setSegaCD_PerfectSync(!state);
			Sync_Gens_Window_CPUMenu();
			break;
		
		default:
#ifdef GENS_DEBUGGER
			if ((menuID & 0xFF00) == IDM_CPU_DEBUG)
			{
				// Debug mode change.
				Options::setDebugMode((DEBUG_MODE)(menuID - IDM_CPU_DEBUG));
				Sync_Gens_Window_CPUMenu();
			}
			else
#endif /* GENS_DEBUGGER */
			{
				// Unknown menu item ID.
				return 0;
			}
			break;
	}
	
	// Menu item handled.
	return 1;
}


static int gens_menu_callback_SoundMenu(uint16_t menuID, uint16_t state)
{
	switch (menuID)
	{
		case IDM_SOUND_ENABLE:
			Options::setSoundEnable(!state);
			break;
		
		case IDM_SOUND_STEREO:
			Options::setSoundStereo(!state);
			break;
		
		case IDM_SOUND_Z80:
			Options::setSoundZ80(!state);
			break;
		
		case IDM_SOUND_YM2612:
			Options::setSoundYM2612(!state);
			break;
		
		case IDM_SOUND_YM2612_IMPROVED:
			Options::setSoundYM2612_Improved(!state);
			break;
		
		case IDM_SOUND_DAC:
			Options::setSoundDAC(!state);
			break;
		
		case IDM_SOUND_PSG:
			Options::setSoundPSG(!state);
			break;
		
		case IDM_SOUND_PSG_SINE:
			Options::setSoundPSG_Sine(!state);
			break;
		
		case IDM_SOUND_PCM:
			Options::setSoundPCM(!state);
			break;
		
		case IDM_SOUND_PWM:
			Options::setSoundPWM(!state);
			break;
		
		case IDM_SOUND_CDDA:
			Options::setSoundCDDA(!state);
			break;
		
		case IDM_SOUND_WAVDUMP:
			// Change WAV dump status.
			if (!WAV_Dumping)
				wav_dump_start();
			else
				wav_dump_stop();
			break;
		
		case IDM_SOUND_GYMDUMP:
			// Change GYM dump status.
			if (!GYM_Dumping)
				gym_dump_start();
			else
				gym_dump_stop();
			break;
		
		default:
			if ((menuID & 0xFF00) == IDM_SOUND_RATE)
			{
				// Sample rate change.
				Options::setSoundSampleRate(menuID - IDM_SOUND_RATE_11025);
			}
			else
			{
				// Unknown menu item ID.
				return 0;
			}
			break;
	}
	
	// Synchronize the Sound Menu
	Sync_Gens_Window_SoundMenu();
	
	// Menu item handled.
	return 1;
}


static int gens_menu_callback_OptionsMenu(uint16_t menuID, uint16_t state)
{
	GSFT_UNUSED_PARAMETER(state);
	
	switch (menuID)
	{
		case IDM_OPTIONS_GENERAL:
			genopt_window_show();
			break;
		
		case IDM_OPTIONS_JOYPADS:
			cc_window_show();
			break;
		
		case IDM_OPTIONS_BIOSMISCFILES:
			bmf_window_show();
			break;
		
		case IDM_OPTIONS_SRAM_ENABLED:
			Options::setSramEnabled(!state);
			Sync_Gens_Window_OptionsMenu();
			break;
		
#ifdef GENS_CDROM
		case IDM_OPTIONS_CURRENT_CD_DRIVE:
			selcd_window_show();
			break;
#endif /* GENS_CDROM */
		
		case IDM_OPTIONS_LOADCONFIG:
			Config::loadAs(Game);
			Sync_Gens_Window();
			break;
		
		case IDM_OPTIONS_DIRECTORIES:
			dir_window_show();
			break;
		
		case IDM_OPTIONS_SAVECONFIGAS:
			Config::saveAs();
			break;
		
		default:
			if ((menuID & 0xFF00) == IDM_OPTIONS_SEGACDSRAMSIZE)
			{
				// SegaCD SRAM Size change.
				Options::setSegaCD_SRAMSize(menuID - IDM_OPTIONS_SEGACDSRAMSIZE_NONE - 1);
				Sync_Gens_Window_OptionsMenu();
			}
			else
			{
				// Unknown menu item ID.
				return 0;
			}
			break;
	}
	
	// Menu item handled.
	return 1;
}


static int gens_menu_callback_PluginsMenu(uint16_t menuID, uint16_t state)
{
	GSFT_UNUSED_PARAMETER(state);
	
	switch (menuID)
	{
		case IDM_PLUGINS_MANAGER:
			pmgr_window_show();
			break;
		
		default:
			// Check if this menu item ID exists.
			mapMenuItems::iterator curMenuItem = PluginMgr::tblMenuItems.find(menuID);
			if (curMenuItem == PluginMgr::tblMenuItems.end())
			{
				// Unknown menu item ID.
				return 0;
			}
			
			// Found the menu item ID.
			// TODO: Handle the MDP error code, if any.
			(*((*curMenuItem).second)).handler(menuID);
			break;
	}
	
	// Menu item handled.
	return 1;
}


static int gens_menu_callback_HelpMenu(uint16_t menuID, uint16_t state)
{
	GSFT_UNUSED_PARAMETER(state);
	
	switch (menuID)
	{
#if !defined(GENS_OS_MACOSX)
		case IDM_HELP_MANUAL:
		{
			// TODO: Make this less hackish.
			
			// TODO: Install the manual in /usr/share/doc/gens using the main install script.
			// Currently, it's only done by the Debian packaging scripts.
			
#if defined(GENS_OS_WIN32)
			// Manual should be "%EXEPATH%\\manual\\index.html".
			string manual_paths[1];
			manual_paths[0] = string(PathNames.Gens_EXE_Path) + "manual\\index.html";
#else
			// Manual could be in a few different places.
			string manual_paths[2];
			manual_paths[0] = GENS_DOC_DIR "/manual/index.html";
			manual_paths[1] = GENS_DOC_DIR "-" + string(VERSION) + "/manual/index.html";
#endif
			
			int i;
			for (i = 0; i < (sizeof(manual_paths) / sizeof(manual_paths[0])); i++)
			{
				if (access(manual_paths[i].c_str(), R_OK) == 0)
				{
					// Found a working manual path.
					GensUI::LaunchBrowser(manual_paths[i].c_str());
					break;
				}
			}
			
			if (i == (sizeof(manual_paths) / sizeof(manual_paths[0])))
			{
				// No working manual path found.
				LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
					"Cannot find the Gens/GS manual.");
			}
			
			break;
		}
#endif
		
		case IDM_HELP_REPORTABUG:
			GensUI::LaunchBrowser("http://gs_server.gerbilsoft.ddns.info/bugs/");
			break;
		
		case IDM_HELP_ABOUT:
			about_window_show();
			break;
		
		default:
			// Unknown menu item ID.
			return 0;
	}
	
	// Menu item handled.
	return 1;
}

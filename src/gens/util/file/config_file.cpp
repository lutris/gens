/***************************************************************************
 * Gens: Configuration file handler.                                       *
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
#endif /* HAVE_CONFIG_H */

// C includes.
#include <stdio.h>
#include <unistd.h>

// C++ includes.
#include <string>
using std::string;

#include "save.hpp"
#include "config_file.hpp"
#include "port/ini.hpp"

#include "emulator/gens.hpp"
#include "emulator/g_main.hpp"
#include "util/gfx/imageutil.hpp"
#include "emulator/md_palette.hpp"
#include "emulator/g_update.hpp"

// CPU
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "mdZ80/mdZ80.h"

// Memory
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/mem/mem_z80.h"
#include "gens_core/io/io.h"
#include "gens_core/io/io_teamplayer.h"
#include "util/file/rom.hpp"

// VDP
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

// Audio
#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"

// SegaCD
#include "segacd/cd_sys.hpp"
#include "gens_core/gfx/gfx_cd.h"
#include "segacd/lc89510.h"
#include "segacd/cd_file.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// UI
#include "emulator/options.hpp"
#include "ui/gens_ui.hpp"

// Plugin Manager, Render Manager, and Event Manager.
#include "plugins/pluginmgr.hpp"
#include "plugins/rendermgr.hpp"
#include "plugins/eventmgr.hpp"

// Video, Audio, Input.
#include "video/vdraw.h"
#include "audio/audio.h"
#include "input/input.h"

// C++ includes
#include <deque>
#include <list>
using std::deque;
using std::list;

// libgsft includes.
#include "libgsft/gsft_file.h"
#include "libgsft/gsft_szprintf.h"

// Needed for SetCurrentDirectory.
#ifdef GENS_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif /* GENS_OS_WIN32 */


/**
 * save(): Save Gens configuration.
 * @param filename Configuration filename.
 */
int Config::save(const string& filename)
{
	char buf[256];
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	// Load the configuration file into the INI handler.
	INI cfg(filename);
	
	// Directories.
	cfg.writeString("Directories", "ROM Path", Rom_Dir);
	cfg.writeString("Directories", "Save Path", State_Dir);
	cfg.writeString("Directories", "SRAM Path", SRAM_Dir);
	cfg.writeString("Directories", "BRAM Path", BRAM_Dir);
	cfg.writeString("Directories", "Dump WAV Path", PathNames.Dump_WAV_Dir);
	cfg.writeString("Directories", "Dump GYM Path", PathNames.Dump_GYM_Dir);
	cfg.writeString("Directories", "Screenshot Path", PathNames.Screenshot_Dir);
	
	// Plugin directories.
	for (list<mdpDir_t>::iterator iter = PluginMgr::lstDirectories.begin();
	     iter != PluginMgr::lstDirectories.end(); iter++)
	{
		char dir_buf[GENS_PATH_MAX];
#ifdef GENS_OS_WIN32
		char dir_buf_rel[GENS_PATH_MAX];
#endif
		
		szprintf(buf, sizeof(buf), "~MDP:%s", (*iter).name.c_str());
		
		// Get the directory from the plugin.
		(*iter).get((*iter).id, dir_buf, sizeof(dir_buf));
		
#ifdef GENS_OS_WIN32
		// Convert the directory from an absolute pathname to a
		// relative pathname, if possible.
		gsft_file_abs_to_rel(dir_buf, PathNames.Gens_EXE_Path,
				     dir_buf_rel, sizeof(dir_buf_rel));
		
		// Save the setting.
		cfg.writeString("Directories", buf, dir_buf_rel);
#else
		// Save the setting.
		cfg.writeString("Directories", buf, dir_buf);
#endif		
	}
	
	// Genesis firmware.
	cfg.writeString("Firmware", "Genesis BIOS", BIOS_Filenames.MD_TMSS);
	
	// SegaCD firmware.
	cfg.writeString("Firmware", "USA CD BIOS", BIOS_Filenames.SegaCD_US);
	cfg.writeString("Firmware", "Europe CD BIOS", BIOS_Filenames.MegaCD_EU);
	cfg.writeString("Firmware", "Japan CD BIOS", BIOS_Filenames.MegaCD_JP);
	
	// 32X firmware.
	cfg.writeString("Firmware", "32X 68000 BIOS", BIOS_Filenames._32X_MC68000);
	cfg.writeString("Firmware", "32X Master SH2 BIOS", BIOS_Filenames._32X_MSH2);
	cfg.writeString("Firmware", "32X Slave SH2 BIOS", BIOS_Filenames._32X_SSH2);
	
	// ROM History. (Last 9 ROMs.)
	int romNum = 1;
	for (deque<ROM::Recent_ROM_t>::iterator rom = ROM::Recent_ROMs.begin();
	     rom != ROM::Recent_ROMs.end(); rom++)
	{
		szprintf(buf, sizeof(buf), "ROM %d", romNum);
		cfg.writeString("ROM History", buf, (*rom).filename);
		
		szprintf(buf, sizeof(buf), "ROM %d Compressed", romNum);
		if (!(*rom).z_filename.empty())
			cfg.writeString("ROM History", buf, (*rom).z_filename);
		else
			cfg.deleteEntry("ROM History", buf);
		
		szprintf(buf, sizeof(buf), "ROM %d Type", romNum);
		cfg.writeInt("ROM History", buf, (*rom).type, true, 4);
		
		romNum++;
	}
	
	// Make sure unused ROM entries are deleted.
	for (int i = romNum; i <= 9; i++)
	{
		szprintf(buf, sizeof(buf), "ROM %d", i);
		cfg.deleteEntry("ROM History", buf);
		
		szprintf(buf, sizeof(buf), "ROM %d Compressed", i);
		cfg.deleteEntry("ROM History", buf);
		
		szprintf(buf, sizeof(buf), "ROM %d Type", i);
		cfg.deleteEntry("ROM History", buf);
	}
	
	// Delete any empty "Compressed" entries.
	for (int i = 1; i <= 9; i++)
	{
		szprintf(buf, sizeof(buf), "ROM %d Compressed", i);
		if (cfg.getString("ROM History", buf, "").empty())
			cfg.deleteEntry("ROM History", buf);
	}
	
#ifdef GENS_CDROM
	// Physical CD-ROM support for SegaCD
#if defined(GENS_OS_WIN32)
	// Win32 ASPI uses a device ID number.
	cfg.writeInt("Options", "CD Drive", cdromDeviceID);
#elif defined(GENS_OS_UNIX)
	// Unix uses a device name.
	cfg.writeString("General", "CD Drive", cdromDeviceName);
#endif /* GENS_OS_WIN32 / GENS_OS_UNIX */
	cfg.writeInt("General", "CD Speed", cdromSpeed);
#endif /* GENS_CDROM */
	
	cfg.writeInt("General", "State Number", Current_State);
	cfg.writeInt("General", "Window X", Window_Pos.x);
	cfg.writeInt("General", "Window Y", Window_Pos.y);
	cfg.writeInt("General", "Intro Style", Intro_Style);
	cfg.writeInt("General", "Free Mode Color", vdraw_get_intro_effect_color());
	cfg.writeInt("General", "Show Menu Bar", Settings.showMenuBar);
	
	// Video adjustments
	cfg.writeInt("Graphics", "Contrast", Contrast_Level);
	cfg.writeInt("Graphics", "Brightness", Brightness_Level);
	cfg.writeInt("Graphics", "Greyscale", Greyscale & 1);
	cfg.writeInt("Graphics", "Invert", Invert_Color & 1);
	cfg.writeInt("Graphics", "Color Scale Method", (int)ColorScaleMethod);
	
	// Video settings
	cfg.writeBool("Graphics", "Full Screen", vdraw_get_fullscreen());
	cfg.writeString("Graphics", "Render Fullscreen", (*rendMode_FS)->tag);
	cfg.writeString("Graphics", "Render Windowed", (*rendMode_W)->tag);
	cfg.writeInt("Graphics", "Full Screen VSync", Video.VSync_FS & 1);
	cfg.writeInt("Graphics", "Windows VSync", Video.VSync_W & 1);
	cfg.writeBool("Graphics", "Border Color Emulation", Video.borderColorEmulation);
	cfg.writeBool("Graphics", "Pause Tint", Video.pauseTint);
	
#ifndef GENS_OS_WIN32
	cfg.writeInt("Graphics", "Bits Per Pixel", bppOut);
#endif /* GENS_OS_WIN32 */
	
	cfg.writeString("Graphics", "Backend", vdraw_backends[vdraw_cur_backend_id]->name);
#ifdef GENS_OPENGL
	cfg.writeInt("Graphics", "OpenGL Width", Video.GL.width);
	cfg.writeInt("Graphics", "OpenGL Height", Video.GL.height);
	cfg.writeInt("Graphics", "OpenGL Filter", Video.GL.glLinearFilter);
#endif /* GENS_OPENGL */
	
	cfg.writeInt("Graphics", "Stretch", Options::stretch());
#ifdef GENS_OS_WIN32
	cfg.writeBool("Graphics", "Software Blit", Options::swRender());
#endif /* GENS_OS_WIN32 */
	cfg.writeInt("Graphics", "Sprite Limit", Sprite_Over & 1);
	cfg.writeInt("Graphics", "Frame Skip", Frame_Skip);
	
	// Sound settings.
	cfg.writeBool("Sound", "State", audio_get_enabled());
	cfg.writeInt("Sound", "Rate", audio_get_sound_rate());
	cfg.writeBool("Sound", "Stereo", audio_get_stereo());
	
	cfg.writeInt("Sound", "Z80 State", Z80_State & Z80_STATE_ENABLED);
	cfg.writeInt("Sound", "YM2612 State", YM2612_Enable & 1);
	cfg.writeInt("Sound", "PSG State", PSG_Enable & 1);
	cfg.writeInt("Sound", "DAC State", DAC_Enable & 1);
	cfg.writeInt("Sound", "PCM State", PCM_Enable & 1);
	cfg.writeInt("Sound", "PWM State", PWM_Enable & 1);
	cfg.writeInt("Sound", "CDDA State", CDDA_Enable & 1);
	
	// Improved sound options.
	cfg.writeInt("Sound", "YM2612 Improvement", YM2612_Improv & 1);
	cfg.writeInt("Sound", "PSG Improvement", PSG_Improv & 1);
	
	// Country codes.
	cfg.writeInt("CPU", "Country", Country);
	cfg.writeInt("CPU", "Prefered Country 1", Country_Order[0]);
	cfg.writeInt("CPU", "Prefered Country 2", Country_Order[1]);
	cfg.writeInt("CPU", "Prefered Country 3", Country_Order[2]);
	
	// CPU options.
	cfg.writeInt("CPU", "Perfect synchro between main and sub CPU (Sega CD)", SegaCD_Accurate);
	cfg.writeInt("CPU", "Main SH2 Speed", MSH2_Speed);
	cfg.writeInt("CPU", "Slave SH2 Speed", SSH2_Speed);
	
	// Various settings.
	cfg.writeBool("Options", "Fast Blur", Options::fastBlur());
	cfg.writeBool("Options", "FPS", vdraw_get_fps_enabled());
	cfg.writeInt("Options", "FPS Style", vdraw_get_fps_style(), true, 2);
	cfg.writeBool("Options", "Message", vdraw_get_msg_enabled());
	cfg.writeInt("Options", "Message Style", vdraw_get_msg_style(), true, 2);
	cfg.writeInt("Options", "LED", Show_LED & 1);
	cfg.writeInt("Options", "Auto Fix Checksum", Auto_Fix_CS & 1);
	cfg.writeInt("Options", "Auto Pause", Auto_Pause & 1);
	
	// SRAM Enabled
	cfg.writeBool("Options", "SRAM Enabled", SRAM_Enabled);
	
	// SegaCD BRAM cartridge.
	if (BRAM_Ex_State & 0x100)
		cfg.writeInt("Options", "RAM Cart Size", BRAM_Ex_Size);
	else
		cfg.writeInt("Options", "RAM Cart Size", -1);
	
	// Miscellaneous files.
	cfg.writeString("Options", "RAR Binary", Misc_Filenames.RAR_Binary);
	
	// Controller settings.
	cfg.writeInt("Input", "P1.Type", (Controller_1_Type & 0x11), true, 2);
	cfg.writeInt("Input", "P2.Type", (Controller_2_Type & 0x11), true, 2);
	
	cfg.writeInt("Input", "P1B.Type", (Controller_1B_Type & 0x01), true, 2);
	cfg.writeInt("Input", "P1C.Type", (Controller_1C_Type & 0x01), true, 2);
	cfg.writeInt("Input", "P1D.Type", (Controller_1D_Type & 0x01), true, 2);
	
	cfg.writeInt("Input", "P2B.Type", (Controller_2B_Type & 0x01), true, 2);
	cfg.writeInt("Input", "P2C.Type", (Controller_2C_Type & 0x01), true, 2);
	cfg.writeInt("Input", "P2D.Type", (Controller_2D_Type & 0x01), true, 2);
	
	// Controller keymaps.
	for (int player = 0; player < 8; player++)
	{
		for (int button = 0; button < 12; button++)
		{
			szprintf(buf, sizeof(buf), "%s.%s", input_player_names[player], input_key_names[button]);
			cfg.writeInt("Input", buf, input_keymap[player].data[button], true, 4);
		}
	}
	
	// Restrict input. (Restricts U+D/L+R)
	cfg.writeBool("Input", "Restrict Input", Settings.restrict_input);
	
	// Tell plugins to save their configurations.
	EventMgr::RaiseEvent(MDP_EVENT_SAVE_CONFIG, NULL);
	
	// Plugin configuration.
	char MDP_section[48];
	
	for (list<mdp_t*>::iterator lstIter = PluginMgr::lstMDP.begin();
	     lstIter != PluginMgr::lstMDP.end(); lstIter++)
	{
		mapPluginConfig::iterator cfgIter;
		mdp_t *plugin = (*lstIter);
		
		// Check if the plugin has any configuration items.
		cfgIter = PluginMgr::tblPluginConfig.find(plugin);
		if (cfgIter == PluginMgr::tblPluginConfig.end())
			continue;
		
		// Write the configuration items.
		// TODO: Use a standardized function for UUID conversion.
		const unsigned char *mdp_uuid = plugin->uuid;
		szprintf(MDP_section, sizeof(MDP_section),
			 "~MDP:%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			 mdp_uuid[0], mdp_uuid[1], mdp_uuid[2], mdp_uuid[3],
			 mdp_uuid[4], mdp_uuid[5],
			 mdp_uuid[6], mdp_uuid[7],
			 mdp_uuid[8], mdp_uuid[9],
			 mdp_uuid[10], mdp_uuid[11], mdp_uuid[12], mdp_uuid[13], mdp_uuid[14], mdp_uuid[15]);
		
		// Write the configuration items.
		mapConfigItems& mapCfg = ((*cfgIter).second);
		for (mapConfigItems::iterator cfgItem = mapCfg.begin();
		     cfgItem != mapCfg.end(); cfgItem++)
		{
			cfg.writeString(MDP_section, (*cfgItem).first, (*cfgItem).second);
		}
		
		// Write the plugin information.
		if (plugin->desc && plugin->desc->name)
			cfg.writeString(MDP_section, "^MDP_Name", plugin->desc->name);
	}
	
	// Save the INI file.
	cfg.save(filename);
	
	// Done.
	return 1;
}


/**
 * saveAs(): Save the current configuration using a user-selected filename.
 * @return 1 if a file was selected.
 */
int Config::saveAs(void)
{
	string filename;
	
	filename = GensUI::saveFile("Save Config As", "", ConfigFile);
	if (filename.length() == 0)
		return 0;
	
	// Filename selected for the config file.
	save(filename);
	string dispText = "Config saved in " + filename;
	vdraw_text_write(dispText.c_str(), 2000);
	return 1;
}


/**
 * load(): Load Gens configuration.
 * @param filename Configuration filename.
 * @param Game_Active ???
 */
int Config::load(const string& filename, void* gameActive)
{
	char buf[256];
	
#ifdef GENS_OS_WIN32
	SetCurrentDirectory(PathNames.Gens_EXE_Path);
#endif /* GENS_OS_WIN32 */
	
	INI cfg(filename);
	
	CRam_Flag = 1;
	
	// Paths.
	cfg.getString("Directories", "ROM Path", PathNames.Gens_Path, Rom_Dir, sizeof(Rom_Dir));
	cfg.getString("Directories", "Save Path", PathNames.Gens_Path, State_Dir, sizeof(State_Dir));
	cfg.getString("Directories", "SRAM Path", PathNames.Gens_Path, SRAM_Dir, sizeof(SRAM_Dir));
	cfg.getString("Directories", "BRAM Path", PathNames.Gens_Path, BRAM_Dir, sizeof(BRAM_Dir));
	cfg.getString("Directories", "Dump WAV Path", PathNames.Gens_Path, PathNames.Dump_WAV_Dir, sizeof(PathNames.Dump_WAV_Dir));
	cfg.getString("Directories", "Dump GYM Path", PathNames.Gens_Path, PathNames.Dump_GYM_Dir, sizeof(PathNames.Dump_GYM_Dir));
	cfg.getString("Directories", "Screenshot Path", PathNames.Gens_Path, PathNames.Screenshot_Dir, sizeof(PathNames.Screenshot_Dir));
	
	// Plugin directories.
	for (list<mdpDir_t>::iterator iter = PluginMgr::lstDirectories.begin();
	     iter != PluginMgr::lstDirectories.end(); iter++)
	{
#ifdef GENS_OS_WIN32
		char dir_buf_abs[GENS_PATH_MAX];
#endif
		szprintf(buf, sizeof(buf), "~MDP:%s", (*iter).name.c_str());
		
		string dir_buf = cfg.getString("Directories", buf, "");
		if (!dir_buf.empty())
		{
#ifdef GENS_OS_WIN32
			// If the pathname is relative, convert it to absolute.
			gsft_file_rel_to_abs(dir_buf.c_str(), PathNames.Gens_EXE_Path,
					     dir_buf_abs, sizeof(dir_buf_abs));
			
			// Set the plugin directory.
			(*iter).set((*iter).id, dir_buf_abs);
#else
			// Set the plugin directory.
			(*iter).set((*iter).id, dir_buf.c_str());
#endif
		}
	}
	
	// Genesis firmware.
	cfg.getString("Firmware", "Genesis BIOS", "", BIOS_Filenames.MD_TMSS, sizeof(BIOS_Filenames.MD_TMSS));
	
	// SegaCD firmware.
	cfg.getString("Firmware", "USA CD BIOS", "", BIOS_Filenames.SegaCD_US, sizeof(BIOS_Filenames.SegaCD_US));
	cfg.getString("Firmware", "Europe CD BIOS", "", BIOS_Filenames.MegaCD_EU, sizeof(BIOS_Filenames.MegaCD_EU));
	cfg.getString("Firmware", "Japan CD BIOS", "", BIOS_Filenames.MegaCD_JP, sizeof(BIOS_Filenames.MegaCD_JP));
	
	// 32X firmware.
	cfg.getString("Firmware", "32X 68000 BIOS", "", BIOS_Filenames._32X_MC68000, sizeof(BIOS_Filenames._32X_MC68000));
	cfg.getString("Firmware", "32X Master SH2 BIOS", "", BIOS_Filenames._32X_MSH2, sizeof(BIOS_Filenames._32X_MSH2));
	cfg.getString("Firmware", "32X Slave SH2 BIOS", "", BIOS_Filenames._32X_SSH2, sizeof(BIOS_Filenames._32X_SSH2));
	
	// ROM History. (Last 9 ROMs.)
	ROM::Recent_ROMs.clear();
	ROM::Recent_ROM_t recentROM;
	string tmpFilename;
	
	for (int romNum = 1; romNum <= 9; romNum++)
	{
		szprintf(buf, sizeof(buf), "ROM %d", romNum);
		tmpFilename = cfg.getString("ROM History", buf, "");
		if (tmpFilename.empty())
			continue;
		
		recentROM.filename = tmpFilename;
		
		szprintf(buf, sizeof(buf), "ROM %d Compressed", romNum);
		recentROM.z_filename = cfg.getString("ROM History", buf, "");
		
		szprintf(buf, sizeof(buf), "ROM %d Type", romNum);
		recentROM.type = cfg.getInt("ROM History", buf, 0);
		
		ROM::Recent_ROMs.push_back(recentROM);
	}
	
#ifdef GENS_CDROM
	// Physical CD-ROM support for SegaCD
#if defined(GENS_OS_WIN32)
	// Win32 ASPI uses a device ID number.
	cdromDeviceID = cfg.getInt("Options", "CD Drive", 0);
#elif defined(GENS_OS_UNIX)
	// Unix uses a device name.
	cfg.getString("General", "CD Drive", "/dev/cdrom", cdromDeviceName, sizeof(cdromDeviceName));
#endif /* GENS_OS_WIN32 / GENS_OS_UNIX */
	cdromSpeed = cfg.getInt("General", "CD Speed", 0);
#endif /* GENS_CDROM */
	
	Current_State = cfg.getInt("General", "State Number", 0);
	Window_Pos.x = cfg.getInt("General", "Window X", 0);
	Window_Pos.y = cfg.getInt("General", "Window Y", 0);
	Intro_Style = cfg.getInt("General", "Intro Style", 0);
	vdraw_set_intro_effect_color(cfg.getInt("General", "Free Mode Color", 7));
	Settings.showMenuBar = cfg.getInt("General", "Show Menu Bar", 1);
	
	// Video adjustments.
	Contrast_Level = cfg.getInt("Graphics", "Contrast", 100);
	Brightness_Level = cfg.getInt("Graphics", "Brightness", 100);
	Greyscale = cfg.getInt("Graphics", "Greyscale", 0);
	Invert_Color = cfg.getInt("Graphics", "Invert", 0);
	ColorScaleMethod = (ColorScaleMethod_t)cfg.getInt("Graphics", "Color Scale Method", COLSCALE_FULL_HS);
	
	// Video settings.
	Video.VSync_FS = cfg.getInt("Graphics", "Full Screen VSync", 0) & 1;
	Video.VSync_W = cfg.getInt("Graphics", "Windows VSync", 0) & 1;
	vdraw_set_fullscreen(cfg.getBool("Graphics", "Full Screen", false));
	Video.borderColorEmulation = cfg.getBool("Graphics", "Border Color Emulation", true);
	Video.pauseTint = cfg.getBool("Graphics", "Pause Tint", true);
	
	// Renderer: Full Screen.
	string renderTag = cfg.getString("Graphics", "Render Fullscreen", "Double");
	rendMode_FS = RenderMgr::getIterFromTag(renderTag);
	if (rendMode_FS == RenderMgr::end())
	{
		// Invalid mode. Set to Normal mode.
		rendMode_FS = RenderMgr::begin();
	}
	
	// Renderer: Windowed.
	renderTag = cfg.getString("Graphics", "Render Windowed", "Double");
	rendMode_W = RenderMgr::getIterFromTag(renderTag);
	if (rendMode_W == RenderMgr::end())
	{
		// Invalid mode. Set to Normal mode.
		rendMode_W = RenderMgr::begin();
	}
	
#ifndef GENS_OS_WIN32
	// TODO: Add a 555/565 override for Win32.
	bppOut = (unsigned char)(cfg.getInt("Graphics", "Bits Per Pixel", 16));
	if (bppOut != 15 && bppOut != 16 && bppOut != 32)
	{
		// Invalid bpp. Set it to 16 by default.
		bppOut = 16;
	}
	bppMD = bppOut;
#endif /* GENS_OS_WIN32 */
	
	string backend = cfg.getString("Graphics", "Backend", "");
	// Determine the initial backend ID.
	if (!backend.empty())
	{
		int backendID = 0;
		while (vdraw_backends[backendID])
		{
			if (strncasecmp(backend.c_str(),
					vdraw_backends[backendID]->name,
					backend.length()) == 0)
			{
				// Found the correct backend.
				vdraw_cur_backend_id = (VDRAW_BACKEND)backendID;
				break;
			}
			
			// Check the next backend.
			backendID++;
		}
		
		if (!vdraw_backends[backendID])
		{
			// Backend not found. Use the first backend.
			vdraw_cur_backend_id = (VDRAW_BACKEND)0;
		}
	}
	else
	{
		// No backend saved in the config file. Use the first backend.
		vdraw_cur_backend_id = (VDRAW_BACKEND)0;
	}
	
#ifdef GENS_OPENGL
	Video.GL.width = cfg.getInt("Graphics", "OpenGL Width", 640);
	Video.GL.height = cfg.getInt("Graphics", "OpenGL Height", 480);
	Video.GL.glLinearFilter = cfg.getInt("Graphics", "OpenGL Filter", 0);
	
	// Set the OpenGL renderer.
	// NOTE: Don't do this while Gens is loading; otherwise, GTK+ raises an assert
	// because the window hasn't been created yet.
//	if (is_gens_running())
// TODO		Options::setOpenGL(Video.OpenGL);
#endif
	
	//Set_Render(Full_Screen, -1, 1);
	
	// Recalculate the MD and 32X palettes using the new color and video mode settings.
	// NOTE: This is only done if Gens is currently running.
	// If Gens isn't running, then this is before Gens had a chance to parse
	// command line arguments, so things may change later.
	if (is_gens_running())
		Recalculate_Palettes();
	
	Options::setStretch(cfg.getInt("Graphics", "Stretch", STRETCH_H));
#ifdef GENS_OS_WIN32
	Options::setSwRender(cfg.getBool("Graphics", "Software Blit", false));
#endif /* GENS_OS_WIN32 */
	Sprite_Over = cfg.getInt("Graphics", "Sprite Limit", 1);
	Frame_Skip = cfg.getInt("Graphics", "Frame Skip", -1);
	
	// Sound settings.
	audio_set_sound_rate(cfg.getInt("Sound", "Rate", 22050));
	audio_set_stereo(cfg.getBool("Sound", "Stereo", true));
	
	if (cfg.getInt("Sound", "Z80 State", 1))
		Z80_State |= Z80_STATE_ENABLED;
	else
		Z80_State &= ~Z80_STATE_ENABLED;
	
	// Only load the IC sound settings if sound can be initialized.
	// TODO: Change it to load the settings unconditionally?
	int new_val = cfg.getInt("Sound", "State", 1);
	if (new_val == audio_get_enabled() ||
	    (new_val != audio_get_enabled() && Options::setSoundEnable(true)))
	{
		YM2612_Enable = cfg.getInt("Sound", "YM2612 State", 1);
		PSG_Enable = cfg.getInt("Sound", "PSG State", 1);
		DAC_Enable = cfg.getInt("Sound", "DAC State", 1);
		PCM_Enable = cfg.getInt("Sound", "PCM State", 1);
		PWM_Enable = cfg.getInt("Sound", "PWM State", 1);
		CDDA_Enable = cfg.getInt("Sound", "CDDA State", 1);
		
		// Improved sound options
		YM2612_Improv = cfg.getInt("Sound", "YM2612 Improvement", 0);
		PSG_Improv = cfg.getInt("Sound", "PSG Improvement", 0);
	}
	
	// Country codes.
	Country = cfg.getInt("CPU", "Country", -1);
	Country_Order[0] = cfg.getInt("CPU", "Prefered Country 1", 0);
	Country_Order[1] = cfg.getInt("CPU", "Prefered Country 2", 1);
	Country_Order[2] = cfg.getInt("CPU", "Prefered Country 3", 2);
	Check_Country_Order();
	
	// CPU options.
	SegaCD_Accurate = cfg.getInt("CPU", "Perfect synchro between main and sub CPU (Sega CD)", 0);
	MSH2_Speed = cfg.getInt("CPU", "Main SH2 Speed", 100);
	SSH2_Speed = cfg.getInt("CPU", "Slave SH2 Speed", 100);
	
	// Make sure the SH2 speeds aren't below 0.
	if (MSH2_Speed < 0)
		MSH2_Speed = 0;
	if (SSH2_Speed < 0)
		SSH2_Speed = 0;
	
	// Various settings.
	Options::setFastBlur(cfg.getBool("Options", "Fast Blur", false));
	vdraw_set_fps_enabled(cfg.getBool("Options", "FPS", false));
	vdraw_set_fps_style(cfg.getInt("Options", "FPS Style", 0x10));
	vdraw_set_msg_enabled(cfg.getBool("Options", "Message", true));
	vdraw_set_msg_style(cfg.getInt("Options", "Message Style", 0x10));
	Show_LED = cfg.getInt("Options", "LED", 1);
	Auto_Fix_CS = cfg.getInt("Options", "Auto Fix Checksum", 0);
	Auto_Pause = cfg.getInt("Options", "Auto Pause", 0);
	Zero_Length_DMA = cfg.getInt("Options", "Zero-Length DMA", 0);
	
	// SRAM Enabled
	SRAM_Enabled = cfg.getBool("Options", "SRAM Enabled", true);
	
	// SegaCD BRAM cartridge size
	BRAM_Ex_Size = cfg.getInt("Options", "RAM Cart Size", 3);
	if (BRAM_Ex_Size == -1)
	{
		BRAM_Ex_State &= 1;
		BRAM_Ex_Size = 0;
	}
	else if (BRAM_Ex_Size < -1 || BRAM_Ex_Size > 3)
		BRAM_Ex_Size = 3;
	else
		BRAM_Ex_State |= 0x100;
	
	// Miscellaneous files.
#if defined(__WIN32__)
	cfg.getString("Options", "RAR Binary", "C:\\Program Files\\WinRAR\\rar.exe",
		      Misc_Filenames.RAR_Binary, sizeof(Misc_Filenames.RAR_Binary));
#else /* !defined(__WIN32__) */
	cfg.getString("Options", "RAR Binary", "/usr/bin/rar",
		      Misc_Filenames.RAR_Binary, sizeof(Misc_Filenames.RAR_Binary));
#endif	
	
	// Controller settings.
	Controller_1_Type = cfg.getInt("Input", "P1.Type", 0x01);
	Controller_2_Type = cfg.getInt("Input", "P2.Type", 0x01);
	
	Controller_1B_Type = cfg.getInt("Input", "P1B.Type", 0x01);
	Controller_1C_Type = cfg.getInt("Input", "P1C.Type", 0x01);
	Controller_1D_Type = cfg.getInt("Input", "P1D.Type", 0x01);
	
	Controller_2B_Type = cfg.getInt("Input", "P2B.Type", 0x01);
	Controller_2C_Type = cfg.getInt("Input", "P2C.Type", 0x01);
	Controller_2D_Type = cfg.getInt("Input", "P2D.Type", 0x01);
	
	// Controller keymaps.
	const input_keymap_t *cur_def_keymap = input_keymap_default;
	for (int player = 0; player < 8; player++)
	{
		for (int button = 0; button < 12; button++)
		{
			szprintf(buf, sizeof(buf), "%s.%s", input_player_names[player], input_key_names[button]);
			input_keymap[player].data[button] = input_update_joykey_format(
					cfg.getInt("Input", buf, cur_def_keymap->data[button]));
		}
		
		// Next default keymap.
		cur_def_keymap++;
	}
	
	// Restrict input. (Restricts U+D/L+R)
	Settings.restrict_input = cfg.getBool("Input", "Restrict Input", true);
	
	// Create the TeamPlayer I/O table.
	Make_IO_Table();
	
	// Plugin configuration.
	char MDP_section[48];
	
	for (list<mdp_t*>::iterator lstIter = PluginMgr::lstMDP.begin();
	     lstIter != PluginMgr::lstMDP.end(); lstIter++)
	{
		mdp_t *plugin = (*lstIter);
		
		// Write the configuration items.
		// TODO: Use a standardized function for UUID conversion.
		const unsigned char *mdp_uuid = plugin->uuid;
		szprintf(MDP_section, sizeof(MDP_section),
			 "~MDP:%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			 mdp_uuid[0], mdp_uuid[1], mdp_uuid[2], mdp_uuid[3],
			 mdp_uuid[4], mdp_uuid[5],
			 mdp_uuid[6], mdp_uuid[7],
			 mdp_uuid[8], mdp_uuid[9],
			 mdp_uuid[10], mdp_uuid[11], mdp_uuid[12], mdp_uuid[13], mdp_uuid[14], mdp_uuid[15]);
		
		// Check if the plugin has any configuration items.
		if (!cfg.sectionExists(MDP_section))
			continue;
		
		// Get the configuration section.
		iniSection cfgSect = cfg.getSection(MDP_section);
		for (iniSection::iterator cfgItem = cfgSect.begin();
		     cfgItem != cfgSect.end(); cfgItem++)
		{
			PluginMgr::tblPluginConfig[plugin][(*cfgItem).first] = (*cfgItem).second;
		}
		
		// Delete the "^MDP_name" key from the configuration.
		PluginMgr::tblPluginConfig[plugin].erase("^MDP_name");
	}
	
	// Plugin configuration is loaded.
	EventMgr::RaiseEvent(MDP_EVENT_LOAD_CONFIG, NULL);
	
	// Done.
	return 1;
}


/**
 * Load_As_Config(): Load a user-selected configuration file.
 * @param Game_Active ???
 * @return 1 if a file was selected.
 */
int Config::loadAs(void* gameActive)
{
	string filename;
	
	filename = GensUI::openFile("Load Config", "", ConfigFile);
	if (filename.length() == 0)
		return 0;
	
	// Filename selected for the config file.
	load(filename, gameActive);
	string dispText = "Config loaded from " + filename;
	vdraw_text_write(dispText.c_str(), 2000);
	return 1;
}

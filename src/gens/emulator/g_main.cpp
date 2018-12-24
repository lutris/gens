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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Message logging.
#include "macros/log_msg.h"

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

// C includes.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

// C++ includes.
#include <string>
using std::string;

#include "g_main.hpp"
#include "gens.hpp"
#include "g_md.hpp"
#include "g_mcd.hpp"
#include "g_32x.hpp"
#include "gens_core/misc/cpuflags.h"
#include "gens_core/vdp/vdp_rend.h"
#include "util/file/save.hpp"
#include "util/file/config_file.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/sound/gym.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/sound/ym2612.hpp"
#include "parse.hpp"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/z80/cpu_z80.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pwm.h"
#include "debugger/debugger.hpp"

// INI handling.
#include "port/ini.hpp"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// Gens UI
#include "gens_ui.hpp"

// Update Emulation functions
#include "g_update.hpp"

// Plugin Manager.
#include "plugins/pluginmgr.hpp"
#include "plugins/rendermgr.hpp"

// File management functions.
#include "util/file/file.hpp"

// Video, Audio, and Input backends.
#include "video/vdraw.h"
#include "audio/audio.h"
#include "input/input.h"

// Video effects.
#include "video/v_effects.hpp"

// Gens Settings structs.
Gens_Settings_t Settings;
Gens_PathNames_t PathNames;
Gens_BIOS_Filenames_t BIOS_Filenames;
Gens_Misc_Filenames_t Misc_Filenames;
Gens_VideoSettings_t Video;

// bpp settings.
uint8_t bppMD;	// MD bpp
uint8_t bppOut;	// Output bpp.

// Renderers
#include <list>
using std::list;
list<mdp_render_t*>::iterator rendMode_FS;
list<mdp_render_t*>::iterator rendMode_W;

int fast_forward = 0;

POINT Window_Pos;

char **language_name = NULL;
int Full_Screen = 0;
int Resolution = 1;
int Show_LED = 0;
int Auto_Pause = 0;
int Auto_Fix_CS = 0;
int Country = -1;
int Country_Order[3];
int Intro_Style = 2;
int SegaCD_Accurate = 0;
int Quick_Exit = 0;
#if 0// TODO: Replace with MDP "exclusive mode" later.
int Net_Play = 0;
int Kaillera_Client_Running = 0;
#endif

static int Gens_Running = 0;


/**
 * Init_PRNG(): Initialize the Pseudo-Random Number Generator.
 */
void Init_PRNG(void)
{
#ifdef HAVE_LIBRT
	struct timespec now = {0, 0};
	clock_gettime(CLOCK_REALTIME, &now);
	srand(now.tv_nsec != 0 ? now.tv_nsec : now.tv_sec);
#else
	struct timeval now = {0, 0};
	gettimeofday(&now, NULL);
	srand(now.tv_usec != 0 ? now.tv_usec : now.tv_sec);
#endif
}


/**
 * Init_Settings(): Initialize the Settings struct.
 * @return 0 on success; non-zero on error.
 */
int Init_Settings(void)
{
	// Initialize video settings.
	Video.borderColorEmulation = 1;
	Video.pauseTint = 1;
#ifdef GENS_OPENGL
	Video.GL.width = 640;
	Video.GL.height = 480;
	Video.GL.glLinearFilter = 0;
#endif
	
	// Assume active and not paused initially.
	Settings.Active = 1;
	Settings.Paused = 0;
	
	// Default bpp.
	bppMD = 32;
	bppOut = 32;
	
	// Old code from InitParameters().
	VDP_Num_Vis_Lines = 224;
#if 0	// TODO: Replace with MDP "exclusive mode" later.
	Net_Play = 0;
#endif
	Sprite_Over = 1;
	
	GYM_Dumping = 0;
	
	Game = NULL;
	Genesis_Started = 0;
	SegaCD_Started = 0;
	_32X_Started = 0;
	CPU_Mode = 0;
	STOP_DEBUGGING();
	
	// Get the default save path.
	get_default_save_path(PathNames.Gens_Path, sizeof(PathNames.Gens_Path));
	
	// Create default language filename.
	szprintf(PathNames.Language_Path, sizeof(PathNames.Language_Path),
		 "%s%s", PathNames.Gens_Path, "language.dat");
	
	// Get the CPU flags.
	getCPUFlags();
	
	// Initialize the Plugin Manager.
	PluginMgr::init();
	
	// Set the default renderers.
	if (RenderMgr::empty())
	{
		// No render plugins found.
		LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
			"Fatal Error: No render plugins found.");
		return 1;	// TODO: Replace with a better error code.
	}
	rendMode_FS = RenderMgr::begin();
	rendMode_W = RenderMgr::begin();
	
	// Load the default configuration.
#ifndef PACKAGE_NAME
#error PACKAGE_NAME not defined!
#endif
	string cfg_filename = string(PathNames.Gens_Path) + PACKAGE_NAME + ".cfg";
	Config::load(cfg_filename, NULL);
	
	// Success.
	return 0;
}


/**
 * close_gens(): Close GENS.
 */
void close_gens(void)
{
	Gens_Running = 0;
}

/**
 * run_gens(): Run GENS.
 */
static inline void run_gens(void)
{
	Gens_Running = 1;
}

/**
 * is_gens_running(): Is GENS running?!
 * @return 1 if it's running.
 */
int is_gens_running(void)
{
	return Gens_Running;
}


/**
 * Init(): Initialize Gens.
 * @return 1 if successful; 0 on errors.
 */
int Init(void)
{
	MSH2_Init();
	SSH2_Init();
	M68K_Init();
	S68K_Init();
	Z80_Init();
	
	YM2612_Init(CLOCK_NTSC / 7, audio_get_sound_rate(), YM2612_Improv);
	PSG_Init(CLOCK_NTSC / 15, audio_get_sound_rate());
	PWM_Init();
	
	// Initialize the CD-ROM drive, if available.
#ifdef GENS_CDROM
	Init_CD_Driver();
#endif
	
	Init_Tab();
	
	run_gens();
	return 1;
}


/**
 * End_All(): Close all functions.
 */
void End_All(void)
{
	ROM::freeROM(Game);
	YM2612_End();
#ifdef GENS_CDROM
	End_CD_Driver();
#endif
	
	// Shut down the Plugin Manager.
	PluginMgr::end();
	
	// Shut down the input subsystem.
	input_end();
	
	// Shut down the audio subsystem.
	audio_end();
	
	// Shut down the video subsystem.
	vdraw_backend_end();
	if (vdraw_shutdown)
		vdraw_shutdown();
}


/**
 * IsAsyncAllowed(): Determines if asynchronous stuff is allowed.
 * @return 0 if not allowed; non-zero otherwise.
 */
int IsAsyncAllowed(void)
{
	// In GENS Re-Recording, async is disabled if:
	// - Async is explicitly disabled due to testing for desyncs.
	// - A movie is being played or recorded.
	// Since none of the above applies here, async is allowed.
	return 1;

	// TODO
#if 0
	if(MainMovie.Status == MOVIE_RECORDING)
		return false;
	if(MainMovie.Status == MOVIE_PLAYING)
		return false;
#endif
}


// The following is stuff imported from g_sdldraw.c.
// TODO: Figure out where to put it.
int (*Update_Frame)(void);
int (*Update_Frame_Fast)(void);

// TODO: Only used for DirectDraw.
int Flag_Clr_Scr = 0;

// VSync flags
int FS_VSync;
int W_VSync;


/**
 * Clear_Screen_MD(): Clears the MD screen.
 */
int ice = 0;
void Clear_Screen_MD(void)
{
	memset(MD_Screen, 0x00, sizeof(MD_Screen));
	memset(MD_Screen32, 0x00, sizeof(MD_Screen32));
}


/**
 * get_mdp_render_t(): Get the current mdp_render_t*. (C wrapper)
 * @return Current mdp_render_t*.
 */
mdp_render_t* get_mdp_render_t(void)
{
	const list<mdp_render_t*>::iterator& rendMode = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
	return (*rendMode);
}


/**
 * check_startup_mode(): Check the startup mode.
 * @param startup Pointer to Gens_StartupInfo_t struct containing the startup mode.
 */
void check_startup_mode(Gens_StartupInfo_t *startup)
{
	if (!startup)
	{
		// No startup mode. Assume idle.
		return;
	}
	
	// Check the startup mode.
	switch (startup->mode)
	{
		case GSM_ROM:
			// Startup ROM specified.
			if (startup->filename[0] != 0x00)
			{
				if (ROM::openROM(startup->filename) == -1)
				{
					// Could not open the startup ROM.
					// TODO: Show a message box?
					LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
						"Failed to load ROM '%s'.", startup->filename);
				}
			}
			break;
		
#ifdef GENS_CDROM
		case GSM_BOOT_CD:
			// Boot SegaCD.
			if (!ASPI_Initialized || !Num_CD_Drive)
			{
				fprintf(stderr, "%s: ASPI not initialized and/or no CD-ROM drive(s) detected.\n", __func__);
				break;
			}
			
			SegaCD_Started = Init_SegaCD(NULL);
			break;
#endif
		
		case GSM_IDLE:
		case GSM_MAX:
		default:
			// Idle.
			break;
	}
}

/**
 * GensLoopIteration(): One iteration of the main program loop.
 */
void GensLoopIteration(void)
{
	// Update the UI.
	GensUI::update();
	
	// Update physical controller inputs.
	input_update();
	
#ifdef GENS_DEBUGGER
	if (IS_DEBUGGING())
	{
		// DEBUG
		Update_Debug_Screen();
		vdraw_flip(1);
		GensUI::sleep(100);
	}
	else
#endif /* GENS_DEBUGGER */
	if (Game != NULL)
	{
		if (Settings.Active && !Settings.Paused)
		{
			// EMULATION ACTIVE
			if (fast_forward)
				Update_Emulation_One();
			else
				Update_Emulation();
			
#ifdef GENS_OS_UNIX
			// Prevent 100% CPU usage.
			// The CPU scheduler will take away CPU time from Gens/GS if
			// it notices that the process is eating up too much CPU time.
			GensUI::sleep(1, true);
#endif
		}
		else
		{
			// EMULATION PAUSED
			if (_32X_Started)
				Do_32X_VDP_Only();
			else
				Do_VDP_Only();
			
			if (Settings.Paused && Video.pauseTint)
			{
				// Emulation is paused.
				veffect_pause_tint();
			}
			
			vdraw_flip(1);
			GensUI::sleep(250);
		}
	}
	else
	{
		// No game is currently running.
		
		// Update the screen.
		vdraw_flip(1);
		
		// Determine how much sleep time to add, based on intro style.
		// TODO: Move this to v_draw.cpp?
		if (audio_get_gym_playing())
		{
			// PLAY GYM
			// TODO: Does this even do anything?
			gym_play();
		}
		else if (Intro_Style == 1)
		{
			// Gens Logo effect.
			GensUI::sleep(5);
		}
		else if (Intro_Style == 2)
		{
			// "Strange" effect.
			GensUI::sleep(10);
		}
		else if (Intro_Style == 3)
		{
			// Genesis BIOS. (TODO: This is broken!)
			GensUI::sleep(20);
		}
		else
		{
			// Blank screen. (MAX IDLE)
			GensUI::sleep(250);
		}
	}
}


/**
 * GensMainLoop(): The main program loop.
 */
void GensMainLoop(void)
{
	while (is_gens_running())
	{
		GensLoopIteration();
	}
}

/***************************************************************************
 * Gens: Option adjustment functions.                                      *
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

// C includes.
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

// C++ includes.
#include <string>
#include <list>
using std::string;
using std::list;

#include "g_main.hpp"
#include "options.hpp"
#include "gens.hpp"
#include "g_md.hpp"
#include "g_mcd.hpp"
#include "g_32x.hpp"
#include "segacd/cd_sys.hpp"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"

#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"

#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "util/file/save.hpp"

#include "ui/gens_ui.hpp"
#include "gens/gens_window_sync.hpp"

// Video, Audio.
#include "video/vdraw.h"
#include "video/vdraw_cpp.hpp"
#include "audio/audio.h"

// WAV dumping.
#include "util/sound/wave.h"

// Render Manager.
#include "plugins/rendermgr.hpp"


/**
 * spriteLimit(): Get the current sprite limit setting.
 * @return Sprite limit setting.
 */
bool Options::spriteLimit(void)
{
	return Sprite_Over;
}

/**
 * setSpriteLimit(): Enable/Disable the sprite limit.
 * @param newSpriteLimit True to enable; False to disable.
 */
void Options::setSpriteLimit(const bool newSpriteLimit)
{
	Sprite_Over = newSpriteLimit;
	
	if (Sprite_Over)
		vdraw_text_write("Sprite Limit Enabled", 1000);
	else
		vdraw_text_write("Sprite Limit Disabled", 1000);
}


/**
 * saveSlot(): Get the current savestate slot.
 * @return Current savestate slot.
 */
int Options::saveSlot(void)
{
	return Current_State;
}

/**
 * setSaveSlot(): Set the current savestate slot.
 * @param newSaveSlot Savestate slot.
 */
void Options::setSaveSlot(const int newSaveSlot)
{
	// Make sure the slot number is in bounds.
	if (newSaveSlot < 0 || newSaveSlot > 9)
		return;
	
	// Set the new savestate slot number.
	Current_State = newSaveSlot;
	
	// Check if the savestate exists.
	string filename = Savestate::GetStateFilename();
	if (!access(filename.c_str(), F_OK))
	{
		// File exists.
		vdraw_text_printf(1500, "SLOT %d [OCCUPIED]", Current_State);
	}
	else
	{
		// Check what the error value is.
		if (errno == ENOENT)
		{
			// File doesn't exist.
			vdraw_text_printf(1500, "SLOT %d [EMPTY]", Current_State);
		}
		else
		{
			// Error checking the file.
			vdraw_text_printf(1500, "SLOT %d [ERROR]", Current_State);
		}
	}
}


/**
 * frameSkip(): Get the frame skip setting.
 * @return Frame skip setting. (-1 == Auto)
 */
int Options::frameSkip(void)
{
	return Frame_Skip;
}

/**
 * setFrameSkip(): Set the frame skip setting.
 * @param newFrameSkip Frames to skip. (-1 == Auto)
 */
void Options::setFrameSkip(const int newFrameSkip)
{
	// Make sure the frame skip setting is in bounds.
	if (newFrameSkip < -1 || newFrameSkip > 8)
		return;
	
	Frame_Skip = newFrameSkip;
	
	if (Frame_Skip != -1)
		vdraw_text_printf(1500, "Frame skip set to %d", Frame_Skip);
	else
		vdraw_text_write("Frame skip set to Auto", 1500);
}


#ifdef GENS_DEBUGGER
/**
 * debugMode(): Get the current debugging mode.
 * @return Current debugging mode.
 */
DEBUG_MODE Options::debugMode(void)
{
	return debug_mode;
}

/**
 * setDebugMode(): Set the current debugging mode.
 * @param newDebugMode New debug mode.
 */
void Options::setDebugMode(const DEBUG_MODE newDebugMode)
{
	if (!Game)
		return;
	
	Flag_Clr_Scr = 1;
	audio_clear_sound_buffer();
	
	if (newDebugMode == debug_mode)
		debug_mode = DEBUG_NONE;
	else
		debug_mode = newDebugMode;
}
#endif /* GENS_DEBUGGER */


/**
 * country(): Get the current country code.
 * @return Current country code.
 */
int Options::country(void)
{
	return Country;
}

/**
 * setCountry(): Set the current country code.
 * @param newCountry New country code.
 */
void Options::setCountry(const int newCountry)
{
	Flag_Clr_Scr = 1;
	
	Country = newCountry;
	switch (Country)
	{
		default:
		case -1:
			// Auto-detect.
			if (Genesis_Started || _32X_Started)
				Detect_Country_Genesis(Game);
			else if (SegaCD_Started)
				Detect_Country_SegaCD();
			break;
		
		case 0:
			// Japan (NTSC)
			Game_Mode = 0;
			CPU_Mode = 0;
			break;
		
		case 1:
			// USA (NTSC)
			Game_Mode = 1;
			CPU_Mode = 0;
			break;
		
		case 2:
			// Europe (PAL)
			Game_Mode = 1;
			CPU_Mode = 1;
			break;
		
		case 3:
			// Japan (PAL)
			Game_Mode = 0;
			CPU_Mode = 1;
			break;
	}
	
	// TODO: Combine this with gens.cpp::Set_Clock_Freq().
	if (CPU_Mode)
	{
		CPL_Z80 = (int)rint((((double)CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = (int)rint((((double)CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 = (int)rint(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
					(double)MSH2_Speed) / 100.0);
		CPL_SSH2 = (int)rint(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
					(double)SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;
		_32X_VDP.Mode &= ~0x8000;
		
		CD_Access_Timer = 2080;
		Timer_Step = 136752;
	}
	else
	{
		CPL_Z80 = (int)rint((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = (int)rint((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		CPL_MSH2 = (int)rint(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
					(double)MSH2_Speed) / 100.0);
		CPL_SSH2 = (int)rint(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
					(double)SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 262;
		VDP_Status &= 0xFFFE;
		_32X_VDP.Mode |= 0x8000;
		
		CD_Access_Timer = 2096;
		Timer_Step = 135708;
	}
	
	if (audio_get_enabled())
	{
		uint32_t psg_state[8];
		uint8_t ym2612_reg[0x200];
		
		PSG_Save_State(psg_state);
		YM2612_Save(ym2612_reg);
		
		audio_end();
		audio_set_enabled(false);
		
		if (CPU_Mode)
		{
			YM2612_Init(CLOCK_PAL / 7, audio_get_sound_rate(), YM2612_Improv);
			PSG_Init(CLOCK_PAL / 15, audio_get_sound_rate());
		}
		else
		{
			YM2612_Init(CLOCK_NTSC / 7, audio_get_sound_rate(), YM2612_Improv);
			PSG_Init(CLOCK_NTSC / 15, audio_get_sound_rate());
		}
		
		if (SegaCD_Started)
			PCM_Set_Rate(audio_get_sound_rate());
		
		YM2612_Restore(ym2612_reg);
		PSG_Restore_State(psg_state);
		
		if (audio_init(AUDIO_BACKEND_DEFAULT))
			return;
		
		audio_set_enabled(true);
		if (audio_play_sound)
			audio_play_sound();
	}
	
	if (Game_Mode)
	{
		if (CPU_Mode)
			vdraw_text_write("Europe system (50 FPS)", 1500);
		else
			vdraw_text_write("USA system (60 FPS)", 1500);
	}
	else
	{
		if (CPU_Mode)
			vdraw_text_write("Japan system (50 FPS)", 1500);
		else
			vdraw_text_write("Japan system (60 FPS)", 1500);
	}
	
	setGameName();
	return;
}


/** Sound **/


/**
 * soundEnable(): Get the sound enable setting.
 * @return Sound enable setting.
 */
bool Options::soundEnable(void)
{
	return audio_get_enabled();
}

/**
 * setSoundEnable(): Enable or disable sound.
 * @param newSoundEnable New sound enable setting.
 * @return 1 on success; 0 on error.
 */
int Options::setSoundEnable(const bool newSoundEnable)
{
	if (newSoundEnable == audio_get_enabled())
		return 0;
	
	// Make sure WAV dumping has stopped.
	if (WAV_Dumping)
		wav_dump_stop();
	
	audio_set_enabled(newSoundEnable);
	
	if (!audio_get_enabled())
	{
		audio_end();
		YM2612_Enable = 0;
		PSG_Enable = 0;
		DAC_Enable = 0;
		PCM_Enable = 0;
		PWM_Enable = 0;
		CDDA_Enable = 0;
		vdraw_text_write("Sound Disabled", 1500);
	}
	else
	{
		if (audio_init(AUDIO_BACKEND_DEFAULT))
		{
			// Error initializing sound.
			audio_set_enabled(false);
			YM2612_Enable = 0;
			PSG_Enable = 0;
			DAC_Enable = 0;
			PCM_Enable = 0;
			PWM_Enable = 0;
			CDDA_Enable = 0;
			return 0;
		}
		
		if (audio_play_sound)
			audio_play_sound();
		
		// Make sure Z80 sound emulation is enabled.
		if (!(Z80_State & Z80_STATE_ENABLED))
			setSoundZ80(true);
		
		YM2612_Enable = 1;
		PSG_Enable = 1;
		DAC_Enable = 1;
		PCM_Enable = 1;
		PWM_Enable = 1;
		CDDA_Enable = 1;
		
		vdraw_text_write("Sound Enabled", 1500);
	}
	
	return 1;
}


/**
 * soundStereo(): Get the stereo sound setting.
 * @return Stereo sound setting.
 */
bool Options::soundStereo(void)
{
	return audio_get_stereo();
}

/**
 * setSoundStereo(): Enable or disable stereo sound.
 * @param newSoundStereo New stereo sound enable setting.
 */
void Options::setSoundStereo(const bool newSoundStereo)
{
	if (newSoundStereo == audio_get_stereo())
		return;
	
	// Make sure WAV dumping has stopped.
	if (WAV_Dumping)
		wav_dump_stop();
	
	// TODO: Move most of this code to the Audio class.
	
	audio_set_stereo(newSoundStereo);
	
	if (!audio_get_stereo())
		vdraw_text_write("Mono sound", 1000);
	else
		vdraw_text_write("Stereo sound", 1000);
	
	if (!audio_get_enabled())
	{
		// Sound isn't enabled, so nothing needs to be changed.
		return;
	}
	
	uint32_t psg_state[8];
	uint8_t ym2612_reg[0x200];
	
	// Save the current sound state.
	// TODO: Use full save instead of partial save?
	PSG_Save_State(psg_state);
	YM2612_Save(ym2612_reg);
	
	// Temporarily disable sound.
	audio_end();
	audio_set_enabled(false);
	
	// Reinitialize the sound processors.
	if (CPU_Mode)
	{
		YM2612_Init(CLOCK_PAL / 7, audio_get_sound_rate(), YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, audio_get_sound_rate());
	}
	else
	{
		YM2612_Init(CLOCK_NTSC / 7, audio_get_sound_rate(), YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, audio_get_sound_rate());
	}
	
	if (SegaCD_Started)
		PCM_Set_Rate(audio_get_sound_rate());
	
	// Restore the sound state.
	YM2612_Restore(ym2612_reg);
	PSG_Restore_State(psg_state);
	
	// Attempt to re-enable sound.
	if (audio_init(AUDIO_BACKEND_DEFAULT))
		return;
	
	// Sound enabled.
	audio_set_enabled(true);
	if (audio_play_sound)
		audio_play_sound();
	
	return;
}


/**
 * soundZ80(): Get the Z80 sound emulation setting.
 * @return Z80 sound emulation setting.
 */
bool Options::soundZ80(void)
{
	return (Z80_State & Z80_STATE_ENABLED);
}

/**
 * setSoundZ80(): Enable or disable Z80 sound emulation.
 * @param newZ80 New Z80 sound enable setting.
 */
void Options::setSoundZ80(const bool newSoundZ80)
{
	if (newSoundZ80)
	{
		Z80_State |= Z80_STATE_ENABLED;
		vdraw_text_write("Z80 Enabled", 1000);
	}
	else
	{
		Z80_State &= ~Z80_STATE_ENABLED;
		vdraw_text_write("Z80 Disabled", 1000);
	}
}


/**
 * soundYM2612(): Get the YM2612 sound emulation setting.
 * @return YM2612 sound emulation setting.
 */
bool Options::soundYM2612(void)
{
	return YM2612_Enable;
}

/**
 * setSoundYM2612(): Enable or disable YM2612 sound emulation.
 * @param newSoundYM2612 New YM2612 sound enable setting.
 */
void Options::setSoundYM2612(const bool newSoundYM2612)
{
	YM2612_Enable = newSoundYM2612;
	
	if (YM2612_Enable)
		vdraw_text_write("YM2612 Enabled", 1000);
	else
		vdraw_text_write("YM2612 Disabled", 1000);
}


/**
 * soundYM2612_Improved(): Get the improved YM2612 sound emulation setting.
 * @return Improved YM2612 sound emulation setting.
 */
bool Options::soundYM2612_Improved(void)
{
	return YM2612_Improv;
}

/**
 * setSoundYM2612_Improved(): Enable or disable improved YM2612 sound emulation.
 * @param newSoundYM2612_Improved New improved YM2612 sound enable setting.
 */
void Options::setSoundYM2612_Improved(const bool newSoundYM2612_Improved)
{
	YM2612_Improv = newSoundYM2612_Improved;
	
	if (YM2612_Improv)
	{
		vdraw_text_write("High Quality YM2612 emulation", 1000);
	}
	else
	{
		vdraw_text_write("Normal YM2612 emulation", 1000);
	}
	
	// Save the YM2612 registers.
	// TODO: Use full save instead of partial save?
	uint8_t ym2612_reg[0x200];
	YM2612_Save(ym2612_reg);
	
	if (CPU_Mode)
		YM2612_Init(CLOCK_PAL / 7, audio_get_sound_rate(), YM2612_Improv);
	else
		YM2612_Init(CLOCK_NTSC / 7, audio_get_sound_rate(), YM2612_Improv);
	
	// Restore the YM2612 registers.
	YM2612_Restore(ym2612_reg);
}


/**
 * soundDAC(): Get the DAC sound emulation setting.
 * @return DAC sound emulation setting.
 */
bool Options::soundDAC(void)
{
	return DAC_Enable;
}

/**
 * setSoundDAC(): Enable or disable DAC sound emulation.
 * @param newSoundDAC New DAC sound emulation setting.
 */
void Options::setSoundDAC(const bool newSoundDAC)
{
	DAC_Enable = newSoundDAC;
	
	if (DAC_Enable)
		vdraw_text_write("DAC Enabled", 1000);
	else
		vdraw_text_write("DAC Disabled", 1000);
}


/**
 * soundPSG(): Get the PSG sound emulation setting.
 * @return PSG sound emulation setting.
 */
bool Options::soundPSG(void)
{
	return PSG_Enable;
}

/**
 * setSoundPSG(): Enable or disable PSG sound emulation.
 * @param newSoundPSG New PSG sound emulation setting.
 */
void Options::setSoundPSG(const bool newSoundPSG)
{
	PSG_Enable = newSoundPSG;
	
	if (PSG_Enable)
		vdraw_text_write("PSG Enabled", 1000);
	else
		vdraw_text_write("PSG Disabled", 1000);
}


/**
 * soundPSG_Sine(): Get the sine wave PSG sound emulation setting.
 * @return Sine wave PSG sound emulation setting.
 */
bool Options::soundPSG_Sine(void)
{
	return PSG_Improv;
}

/**
 * setSoundPSG_Sine(): Enable or disable sine wave PSG sound emulation.
 * @param newSoundPSG_Sine New sine wave PSG sound emulation setting.
 */
void Options::setSoundPSG_Sine(const bool newSoundPSG_Sine)
{
	PSG_Improv = newSoundPSG_Sine;
	
	if (PSG_Improv)
		vdraw_text_write("Sine wave PSG sound", 1000);
	else
		vdraw_text_write("Normal PSG sound", 1000);
}


/**
 * soundPCM(): Get the PCM sound emulation setting.
 * @return PCM sound emulation setting.
 */
bool Options::soundPCM(void)
{
	return PCM_Enable;
}

/**
 * Change_PCM(): Enable or disable PCM sound emulation.
 * @param newSoundPCM New PCM sound emulation setting.
 */
void Options::setSoundPCM(const bool newSoundPCM)
{
	PCM_Enable = newSoundPCM;
	
	if (PCM_Enable)
		vdraw_text_write("PCM Sound Enabled", 1000);
	else
		vdraw_text_write("PCM Sound Disabled", 1000);
}


/**
 * soundPWM(): Get the PWM sound emulation setting.
 * @return PWM sound emulation setting.
 */
bool Options::soundPWM(void)
{
	return PWM_Enable;
}

/**
 * setSoundPWM(): Enable or disable PWM sound emulation.
 * @param newPWM New PWM sound emulation setting.
 */
void Options::setSoundPWM(const bool newSoundPWM)
{
	PWM_Enable = newSoundPWM;
	
	if (PWM_Enable)
		vdraw_text_write("PWM Sound Enabled", 1000);
	else
		vdraw_text_write("PWM Sound Disabled", 1000);
}


/**
 * soundCDDA(): Get the CDDA (CD Audio) sound emulation setting.
 * @return CDDA sound emulation setting.
 */
bool Options::soundCDDA(void)
{
	return CDDA_Enable;
}

/**
 * setSoundCDDA(): Enable or disable CDDA (CD Audio).
 * @param newCDDA New CDDA setting.
 * @return 1 on success.
 */
void Options::setSoundCDDA(const bool newSoundCDDA)
{
	CDDA_Enable = newSoundCDDA;
	
	if (CDDA_Enable)
		vdraw_text_write("CD Audio Enabled", 1000);
	else
		vdraw_text_write("CD Audio Disabled", 1000);
}


/**
 * soundSampleRate(): Get the current sample rate ID.
 * @return Current sample rate ID. (TODO: Make an enum containing the rate IDs.)
 */
int Options::soundSampleRate(void)
{
	switch (audio_get_sound_rate())
	{
		case 11025:
			return 0;
		case 22050:
			return 1;
		case 44100:
			return 2;
		default:
			return -1;
	}
	
	return -1;
}

/**
 * setSoundSampleRate(): Change the sample rate.
 * @param Rate Rate ID. (TODO: Make an enum containing the rate IDs.)
 */
void Options::setSoundSampleRate(const int newRate)
{
	// Make sure WAV dumping has stopped.
	if (WAV_Dumping)
		wav_dump_stop();
	
	// Make sure the rate ID is valid.
	assert(newRate >= 0 && newRate <= 2);
	
	switch (newRate)
	{
		case 0:
			audio_set_sound_rate(11025);
			break;
		case 1:
			audio_set_sound_rate(22050);
			break;
		case 2:
			audio_set_sound_rate(44100);
			break;
	}
	vdraw_text_printf(2500, "Sound rate set to %d Hz", audio_get_sound_rate());
	
	// If sound isn't enabled, we're done.
	if (!audio_get_enabled())
		return;
	
	// Sound's enabled. Reinitialize it.
	uint32_t psg_state[8];
	uint8_t ym2612_reg[0x200];
	
	// Save the sound registers.
	// TODO: Use a full save instead of a partial save?
	PSG_Save_State(psg_state);
	YM2612_Save(ym2612_reg);
	
	// Stop sound.
	audio_end();
	audio_set_enabled(false);
	
	// Reinitialize the sound processors.
	if (CPU_Mode)
	{
		YM2612_Init(CLOCK_PAL / 7, audio_get_sound_rate(), YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, audio_get_sound_rate());
	}
	else
	{
		YM2612_Init(CLOCK_NTSC / 7, audio_get_sound_rate(), YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, audio_get_sound_rate());
	}

	if (SegaCD_Started)
		PCM_Set_Rate(audio_get_sound_rate());
	
	// Restore the sound registers
	YM2612_Restore(ym2612_reg);
	PSG_Restore_State(psg_state);
	
	// Attempt to reinitialize sound.
	if (audio_init(AUDIO_BACKEND_DEFAULT))
		return;
	
	// Sound is reinitialized.
	audio_set_enabled(true);
	if (audio_play_sound)
		audio_play_sound();
	
	return;
}


/** SRAM **/


bool Options::sramEnabled(void)
{
	return SRAM_Enabled;
}


void Options::setSramEnabled(bool newEnabled)
{
	SRAM_Enabled = newEnabled;
	
	if (SRAM_Enabled)
		vdraw_text_write("SRAM enabled.", 1500);
	else
		vdraw_text_write("SRAM disabled.", 1500);
}


/** SegaCD **/


/**
 * segaCD_PerfectSync(): Get SegaCD Perfect Sync.
 * @return SegaCD Perfect Sync.
 */
bool Options::segaCD_PerfectSync(void)
{
	return SegaCD_Accurate;
}

/**
 * setSegaCD_PerfectSync(): Set SegaCD Perfect Sync.
 * @param newPerfectSync True to enable; False to disable.
 */
void Options::setSegaCD_PerfectSync(const bool newSegaCD_PerfectSync)
{
	SegaCD_Accurate = newSegaCD_PerfectSync;
	
	if (!SegaCD_Accurate)
	{
		// SegaCD Perfect Sync disabled.
		if (SegaCD_Started)
		{
			Update_Frame = Do_SegaCD_Frame;
			Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
		}
	
		vdraw_text_write("SegaCD normal mode", 1500);
	}
	else
	{
		// SegaCD Perfect Sync enabled.
		
		if (SegaCD_Started)
		{
			Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
			Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
		}
		
		vdraw_text_write("SegaCD Perfect Sync mode (slower)", 1500);
	}
}


/**
 * segaCD_SRAMSize(): Get the SegaCD SRAM Size ID.
 * @return SegaCD SRAM Size ID. (TODO: Make this an enum?)
 */
int Options::segaCD_SRAMSize(void)
{
	if (BRAM_Ex_State & 0x100)
		return BRAM_Ex_Size;
	else
		return -1;
}

/**
 * setSegaCD_SRAMSize(): Change the Sega CD SRAM size.
 * @param num SegaCD SRAM Size ID. (TODO: Make this an enum?)
 */
void Options::setSegaCD_SRAMSize(const int num)
{
	if (num < -1 || num > 3)
		return;
	
	if (num == -1)
	{
		BRAM_Ex_State &= 1;
		vdraw_text_write("SegaCD SRAM cart removed", 1500);
	}
	else
	{
		BRAM_Ex_State |= 0x100;
		BRAM_Ex_Size = num;
		
		vdraw_text_printf(1500, "SegaCD SRAM cart plugged in (%d KB)", (8 << num));
	}
}


/** Graphics **/

/**
 * fastBlur(): Get the Fast Blur setting.
 * @return True if Fast Blur is enabled; False if Fast Blur is disabled.
 */
bool Options::fastBlur(void)
{
	return vdraw_get_fast_blur();
}

/**
 * setFastBlur() Set the Fast Blur setting.
 * @param newFastBlur True to enable Fast Blur; False to disable Fast Blur.
 */
void Options::setFastBlur(const bool newFastBlur)
{
	if (newFastBlur == vdraw_get_fast_blur())
		return;
	
	Flag_Clr_Scr = 1;
	vdraw_set_fast_blur(newFastBlur);
	
	if (vdraw_get_fast_blur())
		vdraw_text_write("Fast Blur Enabled", 1000);
	else
		vdraw_text_write("Fast Blur Disabled", 1000);
}


/**
 * stretch(): Get the current Stretch setting.
 * @return Current Stretch setting.
 */
uint8_t Options::stretch(void)
{
	return vdraw_get_stretch();
}

/**
 * setStretch(): Set the Stretch setting.
 * @param newStretch New Stretch setting.
 */
void Options::setStretch(const uint8_t newStretch)
{
	if (newStretch > STRETCH_FULL)
	{
		// TODO: Throw an exception.
		return;
	}
	
	Flag_Clr_Scr = 1;
	vdraw_set_stretch(newStretch);
	
	switch (vdraw_get_stretch())
	{
		case STRETCH_NONE:
			vdraw_text_write("Correct ratio mode", 1000);
			break;
		case STRETCH_H:
			vdraw_text_write("Horizontal stretched mode", 1000);
			break;
		case STRETCH_V:
			vdraw_text_write("Vertical stretched mode", 1000);
			break;
		case STRETCH_FULL:
			vdraw_text_write("Full stretched mode", 1000);
			break;
	}
}


/**
 * vsync(): Get the VSync setting.
 * @return True if VSync is enabled; False if VSync is disabled.
 */
bool Options::vsync(void)
{
	int *p_vsync;
	
	p_vsync = (vdraw_get_fullscreen() ? &Video.VSync_FS : &Video.VSync_W);
	return *p_vsync;
}

/**
 * setVSync(): Set the VSync setting.
 * @param newVSync True to enable VSync; False to disable VSync.
 */
void Options::setVSync(const bool newVSync)
{
	int *p_vsync;
	
	p_vsync = (vdraw_get_fullscreen() ? &Video.VSync_FS : &Video.VSync_W);
	*p_vsync = (newVSync == 1 ? 1 : 0);
	
	if (*p_vsync)
		vdraw_text_write("Vertical Sync Enabled", 1000);
	else
		vdraw_text_write("Vertical Sync Disabled", 1000);
	
	// Update VSync.
	if (vdraw_update_vsync)
		vdraw_update_vsync(0);
}


/**
 * backend(): Get the current backend.
 * @return Current backend ID.
 */
VDRAW_BACKEND Options::backend(void)
{
	return vdraw_cur_backend_id;
}

/**
 * setBackend(): Set the current backend.
 * @param newBackend Backend ID.
 */
void Options::setBackend(VDRAW_BACKEND newBackend)
{
	if (newBackend >= VDRAW_BACKEND_MAX)
	{
		// Invalid backend.
		return;
	}
	
	// End the current backend.
	vdraw_backend_end();
	
	// Initialize the new backend.
	vdraw_backend_init(newBackend);
	
	// Synchronize the Graphics Menu.
	Sync_Gens_Window_GraphicsMenu();
	
	// Print a notice about the selected backend.
	if (is_gens_running())
	{
		if (vdraw_cur_backend_id == newBackend)
		{
			vdraw_text_printf(1500, "Selected Video Backend: %s", vdraw_backends[newBackend]->name);
		}
		else
		{
			// New backend is broken.
			vdraw_text_printf(1500, "Could not initialize %s. Reverted to %s.",
					  vdraw_backends[newBackend]->name,
					  vdraw_cur_backend->name);
		}
	}
}


#ifdef GENS_OPENGL
/**
 * setOpenGL_Resolution(): Set the OpenGL resolution.
 * @param w Width.
 * @param h Height.
 */
void Options::setOpenGL_Resolution(const int w, const int h)
{
	// TODO: Move this to VDraw_GL.
	
	if (w <= 0 || h <= 0)
		return;
	if (Video.GL.width == w && Video.GL.height == h)
		return;
	
	// OpenGL resolution has changed.
	Video.GL.width = w;
	Video.GL.height = h;
	
	// Print the resolution information.
	vdraw_text_printf(1500, "Selected %dx%d resolution", w, h);
	
	// If the current backend isn't OpenGL, don't do anything.
	// TODO: Make this not platform-dependent.
#ifdef GENS_OS_UNIX
	if (vdraw_cur_backend_id != VDRAW_BACKEND_SDL_GL)
		return;
#else
	// OpenGL isn't implemented on any other platforms yet.
	return;
#endif
	
	// OpenGL mode is currently enabled. Change the resolution.
	vdraw_reset_renderer(TRUE);
	
	// Synchronize the Graphics Menu.
	Sync_Gens_Window_GraphicsMenu();
}


/**
 * OpenGL_LinearFilter(): Get the current OpenGL Linear Filter state.
 * @return OpenGL Linear Filter state.
 */
bool Options::OpenGL_LinearFilter(void)
{
	return Video.GL.glLinearFilter;
}


/**
 * setOpenGL_LinearFilter(): Set the OpenGL Linear Filter state.
 * @param newFilter New linear filter state.
 */
void Options::setOpenGL_LinearFilter(bool newFilter)
{
	if (Video.GL.glLinearFilter == newFilter)
		return;
	
	Video.GL.glLinearFilter = newFilter;
	
	if (Video.GL.glLinearFilter)
		vdraw_text_write("Enabled OpenGL Linear Filter", 1500);
	else
		vdraw_text_write("Disabled OpenGL Linear Filter", 1500);
	
	// Synchronize the Graphics Menu.
	Sync_Gens_Window_GraphicsMenu();
}
#endif /* GENS_OPENGL */


#ifdef GENS_OS_WIN32
/**
 * swRender(): Get the software rendering setting. (Win32 only)
 * @return Software rendering setting.
 */
bool Options::swRender(void)
{
	return vdraw_get_sw_render();
}

/**
 * setSwRender(): Force software rendering in fullscreen mode. (Win32 only)
 * @param newSwRender New software rendering setting.
 */
void Options::setSwRender(const bool newSwRender)
{
	// TODO: Specify software blit in the parameter.
	
	Flag_Clr_Scr = 1;
	
	vdraw_set_sw_render(newSwRender);
	
	if (vdraw_get_sw_render())
		vdraw_text_write("Force software blit for Full-Screen", 1000);
	else
		vdraw_text_write("Enable hardware blit for Full-Screen", 1000);
}
#endif /* GENS_OS_WIN32 */


/**
 * rendererNext(): Select the next renderer.
 */
void Options::rendererNext(void)
{
	list<mdp_render_t*>::iterator rendMode = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
	rendMode++;
	if (rendMode != RenderMgr::end())
	{
		vdraw_set_renderer(rendMode);
		Sync_Gens_Window_GraphicsMenu();
	}
}


/**
 * rendererPrev(): Select the previous renderer.
 */
void Options::rendererPrev(void)
{
	list<mdp_render_t*>::iterator rendMode = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
	if (rendMode != RenderMgr::begin())
	{
		rendMode--;
		vdraw_set_renderer(rendMode);
		Sync_Gens_Window_GraphicsMenu();
	}
}


/**
 * systemReset(): Reset the active system.
 */
void Options::systemReset(void)
{
	/*
	if (Check_If_Kaillera_Running())
	return 0;
	*/
	
	if (Genesis_Started)
	{
		Reset_Genesis();
		vdraw_text_write("Genesis reset", 1500);
	}
	else if (SegaCD_Started)
	{
		Reset_SegaCD();
		vdraw_text_write("SegaCD reset", 1500);
	}
	else if (_32X_Started)
	{
		Reset_32X();
		vdraw_text_write("32X reset", 1500);
	}
}


/**
 * setGameName(): Sets the title of the window to the system name and the game name.
 * @param system Force system: 0 == MD; 1 == MCD; 2 == 32X
 */
void Options::setGameName(int system)
{
	string systemName;
	string gameName;
	string emptyGameName;
	
	if (Genesis_Started || system == 0)
	{
		if ((CPU_Mode == 1) || (Game_Mode == 0))
			systemName = "Mega Drive";
		else
			systemName = "Genesis";
		gameName = ROM::getRomName(Game, Game_Mode);
	}
	else if (_32X_Started || system == 2)
	{
		if (CPU_Mode == 1)
			systemName = "32X (PAL)";
		else
			systemName = "32X (NTSC)";
		gameName = ROM::getRomName(Game, Game_Mode);
	}
	else if (SegaCD_Started || system == 1)
	{
		if ((CPU_Mode == 1) || (Game_Mode == 0))
			systemName = "MegaCD";
		else
			systemName = "SegaCD";
		gameName = ROM_Filename;
		emptyGameName = "No Disc";
	}
	else
	{
		GensUI::setWindowTitle_Idle();
		return;
	}
	
	GensUI::setWindowTitle_Game(systemName, gameName, emptyGameName);
}

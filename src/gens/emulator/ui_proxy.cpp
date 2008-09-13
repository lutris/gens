#include <assert.h>
#include <stdio.h>

#include "g_main.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"
#include "gens.h"
#include "g_md.hpp"
#include "g_mcd.hpp"
#include "g_32x.hpp"
#include "sdllayer/g_sdlsound.h"
#include "segacd/cd_sys.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"
#include "gens_core/misc/misc.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/mem/mem_s68k.h"
#include "util/file/save.hpp"

#include "ui_proxy.hpp"
#include "ui/gens_ui.hpp"
#include "gens/gens_window_sync.hpp"

#include "gens_core/gfx/renderers.h"

_filters filters;

void
save_state ()
{
  Str_Tmp[0] = 0;
  Get_State_File_Name (Str_Tmp);
  Save_State (Str_Tmp);
}

void
load_state ()
{
  Str_Tmp[0] = 0;
  Get_State_File_Name (Str_Tmp);
  Load_State (Str_Tmp);
}


/**
 * system_reset(): Reset the active system.
 */
void system_reset(void)
{
	/*
	if (Check_If_Kaillera_Running())
		return 0;
	*/
	
	if (Genesis_Started)
	{
		Reset_Genesis();
		MESSAGE_L("Genesis reset", "Genesis reset", 1500);
	}
	else if (SegaCD_Started)
	{
		Reset_SegaCD();
		MESSAGE_L("SegaCD reset", "SegaCD reset", 1500);
	}
	else if (_32X_Started)
	{
		Reset_32X();
		MESSAGE_L("32X reset", "32X reset", 1500);
	}
}


/**
 * Change_SegaCD_SRAM_Size(): Change the Sega CD SRAM size.
 * @param num Index of the SRAM size.
 * @return 1 on success.
 */
int Change_SegaCD_SRAM_Size (int num)
{
	if (num == -1)
	{
		BRAM_Ex_State &= 1;
		MESSAGE_L("SegaCD SRAM cart removed", "SegaCD SRAM cart removed", 1500);
	}
	else
	{
		char bsize[256];
		
		BRAM_Ex_State |= 0x100;
		BRAM_Ex_Size = num;
		
		sprintf (bsize, "SegaCD SRAM cart plugged in (%d KB)", 8 << num);
		MESSAGE_L(bsize, bsize, 1500);
	}

  return 1;
}

/**
 * Change_Sound(): Enable or disable sound.
 * @param newSound New sound enable setting.
 * @return 1 on success.
 */
int Change_Sound(int newSound)
{
	Sound_Enable = newSound;
	if (!Sound_Enable)
	{
		End_Sound ();
		YM2612_Enable = 0;
		PSG_Enable = 0;
		DAC_Enable = 0;
		PCM_Enable = 0;
		PWM_Enable = 0;
		CDDA_Enable = 0;
		MESSAGE_L("Sound Disabled", "Sound Disabled", 1500);
	}
	else
	{
		if (!Init_Sound())
		{
			// Error initializing sound.
			Sound_Enable = 0;
			YM2612_Enable = 0;
			PSG_Enable = 0;
			DAC_Enable = 0;
			PCM_Enable = 0;
			PWM_Enable = 0;
			CDDA_Enable = 0;
			return 0;
		}
		
		Play_Sound ();
		
		// Make sure Z80 sound emulation is enabled.
		if (!(Z80_State & 1))
			Change_Z80(1);
		
		YM2612_Enable = 1;
		PSG_Enable = 1;
		DAC_Enable = 1;
		PCM_Enable = 1;
		PWM_Enable = 1;
		CDDA_Enable = 1;
		
		MESSAGE_L ("Sound Enabled", "Sound Enabled", 1500);
	}
	
	return 1;
}


/**
 * Change_Sound_Stereo(): Enable or disable stereo sound.
 * @param newStereo New stereo sound enable setting.
 * @return 1 on success.
 */
int Change_Sound_Stereo(int newStereo)
{
	unsigned char Reg_1[0x200];
	
	Sound_Stereo = newStereo;
	if (!Sound_Stereo)
	{
		MESSAGE_L("Mono sound", "Mono sound", 1000);
	}
	else
	{
		MESSAGE_L ("Stereo sound", "Stereo sound", 1000);
	}
	
	if (!Sound_Enable)
	{
		// Sound isn't enabled, so nothing needs to be changed.
		return 1;
	}
	
	// Save the current sound state.
	// TODO: Use full save instead of partial save?
	PSG_Save_State();
	YM2612_Save(Reg_1);
	
	// Temporarily disable sound.
	End_Sound();
	Sound_Enable = 0;
	
	// Reinitialize the sound processors.
	if (CPU_Mode)
	{
		YM2612_Init(CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, Sound_Rate);
	}
	else
	{
		YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
	}
	
	if (SegaCD_Started)
		Set_Rate_PCM(Sound_Rate);
	
	// Restore the sound state.
	YM2612_Restore(Reg_1);
	PSG_Restore_State();
	
	// Attempt to re-enable sound.
	if (!Init_Sound())
		return (0);
	
	// Sound enabled.
	Sound_Enable = 1;
	Play_Sound();
	return 1;
}


/**
 * Change_Z80(): Enable or disable Z80 sound emulation.
 * @param newZ80 New Z80 sound enable setting.
 * @return 1 on success.
 */
int Change_Z80(int newZ80)
{
	if (newZ80)
		Z80_State |= 1;
	else
		Z80_State &= ~1;
	
	if (Z80_State & 1)
	{
		MESSAGE_L("Z80 Enabled", "Z80 Enabled", 1000);
	}
	else
	{
		MESSAGE_L("Z80 Disabled", "Z80 Disabled", 1000);
	}
	
	return 1;
}


/**
 * Change_YM2612(): Enable or disable YM2612 sound emulation.
 * @param newYM2612 New YM2612 sound enable setting.
 * @return 1 on success.
 */
int Change_YM2612(int newYM2612)
{
	YM2612_Enable = newYM2612;
	
	if (YM2612_Enable)
	{
		MESSAGE_L("YM2612 Enabled", "YM2612 Enabled", 1000);
	}
	else
	{
		MESSAGE_L("YM2612 Disabled", "YM2612 Disabled", 1000);
	}
	
	return 1;
}


/**
 * Change_YM2612_Improved(): Enable or disable improved YM2612 sound emulation.
 * @param newYM2612Improved New improved YM2612 sound enable setting.
 * @return 1 on success.
 */
int Change_YM2612_Improved(int newYM2612Improved)
{
	unsigned char Reg_1[0x200];
	
	YM2612_Improv = newYM2612Improved;
	if (YM2612_Improv)
	{
		MESSAGE_L("High Quality YM2612 emulation",
			  "High Quality YM2612 emulation", 1000);
	}
	else
	{
		MESSAGE_L("Normal YM2612 emulation",
			  "Normal YM2612 emulation", 1000);
	}
	
	// Save the YM2612 registers.
	// TODO: Use full save instead of partial save?
	YM2612_Save (Reg_1);
	
	if (CPU_Mode)
		YM2612_Init (CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
	else
		YM2612_Init (CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
	
	// Restore the YM2612 registers.
	YM2612_Restore (Reg_1);
	
	return 1;
}


/**
 * Change_DAC(): Enable or disable DAC sound emulation.
 * @param newDAC New DAC sound emulation setting.
 * @return 1 on success.
 */
int Change_DAC(int newDAC)
{
	DAC_Enable = newDAC;
	
	if (DAC_Enable)
	{
		MESSAGE_L("DAC Enabled", "DAC Enabled", 1000);
	}
	else
	{
		MESSAGE_L("DAC Disabled", "DAC Disabled", 1000);
	}
	
	return 1;
}


/**
 * Change_DAC_Improved(): Enable or disable improved DAC sound emulation.
 * @param newDACImproved New improved DAC sound emulation setting.
 * @return 1 on success.
 */

int Change_DAC_Improved(int newDACImproved)
{
	DAC_Improv = newDACImproved;
	
	if (DAC_Improv)
	{
		MESSAGE_L("Improved DAC sound (voices)",
			  "Improved DAC sound (voices)", 1000);
	}
	else
	{
		MESSAGE_L("Normal DAC sound",
			  "Normal DAC sound", 1000);
	}
	
	return 1;
}


/**
 * Change_PSG(): Enable or disable PSG sound emulation.
 * @param newPSG New PSG sound emulation setting.
 * @return 1 on success.
 */
int Change_PSG(int newPSG)
{
	PSG_Enable = newPSG;
	
	if (PSG_Enable)
	{
		MESSAGE_L("PSG Enabled", "PSG Enabled", 1000);
	}
	else
	{
		MESSAGE_L("PSG Disabled", "PSG Disabled", 1000);
	}
	
	return 1;
}


/**
 * Change_PSG_Improved(): Enable or disable improved PSG sound emulation.
 * @param newPSGImproved New improved PSG sound emulation setting.
 * @return 1 on success.
 */
int Change_PSG_Improved(int newPSGImproved)
{
	PSG_Improv = newPSGImproved;
	
	if (PSG_Improv)
	{
		MESSAGE_L("Improved PSG sound", "Improved PSG sound", 1000);
	}
	else
	{
		MESSAGE_L("Normal PSG sound", "Normal PSG sound", 1000);
	}
	
	return 1;
}


/**
 * Change_PCM(): Enable or disable PCM sound emulation.
 * @param newPCM New PCM sound emulation setting.
 * @return 1 on success.
 */
int Change_PCM(int newPCM)
{
	PCM_Enable = newPCM;
	if (PCM_Enable)
	{
		MESSAGE_L("PCM Sound Enabled", "PCM Sound Enabled", 1000);
	}
	else
	{
		MESSAGE_L("PCM Sound Disabled", "PCM Sound Disabled", 1000);
	}
	
	return 1;
}


/**
 * Change_PWM(): Enable or disable PWM sound emulation.
 * @param newPWM New PWM sound emulation setting.
 * @return 1 on success.
 */
int Change_PWM(int newPWM)
{
	PWM_Enable = newPWM;
	if (PWM_Enable)
	{
		MESSAGE_L("PWM Sound Enabled", "PWM Sound Enabled", 1000);
	}
	else
	{
		MESSAGE_L("PWM Sound Disabled", "PWM Sound Disabled", 1000);
	}
	
	return 1;
}


/**
 * Change_CDDA(): Enable or disable CDDA (CD Audio).
 * @param newCDDA New CDDA setting.
 * @return 1 on success.
 */
int Change_CDDA(int newCDDA)
{
	CDDA_Enable = newCDDA;
	
	if (CDDA_Enable)
		MESSAGE_L("CD Audio Enabled", "CD Audio Enabled", 1000);
	else
		MESSAGE_L("CD Audio Disabled", "CD Audio Disabled", 1000);
	
	return 1;
}


int Change_Fast_Blur(void)
{
	Flag_Clr_Scr = 1;
	draw->setFastBlur(!draw->fastBlur());
	
	if (draw->fastBlur())
		MESSAGE_L("Fast Blur Enabled", "Fast Blur Enabled", 1000);
	else
		MESSAGE_L("Fast Blur Disabled", "Fast Blur Disabled", 1000);
	
	return 1;
}


/**
 * Change_Sample_Rate(): Change the sample rate.
 * @param Rate Rate ID. (TODO: Make an enum containing the rate IDs.
 * @return 1 on success.
 */
int Change_Sample_Rate(int Rate)
{
	unsigned char Reg_1[0x200];
	
	// Make sure the rate ID is valid.
	assert (Rate >= 0 && Rate <= 5);
	
	switch (Rate)
	{
		case 0:
			Sound_Rate = 11025;
			break;
		case 1:
			Sound_Rate = 22050;
			break;
		case 2:
			Sound_Rate = 44100;
			break;
		case 3:
			Sound_Rate = 16000;
			break;
		case 4:
			Sound_Rate = 32000;
			break;
		case 5:
			Sound_Rate = 48000;
			break;
	}
	MESSAGE_NUM_L("Sound rate set to %d Hz", "Sound rate set to %d Hz", Sound_Rate, 2500);
	
	// If sound isn't enabled, we're done.
	if (!Sound_Enable)
		return 1;
	
	// Sound's enabled. Reinitialize it.
	
	// Save the sound registers.
	// TODO: Use a full save instead of a partial save?
	PSG_Save_State ();
	YM2612_Save (Reg_1);
	
	// Stop sound.
	End_Sound();
	Sound_Enable = 0;
	
	// Reinitialize the sound processors.
	if (CPU_Mode)
	{
		YM2612_Init(CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, Sound_Rate);
	}
	else
	{
		YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
	}

	if (SegaCD_Started)
		Set_Rate_PCM (Sound_Rate);
	
	// Restore the sound registers
	YM2612_Restore (Reg_1);
	PSG_Restore_State();
	
	// Attempt to reinitialize sound.
	if (!Init_Sound())
		return 0;
	
	// Sound is reinitialized.
	Sound_Enable = 1;
	Play_Sound();
	
	return 1;
}


/**
 * Change_SegaCD_PerfectSync(): Set the SegaCD accuracy level.
 * @param newPerfectSync New accuracy level value.
 * @return 1 on success.
 */
int Change_SegaCD_PerfectSync(int newPerfectSync)
{
	SegaCD_Accurate = newPerfectSync;
	
	if (!SegaCD_Accurate)
	{
		// SegaCD Perfect Sync disabled.
		if (SegaCD_Started)
		{
			Update_Frame = Do_SegaCD_Frame;
			Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
		}
	
		MESSAGE_L("SegaCD normal mode", "SegaCD normal mode", 1500);
	}
	else
	{
		// SegaCD Perfect Sync enabled.
		
		if (SegaCD_Started)
		{
			Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
			Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
		}
		
		MESSAGE_L("SegaCD Perfect Sync mode (SLOW)",
			  "SegaCD Perfect Sync mode (slower)", 1500);
	}
	
	return 1;
}


/**
 * Set_Game_Name(): Sets the title of the window to the system name and the game name.
 */
void Set_Game_Name(void)
{
	char SystemName[16];
	char *Title_Rom_Name;
	
	if (Genesis_Started)
	{
		if ((CPU_Mode == 1) || (Game_Mode == 0))
			strcpy(SystemName, "Mega Drive");
		else
			strcpy(SystemName, "Genesis");
		Title_Rom_Name = Game->Rom_Name_W;
	}
	else if (_32X_Started)
	{
		if (CPU_Mode == 1)
			strcpy(SystemName, "32X (PAL)");
		else
			strcpy(SystemName, "32X (NTSC)");
		Title_Rom_Name = Game->Rom_Name_W;
	}
	else if (SegaCD_Started)
	{
		if ((CPU_Mode == 1) || (Game_Mode == 0))
			strcpy(SystemName, "MegaCD");
		else
			strcpy(SystemName, "SegaCD");
		Title_Rom_Name = Rom_Name;
	}
	else
	{
		GensUI::setWindowTitle_Idle();
		return;
	}
	
	GensUI::setWindowTitle_Game(SystemName, Title_Rom_Name);
}


/**
 * Change_Country(): Change the current country code.
 * @param newCountry New country code.
 * @return 1 on success.
 */
int Change_Country(int newCountry)
{
	unsigned char Reg_1[0x200];
	
	Flag_Clr_Scr = 1;
	
	Country = newCountry;
	switch (Country)
	{
		default:
		case -1:
			// Auto-detect.
			if (Genesis_Started || _32X_Started)
				Detect_Country_Genesis();
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
	
	if (CPU_Mode)
	{
		CPL_Z80 = Round_Double ((((double) CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = Round_Double ((((double) CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 =
			Round_Double (((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
				(double) MSH2_Speed) / 100.0);
		CPL_SSH2 =
			Round_Double (((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
				(double) SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;
		_32X_VDP.Mode &= ~0x8000;
		
		CD_Access_Timer = 2080;
		Timer_Step = 136752;
	}
	else
	{
		CPL_Z80 = Round_Double ((((double) CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = Round_Double ((((double) CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		CPL_MSH2 =
		Round_Double (((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
				(double) MSH2_Speed) / 100.0);
		CPL_SSH2 =
			Round_Double (((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
				(double) SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 262;
		VDP_Status &= 0xFFFE;
		_32X_VDP.Mode |= 0x8000;
		
		CD_Access_Timer = 2096;
		Timer_Step = 135708;
	}
	
	if (Sound_Enable)
	{
		PSG_Save_State();
		YM2612_Save(Reg_1);
		
		End_Sound();
		Sound_Enable = 0;
		
		if (CPU_Mode)
		{
			YM2612_Init(CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
			PSG_Init(CLOCK_PAL / 15, Sound_Rate);
		}
		else
		{
			YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
			PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
		}
		
		if (SegaCD_Started)
			Set_Rate_PCM (Sound_Rate);
		
		YM2612_Restore (Reg_1);
		PSG_Restore_State ();
		
		if (!Init_Sound ())
			return 0;
		
		Sound_Enable = 1;
		Play_Sound();
	}
	
	if (Game_Mode)
	{
		if (CPU_Mode)
		{
			MESSAGE_L("Europe system (50 FPS)", "Europe system (50 FPS)", 1500);
		}
		else
		{
			MESSAGE_L("USA system (60 FPS)", "USA system (60 FPS)", 1500);
		}
	}
	else
	{
		if (CPU_Mode)
		{
			MESSAGE_L("Japan system (50 FPS)", "Japan system (50 FPS)", 1500);
		}
		else
		{
			MESSAGE_L("Japan system (60 FPS)", "Japan system (60 FPS)", 1500);
		}
	}
	
	Set_Game_Name();
	return 1;
}


/**
 * Change_Stretch(): Change the Stretch setting.
 * @param stretch 0 to turn Stretch off; 1 to turn Stretch on.
 * @return 1 on success.
 */
int Change_Stretch(int newStretch)
{
	// TODO: Stretched mode seems to be broken, even though it is being set correctly...
	
	Flag_Clr_Scr = 1;
	
	draw->setStretch(newStretch);
	
	if (draw->stretch())
		MESSAGE_L("Stretched mode", "Stretched mode", 1000);
	else
		MESSAGE_L("Correct ratio mode", "Correct ratio mode", 1000);
	
	return 1;
}


int Change_Blit_Style(void)
{
	// TODO: Specify software blit in the parameter.
	
	Flag_Clr_Scr = 1;
	
	draw->setSwRender(!draw->swRender());
	if (draw->swRender())
		MESSAGE_L("Force software blit for Full-Screen",
			  "Force software blit for Full-Screen", 1000);
	else
		MESSAGE_L("Enable hardware blit for Full-Screen",
			  "Enable hardware blit for Full-Screen", 1000);
	
	return 1;
}


/**
 * Set_Sprite_Limit(): Enable/Disable the sprite limit.
 * @param newLimit 0 to disable; 1 to enable.
 * @return 1 on success. 
 */
int Set_Sprite_Limit(int newLimit)
{
	Sprite_Over = newLimit;
	
	if (Sprite_Over)
		MESSAGE_L("Sprite Limit Enabled", "Sprite Limit Enabled", 1000);
	else
		MESSAGE_L("Sprite Limit Disabled", "Sprite Limit Disabled", 1000);
	
	return 1;
}


/**
 * Change_Debug(): Change the current debugging mode.
 * @param Debug_Mode New debug mode.
 * @return 1 on success.
 */
int Change_Debug(int Debug_Mode)
{
	if (!Game)
		return 0;
	
	Flag_Clr_Scr = 1;
	Clear_Sound_Buffer ();
	
	if (Debug_Mode == Debug)
		Debug = 0;
	else
		Debug = Debug_Mode;
	
	return 1;
}

/**
 * Change_VSync(): Change the VSync setting.
 * @param vsync 0 to turn VSync off; 1 to turn VSync on.
 * @return 1 on success.
 */
int Change_VSync(int newVSync)
{
	int *p_vsync;
	
	// TODO: Get this working.
	
	p_vsync = (draw->fullScreen() ? &Video.VSync_FS : &Video.VSync_W);
	
	*p_vsync = (newVSync == 1 ? 1 : 0);
	
	if (*p_vsync)
		MESSAGE_L("Vertical Sync Enabled", "Vertical Sync Enabled", 1000);
	else
 		MESSAGE_L("Vertical Sync Disabled", "Vertical Sync Disabled", 1000);
	
	// Update VSync.
	draw->updateVSync();
	
	// Return success.
	return 1;
}


/**
 * Set_Frame_Skip(): Set the frame skip setting.
 * @param frames Frames to skip. (-1 == Auto)
 * @return 1 on success.
 */
int Set_Frame_Skip(int frames)
{
	Frame_Skip = frames;
	
	if (Frame_Skip != -1)
		MESSAGE_NUM_L("Frame skip set to %d", "Frame skip set to %d", Frame_Skip, 1500);
	else
		MESSAGE_L("Frame skip set to Auto", "Frame skip set to Auto", 1500);
	
	return 1;
}

/**
 * Set_Current_State(): Set the current savestate slot.
 * @param slot Savestate slot.
 * @return 1 on success.
 */
int Set_Current_State(int slot)
{
	FILE *f;
	
	// Make sure the slot number is in bounds.
	if (slot < 0 || slot > 9)
		return 0;
	
	// Set the new savestate slot number.
	Current_State = slot;
	
	// TODO: Change this to just check if the file exists.
	if ((f = Get_State_File()))
	{
		fclose(f);
		MESSAGE_NUM_L("SLOT %d [OCCUPIED]", "SLOT %d [OCCUPIED]", Current_State, 1500);
	}
	else
	{
		MESSAGE_NUM_L("SLOT %d [EMPTY]", "SLOT %d [EMPTY]", Current_State, 1500);
	}
	Sync_Gens_Window_FileMenu();
	
	return 1;
}

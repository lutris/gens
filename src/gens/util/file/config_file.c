/**
 * GENS: Configuration file handler.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "save.h"
#include "config_file.h"
#include "port/port.h"
#include "port/ini.hpp"

#include "emulator/gens.h"
#include "emulator/g_main.h"
#include "emulator/g_input.h"
#include "sdllayer/g_sdldraw.h"
#include "sdllayer/g_sdlsound.h"
#include "sdllayer/g_sdlinput.h"
#include "util/gfx/scrshot.h"
#include "util/file/ggenie.h"
#include "gens_core/misc/misc.h"
#include "emulator/g_palette.h"

// CPU
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "gens_core/cpu/z80/z80.h"

// Memory
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/mem/mem_z80.h"
#include "gens_core/io/io.h"
#include "util/file/rom.hpp"

// VDP
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

// Audio
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"

// SegaCD
#include "segacd/cd_sys.h"
#include "gens_core/gfx/gfx_cd.h"
#include "segacd/lc89510.h"
#include "segacd/cd_file.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.h"
#endif

// UI
#include "emulator/ui_proxy.h"
#include "ui-common.h"


// TODO: Make this a global array somewhere better.
const char* PlayerNames[8] = {"P1", "P1B", "P1C", "P1D", "P2", "P2B", "P2C", "P2D"};


/**
 * Save_Config(): Save GENS configuration.
 * @param File_Name Configuration filename.
 */
int Save_Config(const char *File_Name)
{
	int i;
	
	// String copy is needed, since the passed variable might be Str_Tmp.
	char Conf_File[GENS_PATH_MAX];
	strncpy(Conf_File, File_Name, GENS_PATH_MAX);
	
	// Paths
	WritePrivateProfileString("General", "Rom path", Rom_Dir, Conf_File);
	WritePrivateProfileString("General", "Save path", State_Dir, Conf_File);
	WritePrivateProfileString("General", "SRAM path", SRAM_Dir, Conf_File);
	WritePrivateProfileString("General", "BRAM path", BRAM_Dir, Conf_File);
	WritePrivateProfileString("General", "Dump path", Dump_Dir, Conf_File);
	WritePrivateProfileString("General", "Dump GYM path", Dump_GYM_Dir, Conf_File);
	WritePrivateProfileString("General", "Screen Shot path", ScrShot_Dir, Conf_File);
	WritePrivateProfileString("General", "Patch path", Patch_Dir, Conf_File);
	WritePrivateProfileString("General", "IPS Patch path", IPS_Dir, Conf_File);
	
	// Genesis BIOS
	WritePrivateProfileString("General", "Genesis Bios", BIOS_Filenames.MD_TMSS, Conf_File);
	
	// SegaCD BIOSes
	WritePrivateProfileString ("General", "USA CD Bios", BIOS_Filenames.SegaCD_US, Conf_File);
	WritePrivateProfileString ("General", "EUROPE CD Bios", BIOS_Filenames.MegaCD_EU, Conf_File);
	WritePrivateProfileString ("General", "JAPAN CD Bios", BIOS_Filenames.MegaCD_JP, Conf_File);
	
	// 32X BIOSes
	WritePrivateProfileString("General", "32X 68000 Bios", BIOS_Filenames._32X_MC68000, Conf_File);
	WritePrivateProfileString("General", "32X Master SH2 Bios", BIOS_Filenames._32X_MSH2, Conf_File);
	WritePrivateProfileString("General", "32X Slave SH2 Bios", BIOS_Filenames._32X_SSH2, Conf_File);
	
	// Last 9 ROMs
	for (i = 0; i < 9; i++)
	{
		sprintf(Str_Tmp, "Rom %d", i + 1);
		WritePrivateProfileString("General", Str_Tmp, Recent_Rom[i], Conf_File);
	}
	
	// SegaCD
	WritePrivateProfileString("General", "CD Drive", CDROM_DEV, Conf_File);
	sprintf(Str_Tmp, "%d", CDROM_SPEED);
	WritePrivateProfileString("General", "CD Speed", Str_Tmp, Conf_File);
	
	sprintf(Str_Tmp, "%d", Current_State);
	WritePrivateProfileString("General", "State Number", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Language);
	WritePrivateProfileString("General", "Language", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Window_Pos.x);
	WritePrivateProfileString("General", "Window X", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Window_Pos.y);
	WritePrivateProfileString("General", "Window Y", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Effect_Color);
	WritePrivateProfileString("General", "Free Mode Color", Str_Tmp, Conf_File);
	
	// Video adjustments
	sprintf(Str_Tmp, "%d", Contrast_Level);
	WritePrivateProfileString("Graphics", "Contrast", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Brightness_Level);
	WritePrivateProfileString("Graphics", "Brightness", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Greyscale & 1);
	WritePrivateProfileString("Graphics", "Greyscale", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Invert_Color & 1);
	WritePrivateProfileString("Graphics", "Invert", Str_Tmp, Conf_File);
	
	// Video settings
	// Render_Mode is incremented by 1 for compatibility with old Gens.
	// TODO: BUG: If "Full Scree VSync" is saved before "Full Screen",
	// the Linux reimplementation of WritePrivateProfileString gets confused.
	sprintf(Str_Tmp, "%d", Video.Full_Screen & 1);
	WritePrivateProfileString("Graphics", "Full Screen", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Video.Render_Mode + 1);
	WritePrivateProfileString("Graphics", "Render Mode", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", FS_VSync & 1);
	WritePrivateProfileString("Graphics", "Full Screen VSync", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", W_VSync & 1);
	WritePrivateProfileString("Graphics", "Windows VSync", Str_Tmp, Conf_File);
	
	sprintf(Str_Tmp, "%d", bpp);
	WritePrivateProfileString("Graphics", "Bits Per Pixel", Str_Tmp, Conf_File);
#ifdef GENS_OPENGL
	sprintf(Str_Tmp, "%d", Video.OpenGL & 1);
	WritePrivateProfileString("Graphics", "Render Opengl", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Video.Width_GL);
	WritePrivateProfileString("Graphics", "Opengl Width", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Video.Height_GL);
	WritePrivateProfileString("Graphics", "Opengl Height", Str_Tmp, Conf_File);
	/*
	sprintf(Str_Tmp, "%d", gl_linear_filter);
	WritePrivateProfileString("Graphics", "Opengl Filter", Str_Tmp, Conf_File);
	*/
#endif
	
	sprintf(Str_Tmp, "%d", Stretch & 1);  
	WritePrivateProfileString("Graphics", "Stretch", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Blit_Soft & 1);
	WritePrivateProfileString("Graphics", "Software Blit", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Sprite_Over & 1);
	WritePrivateProfileString("Graphics", "Sprite limit", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Frame_Skip);
	WritePrivateProfileString("Graphics", "Frame skip", Str_Tmp, Conf_File);
	
	// Sound settings
	sprintf(Str_Tmp, "%d", Sound_Enable & 1);
	WritePrivateProfileString("Sound", "State", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Sound_Rate);
	WritePrivateProfileString("Sound", "Rate", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Sound_Stereo);
	WritePrivateProfileString("Sound", "Stereo", Str_Tmp, Conf_File);
	
	sprintf(Str_Tmp, "%d", Z80_State & 1);
	WritePrivateProfileString("Sound", "Z80 State", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", YM2612_Enable & 1);
	WritePrivateProfileString("Sound", "YM2612 State", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", PSG_Enable & 1);
	WritePrivateProfileString("Sound", "PSG State", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", DAC_Enable & 1);
	WritePrivateProfileString("Sound", "DAC State", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", PCM_Enable & 1);
	WritePrivateProfileString("Sound", "PCM State", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", PWM_Enable & 1);
	WritePrivateProfileString("Sound", "PWM State", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", CDDA_Enable & 1);
	WritePrivateProfileString("Sound", "CDDA State", Str_Tmp, Conf_File);
	
	// Improved sound options
	sprintf(Str_Tmp, "%d", YM2612_Improv & 1);
	WritePrivateProfileString("Sound", "YM2612 Improvement", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", DAC_Improv & 1);
	WritePrivateProfileString("Sound", "DAC Improvement", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", PSG_Improv & 1);
	WritePrivateProfileString("Sound", "PSG Improvement", Str_Tmp, Conf_File);
	
	// Country codes
	sprintf(Str_Tmp, "%d", Country);
	WritePrivateProfileString("CPU", "Country", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Country_Order[0]);
	WritePrivateProfileString("CPU", "Prefered Country 1", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Country_Order[1]);
	WritePrivateProfileString("CPU", "Prefered Country 2", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Country_Order[2]);
	WritePrivateProfileString("CPU", "Prefered Country 3", Str_Tmp, Conf_File);
	
	// CPU options
	
	sprintf(Str_Tmp, "%d", SegaCD_Accurate);
	WritePrivateProfileString("CPU", "Perfect synchro between main and sub CPU (Sega CD)", Str_Tmp, Conf_File);
	
	sprintf(Str_Tmp, "%d", MSH2_Speed);
	WritePrivateProfileString("CPU", "Main SH2 Speed", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", SSH2_Speed);
	WritePrivateProfileString("CPU", "Slave SH2 Speed", Str_Tmp, Conf_File);
	
	// Various settings
	sprintf(Str_Tmp, "%d", Video.Fast_Blur & 1);
	WritePrivateProfileString("Options", "Fast Blur", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Show_FPS & 1);
	WritePrivateProfileString("Options", "FPS", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", FPS_Style);
	WritePrivateProfileString("Options", "FPS Style", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Show_Message & 1);
	WritePrivateProfileString("Options", "Message", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Message_Style);
	WritePrivateProfileString("Options", "Message Style", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Show_LED & 1);
	WritePrivateProfileString("Options", "LED", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Auto_Fix_CS & 1);
	WritePrivateProfileString("Options", "Auto Fix Checksum", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Auto_Pause & 1);
	WritePrivateProfileString("Options", "Auto Pause", Str_Tmp, Conf_File);
#ifdef GENS_CDROM
	sprintf(Str_Tmp, "%d", CUR_DEV);
	WritePrivateProfileString("Options", "CD Drive", Str_Tmp, Conf_File);
#endif
	
	// SegaCD BRAM cartridge
	if (BRAM_Ex_State & 0x100)
	{
		sprintf (Str_Tmp, "%d", BRAM_Ex_Size);
		WritePrivateProfileString ("Options", "Ram Cart Size", Str_Tmp, Conf_File);
	}
	else
	{
		WritePrivateProfileString ("Options", "Ram Cart Size", "-1", Conf_File);
	}
	
	// Miscellaneous files
	WritePrivateProfileString("Options", "7z Binary", Misc_Filenames._7z_Binary, Conf_File);
	WritePrivateProfileString("Options", "GCOffline path", Misc_Filenames.GCOffline, Conf_File);
	WritePrivateProfileString("Options", "Gens manual path", Misc_Filenames.Manual, Conf_File);
	
	// Controller settings
	sprintf(Str_Tmp, "%d", Controller_1_Type & 0x13);
	WritePrivateProfileString("Input", "P1.Type", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Controller_1B_Type & 0x13);
	WritePrivateProfileString("Input", "P1B.Type", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Controller_1C_Type & 0x13);
	WritePrivateProfileString("Input", "P1C.Type", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Controller_1D_Type & 0x13);
	WritePrivateProfileString("Input", "P1D.Type", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Controller_2_Type & 0x13);
	WritePrivateProfileString("Input", "P2.Type", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Controller_2B_Type & 0x13);
	WritePrivateProfileString("Input", "P2B.Type", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Controller_2C_Type & 0x13);
	WritePrivateProfileString("Input", "P2C.Type", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Controller_2D_Type & 0x13);
	WritePrivateProfileString("Input", "P2D.Type", Str_Tmp, Conf_File);
	
	char tmpKey[16];
	for (i = 0; i < 8; i++)
	{
		sprintf(tmpKey, "%s.Up", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].Up);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.Down", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].Down);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.Left", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].Left);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.Right", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].Right);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.Start", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].Start);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.A", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].A);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.B", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].B);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.C", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].C);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.Mode", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].Mode);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.X", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].X);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.Y", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].Y);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
		sprintf(tmpKey, "%s.Z", PlayerNames[i]);
		sprintf (Str_Tmp, "%d", Keys_Def[i].Z);
		WritePrivateProfileString("Input", tmpKey, Str_Tmp, Conf_File);
	}
	
	return 1;
}


/**
 * Save_As_Config(): Save the current configuration using a user-selected filename.
 * @return 1 if a file was selected.
 */
int Save_As_Config(void)
{
	char filename[GENS_PATH_MAX];
	
	if (UI_SaveFile("Save Config As", NULL, ConfigFile, filename) != 0)
		return 0;
	
	// Make sure a filename was actually selected.
	if (strlen(filename) == 0)
		return 0;
	
	// Filename selected for the config file.
	Save_Config(filename);
	strcpy (Str_Tmp, "config saved in ");
	strcat (Str_Tmp, filename);
	Put_Info (Str_Tmp, 2000);
	return 1;
}


/**
 * Load_Config(): Load GENS configuration.
 * @param Conf_File Configuration filename.
 * @param Game_Active ???
 */
int Load_Config(const char *File_Name, void *Game_Active)
{
	int new_val, i;
	char Conf_File[GENS_PATH_MAX];
	char Save_Path[GENS_PATH_MAX];
	
	INI_LoadConfig(File_Name);
	
	// String copy is needed, since the passed variable might be Str_Tmp.
	strncpy(Conf_File, File_Name, GENS_PATH_MAX);
	
	// Get the default save path.
	Get_Save_Path(Save_Path, GENS_PATH_MAX);
	
	CRam_Flag = 1;
	
	// Paths
	INI_GetString("General", "ROM Path", PathNames.Gens_Path, Rom_Dir, sizeof(Rom_Dir));
	INI_GetString("General", "Save Path", PathNames.Gens_Path, State_Dir, sizeof(State_Dir));
	INI_GetString("General", "SRAM Path", PathNames.Gens_Path, SRAM_Dir, sizeof(SRAM_Dir));
	INI_GetString("General", "BRAM Path", PathNames.Gens_Path, BRAM_Dir, sizeof(BRAM_Dir));
	INI_GetString("General", "Dump Path", PathNames.Gens_Path, Dump_Dir, sizeof(Dump_Dir));
	INI_GetString("General", "Dump GYM Path", PathNames.Gens_Path, Dump_GYM_Dir, sizeof(Dump_GYM_Dir));
	INI_GetString("General", "Screen Shot Path", PathNames.Gens_Path, ScrShot_Dir, sizeof(ScrShot_Dir));
	INI_GetString("General", "Patch Path", PathNames.Gens_Path, Patch_Dir, sizeof(Patch_Dir));
	INI_GetString("General", "IPS Patch Path", PathNames.Gens_Path, IPS_Dir, sizeof(IPS_Dir));
	
	// Genesis BIOS
	INI_GetString("General", "Genesis BIOS", "", BIOS_Filenames.MD_TMSS, sizeof(BIOS_Filenames.MD_TMSS));
	
	// SegaCD BIOSes
	INI_GetString("General", "USA CD BIOS", "", BIOS_Filenames.SegaCD_US, sizeof(BIOS_Filenames.SegaCD_US));
	INI_GetString("General", "EUROPE CD BIOS", "", BIOS_Filenames.MegaCD_EU, sizeof(BIOS_Filenames.MegaCD_EU));
	INI_GetString("General", "JAPAN CD BIOS", "", BIOS_Filenames.MegaCD_JP, sizeof(BIOS_Filenames.MegaCD_JP));
	
	// 32X BIOSes
	INI_GetString("General", "32X 68000 BIOS", "", BIOS_Filenames._32X_MC68000, sizeof(BIOS_Filenames._32X_MC68000));
	INI_GetString("General", "32X Master SH2 BIOS", "", BIOS_Filenames._32X_MC68000, sizeof(BIOS_Filenames._32X_MSH2));
	INI_GetString("General", "32X Slave SH2 BIOS", "", BIOS_Filenames._32X_MC68000, sizeof(BIOS_Filenames._32X_SSH2));
	
	// Last 9 ROMs
	for (i = 0; i < 9; i++)
	{
		sprintf(Str_Tmp, "ROM %d", i + 1);
		INI_GetString("General", Str_Tmp, "", Recent_Rom[i], sizeof(Recent_Rom[i]));
	}
	
	// SegaCD
	// TODO: Use a better default for the CD drive.
	INI_GetString("General", "CD Drive", "/dev/cdrom", CDROM_DEV, sizeof(CDROM_DEV));
	CDROM_SPEED = INI_GetInt("General", "CD Speed", 0);
	
	Current_State = INI_GetInt("General", "State Number", 0);
	Language = INI_GetInt("General", "Language", 0);
	Window_Pos.x = INI_GetInt("General", "Window X", 0);
	Window_Pos.y = INI_GetInt("General", "Window Y", 0);
	Intro_Style = INI_GetInt("General", "Intro Style", 0);
	Effect_Color = INI_GetInt("General", "Free Mode Color", 7);
	Sleep_Time = INI_GetInt("General", "Allow Idle", 0) & 1;
	
	// Color settings
	RMax_Level = INI_GetInt("Graphics", "Red Max", 255);
	GMax_Level = INI_GetInt("Graphics", "Green Max", 255);
	BMax_Level = INI_GetInt("Graphics", "Blue Max", 255);
	
	// Video adjustments
	Contrast_Level = INI_GetInt("Graphics", "Contrast", 100);
	Brightness_Level = INI_GetInt("Graphics", "Brightness", 100);
	Greyscale = INI_GetInt("Graphics", "Greyscale", 0);
	Invert_Color = INI_GetInt("Graphics", "Invert", 0);
	
	// Video settings
	// Render_Mode is decremented by 1 for compatibility with old Gens.
	FS_VSync = INI_GetInt("Graphics", "Full Screen VSync", 0);
	W_VSync = INI_GetInt("Graphics", "Windows VSync", 0);
	Video.Full_Screen = INI_GetInt("Graphics", "Full Screen", 0);
	Video.Render_Mode = INI_GetInt("Graphics", "Render Mode", 1) - 1;
	bpp = (unsigned char)(INI_GetInt("Graphics", "Bits Per Pixel", 32));
	if (bpp != 15 && bpp != 16 && bpp != 32)
	{
		// Invalid bpp. Set it to 32 by default.
		bpp = 32;
	}
#ifdef GENS_OPENGL
	Video.OpenGL = INI_GetInt("Graphics", "Render OpenGL", 0);
	Video.Width_GL = INI_GetInt("Graphics", "OpenGL Width", 640);
	Video.Height_GL = INI_GetInt("Graphics", "OpenGL Height", 480);
	//gl_linear_filter = INI_GetInt("Graphics", "OpenGL Filter", 1);
#endif
	//Set_Render(Full_Screen, -1, 1);
	
	// Recalculate the MD and 32X palettes using the new color and video mode settings.
	Recalculate_Palettes();
	
	Stretch = INI_GetInt("Graphics", "Stretch", 0);
	Blit_Soft = INI_GetInt("Graphics", "Software Blit", 0);
	Sprite_Over = INI_GetInt("Graphics", "Sprite limit", 1);
	Frame_Skip = INI_GetInt("Graphics", "Frame skip", -1);
	
	// Sound settings
	Sound_Rate = INI_GetInt("Sound", "Rate", 22050);
	Sound_Stereo = INI_GetInt("Sound", "Stereo", 1);
	
	if (INI_GetInt("Sound", "Z80 State", 1))
		Z80_State |= 1;
	else
		Z80_State &= ~1;
	
	// Only load the IC sound settings if sound can be initialized.
	new_val = INI_GetInt("Sound", "State", 1);
	if (new_val == Sound_Enable ||
	    (new_val != Sound_Enable && Change_Sound(1)))
	{
		YM2612_Enable = INI_GetInt("Sound", "YM2612 State", 1);
		PSG_Enable = INI_GetInt("Sound", "PSG State", 1);
		DAC_Enable = INI_GetInt("Sound", "DAC State", 1);
		PCM_Enable = INI_GetInt("Sound", "PCM State", 1);
		PWM_Enable = INI_GetInt("Sound", "PWM State", 1);
		CDDA_Enable = INI_GetInt("Sound", "CDDA State", 1);
		
		// Improved sound options
		YM2612_Improv = INI_GetInt("Sound", "YM2612 Improvement", 0);
		DAC_Improv = INI_GetInt("Sound", "DAC Improvement", 0);
		PSG_Improv = INI_GetInt("Sound", "PSG Improvement", 0);
	}
	
	// Country codes
	Country = INI_GetInt("CPU", "Country", -1);
	Country_Order[0] = INI_GetInt("CPU", "Prefered Country 1", 0);
	Country_Order[1] = INI_GetInt("CPU", "Prefered Country 2", 1);
	Country_Order[2] = INI_GetInt("CPU", "Prefered Country 3", 2);
	Check_Country_Order();
	
	// CPU options
	
	SegaCD_Accurate = INI_GetInt("CPU", "Perfect synchro between main and sub CPU (Sega CD)", 0);
	MSH2_Speed = INI_GetInt("CPU", "Main SH2 Speed", 100);
	SSH2_Speed = INI_GetInt("CPU", "Slave SH2 Speed", 100);
	
	// Make sure the SH2 speeds aren't below 0.
	if (MSH2_Speed < 0)
		MSH2_Speed = 0;
	if (SSH2_Speed < 0)
		SSH2_Speed = 0;
	
	// Various settings
	Video.Fast_Blur = INI_GetInt("Options", "Fast Blur", 0);
	Show_FPS = INI_GetInt("Options", "FPS", 0);
	FPS_Style = INI_GetInt("Options", "FPS Style", 0);
	Show_Message = INI_GetInt("Options", "Message", 1);
	Message_Style = INI_GetInt("Options", "Message Style", 0);
	Show_LED = INI_GetInt("Options", "LED", 1);
	Auto_Fix_CS = INI_GetInt("Options", "Auto Fix Checksum", 0);
	Auto_Pause = INI_GetInt("Options", "Auto Pause", 0);
#ifdef GENS_CDROM
	CUR_DEV = INI_GetInt("Options", "CD Drive", 0);
#endif
	
	// SegaCD BRAM cartridge size
	BRAM_Ex_Size = INI_GetInt("Options", "RAM Cart Size", 3);
	if (BRAM_Ex_Size == -1)
	{
		BRAM_Ex_State &= 1;
		BRAM_Ex_Size = 0;
	}
	else if (BRAM_Ex_Size < -1 || BRAM_Ex_Size > 3)
		BRAM_Ex_Size = 3;
	else
		BRAM_Ex_State |= 0x100;
	
	// Miscellaneous files
#if defined(__WIN32__)
	INI_GetString("Options", "7z Binary", "C:\\Program Files\\7-Zip\7z.exe",
		      Misc_Filenames._7z_Binary, sizeof(Misc_Filenames._7z_Binary));
#else
	INI_GetString("Options", "7z Binary", "/usr/bin/7z",
		      Misc_Filenames._7z_Binary, sizeof(Misc_Filenames._7z_Binary));
#endif	
	INI_GetString("Options", "GCOffline path", "GCOffline.chm",
		      Misc_Filenames.GCOffline, sizeof(Misc_Filenames.GCOffline));
	INI_GetString("Options", "Gens manual path", "manual.exe",
		      Misc_Filenames.Manual, sizeof(Misc_Filenames.Manual));
	
	// Controller settings
	Controller_1_Type = INI_GetInt("Input", "P1.Type", 1);
	Controller_1B_Type = INI_GetInt("Input", "P1B.Type", 1);
	Controller_1C_Type = INI_GetInt("Input", "P1C.Type", 1);
	Controller_1D_Type = INI_GetInt("Input", "P1D.Type", 1);
	Controller_2_Type = INI_GetInt("Input", "P2.Type", 1);
	Controller_2B_Type = INI_GetInt("Input", "P2B.Type", 1);
	Controller_2C_Type = INI_GetInt("Input", "P2C.Type", 1);
	Controller_2D_Type = INI_GetInt("Input", "P2D.Type", 1);
	
	for (i = 0; i < 8; i++)
	{
		sprintf(Str_Tmp, "%s.Up", PlayerNames[i]);
		Keys_Def[i].Up = INI_GetInt("Input", Str_Tmp, Keys_Default[i].Up);
		sprintf(Str_Tmp, "%s.Down", PlayerNames[i]);
		Keys_Def[i].Down = INI_GetInt("Input", Str_Tmp, Keys_Default[i].Down);
		sprintf(Str_Tmp, "%s.Left", PlayerNames[i]);
		Keys_Def[i].Left = INI_GetInt("Input", Str_Tmp, Keys_Default[i].Left);
		sprintf(Str_Tmp, "%s.Right", PlayerNames[i]);
		Keys_Def[i].Right = INI_GetInt("Input", Str_Tmp, Keys_Default[i].Right);
		sprintf(Str_Tmp, "%s.Start", PlayerNames[i]);
		Keys_Def[i].Start = INI_GetInt("Input", Str_Tmp, Keys_Default[i].Start);
		sprintf(Str_Tmp, "%s.A", PlayerNames[i]);
		Keys_Def[i].A = INI_GetInt("Input", Str_Tmp, Keys_Default[i].A);
		sprintf(Str_Tmp, "%s.B", PlayerNames[i]);
		Keys_Def[i].B = INI_GetInt("Input", Str_Tmp, Keys_Default[i].B);
		sprintf(Str_Tmp, "%s.C", PlayerNames[i]);
		Keys_Def[i].C = INI_GetInt("Input", Str_Tmp, Keys_Default[i].C);
		sprintf(Str_Tmp, "%s.Mode", PlayerNames[i]);
		Keys_Def[i].Mode = INI_GetInt("Input", Str_Tmp, Keys_Default[i].Mode);
		sprintf(Str_Tmp, "%s.X", PlayerNames[i]);
		Keys_Def[i].X = INI_GetInt("Input", Str_Tmp, Keys_Default[i].X);
		sprintf(Str_Tmp, "%s.Y", PlayerNames[i]);
		Keys_Def[i].Y = INI_GetInt("Input", Str_Tmp, Keys_Default[i].Y);
		sprintf(Str_Tmp, "%s.Z", PlayerNames[i]);
		Keys_Def[i].Z = INI_GetInt("Input", Str_Tmp, Keys_Default[i].Z);
	}
	
	// Create the TeamPlayer I/O table.
	Make_IO_Table();
	
	// Done.
	return 1;
}


/**
 * Load_As_Config(): Load a user-selected configuration file.
 * @param Game_Active ???
 * @return 1 if a file was selected.
 */
int Load_As_Config(void *Game_Active)
{
	char filename[GENS_PATH_MAX];
	
	if (UI_OpenFile("Load Config", NULL, ConfigFile, filename) != 0)
		return 0;
	
	// Make sure a filename was actually selected.
	if (strlen(filename) == 0)
		return 0;
	
	// Filename selected for the config file.
	Load_Config (filename, Game_Active);
	strcpy (Str_Tmp, "config loaded from ");
	strcat (Str_Tmp, filename);
	Put_Info (Str_Tmp, 2000);
	return 1;
}

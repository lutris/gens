/**
 * GENS: Configuration file handler.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "save.h"
#include "config_file.h"
#include "port.h"

#include "gens.h"
#include "g_main.h"
#include "g_input.h"
#include "g_sdldraw.h"
#include "g_sdlsound.h"
#include "g_sdlinput.h"
#include "scrshot.h"
#include "ggenie.h"
#include "misc.h"
#include "g_palette.h"

// CPU
#include "cpu_68k.h"
#include "cpu_sh2.h"
#include "sh2.h"
#include "z80.h"

// Memory
#include "mem_m68k.h"
#include "mem_s68k.h"
#include "mem_sh2.h"
#include "mem_z80.h"
#include "io.h"
#include "rom.h"

// VDP
#include "vdp_io.h"
#include "vdp_rend.h"
#include "vdp_32x.h"

// Audio
#include "ym2612.h"
#include "psg.h"
#include "pcm.h"
#include "pwm.h"

// SegaCD
#include "cd_sys.h"
#include "gfx_cd.h"
#include "lc89510.h"
#include "cd_file.h"
#include "cd_aspi.h"

// UI
#include "ui_proxy.h"
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
	WritePrivateProfileString("General", "Genesis Bios", Genesis_Bios, Conf_File);
	
	// SegaCD BIOSes
	WritePrivateProfileString ("General", "USA CD Bios", US_CD_Bios, Conf_File);
	WritePrivateProfileString ("General", "EUROPE CD Bios", EU_CD_Bios, Conf_File);
	WritePrivateProfileString ("General", "JAPAN CD Bios", JA_CD_Bios, Conf_File);
	
	// 32X BIOSes
	WritePrivateProfileString("General", "32X 68000 Bios", _32X_Genesis_Bios, Conf_File);
	WritePrivateProfileString("General", "32X Master SH2 Bios", _32X_Master_Bios, Conf_File);
	WritePrivateProfileString("General", "32X Slave SH2 Bios", _32X_Slave_Bios, Conf_File);
	
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
	// TODO: BUG: If "Full Scree VSync" is saved before "Full Screen",
	// the Linux reimplementation of WritePrivateProfileString gets confused.
	sprintf(Str_Tmp, "%d", Video.Full_Screen & 1);
	WritePrivateProfileString("Graphics", "Full Screen", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", Video.Render_Mode);
	WritePrivateProfileString("Graphics", "Render Mode", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", FS_VSync & 1);
	WritePrivateProfileString("Graphics", "Full Screen VSync", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", W_VSync & 1);
	WritePrivateProfileString("Graphics", "Windows VSync", Str_Tmp, Conf_File);
	
	sprintf(Str_Tmp, "%d", Video.bpp);
	WritePrivateProfileString("Graphics", "Bits Per Pixel", Str_Tmp, Conf_File);
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
	sprintf(Str_Tmp, "%d", CUR_DEV);
	WritePrivateProfileString("Options", "CD Drive", Str_Tmp, Conf_File);
	
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
	
	// Manuals
	WritePrivateProfileString("Options", "GCOffline path", Settings.PathNames.CGOffline_Path, Conf_File);
	WritePrivateProfileString("Options", "Gens manual path", Settings.PathNames.Manual_Path, Conf_File);
	
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

	// String copy is needed, since the passed variable might be Str_Tmp.
	strncpy(Conf_File, File_Name, GENS_PATH_MAX);
	
	// Get the default save path.
	Get_Save_Path(Save_Path, GENS_PATH_MAX);
	
	CRam_Flag = 1;
	
	// Paths
	GetPrivateProfileString("General", "Rom path", Save_Path,
				&Rom_Dir[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "Save path", Settings.PathNames.Gens_Path,
				 &State_Dir[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "SRAM path", Settings.PathNames.Gens_Path,
				&SRAM_Dir[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "BRAM path", Settings.PathNames.Gens_Path,
				&BRAM_Dir[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "Dump path",	Settings.PathNames.Gens_Path,
				&Dump_Dir[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "Dump GYM path", Settings.PathNames.Gens_Path,
				&Dump_GYM_Dir[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "Screen Shot path", Settings.PathNames.Gens_Path,
				&ScrShot_Dir[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "Patch path", Settings.PathNames.Gens_Path,
				&Patch_Dir[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "IPS Patch path", Settings.PathNames.Gens_Path,
				&IPS_Dir[0], GENS_PATH_MAX, Conf_File);
	
	// Genesis BIOS
	GetPrivateProfileString("General", "Genesis Bios", "",
				&Genesis_Bios[0], GENS_PATH_MAX, Conf_File);
	
	// SegaCD BIOSes
	GetPrivateProfileString("General", "USA CD Bios", "",
				&US_CD_Bios[0],	GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "EUROPE CD Bios", "",
				&EU_CD_Bios[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "JAPAN CD Bios", "",
				&JA_CD_Bios[0], GENS_PATH_MAX, Conf_File);
	
	// 32X BIOSes
	GetPrivateProfileString("General", "32X 68000 Bios", "",
				&_32X_Genesis_Bios[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "32X Master SH2 Bios", "",
				&_32X_Master_Bios[0], GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("General", "32X Slave SH2 Bios", "",
				&_32X_Slave_Bios[0], GENS_PATH_MAX, Conf_File);
	
	// Last 9 ROMs
	for (i = 0; i < 9; i++)
	{
		sprintf(Str_Tmp, "Rom %d", i + 1);
		GetPrivateProfileString("General", Str_Tmp, "",
					&Recent_Rom[i][0], GENS_PATH_MAX, Conf_File);
	}
	
	// SegaCD
	// TODO: Use a better default for the CD drive.
	GetPrivateProfileString("General", "CD Drive", "/dev/cdrom",
				&CDROM_DEV[0], 16, Conf_File);
	CDROM_SPEED = GetPrivateProfileInt("General", "CD Speed", 0, Conf_File);
	
	Current_State = GetPrivateProfileInt("General", "State Number", 0, Conf_File);
	Language = GetPrivateProfileInt("General", "Language", 0, Conf_File);
	Window_Pos.x = GetPrivateProfileInt("General", "Window X", 0, Conf_File);
	Window_Pos.y = GetPrivateProfileInt("General", "Window Y", 0, Conf_File);
	Intro_Style = GetPrivateProfileInt("General", "Intro Style", 0, Conf_File);
	Effect_Color = GetPrivateProfileInt("General", "Free Mode Color", 7, Conf_File);
	Sleep_Time = GetPrivateProfileInt("General", "Allow Idle", 0, Conf_File) & 1;
	
	// 555 or 565 mode
	if (GetPrivateProfileInt ("Graphics", "Force 555", 0, Conf_File))
		Mode_555 = 3;
	else if (GetPrivateProfileInt ("Graphics", "Force 565", 0, Conf_File))
		Mode_555 = 2;
	else
		Mode_555 = 0;
	
	// Color settings
	RMax_Level = GetPrivateProfileInt("Graphics", "Red Max", 255, Conf_File);
	GMax_Level = GetPrivateProfileInt("Graphics", "Green Max", 255, Conf_File);
	BMax_Level = GetPrivateProfileInt("Graphics", "Blue Max", 255, Conf_File);
	
	// Video adjustments
	Contrast_Level = GetPrivateProfileInt("Graphics", "Contrast", 100, Conf_File);
	Brightness_Level = GetPrivateProfileInt("Graphics", "Brightness", 100, Conf_File);
	Greyscale = GetPrivateProfileInt("Graphics", "Greyscale", 0, Conf_File);
	Invert_Color = GetPrivateProfileInt("Graphics", "Invert", 0, Conf_File);
	
	// Recalculate the MD and 32X palettes using the new color settings.
	Recalculate_Palettes ();
	
	// Video settings
	FS_VSync = GetPrivateProfileInt("Graphics", "Full Screen VSync", 0, Conf_File);
	W_VSync = GetPrivateProfileInt("Graphics", "Windows VSync", 0, Conf_File);
	Video.Full_Screen = GetPrivateProfileInt("Graphics", "Full Screen", 0, Conf_File);
	Video.Render_Mode = GetPrivateProfileInt ("Graphics", "Render Mode", 1, Conf_File);
	Video.bpp = GetPrivateProfileInt("Graphics", "Bits Per Pixel", 16, Conf_File);
	Video.OpenGL = GetPrivateProfileInt("Graphics", "Render Opengl", 0, Conf_File);
	Video.Width_GL = GetPrivateProfileInt("Graphics", "Opengl Width", 640, Conf_File);
	Video.Height_GL = GetPrivateProfileInt("Graphics", "Opengl Height", 480, Conf_File);
	//gl_linear_filter = GetPrivateProfileInt("Graphics", "Opengl Filter", 1, Conf_File);
	//Set_Render(Full_Screen, -1, 1);
	
	Stretch = GetPrivateProfileInt("Graphics", "Stretch", 0, Conf_File);
	Blit_Soft = GetPrivateProfileInt("Graphics", "Software Blit", 0, Conf_File);
	Sprite_Over = GetPrivateProfileInt("Graphics", "Sprite limit", 1, Conf_File);
	Frame_Skip = GetPrivateProfileInt("Graphics", "Frame skip", -1, Conf_File);
	
	// Sound settings
	Sound_Rate = GetPrivateProfileInt("Sound", "Rate", 22050, Conf_File);
	Sound_Stereo = GetPrivateProfileInt("Sound", "Stereo", 1, Conf_File);
	
	if (GetPrivateProfileInt ("Sound", "Z80 State", 1, Conf_File))
		Z80_State |= 1;
	else
		Z80_State &= ~1;
	
	// Only load the IC sound settings if sound can be initialized.
	new_val = GetPrivateProfileInt ("Sound", "State", 1, Conf_File);
	if (new_val == Sound_Enable ||
	    (new_val != Sound_Enable && Change_Sound(1)))
	{
		YM2612_Enable = GetPrivateProfileInt("Sound", "YM2612 State", 1, Conf_File);
		PSG_Enable = GetPrivateProfileInt("Sound", "PSG State", 1, Conf_File);
		DAC_Enable = GetPrivateProfileInt("Sound", "DAC State", 1, Conf_File);
		PCM_Enable = GetPrivateProfileInt("Sound", "PCM State", 1, Conf_File);
		PWM_Enable = GetPrivateProfileInt("Sound", "PWM State", 1, Conf_File);
		CDDA_Enable = GetPrivateProfileInt("Sound", "CDDA State", 1, Conf_File);
		
		// Improved sound options
		YM2612_Improv = GetPrivateProfileInt("Sound", "YM2612 Improvement", 0, Conf_File);
		DAC_Improv = GetPrivateProfileInt("Sound", "DAC Improvement", 0, Conf_File);
		PSG_Improv = GetPrivateProfileInt("Sound", "PSG Improvement", 0, Conf_File);
	}
	
	// Country codes
	Country = GetPrivateProfileInt("CPU", "Country", -1, Conf_File);
	Country_Order[0] = GetPrivateProfileInt("CPU", "Prefered Country 1", 0, Conf_File);
	Country_Order[1] = GetPrivateProfileInt("CPU", "Prefered Country 2", 1, Conf_File);
	Country_Order[2] = GetPrivateProfileInt("CPU", "Prefered Country 3", 2, Conf_File);
	Check_Country_Order();
	
	// CPU options
	
	SegaCD_Accurate = GetPrivateProfileInt("CPU", "Perfect synchro between main and sub CPU (Sega CD)", 0, Conf_File);
	MSH2_Speed = GetPrivateProfileInt("CPU", "Main SH2 Speed", 100, Conf_File);
	SSH2_Speed = GetPrivateProfileInt("CPU", "Slave SH2 Speed", 100, Conf_File);
	
	// Make sure the SH2 speeds aren't below 0.
	if (MSH2_Speed < 0)
		MSH2_Speed = 0;
	if (SSH2_Speed < 0)
		SSH2_Speed = 0;
	
	// Various settings
	Video.Fast_Blur = GetPrivateProfileInt("Options", "Fast Blur", 0, Conf_File);
	Show_FPS = GetPrivateProfileInt("Options", "FPS", 0, Conf_File);
	FPS_Style = GetPrivateProfileInt("Options", "FPS Style", 0, Conf_File);
	Show_Message = GetPrivateProfileInt("Options", "Message", 1, Conf_File);
	Message_Style = GetPrivateProfileInt("Options", "Message Style", 0, Conf_File);
	Show_LED = GetPrivateProfileInt("Options", "LED", 1, Conf_File);
	Auto_Fix_CS = GetPrivateProfileInt("Options", "Auto Fix Checksum", 0, Conf_File);
	Auto_Pause = GetPrivateProfileInt("Options", "Auto Pause", 0, Conf_File);
	CUR_DEV = GetPrivateProfileInt("Options", "CD Drive", 0, Conf_File);
	
	// SegaCD BRAM cartridge size
	BRAM_Ex_Size = GetPrivateProfileInt ("Options", "Ram Cart Size", 3, Conf_File);
	if (BRAM_Ex_Size == -1)
	{
		BRAM_Ex_State &= 1;
		BRAM_Ex_Size = 0;
	}
	else
		BRAM_Ex_State |= 0x100;
	
	// Manuals
	GetPrivateProfileString("Options", "GCOffline path", "GCOffline.chm",
				Settings.PathNames.CGOffline_Path, GENS_PATH_MAX, Conf_File);
	GetPrivateProfileString("Options", "Gens manual path", "manual.exe",
				Settings.PathNames.Manual_Path, GENS_PATH_MAX, Conf_File);
	
	// Controller settings
	Controller_1_Type = GetPrivateProfileInt("Input", "P1.Type", 1, Conf_File);
	Controller_1B_Type = GetPrivateProfileInt("Input", "P1B.Type", 1, Conf_File);
	Controller_1C_Type = GetPrivateProfileInt("Input", "P1C.Type", 1, Conf_File);
	Controller_1D_Type = GetPrivateProfileInt("Input", "P1D.Type", 1, Conf_File);
	Controller_2_Type = GetPrivateProfileInt("Input", "P2.Type", 1, Conf_File);
	Controller_2B_Type = GetPrivateProfileInt("Input", "P2B.Type", 1, Conf_File);
	Controller_2C_Type = GetPrivateProfileInt("Input", "P2C.Type", 1, Conf_File);
	Controller_2D_Type = GetPrivateProfileInt("Input", "P2D.Type", 1, Conf_File);
	
	for (i = 0; i < 8; i++)
	{
		sprintf(Str_Tmp, "%s.Up", PlayerNames[i]);
		Keys_Def[i].Up = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].Up, Conf_File);
		sprintf(Str_Tmp, "%s.Down", PlayerNames[i]);
		Keys_Def[i].Down = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].Down, Conf_File);
		sprintf(Str_Tmp, "%s.Left", PlayerNames[i]);
		Keys_Def[i].Left = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].Left, Conf_File);
		sprintf(Str_Tmp, "%s.Right", PlayerNames[i]);
		Keys_Def[i].Right = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].Right, Conf_File);
		sprintf(Str_Tmp, "%s.Start", PlayerNames[i]);
		Keys_Def[i].Start = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].Start, Conf_File);
		sprintf(Str_Tmp, "%s.A", PlayerNames[i]);
		Keys_Def[i].A = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].A, Conf_File);
		sprintf(Str_Tmp, "%s.B", PlayerNames[i]);
		Keys_Def[i].B = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].B, Conf_File);
		sprintf(Str_Tmp, "%s.C", PlayerNames[i]);
		Keys_Def[i].C = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].C, Conf_File);
		sprintf(Str_Tmp, "%s.Mode", PlayerNames[i]);
		Keys_Def[i].Mode = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].Mode, Conf_File);
		sprintf(Str_Tmp, "%s.X", PlayerNames[i]);
		Keys_Def[i].X = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].X, Conf_File);
		sprintf(Str_Tmp, "%s.Y", PlayerNames[i]);
		Keys_Def[i].Y = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].Y, Conf_File);
		sprintf(Str_Tmp, "%s.Z", PlayerNames[i]);
		Keys_Def[i].Z = GetPrivateProfileInt("Input", Str_Tmp, Keys_Default[i].Z, Conf_File);
	}
	
	Make_IO_Table();
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

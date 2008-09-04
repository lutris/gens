/**
 * GENS: Configuration file handler.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "save.h"
#include "config_file.hpp"
#include "port/port.h"
#include "port/ini.hpp"

#include "emulator/gens.h"
#include "emulator/g_main.hpp"
#include "emulator/g_input.hpp"
#include "sdllayer/g_sdlsound.h"
#include "sdllayer/g_sdlinput.h"
#include "util/gfx/scrshot.h"
#include "util/file/ggenie.h"
#include "gens_core/misc/misc.h"
#include "emulator/g_palette.h"
#include "emulator/g_update.hpp"

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
#include "segacd/cd_sys.hpp"
#include "gens_core/gfx/gfx_cd.h"
#include "segacd/lc89510.h"
#include "segacd/cd_file.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.h"
#endif

// UI
#include "emulator/ui_proxy.hpp"
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
	
	// Load the configuration file into the INI handler.
	INI_LoadConfig(Conf_File);
	
	// Paths
	INI_WriteString("General", "ROM Path", Rom_Dir);
	INI_WriteString("General", "Save Path", State_Dir);
	INI_WriteString("General", "SRAM Path", SRAM_Dir);
	INI_WriteString("General", "BRAM Path", BRAM_Dir);
	INI_WriteString("General", "Dump Path", Dump_Dir);
	INI_WriteString("General", "Dump GYM Path", Dump_GYM_Dir);
	INI_WriteString("General", "Screen Shot Path", ScrShot_Dir);
	INI_WriteString("General", "Patch Path", Patch_Dir);
	INI_WriteString("General", "IPS Patch Path", IPS_Dir);
	
	// Genesis BIOS
	INI_WriteString("General", "Genesis BIOS", BIOS_Filenames.MD_TMSS);
	
	// SegaCD BIOSes
	INI_WriteString("General", "USA CD BIOS", BIOS_Filenames.SegaCD_US);
	INI_WriteString("General", "Europe CD BIOS", BIOS_Filenames.MegaCD_EU);
	INI_WriteString("General", "Japan CD BIOS", BIOS_Filenames.MegaCD_JP);
	
	// 32X BIOSes
	INI_WriteString("General", "32X 68000 BIOS", BIOS_Filenames._32X_MC68000);
	INI_WriteString("General", "32X Master SH2 BIOS", BIOS_Filenames._32X_MSH2);
	INI_WriteString("General", "32X Slave SH2 BIOS", BIOS_Filenames._32X_SSH2);
	
	// Last 9 ROMs
	for (i = 0; i < 9; i++)
	{
		sprintf(Str_Tmp, "ROM %d", i + 1);
		INI_WriteString("General", Str_Tmp, Recent_Rom[i]);
	}
	
#ifdef GENS_CDROM
	// SegaCD
	INI_WriteString("General", "CD Drive", CDROM_DEV);
	INI_WriteInt("General", "CD Speed", CDROM_SPEED);
	/* TODO: I'm assuming this code is for Win32...
	INI_WriteInt("Options", "CD Drive", CUR_DEV);
	*/
#endif
	
	INI_WriteInt("General", "State Number", Current_State);
	INI_WriteInt("General", "Language", Language);
	INI_WriteInt("General", "Window X", Window_Pos.x);
	INI_WriteInt("General", "Window Y", Window_Pos.y);
	INI_WriteInt("General", "Intro Style", Intro_Style);
	INI_WriteInt("General", "Free Mode Color", draw->introEffectColor());
	
	// Video adjustments
	INI_WriteInt("Graphics", "Contrast", Contrast_Level);
	INI_WriteInt("Graphics", "Brightness", Brightness_Level);
	INI_WriteInt("Graphics", "Greyscale", Greyscale & 1);
	INI_WriteInt("Graphics", "Invert", Invert_Color & 1);
	
	// Video settings
	// Render_Mode is incremented by 1 for compatibility with old Gens.
	// TODO: BUG: If "Full Screen VSync" is saved before "Full Screen",
	// the Linux reimplementation of WritePrivateProfileString gets confused.
	INI_WriteBool("Graphics", "Full Screen", draw->fullScreen());
	INI_WriteInt("Graphics", "Render Fullscreen", Video.Render_FS + 1);
	INI_WriteInt("Graphics", "Full Screen VSync", Video.VSync_FS & 1);
	INI_WriteInt("Graphics", "Windows VSync", Video.VSync_W & 1);
	INI_WriteInt("Graphics", "Render Windowed", Video.Render_W + 1);
	
	INI_WriteInt("Graphics", "Bits Per Pixel", bpp);
#ifdef GENS_OPENGL
	INI_WriteInt("Graphics", "Render OpenGL", (Video.OpenGL ? 1 : 0));
	INI_WriteInt("Graphics", "OpenGL Width", Video.Width_GL);
	INI_WriteInt("Graphics", "OpenGL Height", Video.Height_GL);
	//INI_WriteInt("Graphics", "OpenGL Filter", gl_linear_filter);
#endif
	
	INI_WriteBool("Graphics", "Stretch", draw->stretch());
	INI_WriteBool("Graphics", "Software Blit", draw->swRender());
	INI_WriteInt("Graphics", "Sprite Limit", Sprite_Over & 1);
	INI_WriteInt("Graphics", "Frame Skip", Frame_Skip);
	
	// Sound settings
	INI_WriteInt("Sound", "State", Sound_Enable);
	INI_WriteInt("Sound", "Rate", Sound_Rate);
	INI_WriteInt("Sound", "Stereo", Sound_Stereo);
	
	INI_WriteInt("Sound", "Z80 State", Z80_State & 1);
	INI_WriteInt("Sound", "YM2612 State", YM2612_Enable & 1);
	INI_WriteInt("Sound", "PSG State", PSG_Enable & 1);
	INI_WriteInt("Sound", "DAC State", DAC_Enable & 1);
	INI_WriteInt("Sound", "PCM State", PCM_Enable & 1);
	INI_WriteInt("Sound", "PWM State", PWM_Enable & 1);
	INI_WriteInt("Sound", "CDDA State", CDDA_Enable & 1);
	
	// Improved sound options
	INI_WriteInt("Sound", "YM2612 Improvement", YM2612_Improv & 1);
	INI_WriteInt("Sound", "DAC Improvement", DAC_Improv & 1);
	INI_WriteInt("Sound", "PSG Improvement", PSG_Improv & 1);
	
	// Country codes
	INI_WriteInt("CPU", "Country", Country);
	INI_WriteInt("CPU", "Prefered Country 1", Country_Order[0]);
	INI_WriteInt("CPU", "Prefered Country 2", Country_Order[1]);
	INI_WriteInt("CPU", "Prefered Country 3", Country_Order[2]);
	
	// CPU options
	
	INI_WriteInt("CPU", "Perfect synchro between main and sub CPU (Sega CD)", SegaCD_Accurate);
	
	INI_WriteInt("CPU", "Main SH2 Speed", MSH2_Speed);
	INI_WriteInt("CPU", "Slave SH2 Speed", SSH2_Speed);
	
	// Various settings
	INI_WriteBool("Options", "Fast Blur", draw->fastBlur());
	INI_WriteBool("Options", "FPS", draw->fpsEnabled());
	INI_WriteInt("Options", "FPS Style", draw->fpsStyle());
	INI_WriteBool("Options", "Message", draw->msgEnabled());
	INI_WriteInt("Options", "Message Style", draw->msgStyle());
	INI_WriteInt("Options", "LED", Show_LED & 1);
	INI_WriteInt("Options", "Auto Fix Checksum", Auto_Fix_CS & 1);
	INI_WriteInt("Options", "Auto Pause", Auto_Pause & 1);
	
	// SegaCD BRAM cartridge
	if (BRAM_Ex_State & 0x100)
		INI_WriteInt("Options", "RAM Cart Size", BRAM_Ex_Size);
	else
		INI_WriteInt("Options", "RAM Cart Size", -1);
	
	// Miscellaneous files
	INI_WriteString("Options", "7z Binary", Misc_Filenames._7z_Binary);
	INI_WriteString("Options", "GCOffline path", Misc_Filenames.GCOffline);
	INI_WriteString("Options", "Gens manual path", Misc_Filenames.Manual);
	
	// Controller settings
	INI_WriteInt("Input", "P1.Type", Controller_1_Type & 0x13);
	INI_WriteInt("Input", "P1B.Type", Controller_2B_Type & 0x13);
	INI_WriteInt("Input", "P1C.Type", Controller_2C_Type & 0x13);
	INI_WriteInt("Input", "P1D.Type", Controller_2D_Type & 0x13);
	INI_WriteInt("Input", "P2.Type", Controller_2_Type & 0x13);
	INI_WriteInt("Input", "P2B.Type", Controller_2B_Type & 0x13);
	INI_WriteInt("Input", "P2C.Type", Controller_2C_Type & 0x13);
	INI_WriteInt("Input", "P2D.Type", Controller_2D_Type & 0x13);
	
	char tmpKey[16];
	for (i = 0; i < 8; i++)
	{
		sprintf(tmpKey, "%s.Up", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].Up);
		sprintf(tmpKey, "%s.Down", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].Down);
		sprintf(tmpKey, "%s.Left", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].Left);
		sprintf(tmpKey, "%s.Right", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].Right);
		sprintf(tmpKey, "%s.Start", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].Start);
		sprintf(tmpKey, "%s.A", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].A);
		sprintf(tmpKey, "%s.B", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].B);
		sprintf(tmpKey, "%s.C", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].C);
		sprintf(tmpKey, "%s.Mode", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].Mode);
		sprintf(tmpKey, "%s.X", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].X);
		sprintf(tmpKey, "%s.Y", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].Y);
		sprintf(tmpKey, "%s.Z", PlayerNames[i]);
		INI_WriteInt("Input", tmpKey, Keys_Def[i].Z);
	}
	
	// Save the INI file.
	INI_SaveConfig(Conf_File);
	
	// Clear the loaded INI file.
	INI_Clear();
	
	// Done.
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
	INI_GetString("General", "Europe CD BIOS", "", BIOS_Filenames.MegaCD_EU, sizeof(BIOS_Filenames.MegaCD_EU));
	INI_GetString("General", "Japan CD BIOS", "", BIOS_Filenames.MegaCD_JP, sizeof(BIOS_Filenames.MegaCD_JP));
	
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
	
#ifdef GENS_CDROM
	// SegaCD
	// TODO: Use a better default for the CD drive.
	INI_GetString("General", "CD Drive", "/dev/cdrom", CDROM_DEV, sizeof(CDROM_DEV));
	CDROM_SPEED = INI_GetInt("General", "CD Speed", 0);
	/* TODO: I'm assuming this code is for Win32...
	CUR_DEV = INI_GetInt("Options", "CD Drive", 0);
	*/
#endif
	
	Current_State = INI_GetInt("General", "State Number", 0);
	Language = INI_GetInt("General", "Language", 0);
	Window_Pos.x = INI_GetInt("General", "Window X", 0);
	Window_Pos.y = INI_GetInt("General", "Window Y", 0);
	Intro_Style = INI_GetInt("General", "Intro Style", 0);
	draw->setIntroEffectColor(INI_GetInt("General", "Free Mode Color", 7));
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
	Video.VSync_FS = INI_GetInt("Graphics", "Full Screen VSync", 0) & 1;
	Video.VSync_W = INI_GetInt("Graphics", "Windows VSync", 0) & 1;
	draw->setFullScreen(INI_GetBool("Graphics", "Full Screen", false));
	Video.Render_FS = INI_GetInt("Graphics", "Render Fullscreen", 2) - 1; // Default: Double
	Video.Render_W = INI_GetInt("Graphics", "Render Windowed", 2) - 1;    // Default: Double
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
	// Set the OpenGL renderer.
	// NOTE: Don't do this while Gens is loading; otherwise, GTK+ raises an assert
	// because the window hasn't been created yet.
	if (is_gens_running())
		Change_OpenGL(Video.OpenGL);
	
	//Set_Render(Full_Screen, -1, 1);
	
	// Recalculate the MD and 32X palettes using the new color and video mode settings.
	// NOTE: This is only done if Gens is currently running.
	// If Gens isn't running, then this is before Gens had a chance to parse
	// command line arguments, so things may change later.
	if (is_gens_running())
		Recalculate_Palettes();
	
	draw->setStretch(INI_GetBool("Graphics", "Stretch", false));
	draw->setSwRender(INI_GetBool("Graphics", "Software Blit", false));
	Sprite_Over = INI_GetInt("Graphics", "Sprite Limit", 1);
	Frame_Skip = INI_GetInt("Graphics", "Frame Skip", -1);
	
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
	draw->setFastBlur(INI_GetBool("Options", "Fast Blur", false));
	draw->setFPSEnabled(INI_GetBool("Options", "FPS", false));
	draw->setFPSStyle(INI_GetInt("Options", "FPS Style", 0));
	draw->setMsgEnabled(INI_GetBool("Options", "Message", true));
	draw->setMsgStyle(INI_GetInt("Options", "Message Style", 0));
	Show_LED = INI_GetInt("Options", "LED", 1);
	Auto_Fix_CS = INI_GetInt("Options", "Auto Fix Checksum", 0);
	Auto_Pause = INI_GetInt("Options", "Auto Pause", 0);
	
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

/***************************************************************************
 * Gens: (Win32) Main Window - Callback Functions.                         *
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
#endif

#include "emulator/g_main.hpp"
#include "gens_window.hpp"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"

#include "game_genie/game_genie_window_misc.h"
#include "controller_config/controller_config_window_misc.hpp"
#include "bios_misc_files/bios_misc_files_window_misc.hpp"
#include "directory_config/directory_config_window_misc.hpp"
#include "general_options/general_options_window_misc.hpp"
#include "about/about_window.hpp"
#include "color_adjust/color_adjust_window_misc.h"
#include "country_code/country_code_window_misc.h"

#if 0
#ifdef GENS_OPENGL
#include "opengl_resolution/opengl_resolution_window_misc.h"
#endif /* GENS_OPENGL */
#endif

#ifdef GENS_CDROM
#include "select_cdrom/select_cdrom_window_misc.hpp"
#endif /* GENS_CDROM */

#include "emulator/ui_proxy.hpp"
#include "util/file/config_file.hpp"

#include "ui/gens_ui.hpp"

#include "util/sound/gym.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "util/file/save.hpp"
#include "gens_core/cpu/z80/z80.h"
#include "util/gfx/scrshot.h"

// Sega CD
#include "emulator/g_mcd.hpp"

// 32X
#include "gens_core/cpu/sh2/sh2.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// Needed on Win32
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"
#include "segacd/cd_sys.hpp"

// C++ includes
#include <string>
using std::string;


// For some reason, these aren't extern'd anywhere...
extern "C"
{
	void main68k_reset();
	void sub68k_reset();
}

// Menu Command Definitions
#include "gens_window_menu.h"

// Non-Menu Command Definitions
#include "gens_window_cmds.h"

#include "video/v_draw_ddraw.hpp"
#include "input/input_dinput.hpp"
static bool paintsEnabled = true;

static void on_gens_window_close(void);
static void on_gens_window_FileMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void on_gens_window_GraphicsMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void on_gens_window_CPUMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void on_gens_window_SoundMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void on_gens_window_OptionsMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void on_gens_window_HelpMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void on_gens_window_NonMenuCmd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static void fullScreenPopupMenu(HWND hWnd);


// TODO: If a radio menu item is selected but is already enabled, don't do anything.


/**
 * Gens_Window_WndProc(): The Gens window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
LRESULT CALLBACK Gens_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rectGensWindow;
	
	switch(message)
	{
		case WM_CLOSE:
			on_gens_window_close();
			return 0;
		
		case WM_CREATE:
			Active = 1;
			break;
		
		case WM_MENUSELECT:
		case WM_ENTERSIZEMOVE:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
			// Prevent audio stuttering when one of the following events occurs:
			// - Menu is opened.
			// - Window is resized.
			// - Left/Right mouse button down on title bar.
			audio->clearSoundBuffer();
			break;
		
		case WM_EXITSIZEMOVE:
			if (draw->fullScreen())
				break;
			
			// Save the window coordinates.
			GetWindowRect(hWnd, &rectGensWindow);
			Window_Pos.x = rectGensWindow.left;
			Window_Pos.y = rectGensWindow.top;
			break;
		
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				// Set the DirectInput cooperative level.
				reinterpret_cast<Input_DInput*>(input)->setCooperativeLevel(hWnd);
			}
			break;
		
		case WM_PAINT:
			HDC hDC;
			PAINTSTRUCT ps;
			
			hDC = BeginPaint(hWnd, &ps);
			
			if (paintsEnabled)
			{
				((VDraw_DDraw*)draw)->clearPrimaryScreen();
				draw->flip();
			}
			
			EndPaint(hWnd, &ps);
			break;
		
		case WM_RBUTTONDOWN:
			if (draw->fullScreen())
				fullScreenPopupMenu(hWnd);
			break;
		
		case WM_COMMAND:
			// Menu item.
			switch (LOWORD(wParam) & 0xF000)
			{
				case IDM_FILE_MENU:
					on_gens_window_FileMenu(hWnd, message, wParam, lParam);
					break;
				case IDM_GRAPHICS_MENU:
					on_gens_window_GraphicsMenu(hWnd, message, wParam, lParam);
					break;
				case IDM_CPU_MENU:
					on_gens_window_CPUMenu(hWnd, message, wParam, lParam);
					break;
				case IDM_SOUND_MENU:
					on_gens_window_SoundMenu(hWnd, message, wParam, lParam);
					break;
				case IDM_OPTIONS_MENU:
					on_gens_window_OptionsMenu(hWnd, message, wParam, lParam);
					break;
				case IDM_HELP_MENU:
					on_gens_window_HelpMenu(hWnd, message, wParam, lParam);
					break;
				case IDCMD_NONMENU_COMMANDS:
					on_gens_window_NonMenuCmd(hWnd, message, wParam, lParam);
					break;
			}
			break;
		
#if 0
#ifdef GENS_DEBUGGER
		case WM_KEYDOWN:
			// TODO: Make sure this is correct.
			if (Debug)
				Debug_Event(wParam, 0);
			break;
#endif /* GENS_DEBUGGER */
#endif
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * Window is closed.
 */
static void on_gens_window_close(void)
{
	//Modif N - making sure sound doesn't stutter on exit
	if (audio->soundInitialized())
		audio->clearSoundBuffer();
	
	close_gens();
}


/**
 * on_gens_window_FileMenu(): File Menu item has been selected.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam LOWORD(wParam) == Selected menu item.
 * @param lParam
 */
static void on_gens_window_FileMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDM_FILE_OPENROM:
			if (audio->playingGYM())
				Stop_Play_GYM();
			if (Get_Rom() != -1)
				Sync_Gens_Window();
			break;
		
#ifdef GENS_CDROM
		case IDM_FILE_BOOTCD:
			if (!ASPI_Initialized || !Num_CD_Drive)
			{
				printf("ASPI not initialized and/or no CD-ROM drive(s) detected.\n");
				break;
			}
			
			/*
			if (Check_If_Kaillera_Running())
			return 0;
			*/
			if (audio->playingGYM())
				Stop_Play_GYM();
	
			Free_Rom(Game); // Don't forget it !
			SegaCD_Started = Init_SegaCD(NULL);
			Sync_Gens_Window();
			break;
#endif /* GENS_CDROM */
		
		case IDM_FILE_NETPLAY:
			// Netplay isn't supported yet in Gens/GS.
			// There are only two ways this message could be received:
			// 1. Someone sent the message using another program.
			// 2. Someone used a Win32 API tool to remove the MF_GRAYED style.
			//
			// So, let's give them what they want: A Rick Roll! :)
			Paused = 1;
			//Pause_Screen();
			audio->clearSoundBuffer();
			ShellExecute(NULL, NULL, "http://www.youtube.com/watch?v=oHg5SJYRHA0", NULL, NULL, SW_MAXIMIZE);
			break;
		
		case IDM_FILE_CLOSEROM:
			if (audio->soundInitialized())
				audio->clearSoundBuffer();
	
#ifdef GENS_DEBUGGER
			Debug = 0;
#endif /* GENS_DEBUGGER */
	
			/* TODO: NetPlay
			if (Net_Play)
			{
				if (Video.Full_Screen)
				Set_Render(0, -1, 1);
			}
			*/
			
			Free_Rom(Game);
			Sync_Gens_Window();
			break;
		
		case IDM_FILE_GAMEGENIE:
			Open_Game_Genie();
			break;
		
		case IDM_FILE_ROMHISTORY_0:
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
			if (audio->playingGYM())
				Stop_Play_GYM();
			Open_Rom(Recent_Rom[LOWORD(wParam) - IDM_FILE_ROMHISTORY]);
			Sync_Gens_Window();
			break;
		
		case IDM_FILE_LOADSTATE:
			/*
			if (Check_If_Kaillera_Running())
			return 0;
			*/
			Str_Tmp[0] = 0;
			if (Change_File_L(Str_Tmp, State_Dir) == 1)
				Load_State(Str_Tmp);
			break;
		
		case IDM_FILE_SAVESTATE:
			/*
			if (Check_If_Kaillera_Running())
			return 0;
			*/
			Str_Tmp[0] = 0;
			if (Change_File_S(Str_Tmp, State_Dir) == 1)
				Save_State(Str_Tmp);
			break;
		
		case IDM_FILE_QUICKLOAD:
			/*
			if (Check_If_Kaillera_Running())
			return 0;
			*/
			Str_Tmp[0] = 0;
			Get_State_File_Name(Str_Tmp);
			Load_State(Str_Tmp);
			break;
		
		case IDM_FILE_QUICKSAVE:
			/*
			if (Check_If_Kaillera_Running())
			return 0;
			*/
			Str_Tmp[0] = 0;
			Get_State_File_Name(Str_Tmp);
			Save_State(Str_Tmp);
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
			Set_Current_State(LOWORD(wParam) - IDM_FILE_CHANGESTATE);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDM_FILE_EXIT:
			close_gens();
			break;
	}
}


/**
 * on_gens_window_GraphicsMenu(): Graphics Menu item has been selected.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam LOWORD(wParam) == Selected menu item.
 * @param lParam
 */
static void on_gens_window_GraphicsMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDM_GRAPHICS_FULLSCREEN:
			/*
			if (Full_Screen)
				Set_Render(0, -1, 1);
			else
				Set_Render(1, Render_FS, 1);
			*/
			
			draw->setFullScreen(!draw->fullScreen());
			// TODO: See if draw->setRender() is still needed.
			//draw->setRender(Video.Render_Mode);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDM_GRAPHICS_VSYNC:
			if (draw->fullScreen())
				Change_VSync(!Video.VSync_FS);
			else
				Change_VSync(!Video.VSync_W);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDM_GRAPHICS_STRETCH:
			Change_Stretch(!draw->stretch());
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDM_GRAPHICS_COLORADJUST:
			Open_Color_Adjust();
			break;
				
		case IDM_GRAPHICS_SPRITELIMIT:
			// Sprite Limit
			Set_Sprite_Limit(!Sprite_Over);
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
			Set_Frame_Skip(LOWORD(wParam) - IDM_GRAPHICS_FRAMESKIP - 1);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDM_GRAPHICS_SCREENSHOT:
			audio->clearSoundBuffer();
			Save_Shot();
			break;
		
		default:
			if ((LOWORD(wParam) & 0xFF00) == IDM_GRAPHICS_RENDER)
			{
				// Render mode change.
				draw->setRender(LOWORD(wParam) - IDM_GRAPHICS_RENDER);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
	}
}


/**
 * on_gens_window_CPUMenu(): CPU Menu item has been selected.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam LOWORD(wParam) == Selected menu item.
 * @param lParam
 */
static void on_gens_window_CPUMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDM_CPU_COUNTRY_AUTO:
		case IDM_CPU_COUNTRY_JAPAN_NTSC:
		case IDM_CPU_COUNTRY_USA:
		case IDM_CPU_COUNTRY_EUROPE:
		case IDM_CPU_COUNTRY_JAPAN_PAL:
			Change_Country(LOWORD(wParam) - IDM_CPU_COUNTRY - 1);
			Sync_Gens_Window_CPUMenu();
			break;
		
		case IDM_CPU_COUNTRY_ORDER:
			Open_Country_Code();
			break;
		
		case IDM_CPU_HARDRESET:
			system_reset();
			break;
		
		case IDM_CPU_RESET68K:
		case IDM_CPU_RESETMAIN68K:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			
			if (!Game)
				return;
			
			Paused = 0;
			main68k_reset();
			if (Genesis_Started || _32X_Started)
			{
				MESSAGE_L("68000 CPU reset", "68000 CPU reset", 1000);
			}
			else if(SegaCD_Started)
			{
				MESSAGE_L("Main 68000 CPU reset", "Main 68000 CPU reset", 1000);
			}
			break;
		
		case IDM_CPU_RESETSUB68K:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			
			if (!Game || !SegaCD_Started)
				return;
			
			Paused = 0;
			sub68k_reset();
			MESSAGE_L("Sub 68000 CPU reset", "Sub 68000 CPU reset", 1000);
			break;
		
		case IDM_CPU_RESETMAINSH2:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			
			if (!Game || !_32X_Started)
				return;
			
			Paused = 0;
			SH2_Reset(&M_SH2, 1);
			MESSAGE_L("Master SH2 reset", "Master SH2 reset", 1000);
			break;
		
		case IDM_CPU_RESETSUBSH2:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			
			if (!Game || !_32X_Started)
				return;
			
			Paused = 0;
			SH2_Reset(&S_SH2, 1);
			MESSAGE_L("Slave SH2 reset", "Slave SH2 reset", 1000);
			break;
		
		case IDM_CPU_RESETZ80:
			/*
			if (Check_If_Kaillera_Running())
				return 0;
			*/
			
			if (!Game)
				return;
			
			z80_Reset(&M_Z80);
			MESSAGE_L("Z80 reset", "Z80 reset", 1000);
			break;
		
		case IDM_CPU_SEGACDPERFECTSYNC:
			Change_SegaCD_PerfectSync(!SegaCD_Accurate);
			Sync_Gens_Window_CPUMenu();
			break;
		
		default:
			if ((LOWORD(wParam) & 0xFF00) == IDM_CPU_DEBUG)
			{
				// Debug mode change.
				Change_Debug((LOWORD(wParam) - IDM_CPU_DEBUG) + 1);
				Sync_Gens_Window_CPUMenu();
			}
			break;
	}
}


/**
 * on_gens_window_SoundMenu(): Sound Menu item has been selected.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam LOWORD(wParam) == Selected menu item.
 * @param lParam
 */
static void on_gens_window_SoundMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDM_SOUND_ENABLE:
			Change_Sound(!audio->enabled());
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_STEREO:
			Change_Sound_Stereo(!audio->stereo());
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_Z80:
			Change_Z80(!(Z80_State & 1));
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_YM2612:
			Change_YM2612(!YM2612_Enable);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_YM2612_IMPROVED:
			Change_YM2612_Improved(!YM2612_Improv);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_DAC:
			Change_DAC(!DAC_Enable);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_DAC_IMPROVED:
			Change_DAC_Improved(!DAC_Improv);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_PSG:
			Change_PSG(!PSG_Enable);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_PSG_IMPROVED:
			Change_PSG_Improved(!PSG_Improv);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_PCM:
			Change_PCM(!PCM_Enable);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_PWM:
			Change_PWM(!PWM_Enable);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_CDDA:
			Change_CDDA(!CDDA_Enable);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_WAVDUMP:
			// Change WAV dump status.
			if (!audio->dumpingWAV())
				audio->startWAVDump();
			else
				audio->stopWAVDump();
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDM_SOUND_GYMDUMP:
			// Change GYM dump status.
			if (!GYM_Dumping)
				Start_GYM_Dump();
			else
				Stop_GYM_Dump();
			Sync_Gens_Window_SoundMenu();
			break;
		
		default:
			if ((LOWORD(wParam) & 0xFF00) == IDM_SOUND_RATE)
			{
				// Sample rate change.
				Change_Sample_Rate(LOWORD(wParam) - IDM_SOUND_RATE);
				Sync_Gens_Window_SoundMenu();
			}
			break;
	}
}


/**
 * on_gens_window_OptionsMenu(): Options Menu item has been selected.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam LOWORD(wParam) == Selected menu item.
 * @param lParam
 */
static void on_gens_window_OptionsMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDM_OPTIONS_GENERAL:
			Open_General_Options();
			break;
		
		case IDM_OPTIONS_JOYPADS:
			Open_Controller_Config();
			break;
		
		case IDM_OPTIONS_BIOSMISCFILES:
			Open_BIOS_Misc_Files();
			break;
		
		case IDM_OPTIONS_CURRENT_CD_DRIVE:
			Open_Select_CDROM();
			break;
		
		case IDM_OPTIONS_LOADCONFIG:
			printf("ENABLE: %d\n", audio->enabled());
			Load_As_Config(Game);
			printf("ENABLE: %d\n", audio->enabled());
			Sync_Gens_Window();
			break;
		
		case IDM_OPTIONS_DIRECTORIES:
			Open_Directory_Config();
			break;
		
		case IDM_OPTIONS_SAVECONFIGAS:
			Save_As_Config();
			break;
		
		default:
			if ((LOWORD(wParam) & 0xFF00) == IDM_OPTIONS_SEGACDSRAMSIZE)
			{
				// SegaCD SRAM Size change.
				Change_SegaCD_SRAM_Size(LOWORD(wParam) - IDM_OPTIONS_SEGACDSRAMSIZE - 1);
				Sync_Gens_Window_OptionsMenu();
			}
			break;
	}
}


/**
 * on_gens_window_HelpMenu(): Help Menu item has been selected.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam LOWORD(wParam) == Selected menu item.
 * @param lParam
 */
static void on_gens_window_HelpMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDM_HELP_ABOUT:
			create_about_window();
			break;
	}
}


/**
 * on_gens_window_NonMenuCmd(): Non-Menu Command has been activated.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam LOWORD(wParam) == Command.
 * @param lParam
 */
static void on_gens_window_NonMenuCmd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int rendMode;
	
	switch (LOWORD(wParam))
	{
		case IDCMD_ESC:
			if (Quick_Exit)
				close_gens();
			
			if (Debug)
			{
				Change_Debug(0);
				Paused = 0;
				Sync_Gens_Window_CPUMenu();
			}
			else if (Paused)
			{
				Paused = 0;
			}
			else
			{
				Paused = 1;
				//Pause_Screen();
				audio->clearSoundBuffer();
			}
			break;
		
		case IDCMD_PAUSE:
			if (Paused)
			{
				Paused = 0;
			}
			else
			{
				Paused = 1;
				//Pause_Screen();
				audio->clearSoundBuffer();
			}
			break;
		
		case IDCMD_FRAMESKIP_AUTO:
			Set_Frame_Skip(-1);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDCMD_FRAMESKIP_DEC:
			if (Frame_Skip == -1)
			{
				Set_Frame_Skip(0);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (Frame_Skip > 0)
			{
				Set_Frame_Skip(Frame_Skip - 1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case IDCMD_FRAMESKIP_INC:
			if (Frame_Skip == -1)
			{
				Set_Frame_Skip(1);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (Frame_Skip < 8)
			{
				Set_Frame_Skip(Frame_Skip + 1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case IDCMD_SAVESLOT_DEC:
			Set_Current_State((Current_State + 9) % 10);
			Sync_Gens_Window_FileMenu();
			break;
		
		case IDCMD_SAVESLOT_INC:
			Set_Current_State((Current_State + 1) % 10);
			Sync_Gens_Window_FileMenu();
			break;
		
		case IDCMD_SWBLIT:
			Change_Blit_Style();
			break;
		
		case IDCMD_FASTBLUR:
			Change_Fast_Blur();
			break;
		
		case IDCMD_YM2612_IMPROVED:
			Change_YM2612_Improved(!YM2612_Improv);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDCMD_DAC_IMPROVED:
			Change_DAC_Improved(!DAC_Improv);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDCMD_PSG_IMPROVED:
			Change_PSG_Improved(!PSG_Improv);
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDCMD_FPS:
			draw->setFPSEnabled(!draw->fpsEnabled());
			break;
		
		case IDCMD_RENDERMODE_DEC:
			rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
			if (rendMode > 0)
			{
				draw->setRender(rendMode - 1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case IDCMD_RENDERMODE_INC:
			// TODO: Make filters constants.
			// There's already NB_FILTER, but it has the wrong numbers...
			rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
			if (rendMode < 11)
			{
				draw->setRender(rendMode + 1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case IDCMD_CHANGE_CD:
			if (SegaCD_Started)
				Change_CD();
			break;
		
		default:
			int value = LOWORD(wParam) & 0x0F;
			
			switch (LOWORD(wParam) & 0xFF00)
			{
				case IDCMD_SAVESLOT_NUMERIC:
					// Change the save slot.
					if (value > 9)
						break;
					
					Set_Current_State(value);
					Sync_Gens_Window_FileMenu();
					break;
				
				case IDCMD_ROMHISTORY_NUMERIC:
					// Load a ROM from the ROM History submenu.
					if (value == 0 || value > 9)
						break;
					
					//if ((Check_If_Kaillera_Running())) return 0;
					if (audio->playingGYM())
						Stop_Play_GYM();
					Open_Rom(Recent_Rom[value - 1]);
					Sync_Gens_Window();
					break;
			}
	}
}


/**
 * fullScreenPopupMenu(): Show the Popup Menu while in fullscreen mode.
 * @param hWnd Window handle.
 */
static void fullScreenPopupMenu(HWND hWnd)
{
	// Full Screen, right mouse button click.
	// Show the popup menu.
	audio->clearSoundBuffer();
	
	// Show the mouse pointer.
	while (ShowCursor(false) >= 0) { }
	while (ShowCursor(true) < 0) { }
	
	POINT pt;
	GetCursorPos(&pt);
	SendMessage(hWnd, WM_PAINT, 0, 0);
	reinterpret_cast<VDraw_DDraw*>(draw)->restorePrimary();
	
	// Disable painting while the popup menu is open.
	paintsEnabled = false;
	TrackPopupMenu(MainMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, NULL, hWnd, NULL);
	paintsEnabled = true;
	
	// Hide the mouse pointer.
	while (ShowCursor(true) < 0) { }
	while (ShowCursor(false) >= 0) { }
}

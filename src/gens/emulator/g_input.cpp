/**
 * Gens: Input handler.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "g_input.hpp"
#include "g_mcd.hpp"
#include "ui_proxy.hpp"
#include "util/file/save.hpp"
#include "util/sound/gym.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/gfx/scrshot.h"

// Sound includes.
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

#include "gens/gens_window_sync.hpp"
#include "gens_ui.hpp"

// Due to bugs with SDL and GTK, modifier state has to be tracked manually.
// TODO: Shift-A works, but if shift is still held down and B is pressed, nothing shows up on SDL.
// TODO: This isn't actually a bug with SDL/GTK - it's an issue with keysnooping...
int mod = 0;


/**
 * Input_KeyDown(): Check if a key is pressed.
 * @param key Keycode.
 */
void Input_KeyDown(int key)
{
#ifdef GENS_OS_LINUX
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
		
		case GENS_KEY_PAUSE:
			if (Paused)
				Paused = 0;
			else
			{
				Paused = 1;
				//Pause_Screen();
				audio->clearSoundBuffer();
			}
			break;
		
		case GENS_KEY_BACKSPACE:
			if (draw->fullScreen() && (mod & KMOD_SHIFT))
			{
				audio->clearSoundBuffer();
				Save_Shot();
			}
			break;
		
		case GENS_KEY_TAB:
			system_reset();
			break;
		
		case GENS_KEY_RETURN:
			if (draw->fullScreen() && (mod & GENS_KMOD_ALT))
			{
				draw->setFullScreen(!draw->fullScreen());
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case GENS_KEY_F1:
			fast_forward = 1;
			break;
		
		case GENS_KEY_F2:
			if (draw->fullScreen() && (mod & GENS_KMOD_SHIFT))
			{
				Change_Stretch(!draw->stretch());
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (!mod)
			{
				Set_Frame_Skip(-1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case GENS_KEY_F3:
			if (draw->fullScreen() && (mod & GENS_KMOD_SHIFT))
			{
				int newVSync = !(draw->fullScreen() ? Video.VSync_FS : Video.VSync_W);
				Change_VSync(newVSync);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (!mod)
			{
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
			}
			break;
		
		case GENS_KEY_F4:
			if (!mod)
			{
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
			}
			break;
		
		case GENS_KEY_F5:
			if (!draw->fullScreen())
				break;
			
			//if (Check_If_Kaillera_Running()) return 0;
			
			if (mod == KMOD_SHIFT)
			{
				Str_Tmp[0] = 0;
				if (Change_File_S(Str_Tmp, State_Dir) == 1)
					Save_State(Str_Tmp);
			}
			else if (!mod)
			{
				Str_Tmp[0] = 0;
				Get_State_File_Name(Str_Tmp);
				Save_State(Str_Tmp);
			}
			break;
		
		case GENS_KEY_F6:
			if (!mod)
			{
				Set_Current_State((Current_State + 9) % 10);
				Sync_Gens_Window_FileMenu();
			}
			break;
		
		case GENS_KEY_F7:
			if (!mod)
			{
				Set_Current_State((Current_State + 1) % 10);
				Sync_Gens_Window_FileMenu();
			}
			break;
		
		case GENS_KEY_F8:
			if (!draw->fullScreen())
				break;
			
			//if (Check_If_Kaillera_Running()) return 0;
			
			if (mod == KMOD_SHIFT)
			{
				Str_Tmp[0] = 0;
				if (Change_File_L(Str_Tmp, State_Dir) == 1)
					Load_State(Str_Tmp);
			}
			else if (!mod)
			{
				Str_Tmp[0] = 0;
				Get_State_File_Name(Str_Tmp);
				Load_State(Str_Tmp);
			}
			break;
		
		case GENS_KEY_F9:
			if (mod & GENS_KMOD_SHIFT)
				Change_Blit_Style();
			else // if (!mod)
				Change_Fast_Blur();
			break;
		
		case GENS_KEY_F10:
			if (mod & GENS_KMOD_SHIFT)
			{
				Change_DAC_Improved(!DAC_Improv);
				Sync_Gens_Window_SoundMenu();
			}
			else if (draw->fullScreen() && !mod)
			{
				draw->setFPSEnabled(!draw->fpsEnabled());
			}
			break;
		
		case GENS_KEY_F11:
			if (mod & GENS_KMOD_SHIFT)
			{
				Change_PSG_Improved(!PSG_Improv);
				Sync_Gens_Window_SoundMenu();
			}
			else //if (!mod)
			{
				int rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
				if (rendMode > 0)
				{
					draw->setRender(rendMode - 1);
					Sync_Gens_Window_GraphicsMenu();
				}
			}
			break;
		
		case GENS_KEY_F12:
			if (mod & GENS_KMOD_SHIFT)
			{
				Change_YM2612_Improved(!YM2612_Improv);
				Sync_Gens_Window_SoundMenu();
			}
			else //if (!mod)
			{
				// TODO: Make filters constants.
				// There's already NB_FILTER, but it has the wrong numbers...
				int rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
				if (rendMode < 11)
				{
					draw->setRender(rendMode + 1);
					Sync_Gens_Window_GraphicsMenu();
				}
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
			if (!mod)
			{
				Set_Current_State(key - GENS_KEY_0);
				Sync_Gens_Window_FileMenu();
			}
			else if (key != GENS_KEY_0 && (mod & GENS_KMOD_CTRL))
			{
				//if ((Check_If_Kaillera_Running())) return 0;
				if (audio->playingGYM())
					Stop_Play_GYM();
				ROM::openROM(Recent_Rom[key - GENS_KEY_0]);
				Sync_Gens_Window();
			}
			break;
		
#ifdef GENS_CDROM
		case GENS_KEY_b:
			if (draw->fullScreen() && (mod & GENS_KMOD_CTRL))
			{
				// Ctrl-B: Boot CD
				if (Num_CD_Drive == 0)
					return;	// return 1;
				//if (Check_If_Kaillera_Running()) return 0;
				if (audio->playingGYM())
					Stop_Play_GYM();
				ROM::freeROM(Game);	// Don't forget it !
				SegaCD_Started = Init_SegaCD(NULL);
			}
			break;
#endif /* GENS_CDROM */
		
		/*
		case GENS_KEY_w:
			if (mod & GENS_KMOD_CTRL)
			{
				if (Sound_Initialised)
					Clear_Sound_Buffer ();
				Debug = 0;
				if (Net_Play)
				{
					//if (Full_Screen)
						//Set_Render (0, -1, 1);
					Sync_Gens_Window();
				}
				Free_Rom (Game);
			}
			break;
		*/
		
		/*
		case GENS_KEY_f:
			if (mod & GENS_KMOD_CTRL)
				gl_linear_filter = !gl_linear_filter;
			break;
		*/
		
		/* TODO: Fix MINIMIZE.
		case GENS_KEY_g:
			if (mod & GENS_KMOD_CTRL)
			{
				//if (Check_If_Kaillera_Running()) return 0;                                            
				MINIMIZE;
				// TODO: Re-enable this when the GTK+ GUI is rewritten.
				//open_game_genie();
			}
			break;
		*/
		
		/*
		case GENS_KEY_o:
			if (mod & GENS_KMOD_CTRL)
			{
				//if ((Check_If_Kaillera_Running())) return 0;
				if (GYM_Playing)
					Stop_Play_GYM ();
				MINIMIZE;
				Get_Rom();
			}
			break;
		*/
		
		/* TODO: Fix MINIMIZE.
		case GENS_KEY_p:
			if (mod & (GENS_KMOD_CTRL | GENS_KMOD_SHIFT))
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
		
#ifdef GENS_OPENGL
		case GENS_KEY_r:
			if (draw->fullScreen() && (mod & GENS_KMOD_SHIFT))
			{
				Change_OpenGL(!Video.OpenGL);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
#endif
		
		case GENS_KEY_v:
			if (mod & GENS_KMOD_CTRL)
			{
				if (SegaCD_Started)
					Change_CD();
			}
			break;
		
		default:
			break;
	}
	
#ifdef GENS_DEBUGGER
	// If debugging, pass the key to the debug handler.
	if (Debug != DEBUG_NONE)
		Debug_Event(key, mod);
#endif /* GENS_DEBUGGER */

#endif /* GENS_OS_LINUX */
}


/**
 * Input_KeyUp(): Check if a key is released.
 * @param key Keycode.
 */
void Input_KeyUp(int key)
{
#ifdef GENS_OS_LINUX
	switch(key)
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
#endif /* GENS_OS_LINUX */
}

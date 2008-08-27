/**
 * GENS: Input handler.
 */

#include <stdio.h>
#include <SDL.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "gens.h"
#include "g_main.h"
#include "g_input.h"
#include "g_mcd.h"
#include "g_sdlsound.h"
#include "g_sdldraw.h"
#include "ui_proxy.h"
#include "save.h"
#include "gym.h"
#include "cd_aspi.h"
#include "vdp_io.h"
#include "debug.h"
#include "scrshot.h"

// Sound includes.
#include "ym2612.h"
#include "psg.h"

#include "gens/gens_window_sync.h"
#include "ui-common.h"

// Due to bugs with SDL and GTK, modifier state has to be tracked manually.
// TODO: Shift-A works, but if shift is still held down and B is pressed, nothing shows up on SDL.
int mod = 0;


// Controller layouts.
struct K_Def Keys_Def[8];


// Default controller layouts.
const struct K_Def Keys_Default[8] =
{
	// Player 1
	{GENS_KEY_RETURN, GENS_KEY_RSHIFT,
	 GENS_KEY_a, GENS_KEY_s, GENS_KEY_d,
	 GENS_KEY_z, GENS_KEY_x, GENS_KEY_c,
	 GENS_KEY_UP, GENS_KEY_DOWN, GENS_KEY_LEFT, GENS_KEY_RIGHT},
	
	// Players 1B, 1C, 1D
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	
	// Player 2
	{GENS_KEY_u, GENS_KEY_t,
	 GENS_KEY_k, GENS_KEY_l, GENS_KEY_m,
	 GENS_KEY_i, GENS_KEY_o, GENS_KEY_p,
	 GENS_KEY_y, GENS_KEY_h, GENS_KEY_h, GENS_KEY_j},

	// Players 2B, 2C, 2D
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


/**
 * Input_KeyDown(): Check if a key is pressed.
 * @param key Keycode.
 */
void Input_KeyDown(int key)
{
	Keys[key] = 1;
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
				Paused = 0;
			else
			{
				Paused = 1;
				Pause_Screen ();
				Clear_Sound_Buffer ();
			}
			break;
		
		case GENS_KEY_PAUSE:
			if (Paused)
				Paused = 0;
			else
			{
				Paused = 1;
				Pause_Screen ();
				Clear_Sound_Buffer ();
			}
			break;
		
		/*
		case GENS_KEY_BACKSPACE:
			if (KMOD_SHIFT & mod)
			{
				Clear_Sound_Buffer ();
				Save_Shot();
			}
			break;
		*/
		
		case GENS_KEY_TAB:
			system_reset ();
			break;
		
		case GENS_KEY_RETURN:
			if (mod & GENS_KMOD_ALT)
			{
				/*
				if (Full_Screen)
					Set_Render (0, -1, 1);
				else
					Set_Render (1, Render_FS, 1);
				*/
				
				Full_Screen = !Full_Screen;
				Set_Render(Full_Screen, Render_Mode, 1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case GENS_KEY_F1:
			fast_forward = 1;
			break;
		
		case GENS_KEY_F2:
			/*
			if (mod & GENS_KMOD_SHIFT)
			{
				Change_Stretch();
				Sync_Gens_Window_GraphicsMenu();
			}
			else
			*/
			if (!mod)
			{
				Set_Frame_Skip(-1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case GENS_KEY_F3:
			/*
			if (mod & GENS_KMOD_SHIFT)
			{
				Change_VSync(-1);
				Sync_Gens_Window_GraphicsMenu();
			}
			else */
			if (!mod)
			{
				if (Frame_Skip == -1)
				{
					Set_Frame_Skip (0);
					Sync_Gens_Window_GraphicsMenu();
				}
				else if (Frame_Skip > 0)
				{
					Set_Frame_Skip (Frame_Skip - 1);
					Sync_Gens_Window_GraphicsMenu();
				}
			}
			break;
		
		case GENS_KEY_F4:
			if (!mod)
			{
				if (Frame_Skip == -1)
					Set_Frame_Skip (1);
				else
				{
					if (Frame_Skip < 8)
						Set_Frame_Skip (Frame_Skip + 1);
				}
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		/*
		case GENS_KEY_F5:
			if (mod & GENS_KMOD_SHIFT)
			{
				//if (Check_If_Kaillera_Running()) return 0;
				MINIMIZE Change_File_S (Str_Tmp, State_Dir);
				Save_State (Str_Tmp);
			}
			else // if (!mod)
			{
				//if (Check_If_Kaillera_Running()) return 0;
				Str_Tmp[0] = 0;
				Get_State_File_Name (Str_Tmp);
				Save_State (Str_Tmp);
			}
			break;
		*/
		
		case GENS_KEY_F6:
			if (!mod)
			{
				Set_Current_State ((Current_State + 9) % 10);
				Sync_Gens_Window_FileMenu();
			}
			break;
		
		case GENS_KEY_F7:
			if (!mod)
			{
				Set_Current_State ((Current_State + 1) % 10);
				Sync_Gens_Window_FileMenu();
			}
			break;
		
		/*
		case GENS_KEY_F8:
			if (mod & GENS_KMOD_SHIFT)
			{
				//if (Check_If_Kaillera_Running()) return 0;
				MINIMIZE Str_Tmp[0] = 0;
				Change_File_L (Str_Tmp, State_Dir);
				Load_State (Str_Tmp);
			}
			else //if (!mod)
			{
				//if (Check_If_Kaillera_Running()) return 0;
				Str_Tmp[0] = 0;
				Get_State_File_Name (Str_Tmp);
				Load_State (Str_Tmp);
			}
			break;
		*/
		
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
			else //if (!mod)
			{
				Show_FPS = !Show_FPS;
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
				if (Render_Mode > 1)
				{
					Set_Render(Full_Screen, Render_Mode - 1, 0);
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
				if (Render_Mode < NB_FILTER-1)
				{
					Set_Render(Full_Screen, Render_Mode + 1, 0);
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
			if (mod & GENS_KMOD_SHIFT)
			{
				Set_Current_State (key - GENS_KEY_0);
				Sync_Gens_Window_FileMenu();
			}
			else if (key != GENS_KEY_0 && (mod & GENS_KMOD_CTRL))
			{
				//if ((Check_If_Kaillera_Running())) return 0;
				if (GYM_Playing)
					Stop_Play_GYM ();
				Open_Rom(Recent_Rom[key - GENS_KEY_0]);
				Sync_Gens_Window();
			}
			break;
		
		case GENS_KEY_b:
			if (mod & GENS_KMOD_CTRL)
			{
				if (Num_CD_Drive == 0)
					return;	// return 1;
				//if (Check_If_Kaillera_Running()) return 0;
				if (GYM_Playing)
					Stop_Play_GYM ();
				Free_Rom (Game);	// Don't forget it !
				SegaCD_Started = Init_SegaCD (NULL);
			}
			break;
		
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
		
		case GENS_KEY_g:
			if (mod & GENS_KMOD_CTRL)
			{
				//if (Check_If_Kaillera_Running()) return 0;                                            
				MINIMIZE;
				// TODO: Re-enable this when the GTK+ GUI is rewritten.
				//open_game_genie();
			}
			break;
		
		case GENS_KEY_h:
			if (mod & KMOD_CTRL)
				UI_Set_Window_Visibility(0);
			else
				UI_Set_Window_Visibility(1);
			break;
		
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
		
		/*
		case GENS_KEY_r:
			if (mod & GENS_KMOD_SHIFT)
			{
				Change_OpenGL(!Opengl);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		*/
		
		case GENS_KEY_v:
			if (mod & GENS_KMOD_CTRL)
			{
				if (SegaCD_Started)
				Change_CD ();
			}
			break;
		
		case GENS_KEY_w:
			if (mod & GENS_KMOD_CTRL)
			{
				if (WAV_Dumping)
					Stop_WAV_Dump ();
				else
					Start_WAV_Dump ();
				Sync_Gens_Window_SoundMenu();
			}
			break;
		
		default:
			break;
	}
	
#ifdef GENS_DEBUG
	// If debugging, pass the key to the debug handler.
	if (Debug != DEBUG_NONE)
		Debug_Event(key, mod);
#endif
}


/**
 * Input_KeyUp(): Check if a key is released.
 * @param key Keycode.
 */
void Input_KeyUp(int key)
{
	Keys[key] = 0;
	
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
}

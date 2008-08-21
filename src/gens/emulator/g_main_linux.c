/**
 * GENS: Main loop. (Linux specific code)
 */

#include <stdio.h>
#include <SDL.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "gens.h"
#include "g_main.h"
#include "g_mcd.h"
#include "g_sdlsound.h"
#include "g_sdldraw.h"
#include "g_sdlinput.h"
#include "ui_proxy.h"
#include "support.h"
#include "save.h"
#include "gym.h"
#include "cd_aspi.h"
#include "vdp_io.h"

GtkWidget *gens_window = NULL;

void SDL_Check_KeyDown(SDL_Event *event);
void SDL_Check_KeyUp(SDL_Event *event);
void SDL_Check_Joystick_Axis(SDL_Event *event);

/**
 * SetWindowText(): Sets the window title.
 * @param s New window title.
 */
void SetWindowText (const char *s)
{
	gtk_window_set_title((GtkWindow*)gens_window, s);
}

/**
 * SetWindowVisibility(): Sets window visibility.
 * @param visibility 0 to hide; anything else to show.
 */
void SetWindowVisibility (int visibility)
{
	if (visibility)
		gtk_widget_show(gens_window);
	else
		gtk_widget_hide(gens_window);
}

/**
 * update_SDL_events(): Check for SDL events.
 */
void update_SDL_events ()
{
	SDL_Event event;
	
	while (SDL_PollEvent (&event))
	{
		switch (event.type)
		{
		/*
			case SDL_VIDEORESIZE:
				surface = SDL_SetVideoMode(event.resize.w, event.resize.h, 16, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
				break;
		*/
		case SDL_KEYDOWN:
			Keys[event.key.keysym.sym] = 1;
			SDL_Check_KeyDown(&event);
			break;
			
		case SDL_KEYUP:
			Keys[event.key.keysym.sym] = 0;
			SDL_Check_KeyUp(&event);
			break;
		
		case SDL_JOYAXISMOTION:
			SDL_Check_Joystick_Axis(&event);
			break;
		
		case SDL_JOYBUTTONDOWN:
			joystate[0x10 + 0x100 * event.jbutton.which + event.jbutton.button] = 1;
			break;
		
		case SDL_JOYBUTTONUP:
			joystate[0x10 + 0x100 * event.jbutton.which + event.jbutton.button] = 0;
			break;
		
		case SDL_JOYHATMOTION:
			break;
		
		default:
			break;
		}
	}
}

/**
 * SDL_Check_KeyDown(): Check if a key is pressed.
 * @param *event SDL event structure.
 */
void SDL_Check_KeyDown(SDL_Event *event)
{
	Keys[event->key.keysym.sym] = 1;
	SDLMod mod = SDL_GetModState();
	
	switch (event->key.keysym.sym)
	{
		case SDLK_ESCAPE:
			if (Quick_Exit)
				close_gens();
			if (Debug)
			{
				Change_Debug (0);
				Paused = 0;
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
		
		case SDLK_PAUSE:
			if (Paused)
				Paused = 0;
			else
			{
				Paused = 1;
				Pause_Screen ();
				Clear_Sound_Buffer ();
			}
			break;
		
		case SDLK_BACKSPACE:
			if (KMOD_SHIFT & mod)
			{
				Clear_Sound_Buffer ();
				Take_Shot ();
			}
			break;
		
		case SDLK_TAB:
			system_reset ();
			break;
		
		case SDLK_RETURN:
			if (KMOD_ALT & mod)
			{
				/*
				if (Full_Screen)
					Set_Render (0, -1, 1);
				else
					Set_Render (1, Render_FS, 1);
				*/
				
				Full_Screen = !Full_Screen;
				Set_Render(Full_Screen, Render_Mode, 1);
				sync_gens_ui (UPDATE_GTK);
			}
			break;
		
		case SDLK_F1:
			fast_forward = 1;
			break;
		
		case SDLK_F2:
			if (KMOD_SHIFT & mod)
			{
				Change_Stretch ();
				sync_gens_ui (UPDATE_GTK);
			}
			else // if (!mod)
			{
				Set_Frame_Skip (-1);
				sync_gens_ui (UPDATE_GTK);
			}
			break;
		
		case SDLK_F3:
			if (KMOD_SHIFT & mod)
			{
				Change_VSync ();
				sync_gens_ui (UPDATE_GTK);
			}
			else // if (!mod)
			{
				if (Frame_Skip == -1)
				{
					Set_Frame_Skip (0);
					sync_gens_ui (UPDATE_GTK);
				}
				else if (Frame_Skip > 0)
				{
					Set_Frame_Skip (Frame_Skip - 1);
					sync_gens_ui (UPDATE_GTK);
				}
			}
			break;
		
		case SDLK_F4:
			if (Frame_Skip == -1)
				Set_Frame_Skip (1);
			else
			{
				if (Frame_Skip < 8)
					Set_Frame_Skip (Frame_Skip + 1);
			}
			sync_gens_ui (UPDATE_GTK);
			break;
		
		case SDLK_F5:
			if (KMOD_SHIFT & mod)
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
		
		case SDLK_F6:
			Set_Current_State ((Current_State + 9) % 10);
			sync_gens_ui (UPDATE_GTK);
			break;
		
		case SDLK_F7:
			Set_Current_State ((Current_State + 1) % 10);
			sync_gens_ui (UPDATE_GTK);
			break;
		
		case SDLK_F8:
			if (KMOD_SHIFT & mod)
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
		
		case SDLK_F9:
			if (KMOD_SHIFT & mod)
				Change_Blit_Style ();
			else // if (!mod)
				Change_Fast_Blur ();
			break;
		
		case SDLK_F10:
			if (KMOD_SHIFT & mod)
			{
				Change_DAC_Improv ();
				sync_gens_ui (UPDATE_GTK);
			}
			else //if (!mod)
			{
				Show_FPS = !Show_FPS;
			}
			break;
		
		case SDLK_F11:
			if (KMOD_SHIFT & mod)
			{
				Change_PSG_Improv ();
				sync_gens_ui (UPDATE_GTK);
			}
			else //if (!mod)
			{
				if (Render_Mode > 1)
				{
					Set_Render (Full_Screen, Render_Mode - 1, 0);
					sync_gens_ui (UPDATE_GTK);
				}
			}
			break;
		
		case SDLK_F12:
			if (KMOD_SHIFT & mod)
			{
				Change_YM2612_Improv ();
				sync_gens_ui (UPDATE_GTK);
			}
			else //if (!mod)
			{
				if (Render_Mode < NB_FILTER-1)
				{
					Set_Render (Full_Screen, Render_Mode + 1, 0);
					sync_gens_ui (UPDATE_GTK);
				}
			}
			break;
		
		case SDLK_0:
			if (KMOD_SHIFT & mod)
			{
				Set_Current_State (0);
				sync_gens_ui (UPDATE_GTK);
			}
			break;
		
		case SDLK_1:
		case SDLK_2:
		case SDLK_3:
		case SDLK_4:
		case SDLK_5:
		case SDLK_6:
		case SDLK_7:
		case SDLK_8:
		case SDLK_9:
			if (KMOD_SHIFT & mod)
			{
				Set_Current_State (event->key.keysym.sym - SDLK_0);
				sync_gens_ui (UPDATE_GTK);
			}
			else if (KMOD_CTRL & mod)
			{
				//if ((Check_If_Kaillera_Running())) return 0;
				if (GYM_Playing)
					Stop_Play_GYM ();
				Open_Rom (Recent_Rom[event->key.keysym.sym - SDLK_0]);
				sync_gens_ui (UPDATE_GTK);
			}
			break;
		
		case SDLK_b:
			if (KMOD_CTRL & mod)
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
		
		case SDLK_c:
			if (KMOD_CTRL & mod)
			{
				if (Sound_Initialised)
					Clear_Sound_Buffer ();
				Debug = 0;
				if (Net_Play)
				{
					//if (Full_Screen)
						//Set_Render (0, -1, 1);
					sync_gens_ui (UPDATE_GTK);
				}
				Free_Rom (Game);
			}
			break;
		
		/*
		case SDLK_f:
			if (KMOD_CTRL & mod)
				gl_linear_filter = !gl_linear_filter;
			break;
		*/
		
		case SDLK_g:
			if (KMOD_CTRL & mod)
			{
				//if (Check_If_Kaillera_Running()) return 0;                                            
				MINIMIZE open_game_genie ();
			}
			break;
		
		case SDLK_h:
			if (KMOD_CTRL & mod)
				SetWindowVisibility(0);
			else
				SetWindowVisibility(1);
			break;
		
		case SDLK_o:
			if (KMOD_CTRL & mod)
			{
				//if ((Check_If_Kaillera_Running())) return 0;
				if (GYM_Playing)
					Stop_Play_GYM ();
				MINIMIZE;
				Get_Rom ();
			}
			break;
		
		case SDLK_p:
			if ((KMOD_CTRL | KMOD_SHIFT) & mod)
			{
				if (!Genesis_Started && !SegaCD_Started && !_32X_Started)
				{
					MINIMIZE;
					if (GYM_Playing)
						Stop_Play_GYM ();
					else
						Start_Play_GYM ();
					sync_gens_ui (UPDATE_GTK);
				}
			}
			else if (KMOD_CTRL & mod)
			{
				Change_SegaCD_Synchro ();
				sync_gens_ui (UPDATE_GTK);
			}
			break;
		
		case SDLK_r:
			if (KMOD_SHIFT & mod)
			{
				Change_backend();
				sync_gens_ui(UPDATE_GTK);
			}
			break;
		
		case SDLK_v:
			if (KMOD_CTRL & mod)
			{
				if (SegaCD_Started)
				Change_CD ();
			}
			break;
		
		case SDLK_w:
			if (KMOD_CTRL & mod)
			{
				if (WAV_Dumping)
					Stop_WAV_Dump ();
				else
					Start_WAV_Dump ();
				sync_gens_ui (UPDATE_GTK);
			}
			break;
		
		default:
			break;
	}
}

/**
 * SDL_Check_KeyUp(): Check if a key is released.
 * @param *event SDL event structure.
 */
void SDL_Check_KeyUp(SDL_Event *event)
{
	Keys[event->key.keysym.sym] = 0;
	
	switch(event->key.keysym.sym)
	{
		case SDLK_F1:
			fast_forward = 0;
			break;
	}
}

void SDL_Check_Joystick_Axis(SDL_Event *event)
{
	if (event->jaxis.axis < 6)
	{
		if (event->jaxis.value < -10000)
		{
			if (event->jaxis.axis == 0)
			{
				joystate[0x100 * event->jaxis.which + 0x3] = 1;
				joystate[0x100 * event->jaxis.which + 0x4] = 0;
			}
			else if (event->jaxis.axis == 1)
			{
				joystate[0x100 * event->jaxis.which + 0x1] = 1;
				joystate[0x100 * event->jaxis.which + 0x2] = 0;
			}
			else if (event->jaxis.axis == 2)
			{
				joystate[0x100 * event->jaxis.which + 0x7] = 1;
				joystate[0x100 * event->jaxis.which + 0x8] = 0;
			}
			else if (event->jaxis.axis == 3)
			{
				joystate[0x100 * event->jaxis.which + 0x5] = 1;
				joystate[0x100 * event->jaxis.which + 0x6] = 0;
			}
			else if (event->jaxis.axis == 4)
			{
				joystate[0x100 * event->jaxis.which + 0xB] = 1;
				joystate[0x100 * event->jaxis.which + 0xC] = 0;
			}
			else if (event->jaxis.axis == 5)
			{
				joystate[0x100 * event->jaxis.which + 0x9] = 1;
				joystate[0x100 * event->jaxis.which + 0xA] = 0;
			}
		}
		else if (event->jaxis.value > 10000)
		{
			if (event->jaxis.axis == 0)
			{
				joystate[0x100 * event->jaxis.which + 0x3] = 0;
				joystate[0x100 * event->jaxis.which + 0x4] = 1;
			}
			else if (event->jaxis.axis == 1)
			{
				joystate[0x100 * event->jaxis.which + 0x1] = 0;
				joystate[0x100 * event->jaxis.which + 0x2] = 1;
			}
			else if (event->jaxis.axis == 2)
			{
				joystate[0x100 * event->jaxis.which + 0x7] = 0;
				joystate[0x100 * event->jaxis.which + 0x8] = 1;
			}
			else if (event->jaxis.axis == 3)
			{
				joystate[0x100 * event->jaxis.which + 0x5] = 0;
				joystate[0x100 * event->jaxis.which + 0x6] = 1;
			}
			else if (event->jaxis.axis == 4)
			{
				joystate[0x100 * event->jaxis.which + 0xB] = 0;
				joystate[0x100 * event->jaxis.which + 0xC] = 1;
			}
			else if (event->jaxis.axis == 5)
			{
				joystate[0x100 * event->jaxis.which + 0x9] = 0;
				joystate[0x100 * event->jaxis.which + 0xA] = 1;
			}
		}
		else
		{
			if (event->jaxis.axis == 0)
			{
				joystate[0x100 * event->jaxis.which + 0x4] = 0;
				joystate[0x100 * event->jaxis.which + 0x3] = 0;
			}
			else if (event->jaxis.axis == 1)
			{
				joystate[0x100 * event->jaxis.which + 0x2] = 0;
				joystate[0x100 * event->jaxis.which + 0x1] = 0;
			}
			else if (event->jaxis.axis == 2)
			{
				joystate[0x100 * event->jaxis.which + 0x8] = 0;
				joystate[0x100 * event->jaxis.which + 0x7] = 0;
			}
			else if (event->jaxis.axis == 3)
			{
				joystate[0x100 * event->jaxis.which + 0x6] = 0;
				joystate[0x100 * event->jaxis.which + 0x5] = 0;
			}
			else if (event->jaxis.axis == 4)
			{
				joystate[0x100 * event->jaxis.which + 0xC] = 0;
				joystate[0x100 * event->jaxis.which + 0xB] = 0;
			}
			else if (event->jaxis.axis == 5)
			{
				joystate[0x100 * event->jaxis.which + 0xA] = 0;
				joystate[0x100 * event->jaxis.which + 0x9] = 0;
			}
		}
	}
}

/**
 * GENS_Default_Save_Path(): Create the default save path.
 * @param *buf Buffer to store the default save path in.
 */
void Get_Save_Path(char *buf, size_t n)
{
	strncpy (buf, getenv ("HOME"), n);
	strcat (buf, "/.gens/");
}

/**
 * GENS_Create_Default_Save_Directory(): Create the default save directory.
 * @param *dir Directory name.
 */
void Create_Save_Directory(const char *dir)
{
	mkdir(dir, 0700);
}

/**
 * Init_OS_Graphics(): Initialize the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
int Init_OS_Graphics(void)
{
	if (SDL_InitSubSystem (SDL_INIT_TIMER) < 0)
	{
		fprintf (stderr, SDL_GetError ());
		return -1;
	}
	if (SDL_InitSubSystem (SDL_INIT_VIDEO) < 0)
	{
		fprintf (stderr, SDL_GetError ());
		return -1;
	}
	
	/* Take it back down now that we know it works. */
	SDL_QuitSubSystem (SDL_INIT_VIDEO);
	
	// Initialize joysticks.
	// TODO: If there's an error here, merely disable joystick functionality.
	if (SDL_InitSubSystem (SDL_INIT_JOYSTICK) < 0)
	{
		fprintf (stderr, SDL_GetError ());
		return -1;
	}
	
	// Initialize CD-ROM.
	// TODO: If there's an error here, merely disable CD-ROM functionality.
	if (SDL_InitSubSystem (SDL_INIT_CDROM) < 0)
	{
		fprintf (stderr, SDL_GetError ());
		return -1;
	}
	
	return 0;
}

/**
 * End_OS_Graphics(): Shut down the OS-specific graphics library.
 */
void End_OS_Graphics()
{
	SDL_Quit();
}

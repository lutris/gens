#include <SDL.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "g_main.h"
#include "timer.h"
#include "gens.h"
#include "g_md.h"
#include "g_mcd.h"
#include "g_sdlsound.h"
#include "g_sdldraw.h"
#include "g_sdlinput.h"
#include "port.h"
#include "misc.h"
#include "vdp_rend.h"
#include "save.h"
#include "cd_aspi.h"
#include "vdp_io.h"
#include "gym.h"
#include "mem_m68k.h"
#include "ym2612.h"
#include "interface.h"
#include "support.h"
#include "ui_proxy.h"
#include "parse.h"
#include "cpu_sh2.h"
#include "cpu_68k.h"
#include "cpu_z80.h"
#include "psg.h"
#include "pwm.h"

GtkWidget *gens_window = NULL;
int fast_forward = 0;

POINT Window_Pos;
char Str_Tmp[1024];
char Start_Rom[1024];
char Gens_Path[1024];
char Language_Path[1024];
char CGOffline_Path[1024];
char Manual_Path[1024];

char **language_name = NULL;
int Active = 1;
int Paused = 0;
int Net_Play = 0;
int Full_Screen = 0;
int Resolution = 1;
int Fast_Blur = 0;
int Render_Mode = 1;
int Opengl = 1;
int Width_gl=640;
int Height_gl=480;
int Bpp=16;
int Show_FPS = 0;
int Show_Message = 1;
int Show_LED = 0;
int FS_Minimised = 0;
int Auto_Pause = 0;
int Auto_Fix_CS = 0;
int Language = 0;
int Country = -1;
int Country_Order[3];
int Intro_Style = 2;
int SegaCD_Accurate = 0;
int Kaillera_Client_Running = 0;
int Quick_Exit = 0;

static int Gens_Running = 0;

static int
Build_Language_String (void)
{
  unsigned long nb_lue = 1;
  int sec_alloue = 1, poscar = 0;
  enum etat_sec
  { DEB_LIGNE, SECTION, NORMAL } etat = DEB_LIGNE;

  FILE *LFile;

  char c;

  if (language_name)
    {
      free (language_name);
      language_name = NULL;
    }

  language_name = (char **) malloc (sec_alloue * sizeof (char *));
  language_name[0] = NULL;


  LFile = fopen (Language_Path, "r");
  if (!LFile)
    {
      LFile = fopen (Language_Path, "w");
    }

  while (nb_lue)
    {
      nb_lue = fread (&c, 1, 1, LFile);
      switch (etat)
	{
	case DEB_LIGNE:
	  switch (c)
	    {
	    case '[':
	      etat = SECTION;
	      sec_alloue++;
	      language_name =
		(char **) realloc (language_name,
				   sec_alloue * sizeof (char *));
	      language_name[sec_alloue - 2] =
		(char *) malloc (32 * sizeof (char));
	      language_name[sec_alloue - 1] = NULL;
	      poscar = 0;
	      break;

	    case '\n':
	      break;

	    default:
	      etat = NORMAL;
	      break;
	    }
	  break;

	case NORMAL:
	  switch (c)
	    {
	    case '\n':
	      etat = DEB_LIGNE;
	      break;

	    default:
	      break;
	    }
	  break;

	case SECTION:
	  switch (c)
	    {
	    case ']':
	      language_name[sec_alloue - 2][poscar] = 0;
	      etat = DEB_LIGNE;
	      break;

	    default:
	      if (poscar < 32)
		language_name[sec_alloue - 2][poscar++] = c;
	      break;
	    }
	  break;
	}
    }

  fclose (LFile);

  if (sec_alloue == 1)
    {
      language_name = (char **) realloc (language_name, 2 * sizeof (char *));
      language_name[0] = (char *) malloc (32 * sizeof (char));
      strcpy (language_name[0], "English");
      language_name[1] = NULL;
      WritePrivateProfileString ("English", "Menu Language", "&English menu",
				 Language_Path);
    }

  return (0);
}

/**
 * SetWindowText(): Sets the window title.
 * @param s New window title.
 */
void SetWindowText (const char *s)
{
	gtk_window_set_title((GtkWindow*)gens_window, s);
}


void
Sleep (int i)
{
  // i : millisecond
  struct timespec rqtp = { 0, 0 };
  rqtp.tv_nsec *= 1000000;
  nanosleep (&rqtp, NULL);
}


void
update_SDL_events ()
{
  SDL_Event event;

  SDLMod mod;
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
	  mod = SDL_GetModState ();
	  switch (event.key.keysym.sym)
	    {
	    case SDLK_ESCAPE:
	      if (Quick_Exit)
                {
		  close_gens();
		}
	      
	      if (Debug)
		{
		  Change_Debug (0);
		  Paused = 0;
		}
	      else if (Paused)
		{
		  Paused = 0;
		}
	      else
		{
		  Paused = 1;
		  Pause_Screen ();
		  Clear_Sound_Buffer ();
		}
	      break;

	    case SDLK_PAUSE:
	      if (Paused)
		{
		  Paused = 0;
		}
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
		   //if (Full_Screen)
		    //Set_Render (0, -1, 1);
		   //else
		  //  Set_Render (1, Render_FS, 1);*/
		  
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
		{
		  Set_Frame_Skip (1);
		}
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
		{
		  Change_Blit_Style ();
		}
	      else // if (!mod)
		{
		  Change_Fast_Blur ();
		}
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
		  Set_Current_State (event.key.keysym.sym - SDLK_0);
		  sync_gens_ui (UPDATE_GTK);
		}
	      else if (KMOD_CTRL & mod)
		{
		  //if ((Check_If_Kaillera_Running())) return 0;
		  if (GYM_Playing)
		    Stop_Play_GYM ();
		  Open_Rom (Recent_Rom[event.key.keysym.sym - SDLK_0]);
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
		
//		case SDLK_f:
//		if (KMOD_CTRL & mod)
//		{
//			gl_linear_filter = !gl_linear_filter;
//		}
		
		break;

	    case SDLK_g:
	      if (KMOD_CTRL & mod)
		{
		  //if (Check_If_Kaillera_Running()) return 0;                                            
		  MINIMIZE open_game_genie ();
		}
	      break;
       	case SDLK_h:
           if (KMOD_CTRL & mod)
           {
                   gtk_widget_hide(gens_window);
           }           
           else
           {
                   gtk_widget_show(gens_window);
           }
           break;
                                          
	    case SDLK_o:
	      if (KMOD_CTRL & mod)
		{
		  //if ((Check_If_Kaillera_Running())) return 0;
		  if (GYM_Playing)
		    Stop_Play_GYM ();
		  MINIMIZE Get_Rom ();
		}
	      break;

	    case SDLK_p:
	      if ((KMOD_CTRL | KMOD_SHIFT) & mod)
		{
		  if (!Genesis_Started && !SegaCD_Started && !_32X_Started)
		    {
		      MINIMIZE if (GYM_Playing)
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
	  break;

	case SDL_KEYUP:
	  //Keys[event.key.keysym.sym] = 0;
	  //if (event.key.keysym.sym == SDLK_F1)
	    //{
		switch(event.key.keysym.sym)
		{
			case SDLK_F1:
	      		fast_forward = 0;
	      		break;
			default:
				Keys[event.key.keysym.sym] = 0;
				break;
	    }
	  break;

	case SDL_JOYAXISMOTION:
	  if (event.jaxis.axis < 6)
	    {
	      if (event.jaxis.value < -10000)
		{
		  if (event.jaxis.axis == 0)
		    {
		      joystate[0x100 * event.jaxis.which + 0x3] = 1;
		      joystate[0x100 * event.jaxis.which + 0x4] = 0;
		    }
		  else if (event.jaxis.axis == 1)
		    {
		      joystate[0x100 * event.jaxis.which + 0x1] = 1;
		      joystate[0x100 * event.jaxis.which + 0x2] = 0;
		    }
		  else if (event.jaxis.axis == 2)
		    {
		      joystate[0x100 * event.jaxis.which + 0x7] = 1;
		      joystate[0x100 * event.jaxis.which + 0x8] = 0;
		    }
		  else if (event.jaxis.axis == 3)
		    {
		      joystate[0x100 * event.jaxis.which + 0x5] = 1;
		      joystate[0x100 * event.jaxis.which + 0x6] = 0;
		    }
		  else if (event.jaxis.axis == 4)
		    {
		      joystate[0x100 * event.jaxis.which + 0xB] = 1;
		      joystate[0x100 * event.jaxis.which + 0xC] = 0;
		    }
		  else if (event.jaxis.axis == 5)
		    {
		      joystate[0x100 * event.jaxis.which + 0x9] = 1;
		      joystate[0x100 * event.jaxis.which + 0xA] = 0;
		    }
		}
	      else if (event.jaxis.value > 10000)
	    {
		  if (event.jaxis.axis == 0)
		    {
		      joystate[0x100 * event.jaxis.which + 0x3] = 0;
		      joystate[0x100 * event.jaxis.which + 0x4] = 1;
		    }
		  else if (event.jaxis.axis == 1)
	    {
		      joystate[0x100 * event.jaxis.which + 0x1] = 0;
		      joystate[0x100 * event.jaxis.which + 0x2] = 1;
		    }
		  else if (event.jaxis.axis == 2)
		{
		      joystate[0x100 * event.jaxis.which + 0x7] = 0;
		      joystate[0x100 * event.jaxis.which + 0x8] = 1;
		}
		  else if (event.jaxis.axis == 3)
		{
		      joystate[0x100 * event.jaxis.which + 0x5] = 0;
		      joystate[0x100 * event.jaxis.which + 0x6] = 1;
		    }
		  else if (event.jaxis.axis == 4)
		{
		      joystate[0x100 * event.jaxis.which + 0xB] = 0;
		      joystate[0x100 * event.jaxis.which + 0xC] = 1;
		}
		  else if (event.jaxis.axis == 5)
		{
		      joystate[0x100 * event.jaxis.which + 0x9] = 0;
		      joystate[0x100 * event.jaxis.which + 0xA] = 1;
		    }
		}
	      else
		{
		  if (event.jaxis.axis == 0)
		    {
		      joystate[0x100 * event.jaxis.which + 0x4] = 0;
		      joystate[0x100 * event.jaxis.which + 0x3] = 0;
		    }
		  else if (event.jaxis.axis == 1)
		    {
		      joystate[0x100 * event.jaxis.which + 0x2] = 0;
		      joystate[0x100 * event.jaxis.which + 0x1] = 0;
		    }
		  else if (event.jaxis.axis == 2)
		    {
		      joystate[0x100 * event.jaxis.which + 0x8] = 0;
		      joystate[0x100 * event.jaxis.which + 0x7] = 0;
		    }
		  else if (event.jaxis.axis == 3)
		    {
		      joystate[0x100 * event.jaxis.which + 0x6] = 0;
		      joystate[0x100 * event.jaxis.which + 0x5] = 0;
		    }
		  else if (event.jaxis.axis == 4)
		    {
		      joystate[0x100 * event.jaxis.which + 0xC] = 0;
		      joystate[0x100 * event.jaxis.which + 0xB] = 0;
		    }
		  else if (event.jaxis.axis == 5)
		    {
		      joystate[0x100 * event.jaxis.which + 0xA] = 0;
		      joystate[0x100 * event.jaxis.which + 0x9] = 0;
		    }
		}
	    }

	  break;

	case SDL_JOYBUTTONDOWN:
	  joystate[0x10 + 0x100 * event.jbutton.which +
		   event.jbutton.button] = 1;
	  break;

	case SDL_JOYBUTTONUP:
	  joystate[0x10 + 0x100 * event.jbutton.which +
		   event.jbutton.button] = 0;
	  break;

	case SDL_JOYHATMOTION:
	  break;

	default:
	  break;
	}
    }
}

void
close_gens ()
{
  Gens_Running = 0;
}

void
run_gens ()
{
  Gens_Running = 1;
}

int
is_gens_running ()
{
  return Gens_Running;
}

static void
InitParameters (void)
{
  VDP_Num_Vis_Lines = 224;
  Net_Play = 0;
  Stretch = 0;
  Sprite_Over = 1;
  Render_Mode = 0;
  Show_Message = 1;

  Sound_Enable = 0;
  Sound_Segs = 8;
  Sound_Stereo = 1;
  Sound_Initialised = 0;
  Sound_Is_Playing = 0;
  WAV_Dumping = 0;
  GYM_Dumping = 0;

  FS_Minimised = 0;
  Game = NULL;
  Genesis_Started = 0;
  SegaCD_Started = 0;
  _32X_Started = 0;
  Debug = 0;
  CPU_Mode = 0;

  strncpy (Gens_Path, getenv ("HOME"), 1000);
  strcat (Gens_Path, "/.gens/");
  mkdir (Gens_Path, 0700);

  strncpy (Language_Path, Gens_Path, 1000);
  strcat (Language_Path, "language.dat");

  strncpy (Str_Tmp, Gens_Path, 1000);
  strcat (Str_Tmp, "gens.cfg");

  strcpy (Manual_Path, "");
  strcpy (CGOffline_Path, "");

  Build_Language_String ();
  Load_Config (Str_Tmp, NULL);

  Init_CD_Driver ();
}

int
Init (void)
{
  if (SDL_InitSubSystem (SDL_INIT_TIMER) < 0)
    {
      fprintf (stderr, SDL_GetError ());
      return 0;
    }
  if (SDL_InitSubSystem (SDL_INIT_VIDEO) < 0)
    {
      fprintf (stderr, SDL_GetError ());
      return 0;
    }
  /* Take it back down now that we know it works. */
  SDL_QuitSubSystem (SDL_INIT_VIDEO);
  if (SDL_InitSubSystem (SDL_INIT_JOYSTICK) < 0)
    {
      fprintf (stderr, SDL_GetError ());
      return 0;
    }
  if (SDL_InitSubSystem (SDL_INIT_CDROM) < 0)
    {
      fprintf (stderr, SDL_GetError ());
      return 0;
    }

  init_timer ();

  Identify_CPU ();

  MSH2_Init ();
  SSH2_Init ();
  M68K_Init ();
  S68K_Init ();
  Z80_Init ();

  YM2612_Init (CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
  PSG_Init (CLOCK_NTSC / 15, Sound_Rate);
  PWM_Init ();

  Init_Input ();

  Init_CD_Driver ();
  Init_Tab ();
  run_gens ();

  return 1;
}

void
End_All (void)
{
  Free_Rom (Game);
  End_DDraw ();
  End_Input ();
  YM2612_End ();
  End_Sound ();
  End_CD_Driver ();
  SDL_Quit ();
}


int
main (int argc, char *argv[])
{
  //char sdlbuf[32];

  //sprintf(sdlbuf, "SDL_WINDOWID=%ld",  GDK_WINDOW_XWINDOW(gens_window->window));
  //putenv(sdlbuf);
  GtkWidget *sdlsock;

  Init_Genesis_Bios ();

  InitParameters ();
  parseArgs (argc, argv);

  add_pixmap_directory (DATADIR);
  gtk_init (&argc, &argv);

  gens_window = create_gens_window ();
  sdlsock = gtk_event_box_new();
  gtk_widget_set_name(sdlsock, "sdlsock");
  g_object_set_data_full(G_OBJECT(gens_window), "sdlsock",
       gtk_widget_ref(sdlsock), (GDestroyNotify) gtk_widget_unref);
  gtk_box_pack_end(
       GTK_BOX(lookup_widget(gens_window, "vbox1")),
       sdlsock, 0, 0, 0);

  gtk_widget_show(gens_window);
  //initializeConsoleRomsView(); // not yet finished (? - wryun)

  if (!Init ())
    return 0;
  
  if (0 != strcmp(Start_Rom, "")) {
    if (-1 == Open_Rom(Start_Rom)) {
      fprintf(stderr, "Failed to load %s\n", Start_Rom);
    }
  }

  while (gtk_events_pending ())
    gtk_main_iteration_do (0);

  Set_Render(Full_Screen,Render_Mode,1);
  sync_gens_ui (UPDATE_GTK);

  while (is_gens_running ())
    {
      while (gtk_events_pending ())
		gtk_main_iteration_do (0);
	  update_SDL_events ();
#ifdef GENS_DEBUG
      if (Debug)		// DEBUG
	{
	  Update_Debug_Screen ();
	  Flip ();
	}
      else
#endif
      if (Genesis_Started || _32X_Started || SegaCD_Started)
	{
	  if ((Active) && (!Paused))	// EMULATION
	    {
	    /*if (fast_forward == 1)
		{
		  Update_Emulation_One ();
		}
	      else
		{*/
		  Update_Emulation ();
		//}
	    }
	  else			// EMULATION PAUSED
	    {
		    Do_VDP_Only();
		    Pause_Screen();
	      Flip ();
	      Sleep (100);
	    }
	}
      else if (GYM_Playing)	// PLAY GYM
	{
	  Play_GYM ();
	  Update_Gens_Logo ();
	}
      else if (Intro_Style == 1)	// GENS LOGO EFFECT
	{
	  Update_Gens_Logo ();
	  Sleep (5);
	}
      else if (Intro_Style == 2)	// STRANGE EFFECT
	{
	  Update_Crazy_Effect ();
	  Sleep (10);
	}
      else if (Intro_Style == 3)	// GENESIS BIOS
	{
	  Do_Genesis_Frame ();
	  Flip ();
	  Sleep (20);
	}
      else			// BLANK SCREEN (MAX IDLE)
	{
	  //Clear_Back_Screen();
	  Eff_Screen();
	  Flip ();
	  Sleep (200);
	}
    }

  strncpy (Str_Tmp, getenv ("HOME"), 1024);
  strcat (Str_Tmp, "/.gens/gens.cfg");
  Save_Config (Str_Tmp);

  End_All ();
  return 0;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "g_main.h"
#include "timer.h"
#include "gens.h"
#include "g_md.h"
#include "g_mcd.h"
#include "port.h"
#include "misc.h"
#include "vdp_rend.h"
#include "save.h"
#include "config_file.h"
#include "cd_aspi.h"
#include "vdp_io.h"
#include "gym.h"
#include "mem_m68k.h"
#include "ym2612.h"
#include "ui_proxy.h"
#include "parse.h"
#include "cpu_sh2.h"
#include "cpu_68k.h"
#include "cpu_z80.h"
#include "psg.h"
#include "pwm.h"
#include "debug.h"
#include "ggenie.h"

// TODO: Eliminate the dependency on these files.
#include "g_sdlsound.h"
#include "g_sdldraw.h"
#include "g_sdlinput.h"

#include "ui-common.h"
#include "gtk-misc.h"
#include "gens/gens_window.h"
#include "gens/gens_window_sync.h"

// GENS Settings struct
struct GENS_Settings_t Settings;

int fast_forward = 0;

POINT Window_Pos;

// Stupid temporary string needed for some stuff.
char Str_Tmp[1024];

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


/**
 * Init_Settings(): Initialize the Settings struct.
 */
void Init_Settings(void)
{
	// Nothing yet...
}

static int Build_Language_String (void)
{
	unsigned long nb_lue = 1;
	int sec_alloue = 1, poscar = 0;
	enum etat_sec
	{
		DEB_LIGNE,
		SECTION,
		NORMAL,
	} etat = DEB_LIGNE;
	
	FILE *LFile;
	
	char c;
	
	if (language_name)
	{
		free (language_name);
		language_name = NULL;
	}
	
	language_name = (char **) malloc (sec_alloue * sizeof (char *));
	language_name[0] = NULL;
	
	LFile = fopen (Settings.PathNames.Language_Path, "r");
	if (!LFile)
	{
		LFile = fopen (Settings.PathNames.Language_Path, "w");
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
		WritePrivateProfileString ("English", "Menu Language", "&English menu", Settings.PathNames.Language_Path);
	}
	
	return 0;
}


/**
 * Sleep(): Sleeps for i milliseconds.
 * @param i Milliseconds to sleep.
 */
void Sleep(int i)
{
	//usleep(i * 1000);
}


/**
 * close_gens(): Close GENS.
 */
void close_gens ()
{
	Gens_Running = 0;
}

/**
 * run_gens(): Run GENS.
 */
void run_gens ()
{
	Gens_Running = 1;
}

/**
 * is_gens_running(): Is GENS running?!
 * @return 1 if it's running.
 */
int is_gens_running ()
{
	return Gens_Running;
}

/**
 * InitParameters(): Initialize GENS configuration.
 * TODO: Use a struct, dammit!
 */
static void InitParameters (void)
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
	
	Get_Save_Path(Settings.PathNames.Gens_Path, GENS_PATH_MAX);
	Create_Save_Directory(Settings.PathNames.Gens_Path);
	
	// Create default language filename.
	strncpy (Settings.PathNames.Language_Path, Settings.PathNames.Gens_Path, GENS_PATH_MAX);
	strcat (Settings.PathNames.Language_Path, "language.dat");
	
	// Create default configuration filename.
	strncpy (Str_Tmp, Settings.PathNames.Gens_Path, 1000);
	strcat (Str_Tmp, "gens.cfg");
	
	// Default manual and CGOffline path is empty.
	strcpy (Settings.PathNames.Manual_Path, "");
	strcpy (Settings.PathNames.CGOffline_Path, "");
	
	// Build language strings and load the default configuration.
	Build_Language_String ();
	Load_Config (Str_Tmp, NULL);
	
	Init_CD_Driver ();
}

/**
 * Init(): Initialize GENS.
 * @return 1 if successful; 0 on errors.
 */
int Init (void)
{
	if (Init_OS_Graphics() != 0)
		return 0;
	
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


/**
 * End_All(): Close all functions.
 */
void End_All (void)
{
	Free_Rom (Game);
	End_DDraw ();
	End_Input ();
	YM2612_End ();
	End_Sound ();
	End_CD_Driver ();
	End_OS_Graphics();
}


/**
 * IsAsyncAllowed(): Determines if asynchronous stuff is allowed.
 * @return 0 if not allowed; non-zero otherwise.
 */
int IsAsyncAllowed(void)
{
	// In GENS Re-Recording, async is disabled if:
	// - Async is explicitly disabled due to testing for desyncs.
	// - A movie is being played or recorded.
	// Since none of the above applies here, async is allowed.
	return 1;
}


/**
 * main(): Main loop.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Error code.
 */
int main(int argc, char *argv[])
{
	// Initialize the Settings struct.
	Init_Settings();
	
	// Initialize the Game Genie array.
	Init_GameGenie();
	
	//char sdlbuf[32];
	
	//sprintf(sdlbuf, "SDL_WINDOWID=%ld",  GDK_WINDOW_XWINDOW(gens_window->window));
	//putenv(sdlbuf);
	GtkWidget *sdlsock;
	
	Init_Genesis_Bios ();
	
	InitParameters ();
	parseArgs (argc, argv);
	
	// TODO: Split out GTK+ stuff from main().
	add_pixmap_directory(DATADIR);
	add_pixmap_directory("images");
	gtk_init(&argc, &argv);
	
	gens_window = create_gens_window ();
	sdlsock = gtk_event_box_new();
	gtk_widget_set_name(sdlsock, "sdlsock");
	g_object_set_data_full(G_OBJECT(gens_window), "sdlsock",
	gtk_widget_ref(sdlsock), (GDestroyNotify) gtk_widget_unref);
	gtk_box_pack_end(GTK_BOX(lookup_widget(gens_window, "vbox1")), sdlsock, 0, 0, 0);
	
	
	gtk_widget_show(gens_window);
	//initializeConsoleRomsView(); // not yet finished (? - wryun)
	
	if (!Init ())
		return 0;
	
	if (strcmp(Settings.PathNames.Start_Rom, "") != 0)
	{
		if (Open_Rom(Settings.PathNames.Start_Rom) == -1)
		{
			fprintf(stderr, "Failed to load %s\n", Settings.PathNames.Start_Rom);
		}
	}
	
	while (gtk_events_pending ())
		gtk_main_iteration_do (0);
	
	Set_Render(Full_Screen,Render_Mode,1);
	
	// Synchronize the Gens window.
	Sync_Gens_Window();
	
	while (is_gens_running ())
	{
		while (gtk_events_pending ())
			gtk_main_iteration_do (0);
		update_SDL_events ();
#ifdef GENS_DEBUG
		if (Debug)		// DEBUG
		{
			Update_Debug_Screen();
			Flip ();
		}
		else
#endif
		if (Genesis_Started || _32X_Started || SegaCD_Started)
		{
			if ((Active) && (!Paused))
			{
				// EMULATION ACTIVE
				/*
				if (fast_forward == 1)
				{
					Update_Emulation_One ();
				}
				else
				{
				*/
					Update_Emulation ();
				//}
			}
			else
			{
				// EMULATION PAUSED
				Do_VDP_Only();
				Pause_Screen();
				Flip ();
				Sleep (100);
			}
		}
		else if (GYM_Playing)
		{
			// PLAY GYM
			Play_GYM ();
			Update_Gens_Logo ();
		}
		else if (Intro_Style == 1)
		{
			// GENS LOGO EFFECT
			Update_Gens_Logo ();
			Sleep (5);
		}
		else if (Intro_Style == 2)
		{
			// STRANGE EFFECT
			Update_Crazy_Effect ();
			Sleep (10);
		}
		else if (Intro_Style == 3)
		{
			// GENESIS BIOS
			Do_Genesis_Frame ();
			Flip ();
			Sleep (20);
		}
		else
		{
			// BLANK SCREEN (MAX IDLE)
			// NOTE: GTK+ is running in the same thread, so this causes GTK+ to be laggy.
			// TODO: Fix this lag!
			//Clear_Back_Screen();
			Eff_Screen();
			Flip ();
			Sleep (200);
		}
	}
	
	Get_Save_Path(Str_Tmp, GENS_PATH_MAX);
	strcat(Str_Tmp, "gens.cfg");
	Save_Config(Str_Tmp);
	
	End_All ();
	return 0;
}

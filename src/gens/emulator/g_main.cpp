/**
 * Gens: Main module.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "g_main.hpp"
#include "port/timer.h"
#include "port/port.h"
#include "port/ini.hpp"
#include "gens.h"
#include "g_md.h"
#include "g_mcd.h"
#include "g_32x.h"
#include "gens_core/misc/misc.h"
#include "gens_core/vdp/vdp_rend.h"
#include "util/file/save.h"
#include "util/file/config_file.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "util/sound/gym.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/sound/ym2612.h"
#include "ui_proxy.hpp"
#include "parse.hpp"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/z80/cpu_z80.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pwm.h"
#include "util/file/ggenie.h"
#include "g_update.hpp"
#include "g_palette.h"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.h"
#endif /* GENS_DEBUGGER */

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.h"
#endif

// TODO: Eliminate the dependency on these files.
#include "sdllayer/g_sdlsound.h"
#include "sdllayer/g_sdlinput.h"

#include "ui-common.h"
#include "gtk-misc.h"
#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"

// GENS Settings struct
struct Gens_Settings_t Settings;
struct Gens_PathNames_t PathNames;
struct Gens_BIOS_Filenames_t BIOS_Filenames;
struct Gens_Misc_Filenames_t Misc_Filenames;
struct Gens_VideoSettings_t Video;

// Bits per pixel.
// This is used by asm functions, so it can't be kept in a struct.
unsigned char bpp;

// from gens-rerecording
// TODO: Move somewhere else.
int _XRay = 0; // TODO: Port X-ray support from Gens Rerecording.
unsigned int _Pal32_XRAY[0x10000];

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

// New video layer.
#include "video/v_draw.hpp"
#include "video/v_draw_sdl.hpp"
#include "video/v_draw_sdl_gl.hpp"
VDraw *draw;


// TODO: Rewrite the language system so it doesn't depend on the old INI functions.
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
	
	language_name = (char **)malloc(sec_alloue * sizeof(char*));
	language_name[0] = NULL;
	
	LFile = fopen(PathNames.Language_Path, "r");
	if (!LFile)
	{
		LFile = fopen(PathNames.Language_Path, "w");
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
		WritePrivateProfileString("English", "Menu Language", "&English menu", PathNames.Language_Path);
	}
	
	return 0;
}


/**
 * Init_Settings(): Initialize the Settings struct.
 */
static void Init_Settings(void)
{
	// Initialize video settings.
	Video.Fast_Blur = 0;
	Video.Render_W = 1;	// Double
	Video.Render_FS = 1;	// Double
#ifdef GENS_OPENGL
	Video.OpenGL = 1;
	Video.Width_GL = 640;
	Video.Height_GL = 480;
#endif
	
	// Default bpp.
	bpp = 32;
	
	// Old code from InitParameters().
	VDP_Num_Vis_Lines = 224;
	Net_Play = 0;
	draw->setStretch(false);
	Sprite_Over = 1;
	
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
	CPU_Mode = 0;
	
#ifdef GENS_DEBUGGER
	Debug = 0;
#endif /* GENS_DEBUGGER */
	
	Get_Save_Path(PathNames.Gens_Path, GENS_PATH_MAX);
	Create_Save_Directory(PathNames.Gens_Path);
	
	// Create default language filename.
	strncpy (PathNames.Language_Path, PathNames.Gens_Path, GENS_PATH_MAX);
	strcat (PathNames.Language_Path, "language.dat");
	
	// Create default configuration filename.
	strncpy(Str_Tmp, PathNames.Gens_Path, 1000);
	strcat(Str_Tmp, "gens.cfg");
	
	// Default manual and CGOffline path is empty.
	strcpy(Misc_Filenames.Manual, "");
	strcpy(Misc_Filenames.GCOffline, "");
	
	// Build language strings and load the default configuration.
	Build_Language_String();
	Load_Config(Str_Tmp, NULL);
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
int is_gens_running()
{
	return Gens_Running;
}


/**
 * Init(): Initialize GENS.
 * @return 1 if successful; 0 on errors.
 */
int Init(void)
{
	if (draw->Init_Subsystem() != 0)
		return 0;
	
	init_timer();
	
	Identify_CPU();
	
	MSH2_Init();
	SSH2_Init();
	M68K_Init();
	S68K_Init();
	Z80_Init();
	
	YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
	PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
	PWM_Init();
	
	Init_Input();
	
	// Initialize the CD-ROM drive, if available.
#ifdef GENS_CDROM
	Init_CD_Driver();
#endif
	
	Init_Tab();
	run_gens();
	
	return 1;
}


/**
 * End_All(): Close all functions.
 */
void End_All(void)
{
	Free_Rom(Game);
	End_Input();
	YM2612_End();
	End_Sound();
#ifdef GENS_CDROM
	End_CD_Driver();
#endif
	
	draw->End_Video();
	draw->Shut_Down();
	delete draw;
	draw = NULL;
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
 * MESSAGE_L(): Print a localized message.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param time Time to display the message (in milliseconds).
 */
void MESSAGE_L(const char* str, const char* def, int time)
{
	char buf[1024];
	GetPrivateProfileString(language_name[Language], str, def, buf, 1024, PathNames.Language_Path);
	Put_Info(buf, time);
}


/**
 * MESSAGE_NUM_L(): Print a localized message with one number in a printf()-formatted string.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param num Number.
 * @param time Time to display the message (in milliseconds).
 */
void MESSAGE_NUM_L(const char* str, const char* def, int num, int time)
{
	char msg_tmp[1024];
	char buf[1024];
	GetPrivateProfileString(language_name[Language], str, def, buf, 1024, PathNames.Language_Path);
	sprintf(msg_tmp, buf, num);
	Put_Info(msg_tmp, time);
}


/**
 * MESSAGE_STR_L(): Print a localized message with a substring in a printf()-formatted string.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param str2 Substring.
 * @param time Time to display the message (in milliseconds).
 */
void MESSAGE_STR_L(const char* str, const char* def, const char* str2, int time)
{
	char msg_tmp[1024];
	char buf[1024];
	GetPrivateProfileString(language_name[Language], str, def, buf, 1024, PathNames.Language_Path);
	sprintf(msg_tmp, buf, str2);
	Put_Info(msg_tmp, time);
}


/**
 * MESSAGE_NUM_2L(): Print a localized message with two numbers in a printf()-formatted string.
 * @param str String.
 * @param def Default string if the string isn't found in the language file.
 * @param num1 First number.
 * @param num2 Second number.
 * @param time Time to display the message (in milliseconds).
 */
void MESSAGE_NUM_2L(const char* str, const char* def, int num1, int num2, int time)
{
	char msg_tmp[1024];
	char buf[1024];
	GetPrivateProfileString(language_name[Language], str, def, buf, 1024, PathNames.Language_Path);
	sprintf(msg_tmp, buf, num1, num2);
	Put_Info(msg_tmp, time);
}


/**
 * main(): Main loop.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Error code.
 */
int main(int argc, char *argv[])
{
	// Initialize the drawing object.
	// TODO: Select VDraw_SDL(), VDraw_SDL_GL(), or VDraw_DDraw() depending on other factors.
	draw = new VDraw_SDL();
	
	// Initialize the Settings struct.
	Init_Settings();
	
	// Initialize the Game Genie array.
	Init_GameGenie();
	
	// Parse command line arguments.
	parseArgs(argc, argv);
	
	// Recalculate the palettes, in case a command line argument changed a video setting.
	Recalculate_Palettes();
	
	//char sdlbuf[32];
	
	//sprintf(sdlbuf, "SDL_WINDOWID=%ld",  GDK_WINDOW_XWINDOW(gens_window->window));
	//putenv(sdlbuf);
	GtkWidget *sdlsock;
	
	Init_Genesis_Bios();
	
	// Initialize Gens.
	if (!Init())
		return 0;
	
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
	
	// Check if OpenGL needs to be enabled.
	Change_OpenGL(Video.OpenGL);
	
	if (strcmp(PathNames.Start_Rom, "") != 0)
	{
		if (Open_Rom(PathNames.Start_Rom) == -1)
		{
			fprintf(stderr, "Failed to load %s\n", PathNames.Start_Rom);
		}
	}
	
	while (gtk_events_pending ())
		gtk_main_iteration_do (0);
	
	int rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
	if (!draw->setRender(rendMode))
	{
		// Cannot initialize video mode. Try using render mode 0 (normal).
		if (!draw->setRender(0))
		{
			// Cannot initialize normal mode.
			fprintf(stderr, "FATAL ERROR: Cannot initialize any renderers.\n");
			return 1;
		}
	}
	
	// Synchronize the Gens window.
	Sync_Gens_Window();
	
	while (is_gens_running ())
	{
		while (gtk_events_pending ())
			gtk_main_iteration_do (0);
		update_SDL_events ();
#ifdef GENS_DEBUGGER
		if (Debug)		// DEBUG
		{
			Update_Debug_Screen();
			draw->flip();
		}
		else
#endif /* GENS_DEBUGGER */
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
					Update_Emulation();
				//}
			}
			else
			{
				// EMULATION PAUSED
				if (_32X_Started)
					Do_32X_VDP_Only();
				else
					Do_VDP_Only();
				//Pause_Screen();
				draw->flip();
				Sleep(100);
			}
		}
		else
		{
			// No game is currently running.
			
			// Update the screen.
			draw->flip();
			
			// Determine how much sleep time to add, based on intro style.
			// TODO: Move this to v_draw.cpp?
			if (GYM_Playing)
			{
				// PLAY GYM
				Play_GYM();
			}
			else if (Intro_Style == 1)
			{
				// Gens logo effect. (TODO: This is broken!)
				Sleep(5);
			}
			else if (Intro_Style == 2)
			{
				// "Strange" effect. (TODO: This is broken!)
				Sleep(10);
			}
			else if (Intro_Style == 3)
			{
				// Genesis BIOS. (TODO: This is broken!)
				Sleep(20);
			}
			else
			{
				// Blank screen. (MAX IDLE)
				// NOTE: GTK+ is running in the same thread, so this causes GTK+ to be laggy.
				// TODO: Fix this lag!
				Sleep(200);
			}
		}
	}
	
	Get_Save_Path(Str_Tmp, GENS_PATH_MAX);
	strcat(Str_Tmp, "gens.cfg");
	Save_Config(Str_Tmp);
	
	End_All ();
	return 0;
}


// The following is stuff imported from g_sdldraw.c.
// TODO: Figure out where to put it.
int (*Update_Frame)(void);
int (*Update_Frame_Fast)(void);

// TODO: Only used for DirectDraw.
int Flag_Clr_Scr = 0;

// VSync flags
int FS_VSync;
int W_VSync;

// TODO: Get rid of this.
static void win2linux(char* str)
{
	char* tmp=str;
	for (; *tmp; ++tmp)
	{
		switch((unsigned char)*tmp)
		{
			case 0xE7: *tmp='c';break;//ç
			case 0xE8: *tmp='e';break;//è
			case 0xE9: *tmp='e';break;//é
			case 0xEA: *tmp='e';break;//ê
			case 0xE0: *tmp='a';break;//à
			case 0xEE: *tmp='i';break;//î
			default:break;
		}	
	}
}


/**
 * Put_Info(): Put a message on the screen.
 * @param message Message to write to the screen.
 * @param duration Duration for the message to appear, in milliseconds.
 */
char Info_String[1024];
int Message_Showed = 0;
unsigned int Info_Time = 0;
int ice = 0;
void Put_Info(const char* msg, int duration)
{
	// TODO: Figure out a better place to put this.
	win2linux(Info_String);		// TODO: Get rid of this.
	draw->writeText(msg, duration);
}


/**
 * Clear_Screen_MD(): Clears the MD screen.
 */
void Clear_Screen_MD(void)
{
	memset(MD_Screen, 0x00, sizeof(MD_Screen));
	memset(MD_Screen32, 0x00, sizeof(MD_Screen32));
}


/**
 * Change_OpenGL(): Change the OpenGL setting.
 * @param stretch 0 to turn OpenGL off; 1 to turn OpenGL on.
 */
void Change_OpenGL(int newOpenGL)
{
	// End the current drawing function.
	draw->End_Video();
	
	Video.OpenGL = (newOpenGL == 0 ? 0 : 1);
	VDraw *newDraw;
	if (Video.OpenGL)
	{
		newDraw = new VDraw_SDL_GL(draw);
		newDraw->Init_Video();
		delete draw;
		draw = newDraw;
		MESSAGE_L("Selected OpenGL Renderer", "Selected OpenGL Renderer", 1500);
	}
	else
	{
		newDraw = new VDraw_SDL(draw);
		newDraw->Init_Video();
		delete draw;
		draw = newDraw;
		MESSAGE_L("Selected SDL Renderer", "Selected SDL Renderer", 1500);
	}
}


/**
 * Set_GL_Resolution(): Set the OpenGL resolution.
 * @param w Width.
 * @param h Height.
 */
void Set_GL_Resolution(int w, int h)
{
	// TODO: Move this to VDraw_GL.
	
	if (Video.Width_GL == w && Video.Height_GL == h)
		return;
	
	// OpenGL resolution has changed.
	Video.Width_GL = w;
	Video.Height_GL = h;
	
	// Print the resolution information.
	MESSAGE_NUM_2L("Selected %dx%d resolution",
		       "Selected %dx%d resolution", w, h, 1500);
	
	// Synchronize the Graphics menu.
	Sync_Gens_Window_GraphicsMenu();
	
	// If OpenGL mode isn't enabled, don't do anything.
	if (!Video.OpenGL)
		return;
	
	// OpenGL mode is currently enabled. Change the resolution.
	int rendMode = (draw->fullScreen() ? Video.Render_FS : Video.Render_W);
	draw->setRender(rendMode);
}

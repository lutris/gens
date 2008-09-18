/**
 * Gens: Main loop. (Linux specific code)
 */


#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g_main.hpp"
#include "g_main_linux.hpp"

#include "g_update.hpp"
#include "g_palette.h"
#include "gens_ui.hpp"
#include "parse.hpp"
#include "g_md.hpp"
#include "g_32x.hpp"

#include "gens_core/vdp/vdp_io.h"
#include "util/file/config_file.hpp"
#include "util/file/ggenie.h"
#include "util/sound/gym.hpp"

#include "video/v_draw_sdl.hpp"
#include "video/v_draw_sdl_gl.hpp"
#include "input/input_sdl.hpp"
#include "audio/audio_sdl.hpp"

#include "gens/gens_window_sync.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */


/**
 * GENS_Default_Save_Path(): Create the default save path.
 * @param *buf Buffer to store the default save path in.
 */
void Get_Save_Path(char *buf, size_t n)
{
	strncpy(buf, getenv ("HOME"), n);
	strcat(buf, "/.gens/");
}


/**
 * GENS_Create_Default_Save_Directory(): Create the default save directory.
 * @param *dir Directory name.
 */
void Create_Save_Directory(const char *dir)
{
	mkdir(dir, 0700);
}


// TODO: Get rid of this.
void win2linux(char* str)
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
	
	// Initialize the input object.
	input = new Input_SDL();
	
	// Initialize the audio object.
	audio = new Audio_SDL();
	
	// Initialize the Settings struct.
	Init_Settings();
	
	// Initialize the Game Genie array.
	Init_GameGenie();
	
	// Parse command line arguments.
	parseArgs(argc, argv);
	
	// Recalculate the palettes, in case a command line argument changed a video setting.
	Recalculate_Palettes();
	
	Init_Genesis_Bios();
	
	// Initialize Gens.
	if (!Init())
		return 0;
	
	// Initialize the UI.
	GensUI::init(argc, argv);
	
	// not yet finished (? - wryun)
	//initializeConsoleRomsView();
	
#ifdef GENS_OPENGL
	// Check if OpenGL needs to be enabled.
	// This also initializes SDL or SDL+GL.
	Change_OpenGL(Video.OpenGL);
#else
	// Initialize SDL.
	draw->Init_Video();
#endif
	
	if (strcmp(PathNames.Start_Rom, "") != 0)
	{
		if (Open_Rom(PathNames.Start_Rom) == -1)
		{
			fprintf(stderr, "Failed to load %s\n", PathNames.Start_Rom);
		}
	}
	
	// Update the UI.
	GensUI::update();
	
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
		// Update the UI.
		GensUI::update();
		
		// Update physical controller inputs.
		input->update();
		
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
				if (fast_forward)
					Update_Emulation_One();
				else
					Update_Emulation();
				
				// Prevent 100% CPU usage.
				// The Linux CPU scheduler will take away CPU time from Gens/GS
				// if it notices that the process is eating up too much CPU time.
				GensUI::sleep(1);
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
				GensUI::sleep(100);
			}
		}
		else
		{
			// No game is currently running.
			
			// Update the screen.
			draw->flip();
			
			// Determine how much sleep time to add, based on intro style.
			// TODO: Move this to v_draw.cpp?
			if (audio->playingGYM())
			{
				// PLAY GYM
				Play_GYM();
			}
			else if (Intro_Style == 1)
			{
				// Gens logo effect. (TODO: This is broken!)
				GensUI::sleep(5);
			}
			else if (Intro_Style == 2)
			{
				// "Strange" effect. (TODO: This is broken!)
				GensUI::sleep(10);
			}
			else if (Intro_Style == 3)
			{
				// Genesis BIOS. (TODO: This is broken!)
				GensUI::sleep(20);
			}
			else
			{
				// Blank screen. (MAX IDLE)
				// NOTE: GTK+ is running in the same thread, so this causes GTK+ to be laggy.
				// TODO: Fix this lag!
				GensUI::sleep(200);
			}
		}
	}
	
	Get_Save_Path(Str_Tmp, GENS_PATH_MAX);
	strcat(Str_Tmp, "gens.cfg");
	Save_Config(Str_Tmp);
	
	End_All();
	return 0;
}


#ifdef GENS_OPENGL
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
	draw->setRender(rendMode, true);
}
#endif /* GENS_OPENGL */

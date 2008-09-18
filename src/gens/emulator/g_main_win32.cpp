/**
 * Gens: Main loop. (Win32-specific code)
 */


#include "g_main_win32.hpp"


HWND Gens_hWnd;
HINSTANCE ghInstance;


/**
 * Get_Save_Path(): Get the default save path.
 * @param *buf Buffer to store the default save path in.
 */
void Get_Save_Path(char *buf, size_t n)
{
	GetCurrentDirectory(n, buf);
}


/**
 * Create_Save_Directory(): Create the default save directory.
 * @param *dir Directory name.
 */
void Create_Save_Directory(const char *dir)
{
	// Does nothing on Win32.
}


/**
 * WinMain: Win32 main loop.
 * @param hinst ???
 * @param hPrevInst ???
 * @param lpCmdLine ???
 * @param nCmdShow ???
 * @return Error code.
 */
int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	// Initialize the drawing object.
	draw = new VDraw_DDraw();
	
	// Initialize the input object.
	input = new Input_DInput();
	
	// Initialize the audio object.
	audio = new Audio_DSound();
	
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
	ghInstance = hInst;
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
	
	End_All ();
	return 0;
}

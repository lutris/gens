/**
 * Gens: Main loop. (Win32-specific code)
 */


#include "g_main.hpp"
#include "g_main_win32.hpp"

#include "g_update.hpp"
#include "g_palette.h"
#include "gens_ui.hpp"
#include "parse.hpp"

#include "g_md.hpp"
#include "g_mcd.hpp"
#include "g_32x.hpp"

#include "gens_core/vdp/vdp_io.h"
#include "util/file/config_file.hpp"
#include "util/file/ggenie.h"
#include "util/sound/gym.hpp"

#include "video/v_draw_ddraw.hpp"
#include "input/input_dinput.hpp"
#include "audio/audio_dsound.hpp"

#include "gens/gens_window.hpp"
#include "gens/gens_window_sync.hpp"

#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

#include "ui/charset.hpp"

// Win32 instance
HINSTANCE ghInstance = NULL;

// Windows version
OSVERSIONINFO winVersion;

// If extended Common Controls are enabled, this is set to a non-zero value.
int win32_CommCtrlEx = 0;

// Fonts
HFONT fntMain = NULL;
HFONT fntTitle = NULL;

// Maximum value function
#ifndef max
#define max(a,b)   (((a) > (b)) ? (a) : (b))
#endif /* max */

// Needed for some macros
#include <windowsx.h>

#if 0
// argc/argv conversion.
#include "port/argc_argv.h"
#endif

/**
 * Get_Save_Path(): Get the default save path.
 * @param *buf Buffer to store the default save path in.
 */
void Get_Save_Path(char *buf, size_t n)
{
	strncpy(buf, ".\\", n);
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
 * Win32_centerOnGensWindow(): Center the specified window on the Gens window.
 * @param hWnd Window handle.
 */
void Win32_centerOnGensWindow(HWND hWnd)
{
	// TODO: Move this to GensUI.
	
	RECT r, r2;
	int dx1, dy1, dx2, dy2;
	
	GetWindowRect(Gens_hWnd, &r);
	dx1 = (r.right - r.left) / 2;
	dy1 = (r.bottom - r.top) / 2;
	
	GetWindowRect(hWnd, &r2);
	dx2 = (r2.right - r2.left) / 2;
	dy2 = (r2.bottom - r2.top) / 2;
	
	SetWindowPos(hWnd, NULL,
		     max(0, r.left + (dx1 - dx2)),
		     max(0, r.top + (dy1 - dy2)), 0, 0,
		     SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


/**
 * Win32_setActualWindowSize(): Set the actual window size, including the non-client area.
 * @param hWnd Window handle.
 * @param reqW Required width.
 * @param reqH Required height.
 */
void Win32_setActualWindowSize(HWND hWnd, const int reqW, const int reqH)
{
	RECT r;
	SetRect(&r, 0, 0, reqW, reqH);
	
	// Adjust the rectangle.
	AdjustWindowRectEx(&r, GetWindowStyle(hWnd), (GetMenu(hWnd) != NULL), GetWindowExStyle(hWnd));
	
	// Set the window size.
	SetWindowPos(hWnd, NULL, 0, 0, r.right - r.left, r.bottom - r.top,
		     SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}


/**
 * parseCmdLine_Old(): Old command line parser from Gens/Win32.
 * @param lpCmdLine Command line.
 */
static void parseCmdLine_Old(LPSTR lpCmdLine)
{
	if (!lpCmdLine || !lpCmdLine[0])
		return;
	
	int src = 0;
	
	if (lpCmdLine[src] == '\"')
	{
		// Quoted name.
		src++;
		
		while ((lpCmdLine[src] != '\"') && (lpCmdLine[src]))
		{
			PathNames.Start_Rom[src - 1] = lpCmdLine[src];
			src++;
		}
		
		PathNames.Start_Rom[src - 1] = 0x00;
	}
	else
	{
		// Non-quoted name. Copy it verbatim.
		strncpy(PathNames.Start_Rom, lpCmdLine, GENS_PATH_MAX);
		PathNames.Start_Rom[GENS_PATH_MAX - 1] = 0x00;
	}
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
	// Save hInst for other functions.
	ghInstance = hInst;
	
	// Gens_hWnd is needed before anything else is set up.
	// Initialize the Gens hWnd.
	initGens_hWnd();
	
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
	// TODO: This causes a crash.
#if 0
	argc_argv arg;
	convertCmdLineToArgv(lpCmdLine, &arg);
	parseArgs(arg.c, arg.v);
	deleteArgcArgv(&arg);
#endif
	
	// Old Gens/Win32 command line parser.
	// It merely checks for a ROM filename.
	parseCmdLine_Old(lpCmdLine);
	
	// Recalculate the palettes, in case a command line argument changed a video setting.
	Recalculate_Palettes();
	
	Init_Genesis_Bios();
	
	// Initialize Gens.
	if (!Init())
		return 0;
	
	// Initialize the UI.
	GensUI::init(0, NULL);
	
	// not yet finished (? - wryun)
	//initializeConsoleRomsView();
	
#ifdef GENS_OPENGL
	// Check if OpenGL needs to be enabled.
	// This also initializes SDL or SDL+GL.
	Change_OpenGL(Video.OpenGL);
#else
	// Initialize DirectDraw.
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
	
	// Show the Gens window.
	ShowWindow(Gens_hWnd, nCmdShow);
	
	while (is_gens_running())
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
				GensUI::sleep(200);
			}
		}
	}
	
	audio->clearSoundBuffer();
	Get_Save_Path(Str_Tmp, GENS_PATH_MAX);
	strcat(Str_Tmp, "gens.cfg");
	Save_Config(Str_Tmp);
	
	End_All();
	ChangeDisplaySettings(NULL, 0);
	DestroyWindow(Gens_hWnd);
	
	// Delete the fonts.
	DeleteFont(fntMain);
	DeleteFont(fntTitle);
	
	// Empty the message queue.
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, 0, 0))
			return msg.wParam;
	}
	
	TerminateProcess(GetCurrentProcess(), 0); //Modif N
	
	return 0;
}

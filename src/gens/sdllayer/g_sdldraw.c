#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#ifdef GENS_OPENGL
#include "g_opengl.h"
#endif

#include "port/port.h"
#include "port/timer.h"
#include "g_sdldraw.h"
#include "g_sdlsound.h"
#include "g_sdlinput.h"
#include "emulator/g_main.h"
#include "emulator/gens.h"
#include "emulator/g_md.h"
#include "emulator/g_32x.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/misc.h"
#include "gens_core/gfx/blit.h"
#include "emulator/ui_proxy.h"
//#include "net.h"
void Sleep(int i);
#include "segacd/cdda_mp3.h"
#include "gens_core/gfx/renderers.h"
#include "emulator/g_palette.h"

// Needed to synchronize the Graphics menu after a bpp change.
#include "gens/gens_window_sync.h"

SDL_Surface *screen = NULL;
const int Gens_SDL_Flags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE | SDL_ASYNCBLIT | SDL_HWACCEL;

#include "ui-common.h"


clock_t Last_Time = 0, New_Time = 0;
clock_t Used_Time = 0;

int x_offset, y_offset = 0;

int Flag_Clr_Scr = 0;
int Sleep_Time;
int FS_VSync;
int W_VSync;
int Stretch; 
int Blit_Soft;
int Effect_Color = 7;
int FPS_Style = EMU_MODE | BLANC;
int Message_Style = EMU_MODE | BLANC | SIZE_X2;
int Kaillera_Error = 0;

static gchar *WindowID = NULL;

// Screen shift in 1x rendering mode.
int shift = 0;

// Screen width shrinkage.
// 0 == full width; 64 == 256x224
int Dep = 0;
int Old_Dep = 0;

// Current border color.
unsigned short BorderColor_16B = 0x0000;
unsigned int BorderColor_32B = 0x00000000;

static char Info_String[1024] = "";
static int Message_Showed = 0;
static unsigned int Info_Time = 0;

// Blit functions for Full Screen and Windowed modes.
BlitFn Blit_FS;
BlitFn Blit_W;

int (*Update_Frame)();
int (*Update_Frame_Fast)();

unsigned long GetTickCount();

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

void Put_Info(char *Message, int Duree)
{
	if (Show_Message)
	{
		strcpy(Info_String, Message);
		win2linux(Info_String);
		Info_Time = GetTickCount() + Duree;
		Message_Showed = 1;
	}
}


int Init_Fail(int hwnd, char *err)
{
	End_DDraw();
	fputs(err, stderr);
	//MessageBox(hwnd, err, "Oups ...", MB_OK);
	//DestroyWindow(hwnd);
	exit(0);
	return 0;
}


static int Init_Draw_SDL(int w, int h)
{
	screen = SDL_SetVideoMode(w, h, bpp, Gens_SDL_Flags | (Video.Full_Screen ? SDL_FULLSCREEN : 0));
	
	if (screen == NULL)
	{
		fprintf(stderr, "Error creating SDL primary surface: %s\n", SDL_GetError());
		exit(0);
	}
	
	return 1;
}	

static void End_Draw_SDL()
{
	// Placeholder function in case something needs to be added later.
}


int Init_DDraw()
{	
	int x;
	int w, h;
	
#ifdef GENS_OPENGL
	if (Video.OpenGL)
	{
		w = Video.Width_GL;
		h = Video.Height_GL;
	}
	else
	{
#endif
		if (Video.Render_Mode == 0)
		{
			// Normal render mode. 320x240
			w = 320;
			h = 240;
		}
		else
		{
			w = 640;
			h = 480;
		}
#ifdef GENS_OPENGL
	}
#endif
	
	// TODO: Move Linux/GTK-specific code somewhere else.
	
	if (Video.Full_Screen)
	{
		UI_Hide_Embedded_Window();
		
		if (WindowID)
		{
			unsetenv("SDL_WINDOWID");
			g_free(WindowID);
			WindowID = NULL;
		}
	}
	else
	{
		UI_Show_Embedded_Window(w, h);
		
		/* Let GTK catch up. */
		while (gtk_events_pending())
			gtk_main_iteration_do(FALSE);
		
		if (!WindowID)
		{
			WindowID = g_strdup_printf("%d", UI_Get_Embedded_WindowID());
			setenv("SDL_WINDOWID", WindowID, 1);
		}
	}
	
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		Init_Fail(0,"Couldn't init embedded SDL.\n");
	
	Set_Game_Name();
	
#ifdef GENS_OPENGL
	if (Video.OpenGL)
		x = Init_Draw_GL(w, h);
	else
#endif
		x = Init_Draw_SDL(w, h);
	
	// Disable the cursor in fullscreen mode.
	if (Video.Full_Screen)
		SDL_ShowCursor(SDL_DISABLE);
	
	// Adjust stretch parameters.
	Adjust_Stretch();
	
	// If normal mode is set, disable the video shift.
	shift = (Video.Render_Mode != 0);
	
	// Return the status code of the Init_Draw_* function.
	return x;
}


void End_DDraw()
{	
#ifdef GENS_OPENGL
	if (Video.OpenGL)
		End_Draw_GL();
	else
#endif
		End_Draw_SDL();
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


/**
 * Clear_Screen_SDL(): Clear the SDL screen.
 */
static void Clear_Screen_SDL(void)
{
	SDL_LockSurface(screen);
	SDL_FillRect(screen, 0, 0);
	SDL_UnlockSurface(screen);
}


/**
 * Clear_Screen(): Clear the screen.
 */
void Clear_Screen()
{
#ifdef GENS_OPENGL
	if (Video.OpenGL)
		Clear_Screen_GL();
	else
#endif
		Clear_Screen_SDL();
}


/**
 * Flip_SDL(): Blit the contents of the MD framebuffer to the screen. (SDL method)
 */
static void Flip_SDL(void)
{
	SDL_Rect border;
	
	SDL_LockSurface(screen);
	
	if (!Genesis_Started && !SegaCD_Started && !_32X_Started)
	{
		// No system is active.
		// Make sure the border color is black.
		MD_Palette[0] = 0;
		MD_Palette32[0] = 0;
	}
	
	// Draw the border.
	// TODO: Make this more accurate and/or more efficient.
	// In particular, it only works for 1x and 2x rendering.
	if ((bpp == 15 || bpp == 16) && (BorderColor_16B != MD_Palette[0]))
	{
		BorderColor_16B = MD_Palette[0];
		if (VDP_Num_Vis_Lines < 240)
		{
			// Top/Bottom borders.
			border.x = 0; border.w = screen->w;
			border.h = 240 - VDP_Num_Vis_Lines;
			if (screen->h == 240)
				border.h >>= 1;
			border.y = 0;
			SDL_FillRect(screen, &border, BorderColor_16B);
			border.y = screen->h - border.h;
			SDL_FillRect(screen, &border, BorderColor_16B);
		}
		if (Dep > 0)
		{
			// Left/Right borders.
			if (border.h != 0)
			{
				border.y = 0;
				border.h = 240 - border.h;
			}
			
			border.x = 0; border.h = screen->h;
			border.w = Dep;
			if (screen->w == 320)
				border.w >>= 1;
			border.y = 0;
			SDL_FillRect(screen, &border, BorderColor_16B);
			border.x = screen->w - border.w;
			SDL_FillRect(screen, &border, BorderColor_16B);
		}
	}
	else if ((bpp == 32) && (BorderColor_32B != MD_Palette32[0]))
	{
		BorderColor_32B = MD_Palette32[0];
		if (VDP_Num_Vis_Lines < 240)
		{
			// Top/Bottom borders.
			border.x = 0; border.w = screen->w;
			border.h = 240 - VDP_Num_Vis_Lines;
			if (screen->h == 240)
				border.h >>= 1;
			border.y = 0;
			SDL_FillRect(screen, &border, BorderColor_32B);
			border.y = screen->h - border.h;
			SDL_FillRect(screen, &border, BorderColor_32B);
		}
		if (Dep > 0)
		{
			// Left/Right borders.
			if (border.h != 0)
			{
				border.y = 0;
				border.h = 240 - border.h;
			}
			
			border.x = 0; border.h = screen->h;
			border.w = Dep;
			if (screen->w == 320)
				border.w >>= 1;
			border.y = 0;
			SDL_FillRect(screen, &border, BorderColor_32B);
			border.x = screen->w - border.w;
			SDL_FillRect(screen, &border, BorderColor_32B);
		}
	}
	
	unsigned char bytespp = (bpp == 15 ? 2 : bpp / 8);
	
	// Start of the SDL framebuffer.
	int pitch = screen->w * bytespp;
	int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
	int HBorder = Dep * (bytespp / 2);		// Left border width, in pixels.
	
	int startPos = ((pitch * VBorder) + HBorder) << shift;	// Starting position from within the screen.
	
	unsigned char *start = &(((unsigned char*)(screen->pixels))[startPos]);
	
	if (Video.Full_Screen)		
	{
		Blit_FS(start, pitch, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep*2);
	}
	else
	{
		Blit_W(start, pitch, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep*2);
	}
	
	SDL_UnlockSurface(screen);
	
	SDL_Flip(screen);
}


/**
 * Flip(): Flip the screen buffer.
 * @return 1 on success.
 */
int Flip(void)
{
	//float Ratio_X, Ratio_Y;
	int i;
	static float FPS = 0.0f, frames[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	static unsigned int old_time = 0, view_fps = 0, index_fps = 0, freq_cpu[2] = {0, 0};
	unsigned int new_time[2];
	
	
	// TODO: Print the message and/or FPS counter on the screen buffer only.
	// Don't print it on MD_Screen.
	// Otherwise, messages and the FPS counter show up in screenshots.
	
	if (Message_Showed)
	{
		if (GetTickCount() > Info_Time)
		{
			Message_Showed = 0;
			strcpy(Info_String, "");
		}
		else 
		{
			Print_Text(Info_String, strlen(Info_String), 10, 210, Message_Style);
		}
	}
	else if (Show_FPS && (Genesis_Started || _32X_Started || SegaCD_Started) && !Paused)
	{	
		if (freq_cpu[0] > 1)				// accurate timer ok
		{
			if (++view_fps >= 16)
			{
				GetPerformanceCounter((long long *) new_time);
				if (new_time[0] != old_time)
				{					
					FPS = (float) (freq_cpu[0]) * 16.0f / (float) (new_time[0] - old_time);
					sprintf(Info_String, "%.1f", FPS);
				}
				else
				{
					sprintf(Info_String, "too much...");
				}
				
				old_time = new_time[0];
				view_fps = 0;
			}
		}
		else if (freq_cpu[0] == 1)			// accurate timer not supported
		{
			if (++view_fps >= 10)
			{
				new_time[0] = GetTickCount();
				
				if (new_time[0] != old_time) frames[index_fps] = 10000 / (float)(new_time[0] - old_time);
				else frames[index_fps] = 2000;
				
				index_fps++;
				index_fps &= 7;
				FPS = 0.0f;
				
				for(i = 0; i < 8; i++) FPS += frames[i];
				
				FPS /= 8.0f;
				old_time = new_time[0];
				view_fps = 0;
			}
			
			sprintf(Info_String, "%.1f", FPS);
		}
		else
		{
			GetPerformanceFrequency((long long *) freq_cpu);
			if (freq_cpu[0] == 0) freq_cpu[0] = 1;
			
			// TODO: WTF is this for?
			// Assuming it just clears the string...
			//sprintf(Info_String, "", FPS);
			Info_String[0] = 0;
		}
		
		Print_Text(Info_String, strlen(Info_String), 10, 210, FPS_Style);
	}
	
	// Blur the screen if requested.
	if (Video.Fast_Blur)
		Half_Blur();
	
	// Check if the display width changed.
	Old_Dep = Dep;
	if ((VDP_Reg.Set4 & 0x1) || (Debug))
		Dep = 0;
	else
		Dep = 64;
	
	if (Dep != Old_Dep)
	{
		// Display width change. Adjust the stretch parameters.
		Adjust_Stretch();
	}
	
	if (Dep > Old_Dep)
	{
		// New screen width is smaller than old screen width.
		// Clear the screen.
		Clear_Screen();
	}
	
	// Flip the screen buffer.
#ifdef GENS_OPENGL
	if (Video.OpenGL)
		Flip_GL();
	else
#endif
		Flip_SDL();
	
	// TODO: VSync
	/*
	if(W_VSync || FS_VSync)
		vsync();
	*/
	
	return 1;
}


int Update_Emulation(void)
{
	static int Over_Time = 0;
	int current_div;

	if (Frame_Skip != -1)
	{
		if (Sound_Enable)
		{
			Write_Sound_Buffer(NULL);
		}

		Update_Controllers();

		if (Frame_Number++ < Frame_Skip)
		{
			Update_Frame_Fast();
		}
		else
		{
			Frame_Number = 0;
			Update_Frame();
			Flip();
		}
	}
	else
	{
		if (Sound_Enable)
		{
			// This does auto-frame skip in a fairly dodgy way -
			// only updating the frame when we have 'lots' in
			// the audio buffer. Hence the audio is a couple of
			// cycles ahead of the graphics.
			
			Write_Sound_Buffer(NULL);
			while (!Lots_In_Audio_Buffer())
			{
				Update_Frame_Fast();
				Write_Sound_Buffer(NULL);
			}

			Update_Controllers();
			Update_Frame();
			Flip();
		} //If sound is enabled
		
		else
		{
			if (CPU_Mode) current_div = 20;
			else current_div = 16 + (Over_Time ^= 1);

			New_Time = GetTickCount();
			Used_Time += (New_Time - Last_Time);
			Frame_Number = Used_Time / current_div;
			Used_Time %= current_div;
			Last_Time = New_Time;

			if (Frame_Number > 8) Frame_Number = 8;

			for (; Frame_Number > 1; Frame_Number--)
			{
				Update_Controllers();
				Update_Frame_Fast();
			}

			if (Frame_Number)
			{
				Update_Controllers();
				Update_Frame();
				Flip();
			}
			else {Sleep(Sleep_Time);}
		} //If sound is not enabled
		
	}

	return 1;
}


int Update_Emulation_One(void)
{
	Update_Controllers();
	Update_Frame();
	Flip();

	return 1;
}


#if 0
int Update_Emulation_Netplay(int player, int num_player)
{
	static int Over_Time = 0;
	int current_div;

	if (CPU_Mode) current_div = 20;
	else current_div = 16 + (Over_Time ^= 1);

	New_Time = GetTickCount();
	Used_Time += (New_Time - Last_Time);
	Frame_Number = Used_Time / current_div;
	Used_Time %= current_div;
	Last_Time = New_Time;

	if (Frame_Number > 6) Frame_Number = 6;

	for (; Frame_Number > 1; Frame_Number--)
	{
		if (Sound_Enable)
		{
			if (WP == Get_Current_Seg()) WP = (WP - 1) & (Sound_Segs - 1);
			Write_Sound_Buffer(NULL);
			WP = (WP + 1) & (Sound_Segs - 1);
		}

		Scan_Player_Net(player);
		if (Kaillera_Error != -1) Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		//Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		Update_Controllers_Net(num_player);
		Update_Frame_Fast();
	}

	if (Frame_Number)
	{
		if (Sound_Enable)
		{
			if (WP == Get_Current_Seg()) WP = (WP - 1) & (Sound_Segs - 1);
			Write_Sound_Buffer(NULL);
			WP = (WP + 1) & (Sound_Segs - 1);
		}

		Scan_Player_Net(player);
		if (Kaillera_Error != -1) Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		//Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		Update_Controllers_Net(num_player);
		Update_Frame();
		Flip();
	}
	return 1;
}
#endif


/**
 * Clear_Screen_MD(): Clears the MD screen.
 */
void Clear_Screen_MD(void)
{
	// TODO: Figure out if sizeof(MD_Screen) is correct.
	memset(MD_Screen, 0x00, sizeof(MD_Screen));
	memset(MD_Screen32, 0x00, sizeof(MD_Screen32));
}


int Show_Genesis_Screen(void)
{
	Do_VDP_Only();
	Flip();

	return 1;
}


/**
 * Refresh_Video(): Refresh the video subsystem.
 */
void Refresh_Video(void)
{
	// Reset the border color to make sure it's redrawn.
	BorderColor_16B = ~MD_Palette[0];
	BorderColor_32B = ~MD_Palette32[0];
	
	End_DDraw();
	Init_DDraw();
	Adjust_Stretch();
}


/**
 * Set_bpp(): Sets the bpp value.
 * @param newbpp New bpp value.
 */
void Set_bpp(int newbpp)
{
	if (bpp == newbpp)
		return;
	
	bpp = newbpp;
	End_DDraw();
	Init_DDraw();
	
	// Reset the renderer.
	if (!Set_Render(Video.Full_Screen, Video.Render_Mode, 0))
	{
		// Cannot initialize video mode. Try using render mode 0 (normal).
		if (!Set_Render(Video.Full_Screen, 0, 1))
		{
			// Cannot initialize normal mode.
			fprintf(stderr, "FATAL ERROR: Cannot initialize any renderers.\n");
			exit(1);
		}
	}
	
	// Recalculate palettes.
	Recalculate_Palettes();
	
	// Synchronize the Graphics menu.
	Sync_Gens_Window_GraphicsMenu();
	
	// TODO: After switching color depths, the screen buffer isn't redrawn
	// until something's updated. Figure out how to trick the renderer
	// into updating anyway.
	
	// NOTE: This only seems to be a problem with 15-to-16 or 16-to-15 at the moment.
	
	// TODO: Figure out if 32-bit rendering still occurs in 15/16-bit mode and vice-versa.
}


void Adjust_Stretch(void)
{
#ifdef GENS_OPENGL
	if (Video.OpenGL)
	{
		Adjust_Stretch_GL();
	}
	else
	{
#endif
		// TODO: SDL stretch
#ifdef GENS_OPENGL
	}
#endif
}

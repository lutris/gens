#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include "port.h"
#include "timer.h"
#include "g_sdldraw.h"
#include "g_sdlsound.h"
#include "g_sdlinput.h"
#include "g_main.h"
#include "gens.h"
#include "g_md.h"
#include "mem_m68k.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "misc.h"
#include "blit.h"
#include "scrshot.h"
#include "ui_proxy.h"
//#include "net.h"
void Sleep(int i);
#include "cdda_mp3.h"


SDL_Surface *screen = NULL;


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

int sdl_flags=SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE|SDL_ASYNCBLIT|SDL_HWACCEL;


unsigned short * filter_buffer=0;

GLuint textures[2]={0,0};

int Texture_size=256;
int Nonpow2tex = 0 ;
int row_length;
int gl_linear_filter=1;
int shift = 0;
float h_stretch = 0;
float v_stretch = 0;

int Dep = 0,Old_Dep=0;

static char Info_String[1024] = "";
static int Message_Showed = 0;
static unsigned int Info_Time = 0;

void (*Blit_FS)(unsigned char *Dest, int pitch, int x, int y, int offset);
void (*Blit_W)(unsigned char *Dest, int pitch, int x, int y, int offset);
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


int Init_draw_gl(int w, int h)
{
	screen = SDL_SetVideoMode(w,h,Bpp,sdl_flags|SDL_OPENGL|(Full_Screen?SDL_FULLSCREEN:0));
	
	
	
	if ( screen == NULL)
	{
		fprintf(stderr, "Error creating SDL primary surface : %s\n", SDL_GetError());
		exit(0);
	}

	if (Render_Mode == NORMAL) {
		row_length=320;
		Texture_size=256;
	} else {
		row_length=640;
		Texture_size=512;
	}

	filter_buffer = (unsigned short *) malloc(
			row_length*Texture_size*sizeof(unsigned short) );
	
	//filter_buffer=(unsigned short *) malloc(row_length *row_length*0.75*sizeof(unsigned short) );
	
	glViewport(0, 0, screen->w,screen->h);


	
	glEnable(GL_TEXTURE_2D);
		
	#ifdef GL_TEXTURE_RECTANGLE_NV
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	
	if(glIsEnabled(GL_TEXTURE_RECTANGLE_NV))
	Nonpow2tex = 1;
	#endif
	
	
	glGenTextures(2,textures);
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);	
	
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 6 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE,0 );
	
	
	return 1;
	
}

static gchar *WindowID = NULL;

int Init_draw_sdl(int w, int h)
{
	screen = SDL_SetVideoMode(w,h,16,sdl_flags|(Full_Screen?SDL_FULLSCREEN:0));
	
	if ( screen==NULL)
	{
		fprintf(stderr, "Error creating SDL primary surface : %s\n", SDL_GetError());
		exit(0);
	}
	
	return 1;
}	

int Init_DDraw()
{	
	int x;
	int w, h;
	
	if (Opengl) {
		w = Width_gl;
		h = Height_gl;
	} else {
		if(Render_Mode==1) //1 Equals normal render--> 320*240
		{
			w=320;
		}
		else
		{
			w=640;
		}
		h = w * 0.75; 	/*640*0.75 = 480 , 320*0.75 = 240*/
	}
	
	if (Full_Screen) {
		UI_Hide_Embedded_Window();
		
		if (WindowID) {
			unsetenv("SDL_WINDOWID");
			g_free(WindowID);
			WindowID = NULL;
		}
	} else {
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
	
	if(Opengl) {
	x=Init_draw_gl(w, h);
	
	}
	else
	x=Init_draw_sdl(w, h);
	
	if (Full_Screen )
	{
		SDL_ShowCursor(SDL_DISABLE);
	}
	Adjust_Stretch();
	
	shift=(Render_Mode!=1);
	
	return x;
}


void End_draw_gl()

{
	if (filter_buffer) {
		glDeleteTextures(2, textures);
		free(filter_buffer);
		filter_buffer=NULL;
	}
	
}

void End_draw_sdl()

{
	
}

void End_DDraw()
{	
	if(Opengl)
	End_draw_gl();
	
	else
	End_draw_sdl();
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Clear_Screen()
{
	
	
	if(Opengl)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		memset(filter_buffer,0,640*480*2); //memset(filter_buffer,0,row_length*row_length*1.5); //row_length*row_length*1.5 size in bytes 
	}
	
	else
	{
		SDL_LockSurface(screen);
	
		SDL_FillRect(screen,0,0);
	
		SDL_UnlockSurface(screen);
	
	}
	
}

void Flip_gl()
{
	   
	
	if (Full_Screen)		
	{	Blit_FS((unsigned char *) filter_buffer + (((row_length*2) * ((240 - VDP_Num_Vis_Lines) >> 1) + Dep) << shift ), row_length*2, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);
	}
	else
	{	Blit_W((unsigned char *) filter_buffer + (((row_length*2) * ((240 - VDP_Num_Vis_Lines) >> 1) + Dep) << shift), row_length*2, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);		    
	}
		
	#ifdef GL_TEXTURE_RECTANGLE_NV
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, textures[0]);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);
	
	
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_WRAP_T,GL_CLAMP);
	
	
	glPixelStorei(GL_UNPACK_ROW_LENGTH,row_length);		//Opengl needs to know the width of the texture data
	
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	
	
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, textures[0]);
		
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV,
	0,3,row_length,row_length * 0.75f,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,filter_buffer);

	glBindTexture(GL_TEXTURE_RECTANGLE_NV, textures[0]);
	
	glBegin(GL_QUADS);			

		glTexCoord2f(0.0f + h_stretch, v_stretch );	// Upleft corner of thetexture
		glVertex2f(-1.0f, 1.0f);	// Upleft vertex of the quad


		glTexCoord2f(row_length - h_stretch, v_stretch);// UpRight corner of the texture 
		glVertex2f( 1.0f, 1.0f);		// UpRight vertex of the quad		


		glTexCoord2f(row_length - h_stretch,row_length*0.75f - v_stretch);// DownRight corner of the texture  
		glVertex2f( 1.0f , -1.0f);		// DownRight vertex of the quad


		glTexCoord2f(0.0f + h_stretch, row_length*0.75f - v_stretch);	// DownLeft corner of the first texture 
		glVertex2f(-1.0f,  -1.0f);	// DownLeft vertex of the quad
	
	glEnd();
	
	
	#else

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	
	gl_linear_filter ? glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR):
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	
	
	glPixelStorei(GL_UNPACK_ROW_LENGTH,row_length);		//Opengl needs to know the width of the texture data
	
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	
	glBindTexture(GL_TEXTURE_2D, textures[0]);
		
	glTexImage2D(GL_TEXTURE_2D,
	0,3,Texture_size*2,Texture_size,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,filter_buffer);


	glBindTexture(GL_TEXTURE_2D, textures[0]);
	
	glBegin(GL_QUADS);			

		glTexCoord2f(0.0f + h_stretch, v_stretch );	// Upleft corner of thetexture
	glVertex2f(-1.0f, 1.0f);	// Upleft vertex of the quad


		glTexCoord2f(0.625f - h_stretch, v_stretch );// UpRight corner of the texture 0.625 == 256/320 ,0.625 == 512/640
	glVertex2f( 1, 1.0f);		// UpRight vertex of the quad		


		glTexCoord2f(0.625f - h_stretch, 0.9375f - v_stretch );// DownRight corner of the texture  0.9375 == 256/240 .  0.9375 == 512/480
	glVertex2f( 1 , -1.0f);		// DownRight vertex of the quad


		glTexCoord2f(0.0f + h_stretch, 0.9375f - v_stretch );	// DownLeft corner of the first texture 
	glVertex2f(-1.0f,  -1.0f);	// DownLeft vertex of the quad
	
	glEnd();
	#endif
	
	/*
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	
	gl_linear_filter ? glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR):
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	
	
	glPixelStorei(GL_UNPACK_ROW_LENGTH,row_length);		//Opengl needs to know the width of the texture data
	
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	
	
	glBindTexture(GL_TEXTURE_2D, textures[0]);
		
	glTexImage2D(GL_TEXTURE_2D,
	0,3,Texture_size,Texture_size,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,filter_buffer);

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	
	glBegin(GL_QUADS);			

		glTexCoord2f(0.0f  , 0.0f);	// Upleft corner of thetexture
		glVertex2f(-1.0f, 1.0f);	// Upleft vertex of the quad


		glTexCoord2f(1.0f , 0);// UpRight corner of the texture 0.625 == 256/320 ,0.625 == 512/640
		glVertex2f( 0.6f, 1.0f);		// UpRight vertex of the quad		


		glTexCoord2f(1.0f, 1.0f);// DownRight corner of the texture  0.9375 == 256/240 .  0.9375 == 512/480
		glVertex2f( 0.6f, -1.0f);		// DownRight vertex of the quad


		glTexCoord2f(0.0f , 1.0f);	// DownLeft corner of the first texture 
		glVertex2f(-1.0f,  -1.0f);	// DownLeft vertex of the quad
	
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	
	gl_linear_filter ? glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR):
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	
	
	glPixelStorei(GL_UNPACK_ROW_LENGTH,row_length);		//Opengl needs to know the width of the texture data
	
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	
	
	
	glTexImage2D(GL_TEXTURE_2D,
	0,3,Texture_size/4,Texture_size,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,filter_buffer+256);

	glBindTexture(GL_TEXTURE_2D, textures[1]);
	
	glBegin(GL_QUADS);			

		glTexCoord2f(0.0f  , 0.0f);	// Upleft corner of thetexture
		glVertex2f(2.0f-1.4f, 1.0f);	// Upleft vertex of the quad


		glTexCoord2f(1.0f , 0);// UpRight corner of the texture 0.625 == 256/320 ,0.625 == 512/640
		glVertex2f( 1.0f, 1.0f);		// UpRight vertex of the quad		


		glTexCoord2f(1.0f, 1.0f);// DownRight corner of the texture  0.9375 == 256/240 .  0.9375 == 512/480
		glVertex2f( 1.0f , -1.0f);		// DownRight vertex of the quad


		glTexCoord2f(0.0f , 1.0f);	// DownLeft corner of the first texture 
		glVertex2f(2.0f-1.4f,  -1.0f);	// DownLeft vertex of the quad
	
	glEnd();
	*/
	
	SDL_GL_SwapBuffers();

	
}

void Flip_sdl()
{
	SDL_LockSurface(screen);
	
	if (Full_Screen)		
	{	Blit_FS((unsigned char *) screen->pixels + (((screen->w*2) * ((240 - VDP_Num_Vis_Lines) >> 1) + Dep) << shift ), screen->w*2, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);
	}
	else
	{	Blit_W((unsigned char *) screen->pixels + (((screen->w*2) * ((240 - VDP_Num_Vis_Lines) >> 1) + Dep) << shift), screen->w*2, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * 2);		    
	}
	
	
	SDL_UnlockSurface(screen);
	
	SDL_Flip(screen);
	
}

int Flip(void)
{
	//float Ratio_X, Ratio_Y;
	int i;
	static float FPS = 0.0f, frames[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	static unsigned int old_time = 0, view_fps = 0, index_fps = 0, freq_cpu[2] = {0, 0};
	unsigned int new_time[2];
	
	

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

	if (Fast_Blur) Half_Blur();

	Old_Dep=Dep;
	
	if ((VDP_Reg.Set4 & 0x1) || (Debug))
	{
		Dep = 0;
	}
	else
	{
		Dep = 64;
	}
	if(Dep!=Old_Dep){
	Adjust_Stretch();
		
	
	if(Dep==64 && Old_Dep==0)
	Clear_Screen();
	
	
	}
	
	
	if(Opengl)
	Flip_gl();
	
	else
	Flip_sdl();
	
	/*if(W_VSync || FS_VSync) vsync();*/
	
	return 1;
}


int Update_Gens_Logo(void)
{

	int i, j, m, n;
	static short tab[64000], Init = 0;
	static float renv = 0, /*ang = 0,*/ zoom_x = 0, zoom_y = 0, pas;
	unsigned short c;

	if (!Init)
	{
		SDL_Surface* Logo;

		Logo = SDL_LoadBMP(DATADIR "/gens_big.bmp");
		
		SDL_LockSurface(Logo);
		memcpy(tab, Logo->pixels, 64000);
		SDL_UnlockSurface(Logo);

		pas = 0.05;
		Init = 1;
	}

	renv += pas;
	zoom_x = sin(renv);
	if (zoom_x == 0.0) zoom_x = 0.0000001;
	zoom_x = (1 / zoom_x) * 1;
	zoom_y = 1;

	if (VDP_Reg.Set4 & 0x1)
	{
		for(j = 0; j < 240; j++)
		{
			for(i = 0; i < 320; i++)
			{
				m = (float)(i - 160) * zoom_x;
				n = (float)(j - 120) * zoom_y;

				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5)) MD_Screen[TAB336[j] + i + 8] = c;
				}
			}
		}
	}
	else
	{
		for(j = 0; j < 240; j++)
		{
			for(i = 0; i < 256; i++)
			{
				m = (float)(i - 128) * zoom_x;
				n = (float)(j - 120) * zoom_y;

				if ((m < 130) && (m >= -130) && (n < 90) && (n >= -90))
				{
					c = tab[m + 130 + (n + 90) * 260];
					if ((c > 31) || (c < 5)) MD_Screen[TAB336[j] + i + 8] = c;
				}
			}
		}
	}

	Half_Blur();
	Flip();

	return 1;
}


int Update_Crazy_Effect(void)
{
	int i, j, offset;
	int r = 0, v = 0, b = 0, prev_l, prev_p;
	int RB, G;

 	for(offset = 336 * 240, j = 0; j < 240; j++)
	{
		for(i = 0; i < 336; i++, offset--)
		{
			prev_l = MD_Screen[offset - 336];
			prev_p = MD_Screen[offset - 1];

			if (Mode_555 & 1)
			{
				RB = ((prev_l & 0x7C1F) + (prev_p & 0x7C1F)) >> 1;
				G = ((prev_l & 0x03E0) + (prev_p & 0x03E0)) >> 1;

				if (Effect_Color & 0x4)
				{
					r = RB & 0x7C00;
					if (rand() > 0x2C00) r += 0x0400;
					else r -= 0x0400;
					if (r > 0x7C00) r = 0x7C00;
					else if (r < 0x0400) r = 0;
				}

				if (Effect_Color & 0x2)
				{
					v = G & 0x03E0;
					if (rand() > 0x2C00) v += 0x0020;
					else v -= 0x0020;
					if (v > 0x03E0) v = 0x03E0;
					else if (v < 0x0020) v = 0;
				}

				if (Effect_Color & 0x1)
				{
					b = RB & 0x001F;
					if (rand() > 0x2C00) b++;
					else b--;
					if (b > 0x1F) b = 0x1F;
					else if (b < 0) b = 0;
				}
			}
			else
			{
				RB = ((prev_l & 0xF81F) + (prev_p & 0xF81F)) >> 1;
				G = ((prev_l & 0x07C0) + (prev_p & 0x07C0)) >> 1;

				if (Effect_Color & 0x4)
				{
					r = RB & 0xF800;
					if (rand() > 0x2C00) r += 0x0800;
					else r -= 0x0800;
					if (r > 0xF800) r = 0xF800;
					else if (r < 0x0800) r = 0;
				}

				if (Effect_Color & 0x2)
				{
					v = G & 0x07C0;
					if (rand() > 0x2C00) v += 0x0040;
					else v -= 0x0040;
					if (v > 0x07C0) v = 0x07C0;
					else if (v < 0x0040) v = 0;
				}

				if (Effect_Color & 0x1)
				{
					b = RB & 0x001F;
					if (rand() > 0x2C00) b++;
					else b--;
					if (b > 0x1F) b = 0x1F;
					else if (b < 0) b = 0;
				}
			}

			MD_Screen[offset] = r + v + b;
		}
	}

	Flip();

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


int Update_Emulation_Netplay(int player, int num_player)
{
#if 0
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
#endif
	return 1;
}


int Eff_Screen(void)
{
	int i;

	for(i = 0; i < 336 * 240; i++) MD_Screen[i] = 0;

	return 1;
}


int Pause_Screen(void)
{
	int i, j, offset;
	int r, v, b, nr, nv, nb;

	r = v = b = nr = nv = nb = 0;

	for(offset = j = 0; j < 240; j++)
	{
		for(i = 0; i < 336; i++, offset++)
		{
			if (Mode_555 & 1)
			{
				r = (MD_Screen[offset] & 0x7C00) >> 10;
				v = (MD_Screen[offset] & 0x03E0) >> 5;
				b = (MD_Screen[offset] & 0x001F);
			}
			else
			{
				r = (MD_Screen[offset] & 0xF800) >> 11;
				v = (MD_Screen[offset] & 0x07C0) >> 6;
				b = (MD_Screen[offset] & 0x001F);
			}

			nr = nv = nb = (r + v + b) / 3;
			
			if ((nb <<= 1) > 31) nb = 31;

			nr &= 0x1E;
			nv &= 0x1E;
			nb &= 0x1E;

			if (Mode_555 & 1)
				MD_Screen[offset] = (nr << 10) + (nv << 5) + nb;
			else
				MD_Screen[offset] = (nr << 11) + (nv << 6) + nb;
		}
	}

	return 1;
}


int Show_Genesis_Screen(void)
{
	Do_VDP_Only();
	Flip();

	return 1;
}


int Take_Shot()
{
	if(Opengl)
	Save_Shot((unsigned char *) filter_buffer, Mode_555 & 1, row_length, row_length*0.75, row_length*2);
	
	else
	Save_Shot((unsigned char *) screen->pixels, Mode_555 & 1, screen->w, screen->h, screen->w*2);
	
  	return 0;
}

void Refresh_video()
{
	End_DDraw();
	
	Init_DDraw();
	 
	Adjust_Stretch(); 
}


/**
 * Set_GL_Resolution(): Set the OpenGL resolution.
 * @param w Width.
 * @param h Height.
 */
void Set_GL_Resolution(int w,int h)
{
	if (Width_gl == w && Height_gl == h)
		return;
	
	// OpenGL resolution has changed.
	Width_gl = w;
	Height_gl = h;
	
	// Print the resolution information.
	MESSAGE_NUM_2L("Selected %dx%d resolution",
		       "Selected %dx%d resolution", w, h, 1500);
	
	// If OpenGL mode isn't enabled, don't do anything.
	if (!Opengl)
		return;
	
	// OpenGL mode is currently enabled. Change the resolution.
	End_DDraw();
	Init_DDraw();
	Set_Render(Full_Screen, Render_Mode, 0);
	Refresh_video();
}

void Set_Bpp(int newbpp)
{
	if(Bpp!=newbpp)
	{
		Bpp=newbpp;
		
		End_DDraw();
		
		Init_DDraw();
	}
	
}


/**
 * Change_OpenGL(): Change the OpenGL setting.
 * @param stretch 0 to turn OpenGL off; 1 to turn OpenGL on.
 */
void Change_OpenGL(int newOpenGL)
{
	End_DDraw();
	Opengl = (newOpenGL == 1 ? 1 : 0);
	Init_DDraw();
	
	if (Opengl)
		MESSAGE_L("Selected Opengl Renderer", "Selected Opengl Renderer", 1500)
	else
		MESSAGE_L("Selected SDL Renderer", "Selected SDL Renderer", 1500)
}

/*
void vsync(){
if(Opengl){
	
	
}	
else{
	
}
	
}
*/

void Adjust_Stretch(){
	
	if(Opengl){
	
		if(Stretch){
		
			if(Nonpow2tex)
			{
				v_stretch = ( (240 - VDP_Num_Vis_Lines)/2.0f) *  row_length*0.75  * 	1/240;
				h_stretch = Dep * 32.0f/64.0f * row_length/320;
			}	
		
			else	
			{
				v_stretch = (240 - VDP_Num_Vis_Lines)/240.0f/2;
				h_stretch = Dep * 0.0625f/64.0;	
			}	
		}
	
		else{
		v_stretch = 0;
		h_stretch = 0;	
		}
	}
		
}

/***************************************************************************
 * Gens: OpenGL Renderer.                                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include <GL/gl.h>
#include <GL/glext.h>

#include "emulator/g_main.h"
#include "g_sdldraw.h"
#include "gens_core/vdp/vdp_io.h"

// Needed to synchronize the Graphics menu after a GL resolution change.
#include "gens/gens_window_sync.h"

GLuint textures[2] = {0, 0};

unsigned short *filter_buffer = NULL;
int Texture_size = 256;
int Nonpow2tex = 0 ;
int row_length;
int gl_linear_filter = 1;

// Stretch parameters
float h_stretch = 0;
float v_stretch = 0;

int Init_Draw_GL(int w, int h)
{
	screen = SDL_SetVideoMode(w, h, bpp, Gens_SDL_Flags | SDL_OPENGL | (Video.Full_Screen ? SDL_FULLSCREEN : 0));
	
	if (screen == NULL)
	{
		fprintf(stderr, "Error creating SDL primary surface : %s\n", SDL_GetError());
		exit(0);
	}
	
	if (Video.Render_Mode == 0)
	{
		row_length=320;
		Texture_size=256;
	}
	else
	{
		row_length=640;
		Texture_size=512;
	}
	
	filter_buffer = (unsigned short*)malloc(
			 	row_length * Texture_size * sizeof(unsigned short));
	
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
	
	// TODO: 32-bit OpenGL color support.
	
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 6 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE,0 );
	
	
	return 1;
	
}


void End_Draw_GL(void)
{
	if (filter_buffer)
	{
		glDeleteTextures(2, textures);
		free(filter_buffer);
		filter_buffer=NULL;
	}
}


/**
 * Set_GL_Resolution(): Set the OpenGL resolution.
 * @param w Width.
 * @param h Height.
 */
void Set_GL_Resolution(int w, int h)
{
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
	End_DDraw();
	Init_DDraw();
	Set_Render(Video.Full_Screen, Video.Render_Mode, 0);
	Refresh_Video();
}


/**
 * Change_OpenGL(): Change the OpenGL setting.
 * @param stretch 0 to turn OpenGL off; 1 to turn OpenGL on.
 */
void Change_OpenGL(int newOpenGL)
{
	End_DDraw();
	Video.OpenGL = (newOpenGL == 1 ? 1 : 0);
	Init_DDraw();
	
	if (Video.OpenGL)
	{
		MESSAGE_L("Selected OpenGL Renderer", "Selected OpenGL Renderer", 1500);
	}
	else
	{
		MESSAGE_L("Selected SDL Renderer", "Selected SDL Renderer", 1500);
	}
}


void Adjust_Stretch_GL(void)
{
	if (!Video.OpenGL)
		return;
		
	if (Stretch)
	{
		if(Nonpow2tex)
		{
			v_stretch = ((240 - VDP_Num_Vis_Lines) / 2.0f) * row_length * 0.75 * (1.0/240.0);
			h_stretch = Dep * 32.0f/64.0f * row_length/320.0;
		}
		else	
		{
			v_stretch = (240 - VDP_Num_Vis_Lines) / 240.0f / 2.0;
			h_stretch = Dep * 0.0625f / 64.0;
		}
	}
	else
	{
		v_stretch = 0;
		h_stretch = 0;
	}
}


/**
 * Clear_Screen_GL(): Clears the OpenGL screen.
 */
void Clear_Screen_GL(void)
{
	if (!Video.OpenGL)
		return;
	
	glClear(GL_COLOR_BUFFER_BIT);
	memset(filter_buffer,0,640*480*2); //memset(filter_buffer,0,row_length*row_length*1.5); //row_length*row_length*1.5 size in bytes 
}


/**
 * Flip_GL(): Flip the OpenGL buffer.
 */
void Flip_GL(void)
{
	// TODO: Add border drawing, like in Flip_SDL().   
	unsigned char bytespp = (bpp == 15 ? 2 : bpp / 8);
	
	if (Video.Full_Screen)
	{
		Blit_FS((unsigned char *) filter_buffer + (((row_length*2) * ((240 - VDP_Num_Vis_Lines) >> 1) + Dep) << shift ), row_length*bytespp, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * bytespp);
	}
	else
	{
		Blit_W((unsigned char *) filter_buffer + (((row_length*2) * ((240 - VDP_Num_Vis_Lines) >> 1) + Dep) << shift), row_length*bytespp, 320 - Dep, VDP_Num_Vis_Lines, 32 + Dep * bytespp);
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
	
	gl_linear_filter ? glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR)
			 : glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	
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

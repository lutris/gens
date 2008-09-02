/**
 * Gens: Video Drawing class - SDL + OpenGL
 */ 

#include "v_draw_sdl_gl.hpp"

#include <string.h>
#include <gdk/gdkx.h>

#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

#include "emulator/g_main.hpp"
#include "ui/ui-common.h"
#include "ui/gtk/gtk-misc.h"
#include "gens/gens_window.h"

VDraw_SDL_GL::VDraw_SDL_GL()
{
	// Set the default GL values.
	textures[0] = 0;
	textures[1] = 0;
	textureSize = 256;
	nonpow2tex = 0;
	glLinearFilter = 1;
	filterBuffer = NULL;
	filterBufferSize = 0;
}

VDraw_SDL_GL::VDraw_SDL_GL(VDraw *oldDraw)
	: VDraw(oldDraw)
{
	// Set the default GL values.
	textures[0] = 0;
	textures[1] = 0;
	textureSize = 256;
	nonpow2tex = 0;
	glLinearFilter = 1;
	filterBuffer = NULL;
	filterBufferSize = 0;
}

VDraw_SDL_GL::~VDraw_SDL_GL()
{
}

/**
 * Init_Video(): Initialize the video subsystem.
 * @return 1 on success.
 */
int VDraw_SDL_GL::Init_Video(void)
{
	int x;
	int w, h;
	
	// OpenGL width/height.
	// TODO: Move these values here or something.
	w = Video.Width_GL;
	h = Video.Height_GL;
	
	if (Video.Full_Screen)
	{
		// Hide the embedded SDL window.
		gtk_widget_hide(lookup_widget(gens_window, "sdlsock"));
		
		if (strlen(SDL_WindowID) == 0)
		{
			unsetenv("SDL_WINDOWID");
			strcpy(SDL_WindowID, "");
		}
	}
	else
	{
		// Show the embedded SDL window.
		GtkWidget *sdlsock = lookup_widget(gens_window, "sdlsock");
		gtk_widget_set_size_request(sdlsock, w, h);
		gtk_widget_realize(sdlsock);
		gtk_widget_show(sdlsock);
		
		// Wait for GTK+ to catch up.
		// TODO: If gtk_main_iteration_do() returns TRUE, exit the program.
		while (gtk_events_pending())
			gtk_main_iteration_do(FALSE);
		
		if (strlen(SDL_WindowID) == 0)
		{
			// Get the Window ID of the SDL socket.
			sprintf(SDL_WindowID, "%d", (int)(GDK_WINDOW_XWINDOW(sdlsock->window)));
			setenv("SDL_WINDOWID", SDL_WindowID, 1);
		}
	}
	
	// Initialize SDL.
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		// Error initializing SDL.
		Init_Fail("Couldn't initialize embedded SDL.");
		return 0;
	}
	
	// Initialize the renderer.
	x = Init_SDL_GL_Renderer(w, h);
	
	// Disable the cursor in fullscreen mode.
	SDL_ShowCursor(Video.Full_Screen ? SDL_DISABLE : SDL_ENABLE);
	
	// Adjust stretch parameters.
	stretchAdjustInternal();
	
	// If normal rendering mode is set, disable the video shift.
	m_shift = (Video.Render_Mode != 0);
	
	// Return the status code from Init_SDL_GL_Renderer().
	return x;
}


/**
 * Init_SDL_GL_Renderer(): Initialize the SDL + OpenGL renderer.
 * @param w Width of the screen.
 * @param h Height of the screen.
 * @return 1 on success; 0 on error.
 */
int VDraw_SDL_GL::Init_SDL_GL_Renderer(int w, int h)
{
	screen = SDL_SetVideoMode(w, h, bpp, SDL_GL_Flags | (Video.Full_Screen ? SDL_FULLSCREEN : 0));
	
	if (!screen)
	{
		// Error initializing SDL.
		fprintf(stderr, "Error creating SDL primary surface: %s\n", SDL_GetError());
		exit(0);
	}
	
	if (Video.Render_Mode == 0)
	{
		// 1x rendering.
		rowLength = 320;
		textureSize = 256;
	}
	else
	{
		// 2x rendering.
		rowLength = 640;
		textureSize = 512;
	}
	
	int bytespp = (bpp == 15 ? 2 : bpp / 8);
	filterBufferSize = rowLength * textureSize * bytespp;
	filterBuffer = (unsigned char*)malloc(filterBufferSize);
	
	glViewport(0, 0, screen->w, screen->h);
	glEnable(GL_TEXTURE_2D);
	
#ifdef GL_TEXTURE_RECTANGLE_NV
	// nVidia rectangular texture extension
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	if (glIsEnabled(GL_TEXTURE_RECTANGLE_NV))
		nonpow2tex = 1;
#endif
	
	glGenTextures(2, textures);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	if (bpp == 15)
	{
		// 15-bit color. (Mode 555)
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 15);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   5);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  0);
	}
	else if (bpp == 16)
	{
		// 16-bit color. (Mode 565)
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   5);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  0);
	}
	else //if (bpp == 32)
	{
		// 32-bit color.
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  0);
	}
	
	return 1;
}


/**
 * End_Video(): Closes the SDL + OpenGL renderer.
 */
void VDraw_SDL_GL::End_Video(void)
{
	if (filterBuffer)
	{
		// Delete the GL textures and filter buffer.
		glDeleteTextures(2, textures);
		free(filterBuffer);
		filterBuffer = NULL;
	}
}


/**
 * stretchAdjustInternal(): Adjust stretch parameters.
 * Called by either VDraw or another function in VDraw_SDL_GL.
 */
void VDraw_SDL_GL::stretchAdjustInternal(void)
{
	if (!stretch())
	{
		// Stretch is disabled.
		m_VStretch = 0;
		m_HStretch = 0;
		return;
	}
	
	// Stretch is enabled.
	if (nonpow2tex)
	{
		// Non Power of 2 textures are available.
		m_VStretch = ((240 - VDP_Num_Vis_Lines) / 2.0f) * rowLength * 0.75f * (1.0f / 240.0f);
		m_HStretch = m_HBorder * (32.0f / 64.0f) * (rowLength / 320.0f);
	}
	else
	{
		// Non Power of 2 textures are not available.
		m_VStretch = (((240 - VDP_Num_Vis_Lines) / 240.0f) / 2.0);
		m_HStretch = ((m_HBorder * 0.0625f) / 64.0f);
	}
}


/**
 * Clear_Screen(): Clears the screen.
 */
void VDraw_SDL_GL::Clear_Screen(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	memset(filterBuffer, 0x00, filterBufferSize);
}


/**
 * Flip_internal(): Flip the screen buffer. (Called by v_draw.)
 * @return 1 on success; 0 on error.
 */
int VDraw_SDL_GL::flipInternal(void)
{
	// TODO: Add border drawing, like in v_draw_sdl.
	
	unsigned char bytespp = (bpp == 15 ? 2 : bpp / 8);
	
	// Start of the SDL framebuffer.
	int pitch = rowLength * bytespp;
	int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
	int HBorder = m_HBorder * (bytespp / 2);	// Left border width, in pixels.
	
	int startPos = ((pitch * VBorder) + HBorder) << m_shift;  // Starting position from within the screen.
	
	// Start of the SDL framebuffer.
	unsigned char *start = &(((unsigned char*)(filterBuffer))[startPos]);
	
	if (Video.Full_Screen)
	{
		Blit_FS(start, pitch, 320 - m_HBorder, VDP_Num_Vis_Lines, 32 + (m_HBorder * 2));
	}
	else
	{
		Blit_W(start, pitch, 320 - m_HBorder, VDP_Num_Vis_Lines, 32 + (m_HBorder * 2));
	}
	
	// Determine the pixel type and pixel format based on the bpp setting.
	unsigned int pixelType, pixelFormat;
	if (bpp == 15)
	{
		pixelType = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		pixelFormat = GL_BGRA;
	}
	else if (bpp == 16)
	{
		pixelType = GL_UNSIGNED_SHORT_5_6_5;
		pixelFormat = GL_RGB;
	}
	else //if (bpp == 32)
	{
		pixelType = GL_UNSIGNED_BYTE;
		pixelFormat = GL_BGRA;
	}
	
	// Bind the GL texture.
	
#ifdef GL_TEXTURE_RECTANGLE_NV
	// nVidia rectangular texture extension
	
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// OpenGL needs to know the width of the texture data.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, textures[0]);
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 3, rowLength, rowLength * 0.75f, 0,
		     pixelFormat, pixelType, filterBuffer);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, textures[0]);
	
	// Corners of the rectangle.
	glBegin(GL_QUADS);
	
	glTexCoord2f(0.0f + m_HStretch, m_VStretch);	// Upper-left corner of the texture.
	glVertex2f(-1.0f,  1.0f);			// Upper-left vertex of the quad.
	
	glTexCoord2f(rowLength - m_HStretch, m_VStretch);	// Upper-right corner of the texture.
	glVertex2f( 1.0f,  1.0f);				// Upper-right vertex of the quad.
	
	glTexCoord2f(rowLength - m_HStretch, (rowLength * 0.75f) - m_VStretch);	// Lower-right corner of the texture.
	glVertex2f( 1.0f, -1.0f);						// Lower-right vertex of the quad.
	
	glTexCoord2f(0.0f + m_HStretch, (rowLength * 0.75f) - m_VStretch);	// Lower-left corner of the texture.
	glVertex2f(-1.0f, -1.0f);						// Lower-left vertex of the quad.
	
	glEnd();
#else
	// Regular 2D textures.
	
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	
	if (glLinearFilter)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// OpenGL needs to know the width of the texture data.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, textureSize * 2, textureSize, 0,
		     pixelFormat, pixelType, filterBuffer);
	
	// Corners of the rectangle.
	glBegin(GL_QUADS);
	
	glTexCoord2f(0.0f + m_HStretch, m_VStretch);	// Upper-left corner of the texture.
	glVertex2f(-1.0f,  1.0f);			// Upper-left vertex of the quad.
	
	glTexCoord2f(0.625f - m_HStretch, m_VStretch);	// Upper-right corner of the texture.
	glVertex2f( 1.0f,  1.0f);			// Upper-right vertex of the quad.
	
	// 0.9375 = 256/240; 0.9375 = 512/480
	glTexCoord2f(0.625f - m_HStretch, 0.9375f - m_VStretch);	// Lower-right corner of the texture.
	glVertex2f( 1.0f, -1.0f);					// Lower-right vertex of the quad.
	
	glTexCoord2f(0.0f + m_HStretch, 0.9375f - m_VStretch);	// Lower-left corner of the texture.
	glVertex2f(-1.0f, -1.0f);				// Lower-left corner of the quad.
	
	glEnd();
#endif
	
	// Swap the SDL GL buffers.
	SDL_GL_SwapBuffers();
	
	// TODO: Return appropriate error code.
	return 1;
}


/**
 * Init_Subsystem(): Initialize the OS-specific graphics library.
 * @return 0 on success; non-zero on error.
 */
int VDraw_SDL_GL::Init_Subsystem(void)
{
	if (SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
	{
		fprintf (stderr, SDL_GetError ());
		return -1;
	}
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, SDL_GetError ());
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
	if (SDL_InitSubSystem(SDL_INIT_CDROM) < 0)
	{
		fprintf(stderr, SDL_GetError ());
		return -1;
	}
	
	return 0;
}


/**
 * Shut_Down(): Shut down the graphics subsystem.
 */
int VDraw_SDL_GL::Shut_Down(void)
{
	SDL_Quit();
	return 1;
}

/**
 * Gens: Video Drawing class - SDL + OpenGL
 */ 

#include "v_draw_sdl_gl.hpp"

#include <string.h>
#include <gdk/gdkx.h>

#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "ui/gtk/gtk-misc.h"
#include "gens/gens_window.hpp"

VDraw_SDL_GL::VDraw_SDL_GL()
{
	// Set the default GL values.
	textures[0] = 0;
	textureSize = 256;
	filterBuffer = NULL;
	filterBufferSize = 0;
	
	// TODO: Copy functions to test for VSync from Mesa's glxswapcontrol.c
	/*
	has_OML_sync_control = is_extension_supported("GLX_OML_sync_control");
	has_SGI_swap_control = is_extension_supported("GLX_SGI_swap_control");
	has_MESA_swap_control = is_extension_supported("GLX_MESA_swap_control");
	*/
	set_swap_interval = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI");
	get_swap_interval = (PFNGLXGETSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXGetSwapIntervalSGI");
}

VDraw_SDL_GL::VDraw_SDL_GL(VDraw *oldDraw)
	: VDraw(oldDraw)
{
	// Set the default GL values.
	textures[0] = 0;
	textureSize = 256;
	filterBuffer = NULL;
	filterBufferSize = 0;
	
	// TODO: Copy functions to test for VSync from Mesa's glxswapcontrol.c
	/*
	has_OML_sync_control = is_extension_supported("GLX_OML_sync_control");
	has_SGI_swap_control = is_extension_supported("GLX_SGI_swap_control");
	has_MESA_swap_control = is_extension_supported("GLX_MESA_swap_control");
	*/
	set_swap_interval = (PFNGLXSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI");
	get_swap_interval = (PFNGLXGETSWAPINTERVALMESAPROC)glXGetProcAddressARB((const GLubyte*)"glXGetSwapIntervalSGI");
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
	
	if (m_FullScreen)
	{
		// Hide the embedded SDL window.
		gtk_widget_hide(lookup_widget(gens_window, "sdlsock"));
		
		unsetenv("SDL_WINDOWID");
		strcpy(SDL_WindowID, "");
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
		
		// Get the Window ID of the SDL socket.
		sprintf(SDL_WindowID, "%d", (int)(GDK_WINDOW_XWINDOW(sdlsock->window)));
		setenv("SDL_WINDOWID", SDL_WindowID, 1);
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
	SDL_ShowCursor(m_FullScreen ? SDL_DISABLE : SDL_ENABLE);
	
	// Adjust stretch parameters.
	stretchAdjustInternal();
	
	// If normal rendering mode is set, disable the video shift.
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	m_shift = (rendMode == 0) ? 0 : 1;
	
	// Return the status code from Init_SDL_GL_Renderer().
	return x;
}


/**
 * Init_SDL_GL_Renderer(): Initialize the SDL + OpenGL renderer.
 * @param w Width of the screen.
 * @param h Height of the screen.
 * @param reinitSDL Reinitialize SDL.
 * @return 1 on success; 0 on error.
 */
int VDraw_SDL_GL::Init_SDL_GL_Renderer(int w, int h, bool reinitSDL)
{
	if (reinitSDL)
	{
		screen = SDL_SetVideoMode(w, h, bpp, SDL_GL_Flags | (m_FullScreen ? SDL_FULLSCREEN : 0));
		
		if (!screen)
		{
			// Error initializing SDL.
			fprintf(stderr, "Error creating SDL primary surface: %s\n", SDL_GetError());
			exit(0);
		}
	}
	
	// Update VSync.
	updateVSync(true);
	
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	if (rendMode == 0)
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
	
	// GL Orthographic Projection code imported from Gens/Linux 2.15.4.
	// TODO: Is this stuff really necessary?
	// NOTE: Disabled for now due to garbage problems.
#if 0
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if ((Video.Width_GL * 3 > Video.Height_GL * 4) && Video.Height_GL != 0)
	{
		glOrtho(-((float)Video.Width_GL * 3) / ((float)Video.Height_GL * 4),
			((float)Video.Width_GL * 3) / ((float)Video.Height_GL * 4),
			-1, 1, -1, 1);
	}
	else if ((Video.Width_GL * 3 < Video.Height_GL * 4) && Video.Width_GL != 0)
	{
		glOrtho(-1, 1,
			-((float)Video.Height_GL * 4) / ((float)Video.Width_GL * 3),
			((float)Video.Height_GL * 3) / ((float)Video.Width_GL * 3),
			-1, 1);
	}
	else
	{
		glOrtho(-1, 1, -1, 1, -1, 1);
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#endif
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, textures);
	
	// Enable double buffering.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	// Color depth values.
	if (bpp == 15)
	{
		// 15-bit color. (Mode 555)
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 15);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   5);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  0);
		m_pixelType = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		m_pixelFormat = GL_BGRA;
	}
	else if (bpp == 16)
	{
		// 16-bit color. (Mode 565)
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  6);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   5);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  0);
		m_pixelType = GL_UNSIGNED_SHORT_5_6_5;
		m_pixelFormat = GL_RGB;
	}
	else //if (bpp == 32)
	{
		// 32-bit color.
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  0);
		m_pixelType = GL_UNSIGNED_BYTE;
		m_pixelFormat = GL_BGRA;
	}
	
	// Initialize the texture.
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	clearScreen();
	
	// Set GL clamping parameters.
	// TODO: GL_CLAMP_TO_EDGE or GL_CLAMP?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// GL filters.
	// TODO: Set MAG filter when the linear filter setting is changed.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	// Set the texture format.
	glTexImage2D(GL_TEXTURE_2D, 0, 3, textureSize * 2, textureSize, 0,
		     m_pixelFormat, m_pixelType, NULL);
	
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
		glDeleteTextures(1, textures);
		free(filterBuffer);
		filterBuffer = NULL;
	}
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
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
	m_VStretch = (((240 - VDP_Num_Vis_Lines) / 240.0f) / 2.0);
	m_HStretch = ((m_HBorder * 0.0625f) / 64.0f);
}


/**
 * clearScreen(): Clears the screen.
 */
void VDraw_SDL_GL::clearScreen(void)
{
	// Clear the screen.
	glClear(GL_COLOR_BUFFER_BIT);
	memset(filterBuffer, 0x00, filterBufferSize);
	
	// Reset the border color to make sure it's redrawn.
	m_BorderColor_16B = ~MD_Palette[0];
	m_BorderColor_32B = ~MD_Palette32[0];
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
	
	if (m_FullScreen)
	{
		Blit_FS(start, pitch, 320 - m_HBorder, VDP_Num_Vis_Lines, 32 + (m_HBorder * 2));
	}
	else
	{
		Blit_W(start, pitch, 320 - m_HBorder, VDP_Num_Vis_Lines, 32 + (m_HBorder * 2));
	}
	
	// Draw the message and/or FPS.
	if (m_MsgVisible)
	{
		// Message is visible.
		drawText(filterBuffer, rowLength, rowLength, (rowLength / 4) * 3, m_MsgText.c_str(), m_MsgStyle);
	}
	else if (m_FPSEnabled && (Genesis_Started || _32X_Started || SegaCD_Started) && !Paused)
	{
		// FPS is enabled.
		drawText(filterBuffer, rowLength, rowLength, (rowLength / 4) * 3, m_MsgText.c_str(), m_FPSStyle);
	}
	
	// Set the GL MAG filter.
	// TODO: Only do this when the linear filter setting is changed.
	if (Video.glLinearFilter)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	// OpenGL needs to know the width of the texture data.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
	
	// Set the texture data.
	glTexSubImage2D(GL_TEXTURE_2D, 0,
			0,						// x offset
			((240 - VDP_Num_Vis_Lines) >> 1) << m_shift,	// y offsets
			rowLength,					// width
			((rowLength * 3) / 4) - ((240 - VDP_Num_Vis_Lines) << m_shift),	// height
			m_pixelFormat, m_pixelType,
			filterBuffer + (bytespp * rowLength * ((240 - VDP_Num_Vis_Lines) >> 1) << m_shift));
	
	// Corners of the rectangle.
	glBegin(GL_QUADS);
	
	glTexCoord2f(0.0f + m_HStretch, m_VStretch);	// Upper-left corner of the texture.
	glVertex2i(-1,  1);				// Upper-left vertex of the quad.
	
	glTexCoord2f(0.625f - m_HStretch, m_VStretch);	// Upper-right corner of the texture.
	glVertex2i( 1,  1);				// Upper-right vertex of the quad.
	
	// 0.9375 = 256/240; 0.9375 = 512/480
	glTexCoord2f(0.625f - m_HStretch, 0.9375f - m_VStretch);	// Lower-right corner of the texture.
	glVertex2i( 1, -1);						// Lower-right vertex of the quad.
	
	glTexCoord2f(0.0f + m_HStretch, 0.9375f - m_VStretch);	// Lower-left corner of the texture.
	glVertex2i(-1, -1);					// Lower-left corner of the quad.
	
	glEnd();
	
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
		fprintf (stderr, SDL_GetError());
		return -1;
	}
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr, SDL_GetError());
		return -1;
	}
	
	/* Take it back down now that we know it works. */
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	
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


/**
 * updateRenderer(): Update the renderer.
 */
void VDraw_SDL_GL::updateRenderer(void)
{
	// Check if a resolution switch is needed.
	int rendMode = (m_FullScreen ? Video.Render_FS : Video.Render_W);
	if (rendMode == 0)
	{
		// 1x rendering.
		if (screen->w == Video.Width_GL && screen->h == Video.Height_GL &&
		    rowLength == 320 && textureSize == 256)
		{
			// Already 1x rendering. Simply clear the screen.
			clearScreen();
			return;
		}
	}
	else
	{
		// 2x rendering.
		if (screen->w == Video.Width_GL && screen->h == Video.Height_GL &&
		    rowLength == 640 && textureSize == 512)
		{
			// Already 2x rendering. Simply clear the screen.
			clearScreen();
			return;
		}
	}
	
	// Resolution switch is needed.
	
	// Clear the GL buffers.
	// TODO: Make this a separate function that is also called by End_Video().
	if (filterBuffer)
	{
		// Delete the GL textures and filter buffer.
		glDeleteTextures(2, textures);
		free(filterBuffer);
		filterBuffer = NULL;
	}
	
	// Reinitialize the GL buffers.
	if (screen->w == Video.Width_GL && screen->h == Video.Height_GL)
	{
		// Output resolution is the same. Don't reinitialize SDL.
		Init_SDL_GL_Renderer(Video.Width_GL, Video.Height_GL, false);
	}
	else
	{
		// Output resolution has changed. Reinitialize SDL.
		End_Video();
		Init_Video();
	}
	
	// Clear the screen.
	clearScreen();
	
	// Adjust stretch parameters.
	stretchAdjustInternal();
}


/**
 * updateVSync(): Update VSync value.
 * @param fromInitSDLGL True if being called from Init_SDL_GL_Renderer().
 */
void VDraw_SDL_GL::updateVSync(bool fromInitSDLGL)
{
	// Set the VSync value.
	// TODO: Turning VSync off seems to require a refresh...
	int vsync = (m_FullScreen ? Video.VSync_FS : Video.VSync_W);
	if (set_swap_interval != NULL)
	{
		if (vsync)
			set_swap_interval(1);
		else if (!fromInitSDLGL)
			Refresh_Video();
	}
}

/***************************************************************************
 * Gens: Video Drawing - Base Code.                                        *
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

#include "vdraw.h"

#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MDP Host Services.
#include "mdp/mdp.h"
#include "mdp/mdp_host.h"
#include "plugins/mdp_host_gens.h"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Video Effects.
#include "v_effects.hpp"
#include "gens_core/gfx/fastblur.hpp"
#include "emulator/g_md.hpp"
#include "emulator/g_palette.h"

// Inline video functions.
#include "v_inline.h"

// VDraw C++ functions.
#include "vdraw_cpp.hpp"

// Gens window.
#include "gens/gens_window_sync.hpp"

// Timer functionality.
#ifndef GENS_OS_WIN32
#include "port/timer.h"
#endif /* GENS_OS_WIN32 */


// VDraw backends.
#ifdef GENS_OS_WIN32
	#include "vdraw_ddraw.h"
#else /* !GENS_OS_WIN32 */
	#include "vdraw_sdl.h"
	#ifdef GENS_OPENGL
		#include "vdraw_sdl_gl.h"
	#endif /* GENS_OPENGL */
#endif /* GENS_OS_WIN32 */

static vdraw_backend_t * const vdraw_backends[] =
{
	#ifdef GENS_OS_WIN32
		&vdraw_backend_ddraw,
	#else /* !GENS_OS_WIN32 */
		&vdraw_backend_sdl,
		#ifdef GENS_OPENGL
			&vdraw_backend_sdl_gl,
		#endif /* GENS_OPENGL */
	#endif /* GENS_OS_WIN32 */
};

// Current backend.
vdraw_backend_t	*vdraw_cur_backend = NULL;
VDRAW_BACKEND	vdraw_cur_backend_id;

// Function pointers.
int		(*vdraw_init_subsystem)(void) = NULL;
int		(*vdraw_shutdown)(void) = NULL;
void		(*vdraw_clear_screen)(void) = NULL;
void		(*vdraw_update_vsync)(const int data) = NULL;

// Render functions.
mdp_render_fn vdraw_blitFS;
mdp_render_fn vdraw_blitW;

// Render plugin information.
MDP_Render_Info_t vdraw_rInfo = {.bpp = 0, .renderFlags = 0};

// Properties.
static uint8_t	vdraw_prop_stretch = 0;
static uint8_t	vdraw_prop_intro_effect_color = 0;
static BOOL	vdraw_prop_fullscreen = FALSE;
static BOOL	vdraw_prop_sw_render = FALSE;
static BOOL	vdraw_prop_fast_blur = FALSE;
int		vdraw_scale = 1;

// FPS counter.
BOOL		vdraw_fps_enabled = FALSE;
static float	vdraw_fps_value = 0;
static float	vdraw_fps_frames[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static uint32_t	vdraw_fps_old_time = 0, vdraw_fps_view = 0, vdraw_fps_index = 0;
static uint32_t	vdraw_fps_freq_cpu[2] = {0, 0}, vdraw_fps_new_time[2] = {0, 0};
vdraw_style_t	vdraw_fps_style = {0, 0, 0, FALSE, FALSE};

// On-screen message.
static BOOL	vdraw_msg_enabled = TRUE;
char		vdraw_msg_text[1024];
BOOL		vdraw_msg_visible = FALSE;
static uint32_t	vdraw_msg_time = 0;
vdraw_style_t	vdraw_msg_style = {0, 0, 0, FALSE, FALSE};

// Screen border.
int		vdraw_border_h = 0, vdraw_border_h_old = ~0;
uint16_t	vdraw_border_color_16 = ~0;
uint32_t	vdraw_border_color_32 = ~0;

// 16-bit color to 32-bit color image conversion.
static int	*vdraw_LUT16to32 = NULL;
uint16_t	*vdraw_16to32_surface;
int		vdraw_16to32_scale;
int		vdraw_16to32_pitch;


/**
 * vdraw_init(): Initialize the Video Drawing subsystem.
 * @return 0 on success; non-zero on error.
 */
int vdraw_init(void)
{
	// TODO: Do something here.
	return 0;
}


/**
 * vdraw_init(): Shut down the Video Drawing subsystem.
 * @return 0 on success; non-zero on error.
 */
int vdraw_end(void)
{
	if (vdraw_cur_backend)
	{
		vdraw_cur_backend->end();
		vdraw_cur_backend = NULL;
	}
	
	if (vdraw_LUT16to32)
	{
		// Unreference LUT16to32.
		mdp_host_ptr_unref(MDP_PTR_LUT16to32);
		vdraw_LUT16to32 = NULL;
	}
	
	// Internal surface for rendering the 16-bit temporary image.
	if (vdraw_16to32_surface)
	{
		free(vdraw_16to32_surface);
		vdraw_16to32_surface = NULL;
		vdraw_16to32_scale = 0;
		vdraw_16to32_pitch = 0;
	}
	
	// TODO: Do something here.
	return 0;
}


/**
 * vdraw_backend_init_subsystem(): Initialize a backend's subsystem.
 * @param backend Backend to use.
 * @return 0 on success; non-zero on error.
 */
int vdraw_backend_init_subsystem(VDRAW_BACKEND backend)
{
	if (backend < 0 || backend >= VDRAW_BACKEND_MAX)
	{
		// Invalid backend.
		return -1;
	}
	
	if (vdraw_backends[backend]->init_subsystem)
		vdraw_backends[backend]->init_subsystem();
	
	// Initialized successfully.
	return 0;
}


/**
 * vdraw_backend_init(): Initialize a backend.
 * @param backend Backend to use.
 * @return 0 on success; non-zero on error.
 */
int vdraw_backend_init(VDRAW_BACKEND backend)
{
	if (backend < 0 || backend >= VDRAW_BACKEND_MAX)
	{
		// Invalid backend.
		return -1;
	}
	
	// Set up the variables.
	vdraw_cur_backend = vdraw_backends[backend];
	vdraw_cur_backend_id = backend;
	
	// Copy the function pointers.
	vdraw_init_subsystem = vdraw_cur_backend->init_subsystem;
	vdraw_shutdown = vdraw_cur_backend->shutdown;
	vdraw_clear_screen = vdraw_cur_backend->clear_screen;
	vdraw_update_vsync = vdraw_cur_backend->update_vsync;
	
	// Initialize the backend.
	if (vdraw_cur_backend->init)
		return vdraw_cur_backend->init();
	
	// Initialized successfully.
	return 0;
}


/**
 * vdraw_init_fail(): Called if initialization fails.
 * @param err Error message.
 */
void vdraw_init_fail(const char* err)
{
	if (!err)
		fprintf(stderr, "vdraw initialization failed: %s\n", err);
	else
		fprintf(stderr, "vdraw initialization failed.\n");
	
	exit(0);
}


/**
 * vdraw_backend_end(): Shut down the current backend.
 * @return 0 on success; non-zero on error.
 */
int vdraw_backend_end(void)
{
	if (!vdraw_cur_backend)
		return 1;
	
	vdraw_cur_backend->end();
	vdraw_cur_backend = NULL;
	return 0;
}


/**
 * vdraw_render_16to32(): Convert a 16-bit color image to 32-bit color using a lookup table.
 * @param dest Destination surface.
 * @param src Source surface.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param pitch_dest Pitch of the destination surface.
 * @param pitch_src Pitch of the source surface.
 */
void vdraw_render_16to32(uint32_t *dest, uint16_t *src,
			 int width, int height,
			 int pitch_dest, int pitch_src)
{
	// Make sure the lookup table is referenced.
	if (!vdraw_LUT16to32)
		vdraw_LUT16to32 = (int*)(mdp_host_ptr_ref(MDP_PTR_LUT16to32));
	
	const int pitchDestDiff = ((pitch_dest / 4) - width);
	const int pitchSrcDiff = ((pitch_src / 2) - width);
	
	// Process four pixels at a time.
	width >>= 2;
	
	unsigned int x, y;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			*(dest + 0) = vdraw_LUT16to32[*(src + 0)];
			*(dest + 1) = vdraw_LUT16to32[*(src + 1)];
			*(dest + 2) = vdraw_LUT16to32[*(src + 2)];
			*(dest + 3) = vdraw_LUT16to32[*(src + 3)];
			
			dest += 4;
			src += 4;
		}
		
		dest += pitchDestDiff;
		src += pitchSrcDiff;
	}
}


/**
 * vdraw_flip(): Flip the screen buffer.
 * @return 0 on success; non-zero on error.
 */
int vdraw_flip(void)
{
	// Check if any effects need to be applied.
	// TODO: Make constnats for Intro_Style.
	if (Game != NULL)
	{
		if (Video.pauseTint && (!Active || Paused))
		{
			// Emulation is paused.
			Pause_Screen();
		}
	}
	else if (Intro_Style == 1)
	{
		// Gens logo effect. (TODO: This is broken!)
		Update_Gens_Logo();
	}
	else if (Intro_Style == 2)
	{
		// "Strange" effect. (TODO: This is broken!)
		Update_Crazy_Effect(vdraw_prop_intro_effect_color);
	}
	else if (Intro_Style == 3)
	{
		// Genesis BIOS. (TODO: This is broken!)
		Do_Genesis_Frame();
	}
	else
	{
		// Blank screen.
		Clear_Screen_MD();
	}
	
	if (vdraw_msg_visible)
	{
		if (GetTickCount() > vdraw_msg_time)
		{
			vdraw_msg_visible = FALSE;
			vdraw_msg_text[0] = 0x00;
		}
	}
	else if (vdraw_fps_enabled && (Game != NULL) && !Paused)
	{
		if (vdraw_fps_freq_cpu[0] > 1)	// accurate timer ok
		{
			if (++vdraw_fps_view >= 16)
			{
				#ifdef GENS_OS_WIN32
					QueryPerformanceCounter((LARGE_INTEGER*)vdraw_fps_new_time);
				#else
					QueryPerformanceCounter((long long*)vdraw_fps_new_time);
				#endif
				if (vdraw_fps_new_time[0] != vdraw_fps_old_time)
				{
					vdraw_fps_value = (float)(vdraw_fps_freq_cpu[0]) * 16.0f /
							  (float)(vdraw_fps_new_time[0] - vdraw_fps_old_time);
					sprintf(vdraw_msg_text, "%.1f", vdraw_fps_value);
				}
				else
				{
					// IT'S OVER 9000 FPS!!!111!11!1
					strcpy(vdraw_msg_text, ">9000");
				}
				
				vdraw_fps_old_time = vdraw_fps_new_time[0];
				vdraw_fps_view = 0;
			}
		}
		else if (vdraw_fps_freq_cpu[0] == 1)	// accurate timer not supported
		{
			if (++vdraw_fps_view >= 10)
			{
				vdraw_fps_new_time[0] = GetTickCount();
				
				if (vdraw_fps_new_time[0] != vdraw_fps_old_time)
					vdraw_fps_frames[vdraw_fps_index] = 10000.0f / (float)(vdraw_fps_new_time[0] - vdraw_fps_old_time);
				else
					vdraw_fps_frames[vdraw_fps_index] = 2000;
				
				vdraw_fps_index++;
				vdraw_fps_index &= 7;
				vdraw_fps_value = 0.0f;
				
				unsigned char i;
				for (i = 0; i < 8; i++)
					vdraw_fps_value += vdraw_fps_frames[i];
				
				vdraw_fps_value /= 8.0f;
				vdraw_fps_old_time = vdraw_fps_new_time[0];
				vdraw_fps_view = 0;
			}
			sprintf(vdraw_msg_text, "%.1f", vdraw_fps_value);
		}
		else
		{
			#ifdef GENS_OS_WIN32
				QueryPerformanceFrequency((LARGE_INTEGER*)vdraw_fps_freq_cpu);
			#else
				QueryPerformanceFrequency((long long*)vdraw_fps_freq_cpu);
			#endif
			if (vdraw_fps_freq_cpu[0] == 0)
				vdraw_fps_freq_cpu[0] = 1;
			
			// Clear the message text.
			vdraw_msg_text[0] = 0x00;
		}
	}
	
	// Blur the screen if requested.
	if (vdraw_prop_fast_blur)
		Fast_Blur();
	
	// Check if the display width changed.
	vdraw_border_h_old = vdraw_border_h;
	if (isFullXRes())
		vdraw_border_h = 0;	// 320x224
	else
		vdraw_border_h = 64;	// 256x224
	
	if (vdraw_border_h != vdraw_border_h_old)
	{
		// Display width change. Adjust the stretch parameters.
		if (vdraw_cur_backend->stretch_adjust)
			vdraw_cur_backend->stretch_adjust();
	}
	
	if (vdraw_border_h > vdraw_border_h_old)
	{
		// New screen width is smaller than old screen width.
		// Clear the screen.
		vdraw_cur_backend->clear_screen();
	}
	
	// Flip the screen buffer.
	return vdraw_cur_backend->flip();

}


/**
 * vdraw_set_bpp(): Sets the bpp value.
 * @param new_bpp New bpp value.
 * @param reset_video If true, resets the video subsystem if the bpp needs to be changed.
 */
void vdraw_set_bpp(const int new_bpp, const BOOL reset_video)
{
	if (bppOut == new_bpp)
		return;
	
	bppOut = new_bpp;
	
	if (reset_video && vdraw_cur_backend)
	{
		vdraw_cur_backend->end();
		vdraw_cur_backend->init();
	}
	
	// Reset the renderer.
	vdraw_reset_renderer(reset_video);
	
	// Recalculate palettes.
	Recalculate_Palettes();
	
	// Recalculate the text styles.
	calc_transparency_mask();
	calc_text_style(&vdraw_fps_style);
	calc_text_style(&vdraw_msg_style);
	
	// Synchronize the Graphics menu.
	Sync_Gens_Window_GraphicsMenu();
	
	// TODO: Figure out if 32-bit rendering still occurs in 15/16-bit mode and vice-versa.
}


/**
 * vdraw_write_text(): Write text to the screen.
 * @param msg Message to write.
 * @param duration Duration for the message to appear, in milliseconds.
 */
void vdraw_write_text(const char* msg, const int duration)
{
	if (!vdraw_msg_enabled)
		return;
	
	strncpy(vdraw_msg_text, msg, sizeof(vdraw_msg_text));
	vdraw_msg_text[sizeof(vdraw_msg_text) - 1] = 0x00;
	vdraw_msg_time = GetTickCount() + duration;
	vdraw_msg_visible = TRUE;
}


/**
 * vdraw_refresh_video(): Refresh the video subsystem.
 */
void vdraw_refresh_video(void)
{
	// Reset the border color to make sure it's redrawn.
	vdraw_border_color_16 = ~MD_Palette[0];
	vdraw_border_color_32 = ~MD_Palette32[0];
	
	if (vdraw_cur_backend)
	{
		vdraw_cur_backend->end();
		vdraw_cur_backend->init();
		if (vdraw_cur_backend->stretch_adjust)
			vdraw_cur_backend->stretch_adjust();
	}
}


/** Properties **/


uint8_t vdraw_get_stretch(void)
{
	return vdraw_prop_stretch;
}
void vdraw_set_stretch(const uint8_t new_stretch)
{
	if (vdraw_prop_stretch == new_stretch)
		return;
	
	if (new_stretch > STRETCH_FULL)
	{
		// Invalid stretch value.
		return;
	}
	
	vdraw_prop_stretch = new_stretch;
	if (vdraw_cur_backend && vdraw_cur_backend->stretch_adjust)
		vdraw_cur_backend->stretch_adjust();
}


BOOL vdraw_get_sw_render(void)
{
	return (vdraw_prop_sw_render ? TRUE : FALSE);
}
void vdraw_set_sw_render(const BOOL new_sw_render)
{
	vdraw_prop_sw_render = (new_sw_render ? TRUE : FALSE);
}


BOOL vdraw_get_msg_enabled(void)
{
	return (vdraw_msg_enabled ? TRUE : FALSE);
}
void vdraw_set_msg_enabled(const BOOL new_msg_enable)
{
	vdraw_msg_enabled = (new_msg_enable ? TRUE : FALSE);
}


BOOL vdraw_get_fps_enabled(void)
{
	return (vdraw_fps_enabled ? TRUE : FALSE);
}
void vdraw_set_fps_enabled(const BOOL new_fps_enable)
{
	vdraw_fps_enabled = (new_fps_enable ? TRUE : FALSE);
}


BOOL vdraw_get_fullscreen(void)
{
	return vdraw_prop_fullscreen;
}
void vdraw_set_fullscreen(const BOOL new_fullscreen)
{
	if (vdraw_prop_fullscreen == new_fullscreen)
		return;
	
	vdraw_prop_fullscreen = new_fullscreen;
	
	// Reset the renderer.
	vdraw_reset_renderer(FALSE);
	
	#ifdef GENS_OS_WIN32
		// Reinitialize the Gens window, if necessary.
		if (vdraw_cur_backend && vdraw_cur_backend->reinit_gens_window)
			vdraw_cur_backend->reinit_gens_window();
	#else /* !GENS_OS_WIN32 */
		// Refresh the video subsystem, if Gens is running.
		if (is_gens_running())
			vdraw_refresh_video();
	#endif /* GENS_OS_WIN32 */
}


BOOL vdraw_get_fast_blur(void)
{
	return (vdraw_prop_fast_blur ? TRUE : FALSE);
}
void vdraw_set_fast_blur(const BOOL new_fast_blur)
{
	vdraw_prop_fast_blur = (new_fast_blur ? TRUE : FALSE);
}


/** Style properties **/


uint8_t	vdraw_get_msg_style(void)
{
	return vdraw_msg_style.style;
}
void vdraw_set_msg_style(const uint8_t new_msg_style)
{
	if (vdraw_msg_style.style == new_msg_style)
		return;
	
	vdraw_msg_style.style = new_msg_style;
	calc_text_style(&vdraw_msg_style);
}


uint8_t	vdraw_get_fps_style(void)
{
	return vdraw_fps_style.style;
}
void vdraw_set_fps_style(const uint8_t new_fps_style)
{
	if (vdraw_fps_style.style == new_fps_style)
		return;
	
	vdraw_fps_style.style = new_fps_style;
	calc_text_style(&vdraw_fps_style);
}


uint8_t	vdraw_get_intro_effect_color(void)
{
	return vdraw_prop_intro_effect_color;
}
void vdraw_set_intro_effect_color(const uint8_t new_intro_effect_color)
{
	if (vdraw_prop_intro_effect_color == new_intro_effect_color ||
	    /* new_intro_effect_color < 0 ||*/ new_intro_effect_color > 7)
		return;
	
	vdraw_prop_intro_effect_color = new_intro_effect_color;
	
	// TODO: Figure out what to do here...
}

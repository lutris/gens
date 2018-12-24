/***************************************************************************
 * Gens: Video Drawing - Base Code.                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * Copyright (c) 2009 by Phil Costin                                       *
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

// Message logging.
#include "macros/log_msg.h"

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
#include "emulator/md_palette.hpp"

// VDraw C++ functions.
#include "vdraw_cpp.hpp"

// Gens window.
#include "gens/gens_window_sync.hpp"

// Timer functionality.
#ifndef GENS_OS_WIN32
#include "port/timer.h"
#endif /* !GENS_OS_WIN32 */

// windows.h is needed on Win32 for MessageBox().
#ifdef GENS_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "gens/gens_window.h"
#endif /* GENS_OS_WIN32 */


// VDraw backends.
#if defined(GENS_OS_WIN32)
	#include "vdraw_gdi.h"
	#include "vdraw_ddraw_t.h"
#elif defined(GENS_OS_HAIKU)
	#include "vdraw_haiku_t.h"
	#include "vdraw_haiku.hpp"
#elif defined(GENS_OS_UNIX)
	#include "vdraw_sdl.h"
	#ifdef GENS_OPENGL
		#include "vdraw_sdl_gl.h"
	#endif /* GENS_OPENGL */
#endif

const vdraw_backend_t* const vdraw_backends[] =
{
	#if defined(GENS_OS_WIN32)
		&vdraw_backend_ddraw,
		&vdraw_backend_gdi,
	#elif defined(GENS_OS_HAIKU)
		&vdraw_backend_haiku,
	#elif defined(GENS_OS_UNIX)
		&vdraw_backend_sdl,
		#ifdef GENS_OPENGL
			&vdraw_backend_sdl_gl,
		#endif /* GENS_OPENGL */
	#endif
	NULL
};

// Broken backends array. 0 == works; nonzero == broken.
uint8_t vdraw_backends_broken[VDRAW_BACKEND_MAX] = {0};

// Current backend.
const vdraw_backend_t *vdraw_cur_backend = NULL;
VDRAW_BACKEND vdraw_cur_backend_id = -1;
uint32_t vdraw_cur_backend_flags = 0;

// Function pointers.
int		(*vdraw_init_subsystem)(void) = NULL;
int		(*vdraw_shutdown)(void) = NULL;
void		(*vdraw_clear_screen)(void) = NULL;
void		(*vdraw_update_vsync)(const int data) = NULL;
int		(*vdraw_reinit_gens_window)(void) = NULL;
#ifdef GENS_OS_WIN32
int		(*vdraw_clear_primary_screen)(void) = NULL;
int		(*vdraw_clear_back_screen)(void) = NULL;
int		(*vdraw_restore_primary)(void) = NULL;
int		(*vdraw_set_cooperative_level)(void) = NULL;
#endif /* GENS_OS_WIN32 */

// Render functions.
mdp_render_fn vdraw_blitFS;
mdp_render_fn vdraw_blitW;

// Render plugin information.
mdp_render_info_t vdraw_rInfo = { .vmodeFlags = 0 };

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
vdraw_style_t	vdraw_fps_style;

// On-screen message.
static BOOL	vdraw_msg_enabled = TRUE;
BOOL		vdraw_msg_visible = FALSE;
vdraw_style_t	vdraw_msg_style;

// Screen border.
int		vdraw_border_h = 0, vdraw_border_h_old = ~0;
uint16_t	vdraw_border_color_16 = ~0;
uint32_t	vdraw_border_color_32 = ~0;

// RGB color conversion functions.
#include "vdraw_RGB.h"
BOOL		vdraw_needs_conversion = FALSE;


/**
 * vdraw_init(): Initialize the Video Drawing subsystem.
 * @return 0 on success; non-zero on error.
 */
int vdraw_init(void)
{
	// Calculate the initial text styles.
	memset(&vdraw_fps_style, 0x00, sizeof(vdraw_fps_style));
	memset(&vdraw_msg_style, 0x00, sizeof(vdraw_msg_style));
	
	calc_transparency_mask();
	calc_text_style(&vdraw_fps_style);
	calc_text_style(&vdraw_msg_style);
	
	// Initialized.
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
	if (vdraw_backends_broken[backend])
	{
		// Backend is broken.
		return -2;
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
	
	// Initialize the backend.
	if (vdraw_backends_broken[backend] != 0 ||
	    vdraw_backends[backend]->init() != 0)
	{
		// Error initializing the backend.
		
		// Mark this backend as "broken".
		vdraw_backends_broken[backend] = 1;
		
		// Search for a backend that isn't broken.
		int i;
		for (i = 0; i < VDRAW_BACKEND_MAX; i++)
		{
			if (vdraw_backends_broken[i] == 0)
			{
				// Found a non-broken backend. Try using it.
				return vdraw_backend_init(i);
			}
		}
		
		if (i == VDRAW_BACKEND_MAX)
		{
			// All backends are broken. PANIC!
			LOG_MSG(video, LOG_MSG_LEVEL_CRITICAL,
				"Cannot find a working video backend.");
			exit(1);
		}
	}
	
	// Set up the variables.
	vdraw_cur_backend = vdraw_backends[backend];
	vdraw_cur_backend_id = backend;
	
	// Copy the function pointers.
	vdraw_init_subsystem		= vdraw_cur_backend->init_subsystem;
	vdraw_shutdown			= vdraw_cur_backend->shutdown;
	vdraw_clear_screen		= vdraw_cur_backend->clear_screen;
	vdraw_update_vsync		= vdraw_cur_backend->update_vsync;
	vdraw_reinit_gens_window	= vdraw_cur_backend->reinit_gens_window;
#ifdef GENS_OS_WIN32
	vdraw_clear_primary_screen	= vdraw_cur_backend->clear_primary_screen;
	vdraw_clear_back_screen		= vdraw_cur_backend->clear_back_screen;
	vdraw_restore_primary		= vdraw_cur_backend->restore_primary;
	vdraw_set_cooperative_level	= vdraw_cur_backend->set_cooperative_level;
#endif /* GENS_OS_WIN32 */
	
	// Set the backend flags.
	vdraw_cur_backend_flags		= vdraw_cur_backend->flags;
	
	// If the backend doesn't support fullscreen, force it off.
	if (!(vdraw_cur_backend_flags & VDRAW_BACKEND_FLAG_FULLSCREEN))
		vdraw_prop_fullscreen = FALSE;
	
	// The Gens window must be reinitialized.
	return vdraw_cur_backend->reinit_gens_window();
}


/**
 * vdraw_backend_end(): Shut down the current backend.
 * @return 0 on success; non-zero on error.
 */
int vdraw_backend_end(void)
{
	if (!vdraw_cur_backend)
		return 1;
	
	if (vdraw_cur_backend->end)
		vdraw_cur_backend->end();
	
	vdraw_cur_backend = NULL;
	return 0;
}


/**
 * vdraw_flip(): Flip the screen buffer.
 * @param md_screen_updated Non-zero if the MD screen has been updated.
 * @return 0 on success; non-zero on error.
 */
int vdraw_flip(int md_screen_updated)
{
	if (md_screen_updated)
	{
		// MD screen is updated.
		// Update the FPS counter and onscreen effects.
		
		// Check if any effects need to be applied.
		// TODO: Make constants for Intro_Style.
		if (Game == NULL)
		{
			// Intro effects.
			switch (Intro_Style)
			{
				case 1:
					// Gens Logo effect.
					Update_Gens_Logo();
					break;
				case 2:
					// "Strange" effect.
					Update_Crazy_Effect(vdraw_prop_intro_effect_color);
					break;
				case 3:
					// Genesis BIOS. (TODO: This is broken!)
					Do_Genesis_Frame();
					break;
				default:
					// Blank screen.
					Clear_Screen_MD();
			}
		}
		
		if (vdraw_msg_visible)
		{
			vdraw_msg_timer_update();
		}
		else if (vdraw_fps_enabled && (Game != NULL) && !Settings.Paused)
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
						vdraw_text_printf(0, "%.1f", vdraw_fps_value);
					}
					else
					{
						// IT'S OVER 9000 FPS!!!111!11!1
						vdraw_text_write(">9000", 0);
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
				vdraw_text_printf(0, "%.1f", vdraw_fps_value);
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
	}
	
	// Check if the display width changed.
	vdraw_border_h_old = vdraw_border_h;
	if (vdp_isH40())
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
	
	if (vdraw_cur_backend && !(vdraw_cur_backend->flags & VDRAW_BACKEND_FLAG_FULLSCREEN))
	{
		// Current backend doesn't support fullscreen.
		return;
	}
	
	vdraw_prop_fullscreen = new_fullscreen;
	if (!is_gens_running())
	{
		// Gens isn't running yet.
		// This is probably because Gens has just started up, and is
		// loading the config file or parsing a command-line argument.
		return;
	}
	
	// If fullscreen, the window is definitely active.
	if (vdraw_prop_fullscreen)
		Settings.Active = 1;
	
	// Reset the renderer.
	vdraw_reset_renderer(FALSE);
	
	// Reinitialize the Gens window, if necessary.
	if (vdraw_cur_backend)
		vdraw_cur_backend->reinit_gens_window();
#if !defined(GENS_OS_WIN32)
	// Refresh the video subsystem, if Gens is running.
	if (is_gens_running())
		vdraw_refresh_video();
#endif
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

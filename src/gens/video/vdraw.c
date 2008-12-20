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

// MDP Host Services.
#include "mdp/mdp.h"
#include "mdp/mdp_host.h"
#include "plugins/mdp_host_gens.h"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
static vdraw_backend_t	*vdraw_cur_backend = NULL;
static VDRAW_BACKEND	vdraw_cur_backend_id;

// VDraw styles.
typedef struct
{
	uint8_t		style;
	uint8_t		color;		// STYLE_COLOR_* values
	uint32_t	dot_color;	// Actual RGB color value.
	BOOL		double_size;
	BOOL		transparent;
} vdraw_style_t;

// Function pointers.
int		(*vdraw_init_subsystem)(void);
int		(*vdraw_shutdown)(void);
void		(*vdraw_clear_screen)(void);
void		(*vdraw_update_vsync)(const int data);

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
static int	vdraw_prop_scale = 1;

// Text drawing values.
static uint32_t	vdraw_prop_mask_transparency;

// FPS counter.
static BOOL	vdraw_fps_enabled = FALSE;
static float	vdraw_fps_value = 0;
static float	vdraw_fps_frames[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
static uint32_t	vdraw_fps_old_time = 0, vdraw_fps_view = 0, vdraw_fps_index = 0;
static uint32_t	vdraw_fps_freq[2] = {0, 0}, vdraw_fps_new_time[2] = {0, 0};
static vdraw_style_t	vdraw_fps_style = {0, 0, 0, FALSE, FALSE};

// On-screen message.
static BOOL	vdraw_msg_enabled = TRUE;
static char	vdraw_msg_text[1024];
static BOOL	vdraw_msg_visible = FALSE;
static uint32_t	vdraw_msg_time = 0;
static vdraw_style_t	vdraw_msg_style = {0, 0, 0, FALSE, FALSE};

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
 * vdraw_init(): Initialize video drawing.
 * @param backend Backend to use.
 * @return 0 on success; non-zero on error.
 */
int vdraw_init(VDRAW_BACKEND backend)
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
	vdraw_init_subsystem = vdraw_cur_backend->vdraw_backend_init_subsystem;
	vdraw_shutdown = vdraw_cur_backend->vdraw_backend_shutdown;
	vdraw_clear_screen = vdraw_cur_backend->vdraw_backend_clear_screen;
	vdraw_update_vsync = vdraw_cur_backend->vdraw_backend_update_vsync;
	
	// Initialize the backend.
	if (vdraw_cur_backend->vdraw_backend_init)
		return vdraw_cur_backend->vdraw_backend_init();
	
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
	
	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
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

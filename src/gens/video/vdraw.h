/***************************************************************************
 * Gens: Video Drawing - Base Code.                                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#ifndef GENS_VDRAW_H
#define GENS_VDRAW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "libgsft/gsft_bool.h"

// MDP Render Functions.
#include "mdp/mdp_render.h"

// Stretch mode constants.
#define STRETCH_NONE		0x00
#define STRETCH_H		0x01
#define STRETCH_V		0x02
#define STRETCH_FULL		0x03

// Text drawing functions.
#include "vdraw_text.hpp"


// VDraw backends.
typedef enum
{
	#if defined(GENS_OS_WIN32)
		VDRAW_BACKEND_DDRAW,
		VDRAW_BACKEND_GDI,
	#elif defined(GENS_OS_HAIKU)
		VDRAW_BACKEND_HAIKU,
	#elif defined(GENS_OS_UNIX)
		VDRAW_BACKEND_SDL,
		#ifdef GENS_OPENGL
			VDRAW_BACKEND_SDL_GL,
		#endif /* GENS_OPENGL */
	#endif
	VDRAW_BACKEND_MAX
} VDRAW_BACKEND;

// Broken backends array. 0 == works; nonzero == broken.
extern uint8_t vdraw_backends_broken[VDRAW_BACKEND_MAX];

// VDraw backend flags.
#define VDRAW_BACKEND_FLAG_STRETCH	((uint32_t)(1 << 0))
#define VDRAW_BACKEND_FLAG_VSYNC	((uint32_t)(1 << 1))
#define VDRAW_BACKEND_FLAG_FULLSCREEN	((uint32_t)(1 << 2))
#define VDRAW_BACKEND_FLAG_WINRESIZE	((uint32_t)(1 << 3))

// VDraw backend function pointers.
typedef struct
{
	const char* const name;
	const uint32_t flags;
	
	int	(*init)(void);
	int	(*end)(void);
	
	int	(*init_subsystem)(void);
	int	(*shutdown)(void);
	
	void	(*clear_screen)(void);
	void	(*update_vsync)(const int data);
	
	int	(*flip)(void);
	void	(*stretch_adjust)(void);
	void	(*update_renderer)(void);
	int	(*reinit_gens_window)(void);
	
#ifdef GENS_OS_WIN32
	// Win32-specific functions.
	int	(*clear_primary_screen)(void);
	int	(*clear_back_screen)(void);
	int	(*restore_primary)(void);
	int	(*set_cooperative_level)(void);
#endif /* GENS_OS_WIN32 */
	
} vdraw_backend_t;

extern const vdraw_backend_t* const vdraw_backends[];

int	vdraw_init(void);
int	vdraw_end(void);
int	vdraw_backend_init_subsystem(VDRAW_BACKEND backend);
int	vdraw_backend_init(VDRAW_BACKEND backend);
int	vdraw_backend_end(void);

// Current backend.
extern const vdraw_backend_t *vdraw_cur_backend;
extern VDRAW_BACKEND vdraw_cur_backend_id;
extern uint32_t vdraw_cur_backend_flags;

int	vdraw_flip(int md_screen_updated);
void	vdraw_set_bpp(const int new_bpp, const BOOL reset_video);
void	vdraw_refresh_video(void);

// Function pointers.
extern int	(*vdraw_init_subsystem)(void);
extern int	(*vdraw_shutdown)(void);
extern void	(*vdraw_clear_screen)(void);
extern void	(*vdraw_update_vsync)(const int data);
#ifdef GENS_OS_WIN32
extern int	(*vdraw_reinit_gens_window)(void);
extern int	(*vdraw_clear_primary_screen)(void);
extern int	(*vdraw_clear_back_screen)(void);
extern int	(*vdraw_restore_primary)(void);
extern int	(*vdraw_set_cooperative_level)(void);
#endif /* GENS_OS_WIN32 */

// Message variables used externally.
extern BOOL vdraw_fps_enabled;
extern vdraw_style_t vdraw_fps_style;
extern BOOL vdraw_msg_visible;
extern vdraw_style_t vdraw_msg_style;

// Properties.
uint8_t	vdraw_get_stretch(void);
void	vdraw_set_stretch(const uint8_t new_stretch);
BOOL	vdraw_get_sw_render(void);
void	vdraw_set_sw_render(const BOOL new_sw_render);
BOOL	vdraw_get_msg_enabled(void);
void	vdraw_set_msg_enabled(const BOOL new_msg_enable);
BOOL	vdraw_get_fps_enabled(void);
void	vdraw_set_fps_enabled(const BOOL new_fps_enable);
BOOL	vdraw_get_fullscreen(void);
void	vdraw_set_fullscreen(const BOOL new_fullscreen);
BOOL	vdraw_get_fast_blur(void);
void	vdraw_set_fast_blur(const BOOL new_fast_blur);

// Style properties.
uint8_t	vdraw_get_msg_style(void);
void	vdraw_set_msg_style(const uint8_t new_msg_style);
uint8_t	vdraw_get_fps_style(void);
void	vdraw_set_fps_style(const uint8_t new_fps_style);
uint8_t	vdraw_get_intro_effect_color(void);
void	vdraw_set_intro_effect_color(const uint8_t new_intro_effect_color);

// Render functions.
extern mdp_render_fn vdraw_blitFS;
extern mdp_render_fn vdraw_blitW;

// Render plugin information.
extern mdp_render_info_t vdraw_rInfo;

// Scale factor.
extern int vdraw_scale;

// Screen border.
extern int vdraw_border_h, vdraw_border_h_old;
extern uint16_t vdraw_border_color_16;
extern uint32_t vdraw_border_color_32;

// RGB color conversion variables.
extern BOOL vdraw_needs_conversion;

#ifdef __cplusplus
}
#endif

#endif /* GENS_VDRAW_H */

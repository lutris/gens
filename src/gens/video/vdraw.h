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

#ifndef GENS_VDRAW_H
#define GENS_VDRAW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// MDP Render Functions.
#include "mdp/mdp_render.h"

// Stretch mode constants.
#define STRETCH_NONE		0x00
#define STRETCH_H		0x01
#define STRETCH_V		0x02
#define STRETCH_FULL		0x03

// VDraw backends.
typedef enum
{
	#ifdef GENS_OS_WIN32
		VDRAW_BACKEND_DDRAW,
	#else /* !GENS_OS_WIN32 */
		VDRAW_BACKEND_SDL,
		#ifdef GENS_OPENGL
			VDRAW_BACKEND_SDL_GL,
		#endif /* GENS_OPENGL */
	#endif /* GENS_OS_WIN32 */
	VDRAW_BACKEND_MAX
} VDRAW_BACKEND;

// VDraw backend function pointers.
typedef struct
{
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
} vdraw_backend_t;

int	vdraw_init(void);
int	vdraw_end(void);
int	vdraw_backend_init_subsystem(VDRAW_BACKEND backend);
int	vdraw_backend_init(VDRAW_BACKEND backend);
int	vdraw_backend_end(void);

// Current backend.
extern vdraw_backend_t	*vdraw_cur_backend;
extern VDRAW_BACKEND	vdraw_cur_backend_id;

// Called if initialization fails.
void	vdraw_init_fail(const char* err);

int	vdraw_flip(void);
void	vdraw_set_bpp(const int new_bpp, const BOOL reset_video);
void	vdraw_refresh_video(void);

// Function pointers.
extern int	(*vdraw_init_subsystem)(void);
extern int	(*vdraw_shutdown)(void);
extern void	(*vdraw_clear_screen)(void);
extern void	(*vdraw_update_vsync)(const int data);

void	vdraw_write_text(const char* msg, const int duration);

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
extern MDP_Render_Info_t vdraw_rInfo;

// Scale factor.
extern int vdraw_scale;

// Screen border.
extern int vdraw_border_h, vdraw_border_h_old;
extern uint16_t vdraw_border_color_16;
extern uint32_t vdraw_border_color_32;

// 16-bit to 32-bit conversion.
extern uint16_t	*vdraw_16to32_surface;
extern int	vdraw_16to32_scale;
extern int	vdraw_16to32_pitch;
void vdraw_render_16to32(uint32_t *dest, uint16_t *src,
			 int width, int height,
			 int pitch_dest, int pitch_src);

#ifdef __cplusplus
}
#endif

#endif /* GENS_VDRAW_H */

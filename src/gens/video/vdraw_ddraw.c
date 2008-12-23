/***************************************************************************
 * Gens: Video Drawing - DirectDraw Backend.                               *
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

#include "vdraw_ddraw.h"

#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Windows / DirectDraw includes.
#include <windows.h>
#include <ddraw.h>

// Gens window.
#include "gens/gens_window.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// CPU flags.
#include "gens_core/misc/cpuflags.h"

// Text drawing functions.
#include "vdraw_text.hpp"


// Function prototypes.
static int	vdraw_ddraw_init(void);
static int	vdraw_ddraw_end(void);

static int	vdraw_ddraw_init_subsystem(void);
static int	vdraw_ddraw_shutdown(void);

static void	vdraw_ddraw_clear_screen(void);
static void	vdraw_ddraw_update_vsync(const BOOL fromInitSDLGL);

static int	vdraw_ddraw_flip(void);
static void	vdraw_ddraw_stretch_adjust(void);
static void	vdraw_ddraw_update_renderer(void);


// Win32-specific functions.
static int	vdraw_ddraw_reinit_gens_window(void);
static int	vdraw_ddraw_clear_primary_screen(void);
static int	vdraw_ddraw_clear_back_screen(void);
static int	vdraw_ddraw_restore_primary(void);
static int	vdraw_ddraw_set_cooperative_level(void);


// DirectDraw variables
static LPDIRECTDRAW lpDD_Init = NULL;
static LPDIRECTDRAW4 lpDD = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Primary = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Flip = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Back = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Blit = NULL;
static LPDIRECTDRAWCLIPPER lpDDC_Clipper = NULL;

static HRESULT RestoreGraphics(void);
static void CalculateDrawArea(RECT *RectDest, RECT *RectSrc, float *Ratio_X, float *Ratio_Y, int *Dep);
static inline void vdraw_ddraw_draw_text(DDSURFACEDESC2* pddsd, LPDIRECTDRAWSURFACE4 lpDDS_Surface, const BOOL lock);


// VDraw Backend struct.
vdraw_backend_t vdraw_backend_ddraw =
{
	.init = vdraw_ddraw_init,
	.end = vdraw_ddraw_end,
	
	.init_subsystem = NULL,
	.shutdown = NULL,
	
	.clear_screen = vdraw_ddraw_clear_screen,
	.update_vsync = vdraw_ddraw_update_vsync,
	
	.flip = vdraw_ddraw_flip,
	.stretch_adjust = vdraw_ddraw_stretch_adjust,
	.update_renderer = vdraw_ddraw_update_renderer,
	
	// Win32-specific functions.
	.reinit_gens_window	= vdraw_ddraw_reinit_gens_window,
	.clear_primary_screen	= vdraw_ddraw_clear_primary_screen,
	.clear_back_screen	= vdraw_ddraw_clear_back_screen,
	.restore_primary	= vdraw_ddraw_restore_primary,
	.set_cooperative_level	= vdraw_ddraw_set_cooperative_level
};


static inline void vdraw_ddraw_draw_text(DDSURFACEDESC2* pddsd, LPDIRECTDRAWSURFACE4 lpDDS_Surface, const BOOL lock)
{
	if (lock)
		IDirectDrawSurface4_Lock(lpDDS_Surface, NULL, pddsd, DDLOCK_WAIT, NULL);
	
	// Determine the window size using the scaling factor.
	const int w = 320 * vdraw_scale;
	const int h = 240 * vdraw_scale;
	
	// +(8*bytespp) is needed for the lpSurface pointer because the DDraw module
	// includes the entire 336x240 MD_Screen. The first 8 pixels are offscreen,
	// so they won't show up at all.
	
	unsigned char bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	// NOTE: fullW must be (pddsd->lPitch / bytespp).
	// DirectDraw likes to use absurdly large line lengths in full screen mode.
	// (pddsd->lPitch / bytespp) does match pddsd->dwWidth in windowed mode, though.
	
	if (vdraw_msg_visible)
	{
		// Message is visible.
		draw_text((unsigned char*)pddsd->lpSurface + (8*bytespp), pddsd->lPitch / bytespp,
			  w, h, vdraw_msg_text, &vdraw_msg_style, FALSE);
	}
	else if (vdraw_fps_enabled && (Game != NULL) && Active && !Paused && !Debug)
	{
		// FPS is enabled.
		draw_text((unsigned char*)pddsd->lpSurface + (8*bytespp), pddsd->lPitch / bytespp,
			  w, h, vdraw_msg_text, &vdraw_fps_style, FALSE);
	}
	
	if (lock)
		IDirectDrawSurface4_Unlock(lpDDS_Surface, NULL);
}

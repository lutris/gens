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

// VDraw C++ functions.
#include "vdraw_cpp.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Windows / DirectDraw includes.
#include <windows.h>
#include <ddraw.h>

// Gens window.
#include "gens/gens_window.hpp"
#include "gens/gens_window_sync.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// CPU flags.
#include "gens_core/misc/cpuflags.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// Inline video functions.
#include "v_inline.h"


// Function prototypes.
static int	vdraw_ddraw_init(void);
static int	vdraw_ddraw_end(void);

static void	vdraw_ddraw_clear_screen(void);
static void	vdraw_ddraw_update_vsync(const BOOL fromInitSDLGL);

static int	vdraw_ddraw_flip(void);
static void	vdraw_ddraw_update_renderer(void);


// Win32-specific functions.
static int	vdraw_ddraw_reinit_gens_window(void);
static int	vdraw_ddraw_clear_primary_screen(void);
static int	vdraw_ddraw_clear_back_screen(void);
static int	vdraw_ddraw_restore_primary(void);
static int	vdraw_ddraw_set_cooperative_level(void);


// X and Y resolutions.
static int Res_X;
static int Res_Y;

// DirectDraw variables.
static LPDIRECTDRAW lpDD_Init = NULL;
static LPDIRECTDRAW4 lpDD = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Primary = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Flip = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Back = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Blit = NULL;
static LPDIRECTDRAWCLIPPER lpDDC_Clipper = NULL;

// Miscellaneous DirectDraw-specific functions.
static HRESULT RestoreGraphics(void);
static void vdraw_ddraw_calc_draw_area(RECT *RectDest, RECT *RectSrc, float *pRatio_X, float *pRatio_Y, int *Dep);
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
	.stretch_adjust = NULL,
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


/**
 * vdraw_ddraw_init(): Initialize the DirectDraw video subsystem.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_ddraw_init(void)
{
	DDSURFACEDESC2 ddsd;
	
	vdraw_ddraw_end();
	
	MDP_Render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return -1;
	const int w = 320 * scale;
	const int h = 240 * scale;
	const int mdW = 336 * scale;
	
	if (vdraw_get_fullscreen())
	{
		Res_X = w;
		Res_Y = h;
	}
	
	if (FAILED(DirectDrawCreate(NULL, &lpDD_Init, NULL)))
	{
		vdraw_init_fail("Error with DirectDrawCreate!");
		return -2;
	}
	
	if (FAILED(IDirectDraw4_QueryInterface(lpDD_Init, &IID_IDirectDraw4, (LPVOID*)&lpDD)))
	{
		vdraw_init_fail("Error with QueryInterface!\nUpgrade your DirectX version.");
		return -3;
	}
	
	IDirectDraw4_Release(lpDD_Init);
	lpDD_Init = NULL;
	
	// TODO: 15-bit color override ("Force 555" or "Force 565" in the config file).
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	// TODO: Figure out what FS_No_Res_Change is for.
	// TODO: Figure out if this is correct.
	if (vdraw_get_fullscreen() /* && !FS_No_Res_Change*/)
	{
		// Always use 16-bit color in fullscreen.
		if (FAILED(IDirectDraw4_SetDisplayMode(lpDD, Res_X, Res_Y, 16, 0, 0)))
		{
			vdraw_init_fail("Error with lpDD->SetDisplayMode()!");
			return -4;
		}
	}
	
	// Check the current color depth.
	unsigned char newBpp;
	IDirectDraw4_GetDisplayMode(lpDD, &ddsd);
	switch (ddsd.ddpfPixelFormat.dwGBitMask)
	{
		case 0x03E0:
			// 15-bit color.
			newBpp = 15;
			break;
		case 0x07E0:
			// 16-bit color.
			newBpp = 16;
			break;
		case 0x00FF00:
		default:
			// 32-bit color.
			newBpp = 32;
			break;
	}
	
	if (newBpp != bppOut)
		vdraw_set_bpp(newBpp, FALSE);
	
	vdraw_ddraw_set_cooperative_level();
	
	// Clear ddsd.
	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	if (vdraw_get_fullscreen() && Video.VSync_FS)
	{
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 2;
	}
	else
	{
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}
	
	if (FAILED(IDirectDraw4_CreateSurface(lpDD, &ddsd, &lpDDS_Primary, NULL)))
	{
		vdraw_init_fail("Error with lpDD->CreateSurface()! [lpDDS_Primary]");
		return -5;
	}
	
	if (vdraw_get_fullscreen())
	{
		if (Video.VSync_FS)
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
			
			if (FAILED(IDirectDrawSurface4_GetAttachedSurface(lpDDS_Primary, &ddsd.ddsCaps, &lpDDS_Flip)))
			{
				vdraw_init_fail("Error with lpDDPrimary->GetAttachedSurface()!");
				return -6;
			}
			
			lpDDS_Blit = lpDDS_Flip;
		}
		else
		{
			lpDDS_Blit = lpDDS_Primary;
		}
	}
	else
	{
		if (FAILED(IDirectDraw4_CreateClipper(lpDD, 0, &lpDDC_Clipper, NULL )))
		{
			vdraw_init_fail("Error with lpDD->CreateClipper()!");
			return -7;
		}
		
		if (FAILED(IDirectDrawClipper_SetHWnd(lpDDC_Clipper, 0, Gens_hWnd)))
		{
			vdraw_init_fail("Error with lpDDC_Clipper->SetHWnd()!");
			return -8;
		}
		
		if (FAILED(IDirectDrawSurface4_SetClipper(lpDDS_Primary, lpDDC_Clipper)))
		{
			vdraw_init_fail("Error with lpDDS_Primary->SetClipper()!");
			return -9;
		}
	}
	
	// Clear ddsd again.
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	
	// Determine the width and height.
	// NOTE: For DirectDraw, the actual 336 width is used.
	if (scale == 1)
	{
		// Normal render mode. 320x240 [336x240]
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		ddsd.dwWidth = 336;
		ddsd.dwHeight = 240;
	}
	else
	{
		// Larger than 1x.
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		ddsd.dwWidth = mdW;
		ddsd.dwHeight = h;
	}
	
	if (FAILED(IDirectDraw4_CreateSurface(lpDD, &ddsd, &lpDDS_Back, NULL)))
	{
		vdraw_init_fail("Error with lpDD->CreateSurface()! [lpDDS_Back]");
		return -10;
	}
	
	// TODO: Check if this is right.
	// I think this might be causing the frame counter flicker in full screen mode.
	//if (!m_FullScreen || (rendMode >= 1 && (/*FS_No_Res_Change ||*/ Res_X != 640 || Res_Y != 480)))
	if (!(vdraw_get_fullscreen() && scale == 1))
		lpDDS_Blit = lpDDS_Back;
	
	if (scale == 1)
	{
		// Normal rendering mode uses MD_Screen directly.
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		
		if (FAILED(IDirectDrawSurface4_GetSurfaceDesc(lpDDS_Back, &ddsd)))
		{
			vdraw_init_fail("Error with lpDD_Back->GetSurfaceDesc()!");
			return 11;
		}
		
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_LPSURFACE | DDSD_PIXELFORMAT;
		ddsd.dwWidth = 336;
		ddsd.dwHeight = 240;
		
		if (ddsd.ddpfPixelFormat.dwRGBBitCount > 16)
		{
			// 32-bit color.
			ddsd.lpSurface = MD_Screen32;
			ddsd.lPitch = 336 * 4;
		}
		else
		{
			// 15-bit or 16-bit color.
			ddsd.lpSurface = MD_Screen;
			ddsd.lPitch = 336 * 2;
		}
		
		if (FAILED(IDirectDrawSurface4_SetSurfaceDesc(lpDDS_Back, &ddsd, 0)))
		{
			vdraw_init_fail("Error with lpDD_Back->SetSurfaceDesc()!");
			return 12;
		}
	}
	
	// Reset the render mode.
	vdraw_reset_renderer(FALSE);
	
	// Synchronize menus.
	Sync_Gens_Window();
	
	// vdraw_ddraw initialized.
	return 0;
}


/**
 * vdraw_ddraw_end(): Close the DirectDraw renderer.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_ddraw_end(void)
{
	if (lpDDC_Clipper)
	{
		IDirectDrawClipper_Release(lpDDC_Clipper);
		lpDDC_Clipper = NULL;
	}
	
	if (lpDDS_Back)
	{
		IDirectDrawSurface4_Release(lpDDS_Back);
		lpDDS_Back = NULL;
	}
	
	if (lpDDS_Flip)
	{
		IDirectDrawSurface4_Release(lpDDS_Flip);
		lpDDS_Flip = NULL;
	}
	
	if (lpDDS_Primary)
	{
		IDirectDrawSurface4_Release(lpDDS_Primary);
		lpDDS_Primary = NULL;
	}
	
	if (lpDD)
	{
		IDirectDraw4_SetCooperativeLevel(lpDD, Gens_hWnd, DDSCL_NORMAL);
		IDirectDraw4_Release(lpDD);
		lpDD = NULL;
	}
	
	lpDDS_Blit = NULL;
	return 0;
}


/** 
 * vdraw_ddraw_clear_screen(): Clear the screen.
 */
static void vdraw_ddraw_clear_screen(void)
{
	// Clear both screen buffers.
	vdraw_ddraw_clear_primary_screen();
	vdraw_ddraw_clear_back_screen();
	
	// Reset the border color to make sure it's redrawn.
	vdraw_border_color_16 = ~MD_Palette[0];
	vdraw_border_color_32 = ~MD_Palette32[0];
}


/**
 * vdraw_ddraw_clear_primary_screen(): Clear the primary screen.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_ddraw_clear_primary_screen(void)
{
	if (!lpDD || !lpDDS_Primary)
		return -1;
	
	DDSURFACEDESC2 ddsd;
	DDBLTFX ddbltfx;
	RECT rd;
	POINT p;
	
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = 0; // Black
	
	if (vdraw_get_fullscreen())
	{
		if (Video.VSync_FS)
		{
			IDirectDrawSurface4_Blt(lpDDS_Flip, NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			IDirectDrawSurface4_Flip(lpDDS_Primary, NULL, DDFLIP_WAIT);
			
			IDirectDrawSurface4_Blt(lpDDS_Flip, NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			IDirectDrawSurface4_Flip(lpDDS_Primary, NULL, DDFLIP_WAIT);
			
			IDirectDrawSurface4_Blt(lpDDS_Flip, NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			IDirectDrawSurface4_Flip(lpDDS_Primary, NULL, DDFLIP_WAIT);
		}
		else
		{
			IDirectDrawSurface4_Blt(lpDDS_Primary, NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
		}
	}
	else
	{
		p.x = p.y = 0;
		GetClientRect(Gens_hWnd, &rd);
		ClientToScreen(Gens_hWnd, &p);
		
		rd.left = p.x;
		rd.top = p.y;
		rd.right += p.x;
		rd.bottom += p.y;
		
		if (rd.top < rd.bottom)
			IDirectDrawSurface4_Blt(lpDDS_Primary, &rd, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	}
	
	return 0;
}


/**
 * vdraw_ddraw_clear_back_screen(): Clear the back buffer.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_ddraw_clear_back_screen(void)
{
	if (!lpDD || !lpDDS_Back)
		return -1;
	
	DDSURFACEDESC2 ddsd;
	DDBLTFX ddbltfx;
	
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = 0;
	
	IDirectDrawSurface4_Blt(lpDDS_Back, NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	
	return 1;
}


/**
 * vdraw_ddraw_calc_draw_area(): Calculate the drawing area.
 * @param RectDest [in, out] Destination rectangle.
 * @param RectSrc [out] Source rectangle.
 * @param Ratio_X [out] X ratio.
 * @param Ratio_Y [out] Y ratio.
 * @param Dep [out] Horizontal border.
 */
static void vdraw_ddraw_calc_draw_area(RECT *pRectDest, RECT *pRectSrc, float *pRatio_X, float* pRatio_Y, int *Dep)
{
	// TODO: Is this check really necessary?
	if (!pRectDest || !pRectSrc || !pRatio_X || !pRatio_Y || !Dep)
		return;
	
	*pRatio_X = (float)pRectDest->right / 320.0f;  //Upth-Modif - why use two lines of code
	*pRatio_Y = (float)pRectDest->bottom / 240.0f; //Upth-Modif - when you can do this?
	*pRatio_X = *pRatio_Y = (*pRatio_X < *pRatio_Y) ? *pRatio_X : *pRatio_Y; //Upth-Add - and here we floor the value
	
	POINT q; //Upth-Add - For determining the correct ratio
	q.x = pRectDest->right; //Upth-Add - we need to get
	q.y = pRectDest->bottom; //Upth-Add - the bottom-right corner
	
	uint8_t stretch = vdraw_get_stretch();
	
	if (vdraw_scale == 1)
	{
		pRectSrc->top = 0;
		pRectSrc->bottom = VDP_Num_Vis_Lines;

		if ((VDP_Num_Vis_Lines == 224) && !(stretch & STRETCH_V))
		{
			pRectDest->top = (int) ((q.y - (224 * *pRatio_Y))/2); //Upth-Modif - Centering the screen properly
			pRectDest->bottom = (int) (224 * *pRatio_Y) + pRectDest->top; //Upth-Modif - along the y axis
		}
	}
	else
	{
		if (VDP_Num_Vis_Lines == 224)
		{
			pRectSrc->top = 8 * vdraw_scale;
			pRectSrc->bottom = (224 + 8) * vdraw_scale;

			if (!(stretch & STRETCH_V))
			{
				pRectDest->top = (int) ((q.y - (224 * *pRatio_Y)) / 2); //Upth-Modif - Centering the screen properly
				pRectDest->bottom = (int) (224 * *pRatio_Y) + pRectDest->top; //Upth-Modif - along the y axis again
			}
		}
		else
		{
			pRectSrc->top = 0; //Upth-Modif - Was "0 * 2"
			pRectSrc->bottom = (240 * vdraw_scale);
		}
	}

	if (isFullXRes())
	{
		Dep = 0;

		if (vdraw_scale == 1)
		{
			pRectSrc->left = 8 + 0 ;
			pRectSrc->right = 8 + 320;
		}
		else
		{
			pRectSrc->left = 0; //Upth-Modif - Was "0 * 2"
			pRectSrc->right = 320 * vdraw_scale;
		}
		pRectDest->left = (int) ((q.x - (320 * *pRatio_X)) / 2); //Upth-Add - center the picture
		pRectDest->right = (int) (320 * *pRatio_X) + pRectDest->left; //Upth-Add - along the x axis
	}
	else // less-wide X resolution:
	{
		Dep = 64;
		
		if (!(stretch & STRETCH_H))
		{
			pRectDest->left = (q.x - (int)(256.0f * *pRatio_X)) / 2; //Upth-Modif - center the picture properly
			pRectDest->right = (int)(256.0f * *pRatio_X) + pRectDest->left; //Upth-Modif - along the x axis
		}
		
		if (vdraw_scale == 1)
		{
			pRectSrc->left = 8 + 0;
			pRectSrc->right = 8 + 256;
		}
		else
		{
			pRectSrc->left = 32 * vdraw_scale;
			pRectSrc->right = (32 + 256) * vdraw_scale;
		}
	}
}

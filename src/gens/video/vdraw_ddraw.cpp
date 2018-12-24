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

#include "vdraw.h"
#include "vdraw_ddraw.hpp"

// Message logging.
#include "macros/log_msg.h"

// Unused Parameter macro.
#include "libgsft/gsft_unused.h"

#include "emulator/g_main.hpp"
#include "util/file/rom.hpp"
#include "debugger/debugger.hpp"

// VDraw C++ functions.
#include "vdraw_cpp.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Windows / DirectDraw includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <ddraw.h>

// Gens window.
#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"
#include "ui/gens_ui.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// RGB color conversion functions.
#include "vdraw_RGB.h"

// Audio Handler.
#include "audio/audio.h"

// Render Manager.
#include "plugins/rendermgr.hpp"


// X and Y resolutions.
static int Res_X;
static int Res_Y;

// DirectDraw variables.
static LPDIRECTDRAW4 lpDD = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Primary = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Flip = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Back = NULL;
static LPDIRECTDRAWSURFACE4 lpDDS_Blit = NULL;
static LPDIRECTDRAWCLIPPER lpDDC_Clipper = NULL;

// Miscellaneous DirectDraw-specific functions.
static HRESULT vdraw_ddraw_restore_graphics(void);
static void vdraw_ddraw_calc_draw_area(RECT& RectDest, RECT& RectSrc, float& Ratio_X, float& Ratio_Y, int& Dep);
static inline void vdraw_ddraw_draw_text(DDSURFACEDESC2* pddsd, LPDIRECTDRAWSURFACE4 lpDDS_Surface, const BOOL lock);

// Border drawing.
static void vdraw_ddraw_draw_border(LPDIRECTDRAWSURFACE4 lpDDS_Surface, const RECT& RectDest);
static DDBLTFX ddbltfx_Border_Color;


static inline void vdraw_ddraw_draw_text(DDSURFACEDESC2* pddsd, LPDIRECTDRAWSURFACE4 lpDDS_Surface, const BOOL lock)
{
	if (lock)
		lpDDS_Surface->Lock(NULL, pddsd, DDLOCK_WAIT, NULL);
	
	// Determine the window size using the scaling factor.
	const int w = (vdp_isH40() ? 320 : 256) * vdraw_scale;
	const int h = VDP_Num_Vis_Lines * vdraw_scale;
	
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
			  w, h, vdraw_msg_text, &vdraw_msg_style, true);
	}
	else if (vdraw_fps_enabled && (Game != NULL) && Settings.Active && !Settings.Paused && !IS_DEBUGGING())
	{
		// FPS is enabled.
		draw_text((unsigned char*)pddsd->lpSurface + (8*bytespp), pddsd->lPitch / bytespp,
			  w, h, vdraw_msg_text, &vdraw_fps_style, true);
	}
	
	if (lock)
		lpDDS_Surface->Unlock(NULL);
}


/**
 * vdraw_ddraw_free_all(): Free all DirectDraw objects.
 * @param scl If true, sets the cooperative level of lpDD before freeing it.
 */
static void vdraw_ddraw_free_all(bool scl)
{
	if (lpDDC_Clipper)
	{
		lpDDC_Clipper->Release();
		lpDDC_Clipper = NULL;
	}
	
	if (lpDDS_Back)
	{
		lpDDS_Back->Release();
		lpDDS_Back = NULL;
	}
	
	if (lpDDS_Flip)
	{
		lpDDS_Flip->Release();
		lpDDS_Flip = NULL;
	}
	
	if (lpDDS_Primary)
	{
		lpDDS_Primary->Release();
		lpDDS_Primary = NULL;
	}
	
	if (lpDD)
	{
		if (scl)
			lpDD->SetCooperativeLevel(gens_window, DDSCL_NORMAL);
		lpDD->Release();
		lpDD = NULL;
	}
	
	lpDDS_Blit = NULL;
}


/**
 * vdraw_ddraw_init(): Initialize the DirectDraw video subsystem.
 * @return 0 on success; non-zero on error.
 */
int vdraw_ddraw_init(void)
{
	DDSURFACEDESC2 ddsd;
	
	vdraw_ddraw_end();
	
	mdp_render_t *rendMode = get_mdp_render_t();
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
	
	// Return value.
	int rval;
	
	// Initialize DirectDraw.
	LPDIRECTDRAW lpDD_Init;
	rval = DirectDrawCreate(NULL, &lpDD_Init, NULL);
	if (FAILED(rval))
	{
		LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
			"DirectDrawCreate() failed: 0x%08X", rval);
		return -2;
	}
	
	rval = lpDD_Init->QueryInterface(IID_IDirectDraw4, (LPVOID*)&lpDD);
	if (FAILED(rval))
	{
		if (lpDD_Init)
			lpDD_Init->Release();
		
		LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
			"lpDD_Init->QueryInterface(IID_IDirectDraw4) failed: 0x%08X", rval);
		LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
			"This can usually be fixed by upgrading DirectX.");
		return -3;
	}
	
	// Free the DirectDraw initialization object.
	lpDD_Init->Release();
	
	// TODO: 15-bit color override. ("Force 555" or "Force 565" in the config file.)
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	// TODO: Figure out what FS_No_Res_Change is for.
	// TODO: Figure out if this is correct.
	if (vdraw_get_fullscreen() /* && !FS_No_Res_Change*/)
	{
		// Always use 16-bit color in fullscreen.
		rval = lpDD->SetDisplayMode(Res_X, Res_Y, 16, 0, 0);
		if (FAILED(rval))
		{
			vdraw_ddraw_free_all(false);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"lpDD->SetDisplayMode() failed: 0x%08X", rval);
			
			// If render mode is set to Normal, try using Double instead.
			if (rendMode_FS == RenderMgr::begin() && rendMode_FS != RenderMgr::end())
			{
				LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
					"Renderer is set to Normal; attempting to use Double instead.");
				rendMode_FS++;
				vdraw_set_renderer(rendMode_FS);
				vdraw_text_printf(1500, "Normal rendering failed. Using Double.");
				Sync_Gens_Window_GraphicsMenu();
			}
			
			return -4;
		}
	}
	
	// Check the current color depth.
	unsigned char newBpp;
	lpDD->GetDisplayMode(&ddsd);
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
		vdraw_set_bpp(newBpp, false);
	
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
	
	// Create the primary surface.
	rval = lpDD->CreateSurface(&ddsd, &lpDDS_Primary, NULL);
	if (FAILED(rval))
	{
		vdraw_ddraw_free_all(false);
		LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
			"lpDD->CreateSurface(&lpDDS_Primary) failed: 0x%08X", rval);
		return -5;
	}
	
	if (vdraw_get_fullscreen())
	{
		if (Video.VSync_FS)
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
			
			rval = lpDDS_Primary->GetAttachedSurface(&ddsd.ddsCaps, &lpDDS_Flip);
			if (FAILED(rval))
			{
				vdraw_ddraw_free_all(false);
				LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
					"lpDDS_Primary->GetAttachSurface() failed: 0x%08X", rval);
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
		rval = lpDD->CreateClipper(0, &lpDDC_Clipper, NULL);
		if (FAILED(rval))
		{
			vdraw_ddraw_free_all(false);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"lpDD->CreateClipper() failed: 0x%08X", rval);
			return -7;
		}
		
		rval = lpDDC_Clipper->SetHWnd(0, gens_window);
		if (FAILED(rval))
		{
			vdraw_ddraw_free_all(false);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"lpDDC_Clipper->SetHWnd() failed: 0x%08X", rval);
			return -8;
		}
		
		rval = lpDDS_Primary->SetClipper(lpDDC_Clipper);
		if (FAILED(rval))
		{
			vdraw_ddraw_free_all(false);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"lpDDC_Primary->SetClipper() failed: 0x%08X", rval);
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
	
	// Create the back surface.
	rval = lpDD->CreateSurface(&ddsd, &lpDDS_Back, NULL);
	if (FAILED(rval))
	{
		// Failed to create the back surface.
		// If we attempted to create it in video memory, try system memory instead.
		if (ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
		{
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"lpDD->CreateSurface(&lpDDS_Back, DDSCAPS_VIDEOMEMORY) failed: 0x%08X", rval);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"Attempting to use DDSCAPS_SYSTEMMEMORY instead.");
			ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			rval = lpDD->CreateSurface(&ddsd, &lpDDS_Back, NULL);
			if (FAILED(rval))
			{
				// Failed to create the back surface in system memory.
				vdraw_ddraw_free_all(false);
				LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
					"lpDD->CreateSurface(&lpDDS_Back, DDSCAPS_SYSTEMMEMORY) failed: 0x%08X", rval);
				return -10;
			}
		}
		else
		{
			vdraw_ddraw_free_all(false);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"lpDD->CreateSurface(&lpDDS_Back, DDSCAPS_SYSTEMMEMORY) failed: 0x%08X", rval);
			return -11;
		}
	}
	
	// TODO: Check if this is right.
	// I think this might be causing the frame counter flicker in full screen mode.
	//if (!vdraw_get_fullscreen() || (rendMode >= 1 && (/*FS_No_Res_Change ||*/ Res_X != 640 || Res_Y != 480)))
	if (!(vdraw_get_fullscreen() && scale == 1))
		lpDDS_Blit = lpDDS_Back;
	
	if (scale == 1)
	{
		// Normal rendering mode uses MD_Screen directly.
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		
		rval = lpDDS_Back->GetSurfaceDesc(&ddsd);
		if (FAILED(rval))
		{
			vdraw_ddraw_free_all(false);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"lpDDS_Back->GetSurfaceDesc() failed: 0x%08X", rval);
			return -12;
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
		
		rval = lpDDS_Back->SetSurfaceDesc(&ddsd, 0);
		if (FAILED(rval))
		{
			vdraw_ddraw_free_all(false);
			LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
				"lpDDS_Back->SetSurfaceDesc() failed: 0x%08X", rval);
			return -13;
		}
	}
	
	// Clear ddbltfx for the border color.
	memset(&ddbltfx_Border_Color, 0, sizeof(ddbltfx_Border_Color));
	ddbltfx_Border_Color.dwSize = sizeof(ddbltfx_Border_Color);
	
	// Reset the render mode.
	vdraw_reset_renderer(false);
	
	// Synchronize menus.
	Sync_Gens_Window();
	
	// vdraw_ddraw initialized.
	return 0;
}


/**
 * vdraw_ddraw_end(): Close the DirectDraw renderer.
 * @return 0 on success; non-zero on error.
 */
int vdraw_ddraw_end(void)
{
	vdraw_ddraw_free_all(true);
	return 0;
}


/** 
 * vdraw_ddraw_clear_screen(): Clear the screen.
 */
void vdraw_ddraw_clear_screen(void)
{
	// Clear both screen buffers.
	vdraw_ddraw_clear_primary_screen();
	vdraw_ddraw_clear_back_screen();
}


/**
 * vdraw_ddraw_clear_primary_screen(): Clear the primary screen.
 * @return 0 on success; non-zero on error.
 */
int vdraw_ddraw_clear_primary_screen(void)
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
			lpDDS_Flip->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
			
			lpDDS_Flip->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
			
			lpDDS_Flip->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
			lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
		}
		else
		{
			lpDDS_Primary->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
		}
	}
	else
	{
		p.x = p.y = 0;
		GetClientRect(gens_window, &rd);
		ClientToScreen(gens_window, &p);
		
		rd.left = p.x;
		rd.top = p.y;
		rd.right += p.x;
		rd.bottom += p.y;
		
		if (rd.top < rd.bottom)
			lpDDS_Primary->Blt(&rd, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	}
	
	return 0;
}


/**
 * vdraw_ddraw_clear_back_screen(): Clear the back buffer.
 * @return 0 on success; non-zero on error.
 */
int vdraw_ddraw_clear_back_screen(void)
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
	
	lpDDS_Back->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx);
	
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
static void vdraw_ddraw_calc_draw_area(RECT& RectDest, RECT& RectSrc, float& Ratio_X, float& Ratio_Y, int& Dep)
{
	Ratio_X = (float)RectDest.right / 320.0f;  //Upth-Modif - why use two lines of code
	Ratio_Y = (float)RectDest.bottom / 240.0f; //Upth-Modif - when you can do this?
	Ratio_X = Ratio_Y = (Ratio_X < Ratio_Y) ? Ratio_X : Ratio_Y; //Upth-Add - and here we floor the value
	
	POINT q; //Upth-Add - For determining the correct ratio
	q.x = RectDest.right; //Upth-Add - we need to get
	q.y = RectDest.bottom; //Upth-Add - the bottom-right corner
	
	const uint8_t stretch = vdraw_get_stretch();
	
	if (vdraw_scale == 1)
	{
		RectSrc.top = 0;
		RectSrc.bottom = VDP_Num_Vis_Lines;

		if ((VDP_Num_Vis_Lines == 224) && !(stretch & STRETCH_V))
		{
			RectDest.top = (int) ((q.y - (224 * Ratio_Y))/2); //Upth-Modif - Centering the screen properly
			RectDest.bottom = (int) (224 * Ratio_Y) + RectDest.top; //Upth-Modif - along the y axis
		}
	}
	else
	{
		if (VDP_Num_Vis_Lines == 224)
		{
			RectSrc.top = 8 * vdraw_scale;
			RectSrc.bottom = (224 + 8) * vdraw_scale;

			if (!(stretch & STRETCH_V))
			{
				RectDest.top = (int) ((q.y - (224 * Ratio_Y)) / 2); //Upth-Modif - Centering the screen properly
				RectDest.bottom = (int) (224 * Ratio_Y) + RectDest.top; //Upth-Modif - along the y axis again
			}
		}
		else
		{
			RectSrc.top = 0; //Upth-Modif - Was "0 * 2"
			RectSrc.bottom = (240 * vdraw_scale);
		}
	}

	if (vdp_isH40())
	{
		Dep = 0;
		
		if (vdraw_scale == 1)
		{
			RectSrc.left = 8 + 0 ;
			RectSrc.right = 8 + 320;
		}
		else
		{
			RectSrc.left = 0; //Upth-Modif - Was "0 * 2"
			RectSrc.right = 320 * vdraw_scale;
		}
		RectDest.left = (int) ((q.x - (320 * Ratio_X)) / 2); //Upth-Add - center the picture
		RectDest.right = (int) (320 * Ratio_X) + RectDest.left; //Upth-Add - along the x axis
	}
	else // less-wide X resolution:
	{
		Dep = 64;
		
		if (!(stretch & STRETCH_H))
		{
			RectDest.left = (q.x - (int)(256.0f * Ratio_X)) / 2; //Upth-Modif - center the picture properly
			RectDest.right = (int)(256.0f * Ratio_X) + RectDest.left; //Upth-Modif - along the x axis
		}
		
		if (vdraw_scale == 1)
		{
			RectSrc.left = 8 + 0;
			RectSrc.right = 8 + 256;
		}
		else
		{
			RectSrc.left = 32 * vdraw_scale;
			RectSrc.right = (32 + 256) * vdraw_scale;
		}
	}
}


/**
 * vdraw_ddraw_flip(): Flip the screen buffer. [Called by vdraw_flip().]
 * @return 0 on success; non-zero on error.
 */
int vdraw_ddraw_flip(void)
{
	// TODO: Add border drawing, like in vdraw_sdl.
	
	if (!lpDD)
		return -1;
	
	HRESULT rval = DD_OK;
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	RECT RectDest, RectSrc;
	POINT p;
	float Ratio_X, Ratio_Y;
	int Dep = 0;
	const unsigned char bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	const uint8_t stretch = vdraw_get_stretch();
	
	if (vdraw_get_fullscreen())
	{
		//Upth-Add - So we can set the fullscreen resolution to the current res without changing the value that gets saved to the config
		int FS_X, FS_Y;
		
#if 0
		if (Res_X < (320 << (int)(Video.Render_FS > 0)))
			Res_X = 320 << (int)(Video.Render_FS > 0); //Upth-Add - Flooring the resolution to 320x240
		if (Res_Y < (240 << (int)(Video.Render_FS > 0)))
			Res_Y = 240 << (int)(Video.Render_FS > 0); //Upth-Add - or 640x480, as appropriate
#endif
			
		// TODO: FS_No_Res_Change
#if 0
		if (FS_No_Res_Change)
		{
			//Upth-Add - If we didn't change resolution when we went Full Screen
			DEVMODE temp;
			EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&temp); //Upth-Add - Gets the current screen resolution
			FS_X = temp.dmPelsWidth;
			FS_Y = temp.dmPelsHeight;
		}
		else
		{
#endif
			//Upth-Add - Otherwise use the configured resolution values
			FS_X = Res_X; 
			FS_Y = Res_Y;
#if 0
		}
#endif
		
		Ratio_X = (float)FS_X / 320.0f; //Upth-Add - Find the current size-ratio on the x-axis
		Ratio_Y = (float)FS_Y / 240.0f; //Upth-Add - Find the current size-ratio on the y-axis
		
		Ratio_X = Ratio_Y = (Ratio_X < Ratio_Y) ? Ratio_X : Ratio_Y; //Upth-Add - Floor them to the smaller value for correct ratio display
		
		if (vdp_isH40())
		{
			if (Flag_Clr_Scr != 40)
			{
				// MD resolution change. Clear the screen.
				vdraw_ddraw_clear_screen();
				Flag_Clr_Scr = 40;
			}
			
			Dep = 0;
			RectSrc.left = 0 + 8;
			RectSrc.right = 320 + 8;
			RectDest.left = (int) ((FS_X - (320 * Ratio_X))/2); //Upth-Modif - Offset the left edge of the picture to the center of the screen
			RectDest.right = (int) (320 * Ratio_X) + RectDest.left; //Upth-Modif - Stretch the picture and move the right edge the same amount
			
			RectDest.top = (int) ((FS_Y - (240 * Ratio_Y))/2); //Upth-Add - Centers the screen top-bottom, in case Ratio_X was the floor.
			RectDest.bottom = RectDest.top + (VDP_Num_Vis_Lines * vdraw_scale);
		}
		else
		{
			if (Flag_Clr_Scr != 32)
			{
				// MD resolution change. Clear the screen.
				vdraw_ddraw_clear_screen();
				Flag_Clr_Scr = 32;
			}
			
			Dep = 64;
			RectSrc.left = 0 + 8;
			RectSrc.right = 256 + 8;
			
			if (stretch)
			{
				RectDest.left = 0;
				RectDest.right = FS_X; //Upth-Modif - use the user configured value
				RectDest.top = 0;      //Upth-Add - also, if we have stretch enabled
				RectDest.bottom = FS_Y;//Upth-Add - we don't correct the screen ratio
			}
			else
			{
				RectDest.left = (FS_X - (int)(256.0f * Ratio_X)) / 2; //Upth-Modif - Centering the screen left-right
				RectDest.right = (int)(256.0f * Ratio_X) + RectDest.left; //Upth-modif - again
				RectDest.top = (int) ((FS_Y - (240 * Ratio_Y))/2); //Upth-Add - Centers the screen top-bottom, in case Ratio_X was the floor.
				RectDest.bottom = RectDest.top + (VDP_Num_Vis_Lines * vdraw_scale);
			}
		}
		
		// TODO: Figure out how to get this working.
		// Until I can get it working, fall back to the standard 2x renderer.
#if 0
		if (Video.Render_FS == 1)
		{
			// 2x rendering.
			if (m_swRender)
			{
				// Software rendering is enabled.
				// Ignore the Stretch setting.
				rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
				
				if (FAILED(rval))
					goto cleanup_flip;
				
				int VBorder = ((240 - VDP_Num_Vis_Lines) / 2) << m_shift;	// Top border height, in pixels.
				int HBorder = (Dep * (bytespp / 2)) << m_shift;			// Left border width, in pixels.
				int startPos = (ddsd.lPitch * VBorder) + HBorder;
				unsigned char* start = (unsigned char*)ddsd.lpSurface + startPos;
			
				Blit_FS(start, ddsd.lPitch, 320 - Dep, VDP_Num_Vis_Lines, 32 + (Dep * 2));
				
				lpDDS_Blit->Unlock(NULL);
				
				if (Video.VSync_FS)
				{
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
			}
			else
			{
				RectSrc.top = 0;
				RectSrc.bottom = VDP_Num_Vis_Lines;
				
				if ((VDP_Num_Vis_Lines == 224) && !stretch)
				{
					RectDest.top = (int) ((FS_Y - (224 * Ratio_Y))/2); //Upth-Modif - centering top-bottom
					RectDest.bottom = (int) (224 * Ratio_Y) + RectDest.top; //Upth-Modif - with the method I already described for left-right
				}
				else
				{
					RectDest.top = (int) ((FS_Y - (240 * Ratio_Y))/2); //Upth-Modif - centering top-bottom under other circumstances
					RectDest.bottom = (int) (240 * Ratio_Y) + RectDest.top; //Upth-Modif - using the same method
				}
				RectDest.left = (int) ((FS_X - (320 * Ratio_X))/2); //Upth-Add - Centering left-right
				RectDest.right = (int) (320 * Ratio_X) + RectDest.left; //Upth-Add - I wonder why I had to change the center-stuff three times...

				if (Video.VSync_FS)
				{
					lpDDS_Flip->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
				else
				{
					lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
//					lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
				}
			}
		}
		else
#endif
		if (vdraw_scale == 1)
		{
			// 1x rendering.
			if (vdraw_get_sw_render())
			{
				// Software rendering is enabled.
				// Ignore the Stretch setting.
				
				rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
				
				if (FAILED(rval))
					goto cleanup_flip;
				
				const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
				const int HBorder = Dep * (bytespp / 2);		// Left border width, in pixels.
				
				const int startPos = ((ddsd.lPitch * VBorder) + HBorder) * vdraw_scale;	// Starting position from within the screen.
				
				// Start of the DDraw framebuffer.
				unsigned char* start = (unsigned char*)ddsd.lpSurface + startPos;
				
				vdraw_rInfo.destScreen = (void*)start;
				vdraw_rInfo.width = 320 - vdraw_border_h;
				vdraw_rInfo.height = VDP_Num_Vis_Lines;
				vdraw_rInfo.destPitch = ddsd.lPitch;
				
				vdraw_blitFS(&vdraw_rInfo);
				
				// Draw the text.
				vdraw_ddraw_draw_text(&ddsd, lpDDS_Blit, false);
				
				lpDDS_Blit->Unlock(NULL);
				
				// Draw the border.
				vdraw_ddraw_draw_border(lpDDS_Blit, RectDest);
				
				if (Video.VSync_FS)
				{
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
			}
			else
			{
				// Software rendering is disabled.
				// If Stretch is enabled, stretch the image.
				
				RectSrc.top = 0;
				RectSrc.bottom = VDP_Num_Vis_Lines;
				
				if (!(stretch & STRETCH_V))
				{
					RectDest.top = (int)((FS_Y - VDP_Num_Vis_Lines) / 2); //Upth-Add - But we still
					RectDest.bottom = RectDest.top + VDP_Num_Vis_Lines;   //Upth-Add - center the screen
				}
				else
				{
					RectDest.top = (int)((FS_Y - 240) / 2); //Upth-Add - for both of the
					RectDest.bottom = RectDest.top + 240;   //Upth-Add - predefined conditions
				}
				
				if (!(stretch & STRETCH_H))
				{
					RectDest.left = (int)((FS_X - (320 - Dep))/2); //Upth-Add - and along the
					RectDest.right = (320 - Dep) + RectDest.left;  //Upth-Add - x axis, also
				}
				else
				{
					RectDest.left = (int)((FS_X - 320)/2); //Upth-Add - and along the
					RectDest.right = 320 + RectDest.left;  //Upth-Add - x axis, also
				}
				
				vdraw_ddraw_draw_text(&ddsd, lpDDS_Back, true);
				if (Video.VSync_FS)
				{
					lpDDS_Flip->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
					vdraw_ddraw_draw_border(lpDDS_Flip, RectDest);
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
				else
				{
					vdraw_ddraw_draw_border(lpDDS_Primary, RectDest);
					lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
					//lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
				}
			}
		}
		else
		{
			// Other renderer.
			
			LPDIRECTDRAWSURFACE4 curBlit = lpDDS_Blit;
			rval = curBlit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			
			if (FAILED(rval))
				goto cleanup_flip;
			
			const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
			const int HBorder = Dep * (bytespp / 2);		// Left border width, in pixels.
				
			const int startPos = ((ddsd.lPitch * VBorder) + HBorder) * vdraw_scale;	// Starting position from within the screen.
				
			// Start of the DDraw framebuffer.
			unsigned char* start = (unsigned char*)ddsd.lpSurface + startPos;
			
			vdraw_rInfo.destScreen = (void*)start;
			vdraw_rInfo.width = 320 - vdraw_border_h;
			vdraw_rInfo.height = VDP_Num_Vis_Lines;
			vdraw_rInfo.destPitch = ddsd.lPitch;
			
			if (vdraw_needs_conversion)
			{
				// Color depth conversion is required.
				vdraw_rgb_convert(&vdraw_rInfo);
			}
			else
			{
				// Color conversion is not required.
				vdraw_blitFS(&vdraw_rInfo);
			}
			
			// Draw the text.
			vdraw_ddraw_draw_text(&ddsd, curBlit, false);
			
			curBlit->Unlock(NULL);
			
			// Draw the border.
			// TODO: This doesn't work here due to weird border calculations.
			vdraw_ddraw_draw_border(curBlit, RectDest);
			
			if (curBlit == lpDDS_Back) // note: this can happen in windowed fullscreen, or if CORRECT_256_ASPECT_RATIO is defined and the current display mode is 256 pixels across
			{
				RectDest.left = 0;
				RectDest.top = 0;
				RectDest.right = GetSystemMetrics(SM_CXSCREEN); // not SM_XVIRTUALSCREEN since we only want the primary monitor if there's more than one
				RectDest.bottom = GetSystemMetrics(SM_CYSCREEN);
				
				vdraw_ddraw_calc_draw_area(RectDest, RectSrc, Ratio_X, Ratio_Y, Dep);
				
				if (Video.VSync_FS)
				{
					int vb;
					lpDD->GetVerticalBlankStatus(&vb);
					if (!vb)
						lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
				}
				
				lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
			}
			else
			{
				if (Video.VSync_FS)
				{
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
			}
		}
	}
	else
	{
		// Windowed mode
		GetClientRect(gens_window, &RectDest);
		vdraw_ddraw_calc_draw_area(RectDest, RectSrc, Ratio_X, Ratio_Y, Dep);
		
		int Clr_Cmp_Val = vdp_isH40() ? 40 : 32;
		if (Flag_Clr_Scr != Clr_Cmp_Val)
		{
			// MD resolution change. Clear the screen.
			vdraw_ddraw_clear_screen();
			Flag_Clr_Scr = Clr_Cmp_Val;
		}
		
		if (vdraw_scale > 1)
		{
			rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			
			if (FAILED(rval))
				goto cleanup_flip;
			
			const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
			const int HBorder = Dep * (bytespp / 2);		// Left border width, in pixels.
				
			const int startPos = ((ddsd.lPitch * VBorder) + HBorder) * vdraw_scale;	// Starting position from within the screen.
				
			// Start of the DDraw framebuffer.
			unsigned char* start = (unsigned char*)ddsd.lpSurface + startPos;
			
			vdraw_rInfo.destScreen = (void*)start;
			vdraw_rInfo.width = 320 - vdraw_border_h;
			vdraw_rInfo.height = VDP_Num_Vis_Lines;
			vdraw_rInfo.destPitch = ddsd.lPitch;
			
			if (vdraw_needs_conversion)
			{
				// Color depth conversion is required.
				vdraw_rgb_convert(&vdraw_rInfo);
			}
			else
			{
				// Color conversion is not required.
				vdraw_blitW(&vdraw_rInfo);
			}
			
			// Draw the text.
			vdraw_ddraw_draw_text(&ddsd, lpDDS_Blit, false);
			
			lpDDS_Blit->Unlock(NULL);
		}
		else
		{
			// Draw the text.
			vdraw_ddraw_draw_text(&ddsd, lpDDS_Blit, true);
		}
		
		p.x = p.y = 0;
		ClientToScreen(gens_window, &p);
		
		// Draw the border.
		vdraw_ddraw_draw_border(lpDDS_Primary, RectDest);
		
		RectDest.top += p.y; //Upth-Modif - this part moves the picture into the window
		RectDest.bottom += p.y; //Upth-Modif - I had to move it after all of the centering
		RectDest.left += p.x;   //Upth-Modif - because it modifies the values
		RectDest.right += p.x;  //Upth-Modif - that I use to find the center
		
		if (RectDest.top < RectDest.bottom)
		{
			if (Video.VSync_W)
			{
				int vb;
				lpDD->GetVerticalBlankStatus(&vb);
				if (!vb)
					lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
			}
			
			// Blit the image.
			rval = lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
			//rval = lpDDS_Primary->Blt(&RectDest, lpDDS_Back, &RectSrc, NULL, NULL);
		}
	}

cleanup_flip:
	if (rval == DDERR_SURFACELOST)
		rval = vdraw_ddraw_restore_graphics();
	
	return 1;
}


/**
 * vdraw_ddraw_draw_border(): Draw the border color.
 * Called from vdraw_ddraw_flip().
 * @param lpDDS_Surface
 * @param RectDest Destination rectangle.
 */
static void vdraw_ddraw_draw_border(LPDIRECTDRAWSURFACE4 lpDDS_Surface, const RECT& RectDest)
{
	uint8_t stretch = vdraw_get_stretch();
	if (stretch == STRETCH_FULL)
		return;
	
	if (!Video.borderColorEmulation || (Game == NULL) || IS_DEBUGGING())
	{
		// Border color emulation is disabled.
		// Use a black border.
		ddbltfx_Border_Color.dwFillColor = 0;
	}
	else if (bppOut == 15 || bppOut == 16)
	{
		// 15-bit/16-bit color.
		uint16_t bc16 = MD_Palette[0];
		
		// Check if the border color needs to be converted.
		if (bppMD == 15 && bppOut == 16)
		{
			// MD palette is 15-bit; output is 16-bit.
			// MD:  0RRRRRGG GGGBBBBB
			// Out: RRRRRGGG GGGBBBBB
			bc16 = ((bc16 & 0x7C00) << 1) | ((bc16 & 0x03E0) << 1) | (bc16 & 0x1F);
		}
		else if (bppMD == 16 && bppOut == 15)
		{
			// MD palette is 16-bit; output is 15-bit.
			// MD:  RRRRRGGG GGGBBBBB
			// Out: 0RRRRRGG GGGBBBBB
			bc16 = ((bc16 & 0xF800) >> 1) | ((bc16 & 0x07C0) >> 1) | (bc16 & 0x1F);
		}
		
		ddbltfx_Border_Color.dwFillColor = bc16;
	}
	else //if (bppOut == 32)
	{
		// 32-bit color.
		ddbltfx_Border_Color.dwFillColor = MD_Palette32[0];
	}
	
	RECT rectDD;
	
	if (vdraw_get_fullscreen())
	{
		// Fullscreen.
		rectDD.left = 0;
		rectDD.top = 0;
		rectDD.right = 320 * vdraw_scale;
		rectDD.bottom = 240 * vdraw_scale;
	}
	else
	{
		// Windowed.
		POINT ptWin;
		ptWin.x = 0;
		ptWin.y = 0;
		GetClientRect(gens_window, &rectDD);
		ClientToScreen(gens_window, &ptWin);
		
		rectDD.left += ptWin.x;
		rectDD.top += ptWin.y;
		rectDD.right += ptWin.x;
		rectDD.bottom += ptWin.y;
	}
	
	RECT rectBorder;
	
	int drawHeight = RectDest.bottom - RectDest.top;
	int viewHeight = rectDD.bottom - rectDD.top;
	if ((viewHeight > drawHeight) && !(stretch & STRETCH_V))
	{
		// Vertical stretch is disabled.
		
		// Draw top border.
		rectBorder = rectDD;
		rectBorder.bottom = ((viewHeight - drawHeight) / 2) + rectBorder.top;
		lpDDS_Surface->Blt(&rectBorder, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx_Border_Color);
		
		// Draw bottom border.
		rectBorder = rectDD;
		rectBorder.top = ((viewHeight - drawHeight) / 2) + rectBorder.top + drawHeight;
		lpDDS_Surface->Blt(&rectBorder, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx_Border_Color);
	}
	
	int drawWidth = RectDest.right - RectDest.left;
	int viewWidth = rectDD.right - rectDD.left;
	if ((viewWidth > drawWidth) && !(stretch & STRETCH_H))
	{
		// Horizontal stretch is disabled.
		
		// Draw left border.
		rectBorder = rectDD;
		rectBorder.right = ((viewWidth - drawWidth) / 2) + rectBorder.left;
		lpDDS_Surface->Blt(&rectBorder, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx_Border_Color);
		
		// Draw bottom border.
		rectBorder = rectDD;
		rectBorder.left = ((viewWidth - drawWidth) / 2) + rectBorder.left + drawWidth;
		lpDDS_Surface->Blt(&rectBorder, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbltfx_Border_Color);
	}
}


/**
 * vdraw_ddraw_update_vsync(): Update the VSync value.
 * @param fromInitDDraw If true, this function is being called from vdraw_ddraw_init().
 */
void vdraw_ddraw_update_vsync(const BOOL fromInitDDraw)
{
	GSFT_UNUSED_PARAMETER(fromInitDDraw);
	
	// If Full Screen, reinitialize the video subsystem.
	if (vdraw_get_fullscreen())
		vdraw_refresh_video();
}


/**
 * vdraw_ddraw_reinit_gens_window(): Reinitialize the Gens window.
 * @return 0 on success; non-zero on error.
 */
int vdraw_ddraw_reinit_gens_window(void)
{
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	// Stop DirectDraw.
	vdraw_ddraw_end();
	
	// Reinitialize the Gens window.
	gens_window_reinit();
	
	// Reinitialize DirectDraw.
	return vdraw_ddraw_init();
}


/**
 * vdraw_ddraw_restore_graphics(): Restore the DirectDraw surface if it is lost.
 * @return HRESULT.
 */
HRESULT vdraw_ddraw_restore_graphics(void)
{
	HRESULT rval1 = lpDDS_Primary->Restore();
	HRESULT rval2 = lpDDS_Back->Restore();
	
	// Modif N. -- fixes lost surface handling when the color depth has changed
	if (rval1 == DDERR_WRONGMODE || rval2 == DDERR_WRONGMODE)
		return vdraw_ddraw_init() ? DD_OK : DDERR_GENERIC;
	
	return SUCCEEDED(rval2) ? rval1 : rval2;
}


/**
 * vdraw_ddraw_restore_primary(): Restore the primary DirectDraw surface.
 * @return 0 on success; non-zero on error.
 */
int vdraw_ddraw_restore_primary(void)
{
	if (!lpDD)
		return -1;
	
	if (vdraw_get_fullscreen() && Video.VSync_FS)
	{
		while (lpDDS_Primary->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_SURFACEBUSY) { }
		lpDD->FlipToGDISurface();
	}
	
	return 0;
}


/**
 * vdraw_ddraw_set_cooperative_level(): Sets the DirectDraw cooperative level.
 * @return 0 on success; non-zero on error.
 */
int vdraw_ddraw_set_cooperative_level(void)
{
	if (!gens_window || !lpDD)
		return -1;
	
	HRESULT rval;
#ifdef DISABLE_EXCLUSIVE_FULLSCREEN_LOCK
	Video.VSync_FS = 0;
	rval = lpDD->SetCooperativeLevel(gens_window, DDSCL_NORMAL);
#else
	if (vdraw_get_fullscreen())
		rval = lpDD->SetCooperativeLevel(gens_window, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	else
		rval = lpDD->SetCooperativeLevel(gens_window, DDSCL_NORMAL);
#endif
	
	if (FAILED(rval))
	{
		LOG_MSG(video, LOG_MSG_LEVEL_WARNING,
			"lpDD->SetCooperativeLevel() failed.");
		// TODO: Error handling code.
	}
	else
	{
		LOG_MSG(video, LOG_MSG_LEVEL_INFO,
			"lpDD->SetCooperativeLevel() succeeded.");
	}
	
	return 0;
}

/***************************************************************************
 * Gens: Video Drawing - DirectDraw Backend.                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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
#include <stdint.h>
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
static void WINAPI vdraw_ddraw_calc_draw_area(RECT& RectDest, RECT& RectSrc, int& Dep);


static inline void WINAPI vdraw_ddraw_draw_text(DDSURFACEDESC2* pddsd, LPDIRECTDRAWSURFACE4 lpDDS_Surface, const BOOL lock)
{
	if (lock)
		lpDDS_Surface->Lock(NULL, pddsd, DDLOCK_WAIT, NULL);
	
	// Determine the window size using the scaling factor.
	const int curHPix = vdp_getHPix();
	const int msg_width = curHPix * vdraw_scale;
	
	// +(8*bytespp) is needed for the lpSurface pointer because the DDraw module
	// includes the entire 336x240 MD_Screen. The first 8 pixels are offscreen,
	// so they won't show up at all.
	
	unsigned char bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	// NOTE: fullW must be (pddsd->lPitch / bytespp).
	// DirectDraw likes to use absurdly large line lengths in full screen mode.
	// (pddsd->lPitch / bytespp) does match pddsd->dwWidth in windowed mode, though.
	
	unsigned char *start = (unsigned char*)pddsd->lpSurface;
	start += pddsd->lPitch * (VDP_Lines.Visible.Border_Size * vdraw_scale);
	if (curHPix < 320)
	{
		// Starting position needs to be adjusted for H32.
		start += (((320 - curHPix) / 2) * vdraw_scale) * bytespp;
	}
	
	if (vdraw_scale == 1)
	{
		// DirectDraw's 1x rendering uses MD_Screen / MD_Screen32 directly.
		// Thus, it has an invisible 8px column at the beginning.
		start += (8 * bytespp);
	}
	
	if (vdraw_msg_visible)
	{
		// Message is visible.
		draw_text(start, pddsd->lPitch / bytespp,
			  msg_width, 
			  VDP_Lines.Visible.Total * vdraw_scale,
			  vdraw_msg_text, &vdraw_msg_style);
	}
	else if (vdraw_fps_enabled && (Game != NULL) && Settings.Active && !Settings.Paused && !IS_DEBUGGING())
	{
		// FPS is enabled.
		draw_text(start, pddsd->lPitch / bytespp,
			  msg_width,
			  VDP_Lines.Visible.Total * vdraw_scale,
			  vdraw_msg_text, &vdraw_fps_style);
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
				vdraw_text_write("Normal rendering failed. Using Double.", 1500);
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
int WINAPI vdraw_ddraw_clear_primary_screen(void)
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
int WINAPI vdraw_ddraw_clear_back_screen(void)
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
 * @param Dep [out] Horizontal border.
 */
static void WINAPI vdraw_ddraw_calc_draw_area(RECT& RectDest, RECT& RectSrc, int& Dep)
{
	const uint8_t stretch = vdraw_get_stretch();
	
	if (VDP_Lines.Visible.Total < 240 && (stretch & STRETCH_V))
	{
		// Less than 240 lines, vertical stretch enabled.
		RectSrc.top = VDP_Lines.Visible.Border_Size * vdraw_scale;
		RectSrc.bottom = (VDP_Lines.Visible.Total + VDP_Lines.Visible.Border_Size) * vdraw_scale;
	}
	else
	{
		// 240 lines or vertical stretch disabled.
		RectSrc.top = 0; //Upth-Modif - Was "0 * 2"
		RectSrc.bottom = (240 * vdraw_scale);
	}
	
	const int HPix = vdp_getHPix();
	const int Clr_Cmp_Val = (HPix / 8);
	if (Flag_Clr_Scr != Clr_Cmp_Val)
	{
		// MD resolution change. Clear the screen.
		vdraw_ddraw_clear_screen();
		Flag_Clr_Scr = Clr_Cmp_Val;
	}
	
	Dep = (320 - HPix);
	if (Dep == 0 || !(stretch & STRETCH_H))
	{
		if (vdraw_scale == 1)
		{
			RectSrc.left = 8 + 0;
			RectSrc.right = 8 + 320;
		}
		else
		{
			RectSrc.left = 0; //Upth-Modif - Was "0 * 2"
			RectSrc.right = 320 * vdraw_scale;
		}
	}
	else
	{
		if (vdraw_scale == 1)
		{
			RectSrc.left = 8 + (Dep / 2);
			RectSrc.right = 8 + (Dep / 2) + HPix;
		}
		else
		{
			RectSrc.left = (Dep / 2) * vdraw_scale;
			RectSrc.right = ((Dep / 2) + HPix) * vdraw_scale;
		}
	}
	
	// Check if we're in windowed mode.
	if (!vdraw_get_fullscreen())
	{
		// Windowed. Adjust for the window position.
		POINT p = {0, 0};
		ClientToScreen(gens_window, &p);
		
		RectDest.top += p.y; //Upth-Modif - this part moves the picture into the window
		RectDest.bottom += p.y; //Upth-Modif - I had to move it after all of the centering
		RectDest.left += p.x;   //Upth-Modif - because it modifies the values
		RectDest.right += p.x;  //Upth-Modif - that I use to find the center
	}
}


/**
 * vdraw_ddraw_flip(): Flip the screen buffer. [Called by vdraw_flip().]
 * @return 0 on success; non-zero on error.
 */
int vdraw_ddraw_flip(void)
{
	if (!lpDD)
		return -1;
	
	HRESULT rval = DD_OK;
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	RECT RectDest, RectSrc;
	int Dep = 0;
	const unsigned char bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	const uint8_t stretch = vdraw_get_stretch();
	
	if (vdraw_get_fullscreen())
	{
		RectDest.left = 0;
		RectDest.top = 0;
		RectDest.right = Res_X;
		RectDest.bottom = Res_Y;
		
		vdraw_ddraw_calc_draw_area(RectDest, RectSrc, Dep);
		
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
				
				int VBorder = VDP_Lines.Visible.Border_Size << m_shift;	// Top border height, in pixels.
				int HBorder = (Dep * (bytespp / 2)) << m_shift;			// Left border width, in pixels.
				int startPos = (ddsd.lPitch * VBorder) + HBorder;
				unsigned char* start = (unsigned char*)ddsd.lpSurface + startPos;
			
				Blit_FS(start, ddsd.lPitch, 320 - Dep, VDP_Lines.Visible.Total, 32 + (Dep * 2));
				
				lpDDS_Blit->Unlock(NULL);
				
				if (Video.VSync_FS)
				{
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
			}
			else
			{
				RectSrc.top = 0;
				RectSrc.bottom = VDP_Lines.Visible.Total;
				
				if ((VDP_Lines.Visible.Total == 224) && !stretch)
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
			// TODO: Test this with border color stuff.
			// Wine doesn't seem to have a 320x240 fullscreen mode available...
			if (vdraw_get_sw_render())
			{
				// Software rendering is enabled.
				// Ignore the Stretch setting.
				
				rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
				
				if (FAILED(rval))
					goto cleanup_flip;
				
				vdraw_rInfo.destScreen = (void*)ddsd.lpSurface;
				vdraw_rInfo.width = 320;
				vdraw_rInfo.height = 240;
				vdraw_rInfo.destPitch = ddsd.lPitch;
				
				vdraw_blitFS(&vdraw_rInfo);
				
				// Draw the text.
				vdraw_ddraw_draw_text(&ddsd, lpDDS_Blit, false);
				
				lpDDS_Blit->Unlock(NULL);
				
				if (Video.VSync_FS)
				{
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
			}
			else
			{
				// Software rendering is disabled.
				// TODO: Test this on a system that supports 1x in fullscreen on DirectDraw.
				
				vdraw_ddraw_draw_text(&ddsd, lpDDS_Back, true);
				if (Video.VSync_FS)
				{
					lpDDS_Flip->Blt(&RectDest, lpDDS_Back, &RectSrc, DDBLT_WAIT | DDBLT_ASYNC, NULL);
					lpDDS_Primary->Flip(NULL, DDFLIP_WAIT);
				}
				else
				{
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
			
			vdraw_rInfo.destScreen = (void*)ddsd.lpSurface;
			vdraw_rInfo.width = 320;
			vdraw_rInfo.height = 240;
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
			
			if (curBlit == lpDDS_Back) // note: this can happen in windowed fullscreen, or if CORRECT_256_ASPECT_RATIO is defined and the current display mode is 256 pixels across
			{
				vdraw_ddraw_calc_draw_area(RectDest, RectSrc, Dep);
				
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
		// Windowed mode.
		GetClientRect(gens_window, &RectDest);
		vdraw_ddraw_calc_draw_area(RectDest, RectSrc, Dep);
		
		if (vdraw_scale > 1)
		{
			rval = lpDDS_Blit->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			
			if (FAILED(rval))
				goto cleanup_flip;
			
			vdraw_rInfo.destScreen = (void*)ddsd.lpSurface;
			vdraw_rInfo.width = 320;
			vdraw_rInfo.height = 240;
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
int WINAPI vdraw_ddraw_restore_primary(void)
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
int WINAPI vdraw_ddraw_set_cooperative_level(void)
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

/***************************************************************************
 * Gens: Video Drawing - GDI Backend.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2009-2010 by David Korth                                  *
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
#include "vdraw_gdi.h"

// C includes.
#include <stdint.h>

// Message logging.
#include "macros/log_msg.h"

#include "emulator/g_main.hpp"

// Gens window.
#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"
#include "ui/gens_ui.hpp"
#include "debugger/debugger.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// RGB color conversion functions.
#include "vdraw_RGB.h"

// Audio Handler.
#include "audio/audio.h"

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>

// libgsft includes.
#include "libgsft/gsft_win32.h"


// Function prototypes.
static int	vdraw_gdi_init(void);
static int	vdraw_gdi_end(void);

static void	vdraw_gdi_clear_screen(void);

static int	vdraw_gdi_flip(void);
static void	vdraw_gdi_stretch_adjust(void);
static void	vdraw_gdi_update_renderer(void);
static int	vdraw_gdi_reinit_gens_window(void);

// Win32-specific functions.
static int WINAPI vdraw_gdi_clear_primary_screen(void);
static int WINAPI vdraw_gdi_clear_back_screen(void);
static int WINAPI vdraw_gdi_restore_primary(void) { return 0; }
static int WINAPI vdraw_gdi_set_cooperative_level(void) { return 0; }


// VDraw Backend struct.
const vdraw_backend_t vdraw_backend_gdi =
{
	.name			= "GDI",
	.flags			= VDRAW_BACKEND_FLAG_STRETCH,
	
	.init			= vdraw_gdi_init,
	.end			= vdraw_gdi_end,
	
	.init_subsystem		= NULL,
	.shutdown		= NULL,
	
	.clear_screen		= vdraw_gdi_clear_screen,
	.update_vsync		= NULL,
	
	.flip			= vdraw_gdi_flip,
	.stretch_adjust		= vdraw_gdi_stretch_adjust,
	.update_renderer	= vdraw_gdi_update_renderer,
	.reinit_gens_window	= vdraw_gdi_reinit_gens_window,
	
	// Win32-specific functions.
	.clear_primary_screen	= vdraw_gdi_clear_primary_screen,
	.clear_back_screen	= vdraw_gdi_clear_back_screen,
	.restore_primary	= vdraw_gdi_restore_primary,
	.set_cooperative_level	= vdraw_gdi_set_cooperative_level,
	.adjust_RectDest	= NULL
};


static HDC     hdcComp  = NULL;    // DC associado ao bitmap
static HBITMAP hbmpDraw = NULL;    // bitmap usado como buffer para imprimir a imagem
static LPBYTE  pbmpData = NULL;    // buffer
static SIZE    szGDIBuf;


// Stretch variables.
static int vdraw_gdi_stretch_flags;
static int vdraw_gdi_stretch_srcX;
static int vdraw_gdi_stretch_srcY;
static int vdraw_gdi_stretch_srcW;
static int vdraw_gdi_stretch_srcH;


/**
 * vdraw_gdi_init(): Initialize the GDI video subsystem.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_gdi_init(void)
{
	mdp_render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return 0;
	szGDIBuf.cx = 320 * scale;
	szGDIBuf.cy = 240 * scale;
	
	// Create the DC.
	hdcComp = CreateCompatibleDC(GetDC(gens_window));
	if (!hdcComp)
	{
		// Error creating the DC.
		DWORD dwErr = GetLastError();
		
		LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
			"CreateCompatibleDC() failed: 0x%08lX", dwErr);
		return -1;
	}
	
	// Create the DIB.
	// TODO: Currently always uses 16-bit color.
	// Maybe make it user-specified.
	typedef struct _BITMAPINFO_BI_BITFIELDS
	{
		BITMAPINFOHEADER bmiHeader;
		uint32_t bmiColors[3];
	} BITMAPINFO_BI_BITFIELDS;
	
	BITMAPINFO_BI_BITFIELDS bmi;
	memset(&bmi, 0x00, sizeof(bmi));
	bmi.bmiHeader.biSize	= sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biPlanes	= 1;
	bmi.bmiHeader.biWidth	= szGDIBuf.cx;
	bmi.bmiHeader.biHeight	= -szGDIBuf.cy;
	
	// Determine the bit count, compression, and color table values.
	switch (bppOut)
	{
		case 15:
			// 15-bit color. (555)
			bmi.bmiHeader.biBitCount = 16;
			bmi.bmiHeader.biCompression = BI_RGB;
			break;
		
		case 16:
			// 16-bit color. (565)
			// TODO: BI_BITFIELDS
			bmi.bmiHeader.biBitCount = 16;
			bmi.bmiHeader.biCompression = BI_BITFIELDS;
			bmi.bmiColors[0] = 0xF800; // Red
			bmi.bmiColors[1] = 0x07E0; // Green
			bmi.bmiColors[2] = 0x001F; // Blue
			break;
		
		case 32:
		default:
			// 32-bit color.
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			break;
	}
	
	// Create the DIB.
	hbmpDraw = CreateDIBSection(hdcComp, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (LPVOID*)&pbmpData, NULL, 0);
	if (!hbmpDraw)
	{
		// Error creating the DIB.
		DWORD dwErr = GetLastError();
		
		if (hdcComp)
		{
			DeleteDC(hdcComp);
			hdcComp = NULL;
		}
		
		LOG_MSG(video, LOG_MSG_LEVEL_ERROR,
			"CreateDIBSection() failed: 0x%08lX", dwErr);
		return -2;
	}
	
	// Select the bitmap object on the device context.
	SelectBitmap(hdcComp, hbmpDraw);
	
	// Adjust stretch parameters.
	vdraw_gdi_stretch_adjust();
	
	// GDI initialized.
	return 0;
}


/**
 * vdraw_gdi_end(): Close the GDI renderer.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_gdi_end(void)
{
	if (hdcComp)
	{
		DeleteDC(hdcComp);
		hdcComp = NULL;
	}
	
	if (hbmpDraw)
	{
		DeleteBitmap(hbmpDraw);
		hbmpDraw = NULL;
		pbmpData = NULL;
	}
	
	return 0;
}


/**
 * vdraw_gdi_clear_screen(): Clear the screen.
 */
static void vdraw_gdi_clear_screen(void)
{
	// Clear both screen buffers.
	vdraw_gdi_clear_primary_screen();
	vdraw_gdi_clear_back_screen();
}


/**
 * vdraw_gdi_clear_primary_screen(): Clear the primary screen.
 * @return 0 on success; non-zero on error.
 */
static int WINAPI vdraw_gdi_clear_primary_screen(void)
{
	HDC  hdcDest;
	RECT rectDest;
	
	hdcDest = GetDC(gens_window);
	InvalidateRect(gens_window, NULL, FALSE);
	GetClientRect(gens_window, &rectDest);
	FillRect(hdcDest, &rectDest, (HBRUSH)GetStockObject(BLACK_BRUSH));
	
	return 0;
}


/**
 * vdraw_gdi_clear_back_screen(): Clear the back buffer.
 * @return 0 on success; non-zero on error.
 */
static int WINAPI vdraw_gdi_clear_back_screen(void)
{
	if (!pbmpData)
		return -1;
	
	memset(pbmpData, 0, (szGDIBuf.cx << 1) * szGDIBuf.cy);
	return 0;
}


/**
 * vdraw_gdi_update_renderer(): Update the renderer.
 */
static void vdraw_gdi_update_renderer(void)
{
	// Check if a resolution switch is needed.
	mdp_render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return;
	const int w = 320 * scale;
	const int h = 240 * scale;
	
	if (szGDIBuf.cx == w && szGDIBuf.cy == h)
	{
		// No resolution switch is necessary. Simply clear the screen.
		vdraw_gdi_clear_screen();
		return;
	}
	
	// Resolution switch is needed.
	vdraw_gdi_end();
	vdraw_gdi_init();
	
	// Clear the screen.
	vdraw_gdi_clear_screen();
}


/**
 * vdraw_gdi_flip(): Flip the screen buffer. [Called by vdraw_flip().]
 * @return 0 on success; non-zero on error.
 */
static int vdraw_gdi_flip(void)
{
	const int bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	const int pitch = szGDIBuf.cx * bytespp;
	
	// Start of the GDI framebuffer.
	uint8_t *start = pbmpData;
	
	// Set up the render information.
	// TODO: Optimize rendering if using a stretch mode.
	vdraw_rInfo.destScreen = (void*)start;
	vdraw_rInfo.width = 320;
	vdraw_rInfo.height = 240;
	vdraw_rInfo.destPitch = pitch;
	
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
	
	// Draw the message and/or FPS counter.
	int msg_width = vdp_getHPix();
	start += ((pitch * VDP_Lines.Visible.Border_Size) * vdraw_scale);
	if (msg_width != 320)
		start += (((320 - msg_width) / 2) * bytespp * vdraw_scale);
	msg_width *= vdraw_scale;
	
	if (vdraw_msg_visible)
	{
		// Message is visible.
		draw_text(start, szGDIBuf.cx,
			  msg_width,
			  VDP_Lines.Visible.Total * vdraw_scale,
			  vdraw_msg_text, &vdraw_msg_style);
	}
	else if (vdraw_fps_enabled && (Game != NULL) && Settings.Active && !Settings.Paused && !IS_DEBUGGING())
	{
		// FPS is enabled.
		draw_text(start, szGDIBuf.cx,
			  msg_width,
			  VDP_Lines.Visible.Total * vdraw_scale,
			  vdraw_msg_text, &vdraw_fps_style);
	}
	
	// Get the device context.
	HDC hdcDest = GetDC(gens_window);
	
	// Get the window rectangle.
	RECT rectDest;
	GetClientRect(gens_window, &rectDest);
	
	// Blit the image to the GDI window.
	if (vdraw_gdi_stretch_flags == STRETCH_NONE)
	{
		BitBlt(hdcDest, rectDest.left, rectDest.top, szGDIBuf.cx, szGDIBuf.cy,
			hdcComp, 0, 0, SRCCOPY);
	}
	else
	{
		StretchBlt(hdcDest, rectDest.left, rectDest.top, szGDIBuf.cx, szGDIBuf.cy,
				hdcComp, vdraw_gdi_stretch_srcX, vdraw_gdi_stretch_srcY,
				vdraw_gdi_stretch_srcW, vdraw_gdi_stretch_srcH, SRCCOPY);
	}
	
	InvalidateRect(gens_window, NULL, FALSE);
	ReleaseDC(gens_window, hdcDest);
	return 0;
}


/**
 * vdraw_gdi_reinit_gens_window(): Reinitialize the Gens window.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_gdi_reinit_gens_window(void)
{
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	// Stop DirectDraw.
	vdraw_gdi_end();
	
	// Reinitialize the Gens window.
	gens_window_reinit();
	
	// Reinitialize DirectDraw.
	return vdraw_gdi_init();
}


/**
 * vdraw_gdi_stretch_adjust(): Adjust stretch parameters.
 * Called by either vdraw or another function in vdraw_gdi.
 */
static void vdraw_gdi_stretch_adjust(void)
{
	// Adjust the stretch parameters.
	vdraw_gdi_stretch_flags = vdraw_get_stretch();
	
	const int HPix = vdp_getHPix();
	if ((vdraw_gdi_stretch_flags & STRETCH_H) && HPix < 320)
	{
		// Horizontal stretch.
		vdraw_gdi_stretch_srcX = (vdraw_border_h / 2) * vdraw_scale;
		vdraw_gdi_stretch_srcW = HPix * vdraw_scale;
	}
	else
	{
		vdraw_gdi_stretch_srcX = 0;
		vdraw_gdi_stretch_srcW = 320 * vdraw_scale;
	}
		
	if (vdraw_gdi_stretch_flags & STRETCH_V)
	{
		// Vertical stretch.
		vdraw_gdi_stretch_srcY = VDP_Lines.Visible.Border_Size * vdraw_scale;
		vdraw_gdi_stretch_srcH = VDP_Lines.Visible.Total * vdraw_scale;
	}
	else
	{
		vdraw_gdi_stretch_srcY = 0;
		vdraw_gdi_stretch_srcH = 240 * vdraw_scale;
	}
}

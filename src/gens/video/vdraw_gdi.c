/***************************************************************************
 * Gens: Video Drawing - GDI Backend.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2009 by David Korth                                       *
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
#include "v_inline.h"

#include "emulator/g_main.hpp"

// Gens window.
#include "gens/gens_window.h"
#include "gens/gens_window_sync.hpp"
#include "ui/gens_ui.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// Video effects.
#include "v_effects.hpp"

// Audio Handler.
#include "audio/audio.h"

// Win32 includes.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>


// Function prototypes.
static int	vdraw_gdi_init(void);
static int	vdraw_gdi_end(void);

static void	vdraw_gdi_clear_screen(void);

static int	vdraw_gdi_flip(void);
static void	vdraw_gdi_draw_border(void); // Not used in vdraw_backend_t.
static void	vdraw_gdi_stretch_adjust(void);
static void	vdraw_gdi_update_renderer(void);

// Win32-specific functions.
static int	vdraw_gdi_reinit_gens_window(void);
static int	vdraw_gdi_clear_primary_screen(void);
static int	vdraw_gdi_clear_back_screen(void);
static int	vdraw_gdi_restore_primary(void) { return 0; }
static int	vdraw_gdi_set_cooperative_level(void) { return 0; }


// VDraw Backend struct.
const vdraw_backend_t vdraw_backend_gdi =
{
	.name = "GDI",
	.flags = VDRAW_BACKEND_FLAG_STRETCH,
	
	.init = vdraw_gdi_init,
	.end = vdraw_gdi_end,
	
	.init_subsystem = NULL,
	.shutdown = NULL,
	
	.clear_screen = vdraw_gdi_clear_screen,
	.update_vsync = NULL,
	
	.flip = vdraw_gdi_flip,
	.stretch_adjust = vdraw_gdi_stretch_adjust,
	.update_renderer = vdraw_gdi_update_renderer,
	
	// Win32-specific functions.
	.reinit_gens_window	= vdraw_gdi_reinit_gens_window,
	.clear_primary_screen	= vdraw_gdi_clear_primary_screen,
	.clear_back_screen	= vdraw_gdi_clear_back_screen,
	.restore_primary	= vdraw_gdi_restore_primary,
	.set_cooperative_level	= vdraw_gdi_set_cooperative_level
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
	// TODO: GDI doesn't support fullscreen.
	
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
		vdraw_init_fail("vdraw_gdi_init(): Error in CreateCompatibleDC().");
		return -1;
	}
	
	// Create the DIB.
	// TODO: Currently always uses 16-bit color.
	// Maybe make it user-specified.
	BITMAPINFOHEADER bih;
	memset(&bih, 0x00, sizeof(bih));
	bih.biSize	= sizeof(bih);
	bih.biPlanes	= 1;
	bih.biBitCount	= 16;
	bih.biWidth	= szGDIBuf.cx;
	bih.biHeight	= -szGDIBuf.cy;
	hbmpDraw = CreateDIBSection(hdcComp, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (LPVOID*)&pbmpData, NULL, 0);
	if (!hbmpDraw)
	{
		// Error creating the DIB.
		vdraw_init_fail("vdraw_gdi_init(): Error in CreateDIBSection().");
		return -2;
	}
	
	// Select the bitmap object on the device context.
	SelectBitmap(hdcComp, hbmpDraw);
	
	// Set bpp to 15-bit color.
	// GDI doesn't actually support 16-bit color.
	if (bppOut != 15)
		vdraw_set_bpp(15, FALSE);
	
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
static int vdraw_gdi_clear_primary_screen(void)
{
	if (!pbmpData)
		return -1;
	
	memset(pbmpData, 0, (szGDIBuf.cx << 1) * szGDIBuf.cy);
	
	// Reset the border color to make sure it's redrawn.
	vdraw_border_color_32 = ~MD_Palette32[0];
	
	return 0;
}


/**
 * vdraw_gdi_clear_back_screen(): Clear the back buffer.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_gdi_clear_back_screen(void)
{
	HDC  hdcDest;
	RECT rectDest;
	
	hdcDest = GetDC(gens_window);
	InvalidateRect(gens_window, NULL, FALSE);
	GetClientRect(gens_window, &rectDest);
	FillRect(hdcDest, &rectDest, (HBRUSH)GetStockObject(BLACK_BRUSH));
	
	// Reset the border color to make sure it's redrawn.
	vdraw_border_color_32 = ~MD_Palette32[0];
	
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
	HDC	hdcDest;
	RECT	rectDest;
	
	GetClientRect(gens_window, &rectDest);
	hdcDest = GetDC(gens_window);
	
	const int bytespp = (bppOut == 15 ? 2 : bppOut / 8);
	
	// Start of the GDI framebuffer.
	const int pitch = szGDIBuf.cx * bytespp;
	const int VBorder = (240 - VDP_Num_Vis_Lines) / 2;	// Top border height, in pixels.
	const int HBorder = vdraw_border_h * (bytespp / 2);	// Left border width, in pixels.
	
	const int startPos = ((pitch * VBorder) + HBorder) * vdraw_scale;	// Starting position from within the screen.
	
	// Start of the SDL framebuffer.
	unsigned char *start = &(((unsigned char*)(pbmpData))[startPos]);
	
	// Set up the render information.
	vdraw_rInfo.destScreen = (void*)start;
	vdraw_rInfo.width = 320 - vdraw_border_h;
	vdraw_rInfo.height = VDP_Num_Vis_Lines;
	vdraw_rInfo.destPitch = pitch;
	
	if (bppMD == 16 && bppOut != 16)
	{
		// MDP_RENDER_FLAG_SRC16DST32.
		// Render as 16-bit to an internal surface.
		
		// Make sure the internal surface is initialized.
		if (vdraw_16to32_scale != vdraw_scale)
		{
			if (vdraw_16to32_surface)
				free(vdraw_16to32_surface);
			
			vdraw_16to32_scale = vdraw_scale;
			vdraw_16to32_pitch = 320 * vdraw_scale * 2;
			vdraw_16to32_surface = (uint16_t*)(malloc(vdraw_16to32_pitch * 240 * vdraw_scale));
		}
		
		vdraw_rInfo.destScreen = (void*)vdraw_16to32_surface;
		vdraw_rInfo.destPitch = vdraw_16to32_pitch;
		if (vdraw_get_fullscreen())
			vdraw_blitFS(&vdraw_rInfo);
		else
			vdraw_blitW(&vdraw_rInfo);
		
		// Apply the pause tint, if necessary.
		if (Paused && Video.pauseTint)
		{
			veffect_pause_tint(&vdraw_rInfo, vdraw_scale);
		}
		
		vdraw_render_16to32((uint32_t*)start, vdraw_16to32_surface,
				    vdraw_rInfo.width * vdraw_scale, vdraw_rInfo.height * vdraw_scale,
				    pitch, vdraw_16to32_pitch);
	}
	else
	{
		if (vdraw_get_fullscreen())
			vdraw_blitFS(&vdraw_rInfo);
		else
			vdraw_blitW(&vdraw_rInfo);
		
		// Apply the pause tint, if necessary.
		if (Paused && Video.pauseTint)
		{
			veffect_pause_tint(&vdraw_rInfo, vdraw_scale);
		}
	}
	
	// Draw the border.
	vdraw_gdi_draw_border();
	
	// Draw the message and/or FPS counter.
	if (vdraw_msg_visible)
	{
		// Message is visible.
		draw_text(start, szGDIBuf.cx,
			  vdraw_rInfo.width * vdraw_scale,
			  vdraw_rInfo.height * vdraw_scale,
			  vdraw_msg_text, &vdraw_msg_style, FALSE);
	}
	else if (vdraw_fps_enabled && (Game != NULL) && Active && !Paused && !Debug)
	{
		// FPS is enabled.
		draw_text(start, szGDIBuf.cx,
			  vdraw_rInfo.width * vdraw_scale,
			  vdraw_rInfo.height * vdraw_scale,
			  vdraw_msg_text, &vdraw_fps_style, FALSE);
	}
	
	// Blit the image to the GDI window.
	if (vdraw_gdi_stretch_flags == STRETCH_NONE)
	{
		BitBlt(hdcDest, rectDest.left, rectDest.top, szGDIBuf.cx, szGDIBuf.cy, hdcComp, 0, 0, SRCCOPY);
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
int vdraw_gdi_reinit_gens_window(void)
{
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	// Stop DirectDraw.
	vdraw_gdi_end();
	
	// Rebuild the menu bar.
	// This is needed if the mode is switched from windowed to fullscreen, or vice-versa.
	gens_window_create_menubar();
	
	mdp_render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return -1;
	const int w = 320 * scale;
	const int h = 240 * scale;
	
	if (vdraw_get_fullscreen())
	{
		while (ShowCursor(TRUE) < 1) { }
		while (ShowCursor(FALSE) >= 0) { }
		
		SetWindowLongPtr(gens_window, GWL_STYLE, (LONG_PTR)(NULL));
		SetWindowPos(gens_window, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		while (ShowCursor(FALSE) >= 0) { }
		while (ShowCursor(TRUE) < 1) { }
		
		// MoveWindow / ResizeWindow code
		LONG_PTR curStyle = GetWindowLongPtr(gens_window, GWL_STYLE);
		SetWindowLongPtr(gens_window, GWL_STYLE, (LONG_PTR)(curStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPos(gens_window, NULL, Window_Pos.x, Window_Pos.y, 0, 0,
			     SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		Win32_setActualWindowSize(gens_window, w, h);
	}
	
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
	
	if ((vdraw_gdi_stretch_flags & STRETCH_H) && !isFullXRes())
	{
		// Horizontal stretch.
		vdraw_gdi_stretch_srcX = ((320 - 256) / 2) * vdraw_scale;
		vdraw_gdi_stretch_srcW = 256 * vdraw_scale;
	}
	else
	{
		vdraw_gdi_stretch_srcX = 0;
		vdraw_gdi_stretch_srcW = 320 * vdraw_scale;
	}
		
	if (vdraw_gdi_stretch_flags & STRETCH_V)
	{
		// Vertical stretch.
		vdraw_gdi_stretch_srcY = ((240 - VDP_Num_Vis_Lines) / 2) * vdraw_scale;
		vdraw_gdi_stretch_srcH = VDP_Num_Vis_Lines * vdraw_scale;
	}
	else
	{
		vdraw_gdi_stretch_srcY = 0;
		vdraw_gdi_stretch_srcH = 240 * vdraw_scale;
	}
}


/**
 * vdraw_gdi_draw_border(): Draw the border color.
 * Called from vdraw_gdi_flip().
 */
static void vdraw_gdi_draw_border(void)
{
	if (!Video.borderColorEmulation)
	{
		// Border color emulation is disabled.
		// Don't do anything if the border color is currently black.
		if (vdraw_border_color_32 == 0)
			return;
	}
	
	unsigned int new_border_color_32 = MD_Palette32[0];
	if (!Video.borderColorEmulation || (Game == NULL) || (Debug > 0))
	{
		// Either no game is loaded or the debugger is enabled.
		// Make sure the border color is black.
		new_border_color_32 = 0;
	}
	
	if (vdraw_border_color_32 != new_border_color_32)
	{
		vdraw_border_color_32 = new_border_color_32;
		const uint8_t stretch = vdraw_get_stretch();
		
		// MD color has R and B channels swapped from Windows GDI.
		uint8_t r = (vdraw_border_color_32 >> 16) & 0xFF;
		uint8_t g = (vdraw_border_color_32 >> 8) & 0xFF;
		uint8_t b = (vdraw_border_color_32) & 0xFF;
		HBRUSH hbrBorder = CreateSolidBrush(RGB(r, g, b));
		
		RECT rectBorder;
		
		if (VDP_Num_Vis_Lines < 240 && !(stretch & STRETCH_V))
		{
			// Top/Bottom borders.
			rectBorder.left = 0;
			rectBorder.right = szGDIBuf.cx;
			
			rectBorder.top = 0;
			rectBorder.bottom = 8 * vdraw_scale;
			FillRect(hdcComp, &rectBorder, hbrBorder);
			
			rectBorder.top = szGDIBuf.cy - (8 * vdraw_scale);
			rectBorder.bottom = szGDIBuf.cy;
			FillRect(hdcComp, &rectBorder, hbrBorder);
		}
		
		if (!isFullXRes() && !(stretch & STRETCH_H))
		{
			// Left/Right borders.
			rectBorder.top = 0;
			rectBorder.bottom = szGDIBuf.cy;
			
			rectBorder.left = 0;
			rectBorder.right = 32 * vdraw_scale;
			FillRect(hdcComp, &rectBorder, hbrBorder);
			
			rectBorder.left = szGDIBuf.cx - (32 * vdraw_scale);
			rectBorder.right = szGDIBuf.cx;
			FillRect(hdcComp, &rectBorder, hbrBorder);
		}
		
		// Delete the brush.
		DeleteBrush(hbrBorder);
	}
}

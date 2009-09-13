/***************************************************************************
 * Gens: Video Drawing - Text Drawing.                                     *
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

#include "vdraw_text.hpp"
#include "vdraw.h"
#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"

// C includes.
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// C++ includes.
#include <list>
using std::list;

// Video Drawing.
#include "vdraw.h"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Inline video functions.
#include "v_inline.h"

// MDP Render.
#include "mdp/mdp_render.h"

// Plugin Manager and Render Manager.
#include "plugins/pluginmgr.hpp"
#include "plugins/rendermgr.hpp"

// Commodore 64 character set.
#include "C64_charset.h"


 // Text drawing values.
static uint32_t m_Transparency_Mask;

// Message timer.
#include "port/timer.h"
static uint32_t	vdraw_msg_time = 0;

// Text buffer.
char vdraw_msg_text[1024];

// Prerendered text for the on-screen display.
// 1x: Each byte represents 8 pixels, or one line for a character.
// 2x: Each word represents 16 pixels, or one line for a character.
// TODO: Consolidate 1x and 2x.
uint8_t vdraw_msg_prerender_1x[8][1024];
uint16_t vdraw_msg_prerender_2x[8][1024];


/**
 * vdraw_text_prerender(): Prerender the onscreen text to the character buffer.
 */
static void vdraw_text_prerender(void)
{
	// Prerender the text.
	
	// TODO: This only supports 1x rendering.
	// Do 2x rendering later.
	
	unsigned int len = strlen(vdraw_msg_text);
	
	for (unsigned int chr = 0; chr < len; chr++)
	{
		for (unsigned int row = 0; row < 8; row++)
		{
			unsigned char chr_data = C64_charset[vdraw_msg_text[chr]][row];
			
			// 1x prerender.
			vdraw_msg_prerender_1x[row][chr] = chr_data;
			
			// 2x prerender.
			// Double each pixel's width.
			vdraw_msg_prerender_2x[row][chr] = 0x00;
			for (unsigned int px = 8; px != 0; px--)
			{
				if (chr_data & 0x80)
					vdraw_msg_prerender_2x[row][chr] |= 0x03;
				
				vdraw_msg_prerender_2x[row][chr] <<= 2;
				chr_data <<= 1;
			}
		}
	}
}


template<typename pixel, typename prerender_buf, unsigned int charSize, unsigned int hiPixMask, unsigned int cyShift>
static inline void drawStr_preRender(pixel *screen, const int pitch, const int x, const int y,
					const unsigned int msgWidth, const unsigned int numLines,
					const vdraw_style_t *style, const pixel transparentMask,
					const char *str, const prerender_buf str_prerender)
{
	pixel *screen_start = &screen[y*pitch + x];
	const unsigned int chars_per_line = (msgWidth / charSize);
	unsigned int chr_offset = 0;
	
	for (unsigned int line = 0; line < (numLines * charSize); line += charSize)
	{
		for (unsigned int cy = 0; cy < charSize; cy++)
		{
			pixel *screen_pos = screen_start + ((cy + line) * pitch);
			for (unsigned int cx = 0; cx < chars_per_line; cx++)
			{
				if (str[cx + chr_offset] == 0x00)
				{
					// End of the string.
					break;
				}
				
				unsigned int cRow = str_prerender[cy >> cyShift][cx + chr_offset];
				if (cRow == 0x00)
				{
					// Empty line.
					screen_pos += charSize;
					continue;
				}
				
				for (unsigned int px = charSize; px != 0; px--)
				{
					if (cRow & hiPixMask)
					{
						// Dot is opaque. Draw it.
						// TODO: Original asm version had transparency in a separate function for performance.
						// See if that would actually help.
						// TODO: The transparency method used here might be slow on DDraw when using video memory.
						if (!style->transparent)
							*screen_pos = style->dot_color;
						else
							*screen_pos = ((style->dot_color & transparentMask) >> 1) +
								      ((*screen_pos & transparentMask) >> 1);
					}
					cRow <<= 1;
					screen_pos++;
				}
			}
		}
		
		chr_offset += chars_per_line;
	}
}


template<typename pixel>
static inline void T_drawText(pixel *screen, const int pitch, const int w, const int h,
			      const char *msg, const pixel transparentMask, const vdraw_style_t *style,
			      const bool isDDraw)
{
	int cPos;
	unsigned int x, y, cx, cy;
	unsigned int charSize;
	
	const bool fullScreen = vdraw_get_fullscreen();
	const list<mdp_render_t*>::iterator& rendMode = (fullScreen ? rendMode_FS : rendMode_W);
	
	// The message must be specified.
	if (!msg)
		return;
	
	// Get the message length.
	const unsigned int msgLength = strlen(msg);
	if (msgLength == 0)
		return;
	
	// Character size
	if (style->double_size)
		charSize = 16;
	else
		charSize = 8;
	
	// Bottom-left of the screen.
	x = 8;
	y = h;
	
#if defined(GENS_OS_WIN32)
	if (isDDraw)
	{
		// Check if the text position needs to be adjusted.
		if (vdraw_scale == 1)
		{
			// With the DirectDraw renderer, the vertical shift is weird
			// in normal (1x) rendering.
			
			if (fullScreen && vdraw_get_sw_render())
			{
				// Software rendering.
				x = (isFullXRes() ? 0 : (32 * vdraw_scale));
				
				// Adjust the vertical position, if necessary.
				if (VDP_Num_Vis_Lines < 240)
				{
					y += (((240 - VDP_Num_Vis_Lines) / 2) * vdraw_scale);
				}
			}
		}
		else
		{
			// 2x or higher.
			
			// For whatever reason, text is always shifted over 8 pixels
			// when not using Normal rendering.
			x = (isFullXRes() ? 0 : (32 * vdraw_scale));
			
			// Adjust the vertical position, if necessary.
			if (VDP_Num_Vis_Lines < 240)
			{
				y += (((240 - VDP_Num_Vis_Lines) / 2) * vdraw_scale);
			}
		}
	}
#endif /* defined(GENS_OS_WIN32) */
	
	// Character size is 8x8 normal, 16x16 double.
	y -= (8 + charSize);
	
	// Determine how many linebreaks are needed.
	const unsigned int msgWidth = w - 16;
	const unsigned short lineBreaks = ((msgLength - 1) * charSize) / msgWidth;
	y -= (lineBreaks * charSize);
	
	vdraw_style_t textShadowStyle = *style;
	textShadowStyle.dot_color = 0;
	
	cx = x; cy = y;
	if (style->double_size)
	{
		// 2x text rendering.
		// TODO: Make text shadow an option.
		drawStr_preRender<pixel, uint16_t[8][1024], 16, 0x8000, 1>
					(screen, pitch, cx+1, cy+1, msgWidth, (lineBreaks + 1),
					 &textShadowStyle, transparentMask,
					 msg, vdraw_msg_prerender_2x);
		
		drawStr_preRender<pixel, uint16_t[8][1024], 16, 0x8000, 1>
					(screen, pitch, cx, cy, msgWidth, (lineBreaks + 1),
					 style, transparentMask,
					 msg, vdraw_msg_prerender_2x);
	}
	else
	{
		// 1x text rendering.
		// TODO: Make text shadow an option.
		drawStr_preRender<pixel, uint8_t[8][1024], 8, 0x80, 0>
					(screen, pitch, cx+1, cy+1, msgWidth, (lineBreaks + 1),
					 &textShadowStyle, transparentMask,
					 msg, vdraw_msg_prerender_1x);
		
		drawStr_preRender<pixel, uint8_t[8][1024], 8, 0x80, 0>
					(screen, pitch, cx, cy, msgWidth, (lineBreaks + 1),
					 style, transparentMask,
					 msg, vdraw_msg_prerender_1x);
	}
}


/**
 * draw_text(): Draw text to the screen.
 * @param screen Pointer to screen surface.
 * @param pitch Pitch of the screen surface (in pixels).
 * @param w Width of the viewable area of the screen surface (in pixels).
 * @param h Height of the viewable area of the screen surface (in pixels).
 * @param msg Text to draw to the screen.
 * @param style Pointer to style information.
 * @param DDRAW_adjustForScreenSize
 */
void draw_text(void *screen, const int pitch, const int w, const int h,
	       const char *msg, const vdraw_style_t *style,
	       const BOOL isDDraw)
{
	if (!style)
		return;
	
	if (bppOut == 15 || bppOut == 16)
	{
		// 15/16-bit color.
		T_drawText((uint16_t*)screen, pitch, w, h, msg,
			   (uint16_t)m_Transparency_Mask, style,
			   (isDDraw ? true : false));
	}
	else //if (bppOut == 32)
	{
		// 32-bit color.
		T_drawText((uint32_t*)screen, pitch, w, h, msg,
			   (uint32_t)m_Transparency_Mask, style,
			   (isDDraw ? true : false));
	}
}


/**
 * calc_text_style(): Calculate a text style.
 * @param style Pointer to text style.
 */
void calc_text_style(vdraw_style_t *style)
{
	if (!style)
		return;
	
	// Calculate the dot color.
	if (bppOut == 15)
	{
		switch (style->style & 0x07)
		{
			case STYLE_COLOR_RED:
				style->dot_color = 0x7C00;
				break;
			case STYLE_COLOR_GREEN:
				style->dot_color = 0x03E0;
				break;
			case STYLE_COLOR_BLUE:
				style->dot_color = 0x001F;
				break;
			default: // STYLE_COLOR_WHITE
				style->dot_color = 0x7FFF;
				break;
		}
	}
	else if (bppOut == 16)
	{
		switch (style->style & 0x07)
		{
			case STYLE_COLOR_RED:
				style->dot_color = 0xF800;
				break;
			case STYLE_COLOR_GREEN:
				style->dot_color = 0x7E00;
				break;
			case STYLE_COLOR_BLUE:
				style->dot_color = 0x001F;
				break;
			default: // STYLE_COLOR_WHITE
				style->dot_color = 0xFFFF;
				break;
		}
	}
	else //if (bppOut == 32)
	{
		switch (style->style & 0x07)
		{
			case STYLE_COLOR_RED:
				style->dot_color = 0xFF0000;
				break;
			case STYLE_COLOR_GREEN:
				style->dot_color = 0x00FF00;
				break;
			case STYLE_COLOR_BLUE:
				style->dot_color = 0x0000FF;
				break;
			default: // STYLE_COLOR_WHITE
				style->dot_color = 0xFFFFFF;
				break;
		}
	}
	
	style->double_size = ((style->style & STYLE_DOUBLESIZE) ? TRUE : FALSE);
	style->transparent = ((style->style & STYLE_TRANSPARENT) ? TRUE : FALSE);
}


/**
 * calc_transparency_mask(): Calculate the transparency mask.
 */
void calc_transparency_mask(void)
{
	if (bppOut == 15)
		m_Transparency_Mask = 0x7BDE;
	else if (bppOut == 16)
		m_Transparency_Mask = 0xF7DE;
	else //if (bppOut == 32)
		m_Transparency_Mask = 0xFEFEFE;
}


/**
 * vdraw_text_write(): Write text to the screen.
 * @param msg Message to write.
 * @param duration Duration for the message to appear, in milliseconds. (If <=0, the message won't disappear.)
 */
void vdraw_text_write(const char* msg, const int duration)
{
	if (!vdraw_get_msg_enabled())
		return;
	
	// TODO: Add localization.
	strncpy(vdraw_msg_text, msg, sizeof(vdraw_msg_text));
	vdraw_msg_text[sizeof(vdraw_msg_text) - 1] = 0x00;
	
	// Prerender the text.
	vdraw_text_prerender();
	
	if (duration > 0)
	{
		// Set the message timer.
		vdraw_msg_time = GetTickCount() + duration;
		vdraw_msg_visible = true;
	}
	
	// Force a wakeup.
	GensUI::wakeup();
}


/**
 * vdraw_text_printf(): Print formatted text to the screen.
 * @param duration Duration for the message to appear, in milliseconds. (If <=0, the message won't disappear.)
 * @param msg Message to write. (printf-formatted)
 * @param ... Format arguments.
 */
void vdraw_text_printf(const int duration, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vdraw_text_vprintf(duration, msg, ap);
	va_end(ap);
}


/**
 * vdraw_text_vprintf(): Print formatted text to the screen. (va_list version)
 * @param duration Duration for the message to appear, in milliseconds. (If <=0, the message won't disappear.)
 * @param msg Message to write. (printf-formatted)
 * @param ap Format arguments.
 */
void vdraw_text_vprintf(const int duration, const char* msg, va_list ap)
{
	if (!vdraw_get_msg_enabled())
		return;
	
	// TODO: Add localization.
	vsnprintf(vdraw_msg_text, sizeof(vdraw_msg_text), msg, ap);
	vdraw_msg_text[sizeof(vdraw_msg_text) - 1] = 0x00;
	
	// Prerender the text.
	vdraw_text_prerender();
	
	if (duration > 0)
	{
		// Set the message timer.
		vdraw_msg_time = GetTickCount() + duration;
		vdraw_msg_visible = true;
	}
	
	// Force a wakeup.
	GensUI::wakeup();
}


/**
 * vdraw_msg_timer_update(): Update the message timer.
 */
void vdraw_msg_timer_update(void)
{
	if (GetTickCount() > vdraw_msg_time)
	{
		vdraw_msg_visible = false;
		vdraw_msg_text[0] = 0x00;
		
		// Force a wakeup.
		GensUI::wakeup();
	}
}

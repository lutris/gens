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

// libgsft includes.
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"

// Video Drawing.
#include "vdraw.h"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// MDP Render.
#include "mdp/mdp_render.h"

// Plugin Manager and Render Manager.
#include "plugins/pluginmgr.hpp"
#include "plugins/rendermgr.hpp"

// OSD character set.
#include "osd_charset.hpp"

// Message timer.
#include "port/timer.h"
static uint32_t	vdraw_msg_time = 0;

// Text buffer.
char vdraw_msg_text[1024];

// Prerendered text for the on-screen display.
// Each byte represents 8 pixels, or one line for a character.
static uint8_t vdraw_msg_prerender[16][1024];
static unsigned int vdraw_msg_prerender_len;


template<typename pixel, unsigned int charSize, bool do2x>
static inline void drawStr_preRender(pixel *screen, const int pitch, const int x, const int y,
					const unsigned int msgWidth, const unsigned int numLines,
					const vdraw_style_t *style)
{
	pixel *screen_start = &screen[y*pitch + x];
	const unsigned int chars_per_line = (msgWidth / charSize);
	unsigned int chr_offset = 0;
	
	for (unsigned int line = 0; line < (numLines * charSize * 2); line += (charSize * 2))
	{
		for (unsigned int cy = 0; cy < (charSize * 2); cy++)
		{
			pixel *screen_pos, *screen_pos_sh;
			if (do2x)
			{
				// 2x text drawing.
				screen_pos = screen_start - 1 + ((cy + line - 1) * pitch);
				screen_pos_sh = screen_start + 1 + ((cy + line + 1) * pitch);
			}
			else
			{
				// 1x text drawing.
				screen_pos = screen_start + ((cy + line) * pitch);
				screen_pos_sh = screen_start + 1 + ((cy + line + 1) * pitch);
			}
			
			for (unsigned int cx = 0; cx < chars_per_line; cx++)
			{
				if (cx + chr_offset >= vdraw_msg_prerender_len)
				{
					// End of string.
					break;
				}
				
				unsigned char cRow = vdraw_msg_prerender[cy >> (do2x ? 1 : 0)][cx + chr_offset];
				if (cRow == 0x00)
				{
					// Empty line.
					screen_pos += charSize;
					screen_pos_sh += charSize;
					continue;
				}
				
				for (unsigned int px = 8; px != 0; px--)
				{
					if (cRow & 0x80)
					{
						// Dot is opaque. Draw it.
						if (do2x)
						{
							*screen_pos = style->dot_color;
							*(screen_pos + 1) = style->dot_color;
							*screen_pos_sh = 0;
							*(screen_pos_sh + 1) = 0;
						}
						else
						{
							*screen_pos = style->dot_color;
							*screen_pos_sh = 0;
						}
					}
					
					cRow <<= 1;
					
					if (do2x)
					{
						screen_pos += 2;
						screen_pos_sh += 2;
					}
					else
					{
						screen_pos++;
						screen_pos_sh++;
					}
				}
			}
		}
		
		chr_offset += chars_per_line;
	}
}


template<typename pixel>
static inline void T_drawText(pixel *screen, const int pitch, const int w, const int h,
			      const char *msg, const vdraw_style_t *style)
{
	unsigned int x, y;
	unsigned int charSize;
	
	// The message must be specified.
	if (!msg)
		return;
	
	// Message must be longer than 0 characters.
	if (vdraw_msg_prerender_len == 0)
		return;
	
	// Character size
	if (style->double_size)
		charSize = 16;
	else
		charSize = 8;
	
	// Bottom-left of the screen.
	x = 8;
	y = h;
	
	// Character size is 8x16 normal, 16x32 double.
	y -= (8 + (charSize * 2));
	
	// Determine how many linebreaks are needed.
	const unsigned int msgWidth = w - 16;
	const unsigned short lineBreaks = ((vdraw_msg_prerender_len - 1) * charSize) / msgWidth;
	y -= (lineBreaks * (charSize * 2));
	
	vdraw_style_t textShadowStyle = *style;
	textShadowStyle.dot_color = 0;
	
	if (style->double_size)
	{
		// 2x text rendering.
		drawStr_preRender<pixel, 16, true>(screen, pitch, x, y, msgWidth, (lineBreaks + 1), style);
	}
	else
	{
		// 1x text rendering.
		// TODO: Make text shadow an option.
		drawStr_preRender<pixel, 8, false>(screen, pitch, x, y, msgWidth, (lineBreaks + 1), style);
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
 */
void draw_text(void *screen, const int pitch, const int w, const int h,
	       const char *msg, const vdraw_style_t *style)
{
	if (!style)
		return;
	
	if (bppOut == 15 || bppOut == 16)
	{
		// 15/16-bit color.
		T_drawText((uint16_t*)screen, pitch, w, h, msg, style);
	}
	else //if (bppOut == 32)
	{
		// 32-bit color.
		T_drawText((uint32_t*)screen, pitch, w, h, msg, style);
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
	
	// If we're not using 32-bit color, calculate the 15-bit or 16-bit dot color.
	uint32_t color32 = style->color;
	if (bppOut == 15)
	{
		// 15-bit. (RGB555)
		style->dot_color = ((color32 >> 9) & 0x7C00) |
				   ((color32 >> 6) & 0x03E0) |
				   ((color32 >> 3) & 0x001F);
	}
	else if (bppOut == 16)
	{
		// 16-bit. (RGB565)
		style->dot_color = ((color32 >> 8) & 0xF800) |
				   ((color32 >> 5) & 0x07E0) |
				   ((color32 >> 3) & 0x001F);
	}
	else
	{
		// 32-bit. Use the original color.
		style->dot_color = color32;
	}
	
	style->double_size = ((style->style & STYLE_DOUBLESIZE) ? TRUE : FALSE);
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
	strlcpy(vdraw_msg_text, msg, sizeof(vdraw_msg_text));
	
	// Prerender the text.
	vdraw_msg_prerender_len = osd_charset_prerender(vdraw_msg_text, vdraw_msg_prerender);
	
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
	vszprintf(vdraw_msg_text, sizeof(vdraw_msg_text), msg, ap);
	
	// Prerender the text.
	vdraw_msg_prerender_len = osd_charset_prerender(vdraw_msg_text, vdraw_msg_prerender);
	
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
 * vdraw_text_clear(): Clear the message text.
 */
void vdraw_text_clear(void)
{
	vdraw_msg_text[0] = 0x00;
	vdraw_msg_prerender_len = 0;
}


/**
 * vdraw_msg_timer_update(): Update the message timer.
 */
void vdraw_msg_timer_update(void)
{
	if (GetTickCount() <= vdraw_msg_time)
		return;
	
	// Message timer has expired. Clear the message.
	vdraw_msg_visible = false;
	vdraw_text_clear();
	
	// Force a wakeup.
	GensUI::wakeup();
}

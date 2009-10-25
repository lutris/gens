/***************************************************************************
 * Gens: Palette Handler.                                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#ifndef GENS_MD_PALETTE_HPP
#define GENS_MD_PALETTE_HPP

#ifdef __cplusplus
extern "C" {
#endif

extern int Contrast_Level;
extern int Brightness_Level;
extern int Greyscale;
extern int Invert_Color;

typedef enum _ColorScaleMethod_t
{
	COLSCALE_RAW = 0,	// Raw colors: 0xEEE -> 0xE0E0E0
	COLSCALE_FULL = 1,	// Full colors: 0xEEE -> 0xFFFFFF
	COLSCALE_FULL_HS = 2,	// Full colors with Highlight/Shadow: 0xEEE -> 0xEEEEEE for highlight
} ColorScaleMethod_t;

extern ColorScaleMethod_t ColorScaleMethod;

void Recalculate_Palettes(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MD_PALETTE_HPP */

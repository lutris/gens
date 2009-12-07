/***************************************************************************
 * Gens: On-Screen Display Font Handler.                                   *
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

#ifndef GENS_OSD_FONT_H
#define GENS_OSD_FONT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union
{
	uint8_t *p_u8;
	uint16_t *p_u16;
	void *p_v;
} osd_ptr_t;

// Character flags.
#define OSD_FLAG_HALFWIDTH	0
#define OSD_FLAG_FULLWIDTH	(1 << 0)

extern osd_ptr_t osd_font_data[65536];	// Pointers to character data.
extern uint8_t osd_font_flags[65536];	// Character flags.

void osd_font_clear(void);
void osd_font_init_ASCII(void);
int osd_font_load(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* GENS_OSD_FONT_H */

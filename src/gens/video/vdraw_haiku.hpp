/***************************************************************************
 * Gens: Video Drawing - Haiku Backend.                                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
 * Copyright (c) 2009 by Phil Costin                                       *
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

#ifndef GENS_VDRAW_HAIKU_HPP
#define GENS_VDRAW_HAIKU_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "libgsft/gsft_bool.h"

// Function prototypes.
int	vdraw_haiku_init(void);
int	vdraw_haiku_end(void);

int	vdraw_haiku_flip(void);
void vdraw_haiku_clear_screen(void);
void vdraw_haiku_update_vsync(const BOOL fromInitHaiku);
int	vdraw_haiku_reinit_gens_window(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_VDRAW_HAIKU_HPP */

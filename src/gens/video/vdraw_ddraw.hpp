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

#ifndef GENS_VDRAW_DDRAW_HPP
#define GENS_VDRAW_DDRAW_HPP

#include "libgsft/gsft_bool.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes.
int	vdraw_ddraw_init(void);
int	vdraw_ddraw_end(void);

int	vdraw_ddraw_flip(void);
void	vdraw_ddraw_clear_screen(void);
void	vdraw_ddraw_update_vsync(const BOOL fromInitDDraw);
int	vdraw_ddraw_reinit_gens_window(void);

// Win32-specific functions.
int	vdraw_ddraw_clear_primary_screen(void);
int	vdraw_ddraw_clear_back_screen(void);
int	vdraw_ddraw_restore_primary(void);
int	vdraw_ddraw_set_cooperative_level(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_VDRAW_DDRAW_HPP */

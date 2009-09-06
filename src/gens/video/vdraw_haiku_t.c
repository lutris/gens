/***************************************************************************
 * Gens: Video Drawing - Hailu Backend.                                    *
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

#include "vdraw_haiku.hpp"
#include "vdraw_haiku_t.h"

// VDraw Backend struct.
const vdraw_backend_t vdraw_backend_haiku =
{
	.name			= "BDirectWindow",
	.flags			= 0x0,
	
	.init			= vdraw_haiku_init,
	.end			= vdraw_haiku_end,
	
	.init_subsystem		= NULL,
	.shutdown		= NULL,
	
	.clear_screen		= vdraw_haiku_clear_screen,
	.update_vsync		= vdraw_haiku_update_vsync,
	
	.flip = vdraw_haiku_flip,
	.stretch_adjust = NULL,
	.update_renderer = NULL,
	.reinit_gens_window	= vdraw_haiku_reinit_gens_window
};

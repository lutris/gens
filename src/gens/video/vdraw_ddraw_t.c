/***************************************************************************
 * Gens: Video Drawing - DirectDraw Backend. (vdraw_backend_t struct)      *
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

#include "vdraw_ddraw_t.h"
#include "vdraw_ddraw.hpp"

// VDraw Backend struct.
const vdraw_backend_t vdraw_backend_ddraw =
{
	.name			= "DirectDraw 4",
	.flags			= VDRAW_BACKEND_FLAG_VSYNC | \
				  VDRAW_BACKEND_FLAG_STRETCH |
				  VDRAW_BACKEND_FLAG_FULLSCREEN |
				  VDRAW_BACKEND_FLAG_WINRESIZE,
	
	.init			= vdraw_ddraw_init,
	.end			= vdraw_ddraw_end,
	
	.init_subsystem		= NULL,
	.shutdown		= NULL,
	
	.clear_screen		= vdraw_ddraw_clear_screen,
	.update_vsync		= vdraw_ddraw_update_vsync,
	
	.flip = vdraw_ddraw_flip,
	.stretch_adjust = NULL,
	.update_renderer = NULL,
	.reinit_gens_window	= vdraw_ddraw_reinit_gens_window,
	
	// Win32-specific functions.
	.clear_primary_screen	= vdraw_ddraw_clear_primary_screen,
	.clear_back_screen	= vdraw_ddraw_clear_back_screen,
	.restore_primary	= vdraw_ddraw_restore_primary,
	.set_cooperative_level	= vdraw_ddraw_set_cooperative_level
};

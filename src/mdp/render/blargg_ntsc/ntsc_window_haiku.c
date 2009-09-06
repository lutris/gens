/***************************************************************************
 * MDP: Blargg's NTSC Filter. (Window Code) (Haiku)                        *
 *                                                                         *
 * Copyright (c) 2006 by Shay Green                                        *
 * MDP version Copyright (c) 2008-2009 by David Korth                      *
 * MDP version Copyright (c) 2009 by Phil Costin                           *
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

// C includes.
#include <math.h>
#include <stdio.h>

#include "ntsc_window.h"
#include "ntsc_window_common.h"

#include "mdp_render_blargg_ntsc.h"
#include "mdp_render_blargg_ntsc_plugin.h"
#include "md_ntsc.hpp"

// MDP error codes.
#include "mdp/mdp_error.h"

// Callbacks.

/**
 * ntsc_window_show(): Show the NTSC Plugin Options window.
 * @param parent Parent window.
 */
void ntsc_window_show(void *parent)
{
}


/**
 * ntsc_window_close(): Close the VDP Layer Options window.
 */
void ntsc_window_close(void)
{
}

/**
 * ntsc_window_load_settings(): Load the NTSC settings.
 */
void MDP_FNCALL ntsc_window_load_settings(void)
{
}

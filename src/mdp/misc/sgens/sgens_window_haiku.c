/***************************************************************************
 * MDP: Sonic Gens. (Window Code) (Haiku)                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * SGens Copyright (c) 2002 by LOst                                        *
 * MDP port Copyright (c) 2008-2009 by David Korth                         *
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
#include <stdio.h>

#include "sgens_window.h"
#include "sgens_plugin.h"
#include "sgens.h"

// XPM icons.
#include "sgens_16x16.xpm"
#include "sgens_32x32.xpm"

// SGens ROM type information and widget information.
#include "sgens_rom_type.h"
#include "sgens_widget_info.h"

// MDP includes.
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_mem.h"


/**
 * sgens_window_show(): Show the VDP Layer Options window.
 * @param parent Parent window.
 */
void MDP_FNCALL sgens_window_show(void *parent)
{
}


/**
 * sgens_window_close(): Close the VDP Layer Options window.
 */
void MDP_FNCALL sgens_window_close(void)
{
}


/**
 * sgens_window_update_rom_type(): Update the current ROM type.
 */
void MDP_FNCALL sgens_window_update_rom_type(void)
{
}


/**
 * sgens_window_update(): Update the information display.
 */
void MDP_FNCALL sgens_window_update(void)
{
}

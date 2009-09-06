/***************************************************************************
 * Gens: [MDP] VDP Layer Options. (Window Code) (Haiku)                    *
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


// C includes.
#include <stdio.h>

#include "vlopt_window.h"
#include "vlopt_options.h"
#include "vlopt_plugin.h"
#include "vlopt.h"

// XPM icons.
#include "vlopt_16x16.xpm"
#include "vlopt_32x32.xpm"

// MDP error codes.
#include "mdp/mdp_error.h"

// Custom response IDs.
#define VLOPT_RESPONSE_RESET 1

// Option handling functions.
static void vlopt_window_load_options(void);
static void vlopt_window_save_options(void);


/**
 * vlopt_window_show(): Show the VDP Layer Options window.
 * @param parent Parent window.
 */
void vlopt_window_show(void *parent)
{
}


/**
 * vlopt_window_close(): Close the VDP Layer Options window.
 */
void vlopt_window_close(void)
{
}


/**
 * vlopt_window_load_options(): Load the options from MDP_VAL_VDP_LAYER_OPTIONS.
 */
static void vlopt_window_load_options(void)
{
}


/**
 * vlopt_window_save_options(): Save the options to MDP_VAL_VDP_LAYER_OPTIONS.
 */
static void vlopt_window_save_options(void)
{
}

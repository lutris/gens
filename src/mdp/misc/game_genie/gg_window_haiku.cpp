/***************************************************************************
 * MDP: Game Genie. (Window Code) (Haiku)                                  *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// C includes.
#include <string.h>
#include <stdlib.h>

// C++ includes.
#include <list>
using std::list;

#include "gg_window.hpp"
#include "gg_plugin.h"
#include "gg.hpp"
#include "gg_code.h"
#include "gg_engine.hpp"

// XPM icon.
#include "gg_32x32.xpm"

// Custom response IDs.
#define GG_RESPONSE_DELETE		1
#define GG_RESPONSE_DEACTIVATE_ALL	2

// Code load/save functions.
static void	gg_window_init(void);
static void	gg_window_save(void);

static void	gg_window_callback_delete(void);
static void	gg_window_callback_deactivate_all(void);

// Miscellaneous.
static int	gg_window_add_code_from_textboxes(void);
static int	gg_window_add_code(const gg_code_t *gg_code, const char* name);


/**
 * gg_window_show(): Show the Game Genie window.
 * @param parent Parent window.
 */
void gg_window_show(void *parent)
{
}


/**
 * gg_window_close(): Close the Game Genie window.
 */
void gg_window_close(void)
{
}


/**
 * gg_window_init(): Initialize the Game Genie code treeview.
 */
static void gg_window_init(void)
{
}


/**
 * gg_window_save(): Save the Game Genie codes.
 */
static void gg_window_save(void)
{
}


/**
 * gg_window_add_code_from_textboxes(): Add a code to the treeview from the textboxes.
 * @return 0 on success; non-zero on error.
 */
static int gg_window_add_code_from_textboxes(void)
{
}


/**
 * gg_window_add_code(): Add a code to the treeview.
 * @param gg_code Pointer to gg_code_t containing the code to add.
 * @param name Name of the code. (If NULL, the name in gg_code is used.)
 * @return 0 on success; non-zero on error.
 */
static int gg_window_add_code(const gg_code_t *gg_code, const char* name)
{
	return 0;
}


/**
 * gg_window_callback_delete(): Delete the selected code(s).
 */
static void gg_window_callback_delete(void)
{
}


/**
 * gg_window_callback_deactivate_all(): Deactivate all codes.
 */
static void gg_window_callback_deactivate_all(void)
{
}

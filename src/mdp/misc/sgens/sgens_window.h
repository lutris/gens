/***************************************************************************
 * Gens: [MDP] Sonic Gens. (Window Code)                                   *
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

#ifndef _MDP_MISC_SGENS_WINDOW_H
#define _MDP_MISC_SGENS_WINDOW_H

#include "mdp/mdp.h"
#include "mdp/mdp_fncall.h"

#ifdef __cplusplus
extern "C" {
#endif

DLL_LOCAL void MDP_FNCALL sgens_window_show(void *parent);
DLL_LOCAL void MDP_FNCALL sgens_window_close(void);
DLL_LOCAL void MDP_FNCALL sgens_window_update_rom_type(void);
DLL_LOCAL void MDP_FNCALL sgens_window_update(void);

#ifdef __cplusplus
}
#endif

#endif /* _MDP_MISC_SGENS_WINDOW_H */

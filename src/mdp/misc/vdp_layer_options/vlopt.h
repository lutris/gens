/***************************************************************************
 * Gens: [MDP] VDP Layer Options.                                          *
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

#ifndef _MDP_MISC_VDP_LAYER_OPTIONS_H
#define _MDP_MISC_VDP_LAYER_OPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mdp/mdp.h"
#include "mdp/mdp_host.h"

DLL_LOCAL int MDP_FNCALL vlopt_init(const mdp_host_t *host_srv);
DLL_LOCAL int MDP_FNCALL vlopt_end(void);

DLL_LOCAL extern const mdp_host_t *vlopt_host_srv;

#ifdef __cplusplus
}
#endif

#endif /* _MDP_MISC_VDP_LAYER_OPTIONS_H */

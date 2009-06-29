/***************************************************************************
 * MDP: Blargg's NTSC Filter.                                              *
 *                                                                         *
 * Copyright (c) 2006 by Shay Green                                        *
 * MDP version Copyright (c) 2008-2009 by David Korth                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2.1 of the License, or  *
 * (at your option) any later version.                                     *
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

#ifndef MDP_RENDER_BLARGG_NTSC_H
#define MDP_RENDER_BLARGG_NTSC_H

#include "mdp/mdp.h"
#include "mdp/mdp_render.h"
#include "mdp/mdp_host.h"

#ifdef __cplusplus
extern "C" {
#endif

DLL_LOCAL int MDP_FNCALL mdp_render_blargg_ntsc_init(const mdp_host_t *host_srv);
DLL_LOCAL int MDP_FNCALL mdp_render_blargg_ntsc_end(void);

DLL_LOCAL extern const mdp_host_t *ntsc_host_srv;

#ifdef __cplusplus
}
#endif

#endif /* MDP_RENDER_BLARGG_NTSC_H */

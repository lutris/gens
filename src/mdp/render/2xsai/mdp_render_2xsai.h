/***************************************************************************
 * Gens: [MDP] 2xSaI renderer.                                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * 2xSaI Copyright (c) by Derek Liauw Kie Fa and Robert J. Ohannessian     *
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

#ifndef MDP_RENDER_2XSAI_H
#define MDP_RENDER_2XSAI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mdp/mdp.h"
#include "mdp/mdp_render.h"
#include "mdp/mdp_host.h"

DLL_LOCAL int MDP_FNCALL mdp_render_2xsai_init(const mdp_host_t *host_srv);
DLL_LOCAL int MDP_FNCALL mdp_render_2xsai_end(void);

DLL_LOCAL int MDP_FNCALL mdp_render_2xsai_cpp(const mdp_render_info_t *render_info);

#ifdef __cplusplus
}
#endif

#endif /* MDP_RENDER_2XSAI_H */

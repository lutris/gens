/***************************************************************************
 * MDP: Mega Drive Plugins - Test Suite.                                   *
 * render.cpp: Renderer Tests.                                             *
 *                                                                         *
 * Copyright (c) 2008-2009 by David Korth.                                 *
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

#ifndef __MDP_TEST_RENDER_HPP
#define __MDP_TEST_RENDER_HPP

#include "mdp/mdp.h"
#include "mdp/mdp_render.h"

#ifdef __cplusplus
extern "C" {
#endif

int mdp_host_renderer_register(mdp_t *plugin, mdp_render_t *renderer);
int mdp_host_renderer_unregister(mdp_t *plugin, mdp_render_t *renderer);

int renderer_clear(void);

int renderer_test_all(void);

#ifdef __cplusplus
}
#endif

#endif /* __MDP_TEST_RENDER_HPP */

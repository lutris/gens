/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (Event Handler)           *
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

// C++ includes.
#include <algorithm>
#include <string>
#include <list>
using std::string;
using std::list;

#include "mdp_host_gens_event.hpp"

// MDP includes.
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"

// Plugin Manager.
#include "pluginmgr.hpp"


/**
 * mdp_host_event_register(): Register an event handler.
 * @param plugin MDP_t requesting plugin registration.
 * @param event Event ID.
 * @param handler Event handler function.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_event_register(struct MDP_t *plugin, int event_id, void *handler())
{
	// TODO: Implement this function.
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}


/**
 * mdp_host_event_unregister(): Unregister an event handler.
 * @param plugin MDP_t requesting plugin unregistration.
 * @param event Event ID.
 * @param handler Event handler function.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_event_unregister(struct MDP_t *plugin, int event_id, void *handler())
{
	// TODO: Implement this function.
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}

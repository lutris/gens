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

#include "mdp_host_gens_event.hpp"

// C++ includes.
#include <algorithm>
#include <string>
#include <list>
using std::string;
using std::list;

// MDP includes.
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"

// Plugin Manager.
#include "pluginmgr.hpp"

// Event Manager.
#include "eventmgr.hpp"


/**
 * mdp_host_event_register(): Register an event handler.
 * @param plugin mdp_t requesting plugin registration.
 * @param event Event ID.
 * @param handler Event handler function.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_event_register(mdp_t *plugin, int event_id,
				       mdp_event_handler_fn handler)
{
	// Check that tne event ID is valid.
	if (event_id <= MDP_EVENT_UNKNOWN || event_id >= MDP_EVENT_MAX)
		return -MDP_ERR_EVENT_INVALID_EVENTID;
	
	// Check if this plugin and handler are already registered for this event.
	for (list<mdpEventItem_t>::iterator lstIter = EventMgr::lstEvents[event_id - 1].begin();
	     lstIter != EventMgr::lstEvents[event_id - 1].end(); lstIter++)
	{
		if ((*lstIter).handler == handler && (*lstIter).owner == plugin)
			return -MDP_ERR_EVENT_ALREADY_REGISTERED;
	}
	
	// Not registered. Register this event handler.
	mdpEventItem_t event;
	event.owner = plugin;
	event.handler = handler;
	EventMgr::lstEvents[event_id - 1].push_back(event);
	
	// Event handler registered.
	return MDP_ERR_OK;
}


/**
 * mdp_host_event_unregister(): Unregister an event handler.
 * @param plugin mdp_t requesting plugin unregistration.
 * @param event Event ID.
 * @param handler Event handler function.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_event_unregister(mdp_t *plugin, int event_id,
					 mdp_event_handler_fn handler)
{
	// Check that tne event ID is valid.
	if (event_id <= MDP_EVENT_UNKNOWN || event_id >= MDP_EVENT_MAX)
		return -MDP_ERR_EVENT_INVALID_EVENTID;
	
	// Check if this plugin and handler are registered for this event.
	for (list<mdpEventItem_t>::iterator lstIter = EventMgr::lstEvents[event_id - 1].begin();
	     lstIter != EventMgr::lstEvents[event_id - 1].end(); lstIter++)
	{
		if ((*lstIter).handler == handler && (*lstIter).owner == plugin)
		{
			// Found the registration. Remove it.
			EventMgr::lstEvents[event_id - 1].erase(lstIter);
			return MDP_ERR_OK;
		}
	}
	
	// Not registered. Return an error.
	return -MDP_ERR_EVENT_NOT_REGISTERED;
}

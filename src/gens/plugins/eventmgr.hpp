/***************************************************************************
 * Gens: Event Manager.                                                    *
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

#ifndef GENS_EVENTMGR_HPP
#define GENS_EVENTMGR_HPP

#include "mdp/mdp.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_constants.h"

#include <string>
#include <list>

// Event item.
struct mdpEventItem_t
{
	mdp_t			*owner;
	mdp_event_handler_fn	handler;
};

class EventMgr
{
	public:
		// Event lists.
		static std::list<mdpEventItem_t> lstEvents[MDP_EVENT_MAX];
		
		// Events.
		static void RaiseEvent(int event_id, void *event_info);
};

#endif /* GENS_EVENTMGR_HPP */

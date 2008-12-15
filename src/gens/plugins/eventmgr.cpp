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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "eventmgr.hpp"
#include "macros/hashtable.hpp"

// C++ includes
#include <algorithm>
#include <string>
#include <utility>
#include <list>
using std::pair;
using std::string;
using std::list;


// Event lists.
list<mdpEventItem_t> EventMgr::lstEvents[MaxEventID];


void EventMgr::RaiseEvent_load_rom(const char* rom_name, int system_id)
{
	for (list<mdpEventItem_t>::iterator lstIter = lstEvents[MDP_EVENT_LOAD_ROM - 1].begin();
	     lstIter != lstEvents[MDP_EVENT_LOAD_ROM - 1].end(); lstIter++)
	{
		mdp_event_load_rom_fn handler = (mdp_event_load_rom_fn)((*lstIter).handler);
		
		// TODO: Check return value.
		handler(rom_name, system_id);
	}
}


void EventMgr::RaiseEvent_close_rom(void)
{
	for (list<mdpEventItem_t>::iterator lstIter = lstEvents[MDP_EVENT_LOAD_ROM - 1].begin();
	     lstIter != lstEvents[MDP_EVENT_LOAD_ROM - 1].end(); lstIter++)
	{
		mdp_event_close_rom_fn handler = (mdp_event_close_rom_fn)((*lstIter).handler);
		
		// TODO: Check return value.
		handler();
	}
}

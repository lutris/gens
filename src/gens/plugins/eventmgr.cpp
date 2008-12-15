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


void EventMgr::RaiseEvent(int event_id, void *event_info)
{
	if (event_id <= 0 || event_id > MaxEventID)
		return;
	
	// Subtract one from the Event ID to get the array index.
	event_id--;
	
	for (list<mdpEventItem_t>::iterator lstIter = lstEvents[event_id].begin();
	     lstIter != lstEvents[event_id].end(); lstIter++)
	{
		// TODO: Check the return value.
		(*lstIter).handler(event_id, event_info);
	}
}

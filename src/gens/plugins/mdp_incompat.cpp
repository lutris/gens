/***************************************************************************
 * Gens: MDP Incompatibility Class.                                        *
 * Maintains a list of incompatible MDP plugins.                           *
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

#include "mdp_incompat.hpp"
#include "mdp/mdp.h"

// C includes.
#include <stdlib.h>
#include <string.h>

// C++ includes.
#include <string>
#include <list>
using std::string;
using std::list;

// libgsft includes.
#include "libgsft/gsft_strdup.h"


MDP_Incompat::MDP_Incompat()
{
}


MDP_Incompat::~MDP_Incompat()
{
	// Clear the list.
	for (list<mdp_incompat_t>::iterator iter = m_lstIncompat.begin();
	     iter != m_lstIncompat.end(); iter++)
	{
		mdp_t *mdp = (*iter).mdp;
		if (!mdp)
			continue;
		
		// An mdp_t has been allocated for this list item.
		// Check if a description field has been allocated.
		if (mdp->desc)
		{
			// Description field allocated. Free all of the strings.
			mdp_desc_t *mdp_desc = mdp->desc;
			
			free((void*)(mdp_desc->name));
			free((void*)(mdp_desc->author_mdp));
			free((void*)(mdp_desc->author_orig));
			free((void*)(mdp_desc->description));
			free((void*)(mdp_desc->website));
			free((void*)(mdp_desc->license));
			free((void*)(mdp_desc->icon));
			
			// Free the description field.
			free(mdp_desc);
		}
		
		// Free the mdp_t.
		free(mdp);
	}
	
	m_lstIncompat.clear();
}


/**
 * add(): Add a plugin to the incompatible plugins list.
 * @param plugin MDP plugin.
 * @param err Error number.
 * @param filename Filename of the plugin. (Empty if internal.)
 */
void MDP_Incompat::add(mdp_t *plugin, int err, const string& filename)
{
	mdp_incompat_t incompat;
	
	incompat.filename = filename;
	incompat.err = err;
	
	// Copy the contents of the mdp_t.
	if (plugin)
	{
		mdp_t *tmp_mdp = (mdp_t*)(malloc(sizeof(mdp_t)));
		memcpy(tmp_mdp, plugin, sizeof(mdp_t));
		tmp_mdp->func = NULL;
		
		// Check if a description field is available.
		if (tmp_mdp->desc)
		{
			// Copy the MDP description field.
			mdp_desc_t *orig_desc = tmp_mdp->desc;
			mdp_desc_t *tmp_mdp_desc = (mdp_desc_t*)(malloc(sizeof(mdp_desc_t)));
			memset(tmp_mdp_desc, 0x00, sizeof(mdp_desc_t));
			
			if (orig_desc->name)
				tmp_mdp_desc->name = strdup(orig_desc->name);
			if (orig_desc->author_mdp)
				tmp_mdp_desc->author_mdp = strdup(orig_desc->author_mdp);
			if (orig_desc->author_orig)
				tmp_mdp_desc->author_orig = strdup(orig_desc->author_orig);
			if (orig_desc->description)
				tmp_mdp_desc->description = strdup(orig_desc->description);
			if (orig_desc->website)
				tmp_mdp_desc->website = strdup(orig_desc->website);
			if (orig_desc->license)
				tmp_mdp_desc->license = strdup(orig_desc->license);
			
			unsigned int *iconLength = const_cast<unsigned int*>(&tmp_mdp_desc->iconLength);
			*iconLength = orig_desc->iconLength;
			if (orig_desc->icon)
			{
				// Copy the icon.
				tmp_mdp_desc->icon = (unsigned char*)malloc(*iconLength);
				memcpy(const_cast<unsigned char*>(tmp_mdp_desc->icon), orig_desc->icon, *iconLength);
			}
			
			tmp_mdp->desc = tmp_mdp_desc;
		}
		else
		{
			tmp_mdp->desc = NULL;
		}
		
		incompat.mdp = tmp_mdp;
	}
	else
	{
		incompat.mdp = NULL;
	}
	
	// Add the incompatibility information to the list.
	m_lstIncompat.push_back(incompat);
}

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

#ifndef GENS_MDP_INCOMPAT_HPP
#define GENS_MDP_INCOMPAT_HPP

#include "mdp/mdp.h"

// C++ includes.
#include <string>
#include <list>

// Struct for storing information about incompatible MDP plugins.
typedef struct _mdp_incompat_t
{
	mdp_t		*mdp;		// Copy of the mdp_t. (NULL if the plugin couldn't be loaded.)
	int		err;		// Error code.
	std::string	filename;	// Filename of the plugin. (Empty if internal.)
} mdp_incompat_t;

class MDP_Incompat
{
	public:
		MDP_Incompat();
		~MDP_Incompat();
		
		void add(mdp_t *plugin, int err, const std::string& filename);
		
		// TODO: Make these return read-only iterators.
		std::list<mdp_incompat_t>::iterator begin(void) { return m_lstIncompat.begin(); }
		std::list<mdp_incompat_t>::iterator end(void) { return m_lstIncompat.end(); }
	
	protected:
		std::list<mdp_incompat_t> m_lstIncompat;
};

#endif /* GENS_MDP_INCOMPAT_HPP */

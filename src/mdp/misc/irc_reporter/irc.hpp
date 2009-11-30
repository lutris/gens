/***************************************************************************
 * MDP: IRC Reporter. (Plugin Data File)                                   *
 *                                                                         *
 * Copyright (c) 2009 by David Korth                                       *
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

#ifndef MDP_MISC_IRC_REPORTER_HPP
#define MDP_MISC_IRC_REPORTER_HPP

// C includes.
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "mdp/mdp.h"
#include "mdp/mdp_host.h"

DLL_LOCAL int MDP_FNCALL irc_init(const mdp_host_t *host_srv);
DLL_LOCAL int MDP_FNCALL irc_end(void);

DLL_LOCAL extern const mdp_host_t *irc_host_srv;

// Data for the IPC modules.
DLL_LOCAL extern time_t	irc_last_modified;
DLL_LOCAL extern int	irc_is_rom_loaded;
DLL_LOCAL extern char	irc_rom_string[1024];

#ifdef __cplusplus
}
#endif

#endif /* MDP_MISC_IRC_REPORTER_HPP */

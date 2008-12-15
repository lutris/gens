/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Constants.                               *
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

#ifndef __MDP_CONSTANTS_H
#define __MDP_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

// System ID constants.
enum MDP_SYSID
{
	MDP_SYSID_UNKNOWN = 0,
	MDP_SYSID_MD      = 1,
	MDP_SYSID_MCD     = 2,
	MDP_SYSID_32X     = 3,
	MDP_SYSID_MCD32X  = 4,
	MDP_SYSID_SMS     = 5,
	MDP_SYSID_GG      = 6,
	MDP_SYSID_SG1000  = 7,
	MDP_SYSID_PICO    = 8,
};

#ifdef __cplusplus
}
#endif

#endif /* __MDP_CONSTANTS_H */

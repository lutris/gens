/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (File Compression)        *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mdp_host_gens_z.h"
#include "mdp/mdp_error.h"

#ifdef GENS_ZLIB
#include <zlib.h>
#endif


/**
 * mdp_host_crc32(): Calculate the CRC32 of the specified data.
 * @param buf		[in]  Data.
 * @param length	[in]  Length of the data.
 * @param crc32_out	[out] Pointer to variable to store the CRC32 in.
 * @return MDP error code.
 */
uint32_t MDP_FNCALL mdp_host_crc32(const uint8_t* buf, int length, uint32_t *crc32_out)
{
#ifndef GENS_ZLIB
	/* ZLib support wasn't compiled in. */
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
#else
	/* ZLib support was compiled in. */
	if (!crc32_out)
		return -MDP_ERR_UNKNOWN;	/* TODO: Add a specific error for this. */
	
	*crc32_out = crc32(0, buf, length);
	return MDP_ERR_OK;
#endif
}
/***************************************************************************
 * MDP: Mega Drive Plugins - Data Compression.                             *
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

#ifndef __MDP_Z_H
#define __MDP_Z_H

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _mdp_z_entry_t
{
	const char*	filename;	/* Filename. (allocated via strdup()) */
	size_t		filesize;	/* Filesize. */
	
	struct _mdp_z_entry_t *next;	/* Next file. */
} mdp_z_entry_t;


typedef struct _mdp_z_t
{
	mdp_z_entry_t *files;	/* List containing all files in the archive. */
	
	/* The following variables should not be accessed by plugins directly. */
	/* They are reserved for the emulator. */
	FILE *f;		/* Open file handle. */
	const char *filename;	/* Filename. (allocated via strdup()) */
} mdp_z_t;


#ifdef __cplusplus
}
#endif

#endif /* __MDP_Z_H */

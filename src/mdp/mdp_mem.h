/***************************************************************************
 * MDP: Mega Drive Plugins - Memory Defines.                               *
 *                                                                         *
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

#ifndef __MDP_MEM_H
#define __MDP_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

/** MDP_MEM: Memory IDs. **/
typedef enum _MDP_MEM
{
	/*! BEGIN: MDP v1.0 memory IDs. !*/
	MDP_MEM_NULL		= 0,
	MDP_MEM_MD_ROM		= 1,
	MDP_MEM_MD_RAM		= 2,
	MDP_MEM_MD_VRAM		= 3,
	MDP_MEM_MD_CRAM		= 4,
	/*! END: MDP v1.0 memory IDs. !*/
	
#if 0
	/* TODO: These will be implemented in a future version of MDP. */
	MDP_MEM_Z80_RAM		= 5,
	MDP_MEM_MCD_PRG_RAM	= 6,
	MDP_MEM_MCD_WORD_RAM	= 7,
	MDP_MEM_32X_RAM		= 8
#endif
	
	MDP_MEM_MAX
} MDP_MEM;

#ifdef __cplusplus
}
#endif

#endif /* __MDP_MEM_H */

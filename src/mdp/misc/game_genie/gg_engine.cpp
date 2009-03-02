/***************************************************************************
 * MDP: Game Genie. (Code Engine)                                          *
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

#include "gg_engine.hpp"

#include "gg.hpp"
#include "gg_code.h"

// MDP includes.
#include "mdp/mdp.h"
#include "mdp/mdp_host.h"
#include "mdp/mdp_fncall.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_error.h"

// C++ includes.
#include <list>
using std::list;


/**
 * gg_engine_apply_rom_codes(): Apply ROM codes.
 */
void MDP_FNCALL gg_engine_apply_rom_codes(void)
{
}


/**
 * gg_engine_unapply_rom_codes(): Unapply ROM codes.
 */
void MDP_FNCALL gg_engine_unapply_rom_codes(void)
{
}


/**
 * gg_engine_pre_frame(): Pre-frame event handler.
 * This function applies RAM codes.
 * @return MDP error code.
 */
int MDP_FNCALL gg_engine_pre_frame(int event_id, void *event_info)
{
	if (event_id != MDP_EVENT_PRE_FRAME)
		return MDP_ERR_OK;
	
	// Apply RAM codes.
	for (list<gg_code_t>::iterator iter = gg_code_list.begin();
	     iter != gg_code_list.end(); iter++)
	{
		const gg_code_t& gg_code = (*iter);
		
		if (!gg_code.enabled)
			continue;
		
		switch (gg_code.cpu)
		{
			case CPU_M68K:
			{
				// Check if this code is in RAM.
				uint8_t page = (gg_code.address >> 16) & 0xFF;
				if (page < 0xE0 || page > 0xFF)
					break;
				
				// Code is in RAM.
				uint32_t address = (gg_code.address & 0xFFFF);
				
				switch (gg_code.datasize)
				{
					case DS_BYTE:
						MDP_MEM_BE_8(gg_mdp_ptr_ram_md, address) = (gg_code.data & 0xFF);
						break;
					case DS_WORD:
						MDP_MEM_16(gg_mdp_ptr_ram_md, address) = (gg_code.data & 0xFFFF);
						break;
					case DS_DWORD:
						MDP_MEM_BE_32_WRITE(gg_mdp_ptr_ram_md, address, gg_code.data);
						break;
				}
				break;
			}
			
			case CPU_S68K:
			case CPU_Z80:
			case CPU_MSH2:
			case CPU_SSH2:
			default:
				// TODO: Add support for these CPUs later.
				break;
		}
	}
}

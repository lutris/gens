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

// C includes.
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "irc.h"
#include "irc_plugin.h"
#include "dbus-service.h"

// libgsft includes.
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"
#include "libgsft/gsft_space_elim.h"

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_constants.h"
#include "mdp/mdp_mem.h"

// MDP Host Services.
const mdp_host_t *irc_host_srv = NULL;

static int MDP_FNCALL irc_event_handler(int event_id, void *event_info);
static void MDP_FNCALL irc_init_rom(int system_id);

// Data for the IPC modules.
time_t	irc_last_modified;
int	irc_is_rom_loaded;
char	irc_rom_string[1024];


/**
 * irc_init(): Initialize the IRC Reporter plugin.
 * @return MDP error code.
 */
int MDP_FNCALL irc_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	irc_host_srv = host_srv;
	
	// Register the event handler.
	irc_host_srv->event_register(&mdp, MDP_EVENT_OPEN_ROM, irc_event_handler);
	irc_host_srv->event_register(&mdp, MDP_EVENT_CLOSE_ROM, irc_event_handler);
	
	// Clear the variables.
	irc_last_modified = 0;
	irc_is_rom_loaded = 0;
	irc_rom_string[0] = 0x00;
	
	// Initialize the D-BUS service.
	irc_dbus_init();
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * irc_end(): Shut down the IRC Reporter plugin.
 * @return MDP error code.
 */
int MDP_FNCALL irc_end(void)
{
	if (!irc_host_srv)
		return MDP_ERR_OK;
	
	// Unregister the event handlers.
	irc_host_srv->event_unregister(&mdp, MDP_EVENT_OPEN_ROM, irc_event_handler);
	irc_host_srv->event_unregister(&mdp, MDP_EVENT_CLOSE_ROM, irc_event_handler);
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


static int MDP_FNCALL irc_event_handler(int event_id, void *event_info)
{
	if (event_id == MDP_EVENT_OPEN_ROM)
	{
		// ROM opened.
		
		// Create the ROM string.
		mdp_event_open_rom_t *openROM = (mdp_event_open_rom_t*)event_info;
		irc_init_rom(openROM->system_id);
	}
	else if (event_id == MDP_EVENT_CLOSE_ROM)
	{
		// ROM closed.
		irc_rom_string[0] = 0x00;
		irc_is_rom_loaded = 0;
		irc_last_modified = time(NULL);
	}
	
	// TODO
	return MDP_ERR_OK;
}


/**
 * irc_init_rom(): Initialize the ROM string.
 * @param system_id System ID.
 */
static void MDP_FNCALL irc_init_rom(int system_id)
{
	static const char *sysIDs[] =
	{
		"???", "MD", "MCD",
		"32X", "MCD/32X", "SMS",
		"GG", "SG-1000", "PICO", NULL
	};
	
	const char *system;
	if (system_id >= MDP_SYSTEM_UNKNOWN &&
	    system_id < MDP_SYSTEM_MAX)
	{
		system = sysIDs[system_id];
	}
	else
	{
		system = sysIDs[0];
	}
	
	// Get the ROM name from the ROM header.
	char rom_name_raw[128];
	char rom_name[129];
	switch (system_id)
	{
		case MDP_SYSTEM_MD:
		case MDP_SYSTEM_32X:
		case MDP_SYSTEM_MCD:
		case MDP_SYSTEM_MCD32X:
		case MDP_SYSTEM_PICO:
			// MD or 32X ROM. Read the header.
			// TODO: Get the emulated country code to determine if we should use Domestic or Overseas.
			// For now, just use Overseas.
			// TODO: MCD and MCD32X will show the SegaCD firmware ROM name instead of the
			// actual game name, since MDP v1.0 doesn't support reading data from
			// the CD-ROM. This may be added in MDP v1.1.
			irc_host_srv->mem_read_block_8(MDP_MEM_MD_ROM, 0x150, (uint8_t*)rom_name_raw, 0x30);
			gsft_space_elim(rom_name_raw, 0x30, rom_name);
			break;
		default:
			// Not sure how to get the ROM name for this system ID.
			strlcpy(rom_name, "(ROM Name Unknown)", sizeof(rom_name));
			break;
	}
	
	// Get the ROM build date from the ROM header.
	char build_date[512];
	switch (system_id)
	{
		case MDP_SYSTEM_MD:
		case MDP_SYSTEM_32X:
			// MD or 32X ROM. Read the header.
			// TODO: Get the emulated country code to determine if we should use Domestic or Overseas.
			// For now, just use Overseas.
			irc_host_srv->mem_read_block_8(MDP_MEM_MD_ROM, 0x118, (uint8_t*)build_date, 8);
			build_date[8] = 0x00;
			break;
		default:
			// Not sure how to get the ROM name for this system ID.
			build_date[0] = 0x00;
			break;
	}
	
	// Get the ROM size.
	int size_bytes = irc_host_srv->mem_size_get(MDP_MEM_MD_ROM);
	
	// Convert to megabits.
	int mbits = size_bytes / (128*1024);
	int remainder = size_bytes % (128*1024);
	if (remainder != 0)
		mbits++;
	
	// Create the string.
	// TODO: Allow the user to customize this.
	szprintf(irc_rom_string, sizeof(irc_rom_string), "[%s] %s (%s%s%d megabits)",
		 system, rom_name, build_date, (build_date[0] != 0 ? "; " : ""), mbits);
	
	// Indicate that a ROM is loaded.
	irc_is_rom_loaded = 1;
	irc_last_modified = time(NULL);
}

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

#include "irc.h"
#include "irc_plugin.h"

// libgsft includes.
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_constants.h"

// MDP Host Services.
const mdp_host_t *irc_host_srv = NULL;

static int MDP_FNCALL irc_event_handler(int event_id, void *event_info);


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
		irc_dbus_end();
	}
#if 0
		mdp_event_open_rom_t *openROM = (mdp_event_open_rom_t*)(event_info);
		if (!openROM->rom_name)
			return MDP_ERR_OK;
		
		// IPS patch file is [save directory]/ROM_name.irc.
		char patch_filename[1024];
		szprintf(patch_filename, sizeof(patch_filename),
			 "%s/%s.irc", irc_save_path, openROM->rom_name);
		
		// Attempt to load the patch.
		// TODO: Make autoloading based on filename user-configurable.
		irc_file_load(patch_filename);
	}
#endif
	
	// TODO
	return MDP_ERR_OK;
}

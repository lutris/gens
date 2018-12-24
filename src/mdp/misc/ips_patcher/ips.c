/***************************************************************************
 * MDP: IPS Patcher.                                                       *
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

// C includes.
#include <string.h>
#include <stdio.h>

#include "ips.h"
#include "ips_plugin.h"
#include "ips_file.hpp"

// libgsft includes.
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_cpuflags.h"
#include "mdp/mdp_error.h"
#include "mdp/mdp_event.h"
#include "mdp/mdp_constants.h"

// MDP Host Services.
const mdp_host_t *ips_host_srv = NULL;

static int MDP_FNCALL ips_event_handler(int event_id, void *event_info);

// Directory registration.
static int ips_dir_id = -1;
static char ips_save_path[1024];
static int MDP_FNCALL ips_dir_get(int dir_id, char *out_buf, unsigned int size);
static int MDP_FNCALL ips_dir_set(int dir_id, const char *buf);


/**
 * ips_init(): Initialize the IPS Patcher plugin.
 * @return MDP error code.
 */
int MDP_FNCALL ips_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	ips_host_srv = host_srv;
	
	// Set the Game Genie directory to the default save path initially.
	ips_host_srv->dir_get_default_save_path(ips_save_path, sizeof(ips_save_path));
	
	// Attempt to register a directory.
	// If this fails, gg_dir_id will be less than 0.
	ips_dir_id = ips_host_srv->dir_register(&mdp, "IPS Patches", ips_dir_get, ips_dir_set);
	
	// Register the event handler. (TODO)
	ips_host_srv->event_register(&mdp, MDP_EVENT_OPEN_ROM, ips_event_handler);
	ips_host_srv->event_register(&mdp, MDP_EVENT_CLOSE_ROM, ips_event_handler);
	
	// Initialized.
	return MDP_ERR_OK;
}


/**
 * ips_end(): Shut down the IPS Patcher plugin.
 * @return MDP error code.
 */
int MDP_FNCALL ips_end(void)
{
	if (!ips_host_srv)
		return MDP_ERR_OK;
	
	// If a directory was registered, unregister it.
	if (ips_dir_id >= 0)
	{
		ips_host_srv->dir_unregister(&mdp, ips_dir_id);
		ips_dir_id = -1;
	}
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


static int MDP_FNCALL ips_event_handler(int event_id, void *event_info)
{
	if (event_id == MDP_EVENT_OPEN_ROM)
	{
		mdp_event_open_rom_t *openROM = (mdp_event_open_rom_t*)(event_info);
		if (!openROM->rom_name)
			return MDP_ERR_OK;
		
		// IPS patch file is [save directory]/ROM_name.ips.
		char patch_filename[1024];
		szprintf(patch_filename, sizeof(patch_filename),
			 "%s/%s.ips", ips_save_path, openROM->rom_name);
		
		// Attempt to load the patch.
		// TODO: Make autoloading based on filename user-configurable.
		ips_file_load(patch_filename);
	}
	
	// TODO
	return MDP_ERR_OK;
}


/**
 * ips_dir_get(): Get the IPS Patcher directory.
 * @param dir_id Directory ID.
 * @param out_buf Buffer to store the IPS Patcher directory in.
 * @param size Size of the buffer.
 * @return MDP error code.
 */
static int MDP_FNCALL ips_dir_get(int dir_id, char *out_buf, unsigned int size)
{
	if (dir_id != ips_dir_id)
		return -MDP_ERR_DIR_INVALID_DIRID;
	
	strlcpy(out_buf, ips_save_path, size);
	return MDP_ERR_OK;
}


/**
 * ips_dir_set(): Set the IPS Patcher directory.
 * @param dir_id Directory ID.
 * @param buf New directory.
 * @return MDP error code.
 */
static int MDP_FNCALL ips_dir_set(int dir_id, const char *buf)
{
	if (dir_id != ips_dir_id)
		return -MDP_ERR_DIR_INVALID_DIRID;
	
	strlcpy(ips_save_path, buf, sizeof(ips_save_path));
	return MDP_ERR_OK;
}

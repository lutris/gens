/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (Plugin Configuration)    *
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

#include "mdp_host_gens_config.hpp"
#include "mdp/mdp_error.h"

// libgsft includes.
#include "libgsft/gsft_strlcpy.h"

// C++ includes.
#include <string>
using std::string;

// Plugin Manager.
#include "pluginmgr.hpp"


/**
 * mdp_host_config_get(): Get a configuration setting.
 * @param plugin	[in]  Plugin requesting the setting.
 * @param key		[in]  Key name.
 * @param def		[in]  Default value if the key doesn't exist.
 * @param out_buf	[out] Output buffer.
 * @param size		[in]  Size of the output buffer.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_config_get(mdp_t *plugin, const char* key, const char* def,
				   char *out_buf, unsigned int size)
{
	if (!plugin || !key || !out_buf || size == 0)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Find the plugin section.
	mapPluginConfig::iterator cfgIter;
	
	cfgIter = PluginMgr::tblPluginConfig.find(plugin);
	if (cfgIter == PluginMgr::tblPluginConfig.end())
	{
		// Plugin section not found. Return the default value.
		if (!def)
			out_buf[0] = 0x00;
		else
			strlcpy(out_buf, def, size);
		return MDP_ERR_OK;
	}
	
	// Find the key.
	mapConfigItems& mapCfg = ((*cfgIter).second);
	
	mapConfigItems::iterator cfgItem = mapCfg.find(key);
	if (cfgItem == mapCfg.end())
	{
		// Key not found. Return the default value.
		if (!def)
			out_buf[0] = 0x00;
		else
			strlcpy(out_buf, def, size);
		return MDP_ERR_OK;
	}
	
	// Key found.
	strlcpy(out_buf, (*cfgItem).second.c_str(), size);
	return MDP_ERR_OK;
}


/**
 * mdp_host_config_get(): Get a configuration setting.
 * @param plugin	[in]  Plugin requesting the setting.
 * @param key		[in]  Key name.
 * @param value		[in]  Value to set.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_host_config_set(mdp_t *plugin, const char* key, const char* value)
{
	if (!plugin || !key || !value)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	PluginMgr::tblPluginConfig[plugin][key] = value;
	return MDP_ERR_OK;
}

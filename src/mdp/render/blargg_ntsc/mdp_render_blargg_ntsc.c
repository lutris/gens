/***************************************************************************
 * MDP: Blargg's NTSC Filter.                                              *
 *                                                                         *
 * Copyright (c) 2006 by Shay Green                                        *
 * MDP version Copyright (c) 2008-2009 by David Korth                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2.1 of the License, or  *
 * (at your option) any later version.                                     *
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

#include "mdp_render_blargg_ntsc.h"
#include "mdp_render_blargg_ntsc_plugin.h"
#include "md_ntsc.hpp"

#include "ntsc_window.h"
#include "ntsc_window_common.h"

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_error.h"

// C includes.
#include <stdlib.h>
#include <errno.h>

// libgsft includes.
#include "libgsft/gsft_szprintf.h"

// MDP Host Services.
const mdp_host_t *ntsc_host_srv = NULL;
static int ntsc_menuItemID = -1;

static int MDP_FNCALL ntsc_menu_handler(int menu_item_id);
static int MDP_FNCALL ntsc_event_handler(int event_id, void *event_info);


/**
 * mdp_render_blargg_ntsc_init(): Initialize the Blargg NTSC rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_blargg_ntsc_init(const mdp_host_t *host_srv)
{
	if (!host_srv)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	// Save the MDP Host Services pointer.
	ntsc_host_srv = host_srv;
	
	// Initialize the NTSC filter.
	mdp_md_ntsc_init();
	
	// Register the renderer.
	int rval = ntsc_host_srv->renderer_register(&mdp, &mdp_render);
	if (rval != MDP_ERR_OK)
		return rval;
	
	// Create a menu item.
	ntsc_menuItemID = ntsc_host_srv->menu_item_add(&mdp, &ntsc_menu_handler, 0, "Blargg's &NTSC Filter");
	
	// Register configuration load/save events.
	ntsc_host_srv->event_register(&mdp, MDP_EVENT_LOAD_CONFIG, ntsc_event_handler);
	ntsc_host_srv->event_register(&mdp, MDP_EVENT_SAVE_CONFIG, ntsc_event_handler);
	
	// Plugin has been initialized.
	return MDP_ERR_OK;
}


/**
 * mdp_render_blargg_ntsc_end(): Shut down the Blargg NTSC rendering plugin.
 * @return MDP error code.
 */
int MDP_FNCALL mdp_render_blargg_ntsc_end(void)
{
	if (!ntsc_host_srv)
		return MDP_ERR_OK;
	
	// Unregister configuration load/save events.
	ntsc_host_srv->event_unregister(&mdp, MDP_EVENT_LOAD_CONFIG, ntsc_event_handler);
	ntsc_host_srv->event_unregister(&mdp, MDP_EVENT_SAVE_CONFIG, ntsc_event_handler);
	
	// Remove the menu item.
	if (ntsc_menuItemID >= 0)
	{
		ntsc_host_srv->menu_item_remove(&mdp, ntsc_menuItemID);
		ntsc_menuItemID = -1;
	}
	
	// Unregister the renderer.
	ntsc_host_srv->renderer_unregister(&mdp, &mdp_render);
	
	// Shut down the NTSC filter.
	mdp_md_ntsc_end();
	
	// Plugin is shut down.
	return MDP_ERR_OK;
}


/**
 * ntsc_menu_handler(): Menu handler function.
 * @param menu_item_id Menu item ID.
 * @return MDP error code.
 */
static int MDP_FNCALL ntsc_menu_handler(int menu_item_id)
{
	if (menu_item_id != ntsc_menuItemID)
		return -MDP_ERR_MENU_INVALID_MENUID;
	
	// Show the NTSC Plugin Options window.
	ntsc_window_show(ntsc_host_srv->window_get_main());
	return MDP_ERR_OK;
}


/**
 * ntsc_event_handler(): Event handler function.
 * @param event_id Event ID.
 * @param event_info Event information.
 * @return MDP error code.
 */
static int MDP_FNCALL ntsc_event_handler(int event_id, void *event_info)
{
	MDP_UNUSED_PARAMETER(event_info);
	
	int i, val;
	char buf[128];
	
	switch (event_id)
	{
		case MDP_EVENT_LOAD_CONFIG:
			// Load NTSC configuration.
			
			// Check for presets.
			ntsc_host_srv->config_get(&mdp, "_Preset", NULL, buf, sizeof(buf));
			for (i = 0; i < NTSC_PRESETS_COUNT; i++)
			{
				if (!ntsc_presets[i].setup)
				{
					// "Custom". This is the last item in the predefined list.
					// Since the current setup doesn't match anything else,
					// it must be a custom setup.
					break;
				}
				else
				{
					// Check if this preset matches the current setup.
					if (!strncasecmp(buf, ntsc_presets[i].name, sizeof(buf)))
					{
						// Match found!
						memcpy(&mdp_md_ntsc_setup, ntsc_presets[i].setup, sizeof(mdp_md_ntsc_setup));
						break;
					}
				}
			}
			
			// If "Custom", load customized values.
			if (i != NTSC_PRESETS_COUNT)
			{
				for (i = 0; i < NTSC_CTRL_COUNT; i++)
				{
					ntsc_host_srv->config_get(&mdp, ntsc_controls[i].name, NULL, buf, sizeof(buf));
					
					if (buf[0] == 0x00)
					{
						// Empty value. Use "Composite" preset.
						mdp_md_ntsc_setup.params[i] = md_ntsc_composite.params[i];
					}
					else
					{
						// Non-empty value. Convert it to an integer.
						errno = 0;
						val = strtol(buf, NULL, 0);
						if (errno != 0)
						{
							// Error occurred while converting the number.
							// Use the default value. ("Composite" preset.)
							mdp_md_ntsc_setup.params[i] = md_ntsc_composite.params[i];
						}
						else
						{
							// Number converted.
							// Convert it to an internal NTSC value.
							mdp_md_ntsc_setup.params[i] = ntsc_display_to_internal(i, val);
						}
					}
				}
			}
			
			// Effects.
			mdp_md_ntsc_effects = 0;
			
			// Scanlines.
			ntsc_host_srv->config_get(&mdp, "_Scanlines", "1", buf, sizeof(buf));
			if (buf[0])
			{
				errno = 0;
				val = strtol(buf, NULL, 0);
				if (errno != 0)
					val = 1;
			}
			else
				val = 1;
			
			if (val)
				mdp_md_ntsc_effects |= MDP_MD_NTSC_EFFECT_SCANLINE;
			
			// Interpolation.
			ntsc_host_srv->config_get(&mdp, "_Interpolation", "1", buf, sizeof(buf));
			if (buf[0])
			{
				errno = 0;
				val = strtol(buf, NULL, 0);
				if (errno != 0)
					val = 1;
			}
			else
				val = 1;
			if (val)
				mdp_md_ntsc_effects |= MDP_MD_NTSC_EFFECT_INTERP;
			
			// Sony CXA2025AS US decoder matrix.
			ntsc_host_srv->config_get(&mdp, "_Sony_CXA2025AS_US", "0", buf, sizeof(buf));
			if (buf[0])
			{
				errno = 0;
				val = strtol(buf, NULL, 0);
				if (errno != 0)
					val = 0;
			}
			else
				val = 0;
			if (val)
				mdp_md_ntsc_effects |= MDP_MD_NTSC_EFFECT_CXA2025AS;
			
			// Reinitialize the NTSC settings.
			mdp_md_ntsc_reinit_setup();
			ntsc_window_load_settings();
			
			break;
		
		case MDP_EVENT_SAVE_CONFIG:
			// Save NTSC configuration.
			
			// Check if the current configuration is a preset.
			for (i = 0; i < NTSC_PRESETS_COUNT; i++)
			{
				if (!ntsc_presets[i].setup)
				{
					// "Custom". This is the last item in the predefined list.
					// Since the current setup doesn't match anything else,
					// it must be a custom setup.
					ntsc_host_srv->config_set(&mdp, "_Preset", ntsc_presets[i].name);
					break;
				}
				else
				{
					// Check if this preset matches the current setup.
					if (!memcmp(&mdp_md_ntsc_setup, ntsc_presets[i].setup, sizeof(mdp_md_ntsc_setup)))
					{
						// Match found!
						ntsc_host_srv->config_set(&mdp, "_Preset", ntsc_presets[i].name);
						break;
					}
				}
			}
			
			// Save individual values.
			for (i = 0; i < NTSC_CTRL_COUNT; i++)
			{
				szprintf(buf, sizeof(buf), "%d", ntsc_internal_to_display(i, mdp_md_ntsc_setup.params[i]));
				ntsc_host_srv->config_set(&mdp, ntsc_controls[i].name, buf);
			}
			
			// Scanlines.
			buf[1] = 0x00;
			buf[0] = ((mdp_md_ntsc_effects & MDP_MD_NTSC_EFFECT_SCANLINE) ? '1' : '0');
			ntsc_host_srv->config_set(&mdp, "_Scanlines", buf);
			
			// Interpolation.
			buf[0] = ((mdp_md_ntsc_effects & MDP_MD_NTSC_EFFECT_INTERP) ? '1' : '0');
			ntsc_host_srv->config_set(&mdp, "_Interpolation", buf);
			
			// Sony CXA2025AS US decoder matrix.
			buf[0] = ((mdp_md_ntsc_effects & MDP_MD_NTSC_EFFECT_CXA2025AS) ? '1' : '0');
			ntsc_host_srv->config_set(&mdp, "_Sony_CXA2025AS_US", buf);
			
			break;
		
		default:
			// Unhandled event.
			return -MDP_ERR_EVENT_NOT_REGISTERED;
	}
	
	return MDP_ERR_OK;
}

/***************************************************************************
 * Gens: Video Drawing - C++ functions.                                    *
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

#include "vdraw_cpp.hpp"
#include "vdraw.h"

#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>

// C++ includes.
#include <list>
using std::list;

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// Plugin Manager.
#include "plugins/pluginmgr.hpp"


void vdraw_reset_renderer(const BOOL reset_video)
{
	// Reset the renderer.
	const list<MDP_Render_t*>::iterator& rendMode = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
	if (!vdraw_set_renderer(rendMode, reset_video))
	{
		// Cannot initialize video mode. Try using render mode 0 (normal).
		if (!vdraw_set_renderer(PluginMgr::lstRenderPlugins.begin(), reset_video))
		{
			// Cannot initialize normal mode.
			fprintf(stderr, "%s(): FATAL ERROR: Cannot initialize any renderers.\n", __func__);
			exit(1);
		}
	}
}


/**
 * vdraw_set_renderer(): Set the rendering mode.
 * @param newMode Rendering mode / filter.
 * @param forceUpdate If true, forces a renderer update.
 * @return 0 on success; non-zero on error.
 */
int vdraw_set_renderer(const list<MDP_Render_t*>::iterator& newMode, const bool forceUpdate)
{
	if (PluginMgr::lstRenderPlugins.size() == 0 ||
	    newMode == PluginMgr::lstRenderPlugins.end())
	{
		return 1;
	}
	
	list<MDP_Render_t*>::iterator& Rend = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
	list<MDP_Render_t*>::iterator oldRend = Rend;
	mdp_render_fn& rendFn = (vdraw_get_fullscreen() ? vdraw_blitFS : vdraw_blitW);
	
	bool reinit = false;
	
	// Get the old scaling factor.
	const int oldScale = (*oldRend)->scale;
	
	// Renderer function found.
	MDP_Render_t *rendPlugin = (*newMode);
	rendFn = rendPlugin->blit;
	
	if (Rend != newMode)
	{
		MESSAGE_STR_L("Render Mode: %s", "Render Mode: %s", rendPlugin->tag, 1500);
	}
	else
	{
		reinit = true;
	}
	
	// Set the new render mode.
	Rend = newMode;
	
	// Set the scaling value.
	vdraw_scale = rendPlugin->scale;
	
	// Set the MD bpp output value.
	if (bppOut != 32)
	{
		// Not 32-bit color. Always use the destination surface color depth.
		bppMD = bppOut;
		vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
	}
	else
	{
		if (rendPlugin->flags & MDP_RENDER_FLAG_SRC16DST32)
		{
			// Render plugin only supports 16-bit color.
			bppMD = 16;
			vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
		}
		else
		{
			// MD surface should be the same color depth as the destination surface.
			bppMD = bppOut;
			vdraw_rInfo.mdScreen = (void*)(&MD_Screen32[8]);
		}
	}
	
	// Set the source pitch.
	vdraw_rInfo.srcPitch = 336 * (bppMD == 15 ? 2 : bppMD / 8);
	
	//if (Num>3 || Num<10)
	//Clear_Screen();
	// if( (Old_Rend==NORMAL && Num==DOUBLE)||(Old_Rend==DOUBLE && Num==NORMAL) ||Opengl)
	// this doesn't cover hq2x etc. properly. Let's just always refresh.
	
	// Update the renderer.
	if (vdraw_cur_backend)
	{
		if (forceUpdate && is_gens_running())
			vdraw_cur_backend->update_renderer();
		
		if ((reinit && forceUpdate) || (oldScale != vdraw_scale))
		{
			// The Gens window must be reinitialized.
			if (vdraw_cur_backend->reinit_gens_window)
				return vdraw_cur_backend->reinit_gens_window();
		}
	}
	
	return 0;
}

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vdraw_cpp.hpp"
#include "vdraw.h"

// Message logging.
#include "macros/log_msg.h"

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

// CPU flags.
#include "gens_core/misc/cpuflags.h"

// Text drawing.
#include "video/vdraw_text.hpp"

// Plugin Manager and Render Manager.
#include "plugins/pluginmgr.hpp"
#include "plugins/rendermgr.hpp"

// Palette handler.
#include "emulator/md_palette.hpp"


void vdraw_reset_renderer(const BOOL reset_video)
{
	// Reset the renderer.
	const list<mdp_render_t*>::iterator& rendMode = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
	
	if (vdraw_set_renderer(rendMode, reset_video))
	{
		// Cannot initialize video mode. Try using render mode 0 (normal).
		if (vdraw_set_renderer(RenderMgr::begin(), reset_video))
		{
			// Cannot initialize normal mode.
			LOG_MSG(video, LOG_MSG_LEVEL_CRITICAL,
				"FATAL ERROR: Cannot initialize any renderers.");
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
int vdraw_set_renderer(const list<mdp_render_t*>::iterator& newMode, const bool forceUpdate)
{
	if (RenderMgr::size() == 0 ||
	    newMode == RenderMgr::end())
	{
		return -1;
	}
	
	list<mdp_render_t*>::iterator& Rend = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
	list<mdp_render_t*>::iterator oldRend = Rend;
	mdp_render_fn& rendFn = (vdraw_get_fullscreen() ? vdraw_blitFS : vdraw_blitW);
	
	bool reinit = false;
	
#ifdef GENS_OS_WIN32
	// Get the old scaling factor.
	const int oldScale = (*oldRend)->scale;
#endif
		
	// Renderer function found.
	mdp_render_t *rendPlugin = (*newMode);
	rendFn = rendPlugin->blit;
	
	if (Rend != newMode)
		vdraw_text_printf(1500, "Render Mode: %s", rendPlugin->tag);
	else
		reinit = true;
	
	// Set the new render mode.
	Rend = newMode;
	
	// Set the scaling value.
	vdraw_scale = rendPlugin->scale;
	if (vdraw_cur_backend && vdraw_cur_backend->stretch_adjust)
		vdraw_cur_backend->stretch_adjust();
	
	// Set the MD bpp output value and video mode flags.
	vdraw_rInfo.vmodeFlags = 0;
	const uint8_t bppMD_old = bppMD;
	vdraw_needs_conversion = false;
	
	switch (bppOut)
	{
		case 15:
			// 15-bit color. (DST == 555)
			if (rendPlugin->flags & MDP_RENDER_FLAG_RGB_555to555)
			{
				// Plugin supports 555to555.
				bppMD = 15;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_555, MDP_RENDER_VMODE_RGB_555);
			}
			else if (rendPlugin->flags & MDP_RENDER_FLAG_RGB_565to555)
			{
				// Plugin supports 565to555.
				bppMD = 16;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_565, MDP_RENDER_VMODE_RGB_555);
			}
			else if (rendPlugin->flags & MDP_RENDER_FLAG_RGB_888to555)
			{
				// Plugin supports 888to555.
				bppMD = 32;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen32[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_888, MDP_RENDER_VMODE_RGB_555);
			}
			else
			{
				// Plugin doesn't support 555 output at all.
				// TODO: Add a variable to indicate automatic color depth conversion.
				// For now, just outputs 555.
				bppMD = 15;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_555, MDP_RENDER_VMODE_RGB_555);
				vdraw_needs_conversion = true;
			}
			break;
		
		case 16:
		default:
			// 16-bit color. (DST == 565)
			if (rendPlugin->flags & MDP_RENDER_FLAG_RGB_565to565)
			{
				// Plugin supports 565to565.
				bppMD = 16;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_565, MDP_RENDER_VMODE_RGB_565);
			}
			else if (rendPlugin->flags & MDP_RENDER_FLAG_RGB_555to565)
			{
				// Plugin supports 555to565.
				bppMD = 15;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_555, MDP_RENDER_VMODE_RGB_565);
			}
			else if (rendPlugin->flags & MDP_RENDER_FLAG_RGB_888to565)
			{
				// Plugin supports 888to565.
				bppMD = 32;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen32[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_888, MDP_RENDER_VMODE_RGB_565);
			}
			else
			{
				// Plugin doesn't support 565 output at all.
				// TODO: Add a variable to indicate automatic color depth conversion.
				// For now, just outputs 565.
				bppMD = 16;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_565, MDP_RENDER_VMODE_RGB_565);
				vdraw_needs_conversion = true;
			}
			break;
		
		case 32:
			// 32-bit color. (DST == 888)
			if (rendPlugin->flags & MDP_RENDER_FLAG_RGB_888to888)
			{
				// Plugin supports 888to888.
				bppMD = 32;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen32[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_888, MDP_RENDER_VMODE_RGB_888);
			}
			else if (rendPlugin->flags & MDP_RENDER_FLAG_RGB_565to888)
			{
				// Plugin supports 565to888.
				bppMD = 16;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_565, MDP_RENDER_VMODE_RGB_888);
			}
			else if (rendPlugin->flags & MDP_RENDER_FLAG_RGB_555to888)
			{
				// Plugin supports 555to888.
				bppMD = 15;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_555, MDP_RENDER_VMODE_RGB_888);
			}
			else
			{
				// Plugin doesn't support 888 output at all.
				// TODO: Add a variable to indicate automatic color depth conversion.
				// For now, just outputs 888.
				bppMD = 16;
				vdraw_rInfo.mdScreen = (void*)(&MD_Screen[8]);
				vdraw_rInfo.vmodeFlags |=
					MDP_RENDER_VMODE_CREATE(MDP_RENDER_VMODE_RGB_565, MDP_RENDER_VMODE_RGB_565);
				vdraw_needs_conversion = true;
			}
			break;
	}
	
	if (bppMD_old != bppMD)
	{
		// MD bpp has changed. Recalculate the palettes.
		Recalculate_Palettes();
	}
	
	// Set the CPU flags.
	vdraw_rInfo.cpuFlags = CPU_Flags;
	
	// Set the source pitch.
	vdraw_rInfo.srcPitch = 336 * (bppMD == 15 ? 2 : bppMD / 8);
	
	//if (Num>3 || Num<10)
	//Clear_Screen();
	// if( (Old_Rend==NORMAL && Num==DOUBLE)||(Old_Rend==DOUBLE && Num==NORMAL) ||Opengl)
	// this doesn't cover hq2x etc. properly. Let's just always refresh.
	
	// Update the renderer.
	if (vdraw_cur_backend)
	{
		if (forceUpdate && is_gens_running() && vdraw_cur_backend->update_renderer)
			vdraw_cur_backend->update_renderer();
		
#ifdef GENS_OS_WIN32
		if ((reinit && forceUpdate) || (oldScale != vdraw_scale))
		{
			// The Gens window must be reinitialized.
			if (vdraw_cur_backend->reinit_gens_window)
				return vdraw_cur_backend->reinit_gens_window();
		}
#endif /* GENS_OS_WIN32 */
	}
	
	return 0;
}

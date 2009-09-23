/***************************************************************************
 * Gens: (Haiku) Main Window.                                              *
 *                                                                         *
 * Copyright (c) 2009 by Phil Costin                                       *
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

#include "gens_app.hpp"
#include "gens_window.h"
#include "gens_ui.hpp"
#include "input/input_haiku_t.h"

#include "audio/audio.h"
#include "util/sound/gym.hpp"

#include "video/vdraw_haiku_t.h"
#include "video/v_effects.hpp"

#include "emulator/g_update.hpp"
#include "emulator/g_main.hpp"
#include "emulator/g_32x.hpp"
#include "emulator/g_md.hpp"
#include "emulator/gens.hpp"

extern int Genesis_Started, _32X_Started, SegaCD_Started;

GensApplication::GensApplication() : BApplication("application/x-vnd.gensgs") {
	window = new GensWindow();
	window->Show();
}

void GensApplication::Pulse() {
	// Update the UI.
	GensUI::update();
	
	// Update physical controller inputs.
	input_update();
	
#ifdef GENS_DEBUGGER
	if (IS_DEBUGGING())
	{
		// DEBUG
		Update_Debug_Screen();
		vdraw_flip(1);
		GensUI::sleep(100);
	}
	else
#endif /* GENS_DEBUGGER */
	if (Genesis_Started || _32X_Started || SegaCD_Started)
	{
		if ((Active) && (!Paused))
		{
			// EMULATION ACTIVE
			if (fast_forward)
				Update_Emulation_One();
			else
				Update_Emulation();
			
			#ifdef GENS_OS_UNIX
			// Prevent 100% CPU usage.
			// The CPU scheduler will take away CPU time from Gens/GS if
			// it notices that the process is eating up too much CPU time.
			GensUI::sleep(1, true);
			#endif
		}
		else
		{
			// EMULATION PAUSED
			if (_32X_Started)
				Do_32X_VDP_Only();
			else
				Do_VDP_Only();
			
			if (Paused && Video.pauseTint)
			{
				// Emulation is paused.
				veffect_pause_tint();
			}
			
			vdraw_flip(1);
			GensUI::sleep(250);
		}
	}
	else
	{
		// No game is currently running.
		
		// Update the screen.
		vdraw_flip(1);
		
		// Determine how much sleep time to add, based on intro style.
		// TODO: Move this to v_draw.cpp?
		if (audio_get_gym_playing())
		{
			// PLAY GYM
			// TODO: Does this even do anything?
			gym_play();
		}
		else if (Intro_Style == 1)
		{
			// Gens Logo effect.
			GensUI::sleep(5);
		}
		else if (Intro_Style == 2)
		{
			// "Strange" effect.
			GensUI::sleep(10);
		}
		else if (Intro_Style == 3)
		{
			// Genesis BIOS. (TODO: This is broken!)
			GensUI::sleep(20);
		}
		else
		{
			// Blank screen. (MAX IDLE)
			GensUI::sleep(250);
		}
	}
}

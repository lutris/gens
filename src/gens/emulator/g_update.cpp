/***************************************************************************
 * Gens: Update Emulation functions.                                       *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>

#include "g_update.hpp"
#include "gens.hpp"
#include "g_main.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "ui/gens_ui.hpp"
#include "debugger/debugger.hpp"

#ifndef GENS_OS_WIN32
#include "port/timer.h"
#endif

#ifndef NULL
#define NULL 0
#endif

// Video, Audio, Input.
#include "video/vdraw.h"
#include "audio/audio.h"
#include "input/input.h"
#include "input/input_update.h"

// Byteswapping functions.
#include "libgsft/gsft_byteswap.h"

clock_t Last_Time = 0;
clock_t New_Time = 0;
clock_t Used_Time = 0;


/**
 * Reset_Update_Timers(): Reset the update timers.
 */
void Reset_Update_Timers(void)
{
	Last_Time = GetTickCount();
	New_Time = 0;
	Used_Time = 0;
}


int Update_Emulation(void)
{
	static int Over_Time = 0;
	int current_div;

	if (Frame_Skip != -1)
	{
#ifdef GENS_OS_UNIX
		if (audio_get_enabled())
		{
			audio_write_sound_buffer(NULL);
		}
#endif /* GENS_OS_UNIX */
		
		input_update_controllers();
		
		if (Frame_Number++ < Frame_Skip)
		{
			Update_Frame_Fast();
		}
		else
		{
#ifdef GENS_OS_WIN32
			if (audio_get_enabled())
			{
				audio_wp_inc();
				audio_write_sound_buffer(NULL);
			}
#endif /* GENS_OS_WIN32 */
			Frame_Number = 0;
			Update_Frame();
			if (!IS_DEBUGGING())
				vdraw_flip(1);
		}
	}
	else
	{
		if (audio_get_enabled())
		{
			// This does auto-frame skip in a fairly dodgy way -
			// only updating the frame when we have 'lots' in
			// the audio buffer. Hence the audio is a couple of
			// cycles ahead of the graphics.
			
#ifdef GENS_OS_WIN32
			// Win32 specific.
			audio_wp_seg_wait();
#endif /* GENS_OS_WIN32 */
			
			// Wait for the audio buffer to empty out.
			audio_wait_for_audio_buffer();
			
			// Audio buffer is empty.
			input_update_controllers();
			Update_Frame();
			if (!IS_DEBUGGING())
				vdraw_flip(1);
		}
		else
		{
			// Sound is not enabled.
			
			if (CPU_Mode)
				current_div = 20;
			else
				current_div = 16 + (Over_Time ^= 1);
			
			New_Time = GetTickCount();
			Used_Time += (New_Time - Last_Time);
			Frame_Number = Used_Time / current_div;
			Used_Time %= current_div;
			Last_Time = New_Time;
			
			if (Frame_Number > 8) Frame_Number = 8;
			
			for (; Frame_Number > 1; Frame_Number--)
			{
				input_update_controllers();
				Update_Frame_Fast();
			}
			
			if (Frame_Number)
			{
				input_update_controllers();
				Update_Frame();
				if (!IS_DEBUGGING())
					vdraw_flip(1);
			}
		}
	}
	
	if (ice >= 1)
	{
		if (ice >= 3)
			ice = 1;
#if GSFT_BYTEORDER == GSFT_LIL_ENDIAN
		if (Ram_68k[0xFFB0] == 7)
#else // GSFT_BYTEORDER == GSFT_BIG_ENDIAN
		if (Ram_68k[0xFFB1] == 7)
#endif
		{
			if (ice == 1)
				ice = 2;
		}
		else
		{
			if (ice > 1)
				ice = 1;
		}
		if (*((unsigned int*)&Ram_68k[0xFFD4]))
			ice = 0;
	}

	return 1;
}


int Update_Emulation_One(void)
{
	input_update_controllers();
	Update_Frame();
	
	if (IS_DEBUGGING())
		vdraw_flip(1);
	
	return 1;
}


#if 0
int Update_Emulation_Netplay(int player, int num_player)
{
	static int Over_Time = 0;
	int current_div;
	
	if (CPU_Mode) current_div = 20;
	else current_div = 16 + (Over_Time ^= 1);
	
	New_Time = GetTickCount();
	Used_Time += (New_Time - Last_Time);
	Frame_Number = Used_Time / current_div;
	Used_Time %= current_div;
	Last_Time = New_Time;
	
	if (Frame_Number > 6) Frame_Number = 6;
	
	for (; Frame_Number > 1; Frame_Number--)
	{
		if (Sound_Enable)
		{
			if (WP == Get_Current_Seg()) WP = (WP - 1) & (Sound_Segs - 1);
			Write_Sound_Buffer(NULL);
			WP = (WP + 1) & (Sound_Segs - 1);
		}
		
		Scan_Player_Net(player);
		if (Kaillera_Error != -1) Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		//Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		Update_Controllers_Net(num_player);
		Update_Frame_Fast();
	}

	if (Frame_Number)
	{
		if (Sound_Enable)
		{
			if (WP == Get_Current_Seg()) WP = (WP - 1) & (Sound_Segs - 1);
			Write_Sound_Buffer(NULL);
			WP = (WP + 1) & (Sound_Segs - 1);
		}
		
		Scan_Player_Net(player);
		if (Kaillera_Error != -1) Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		//Kaillera_Error = Kaillera_Modify_Play_Values((void *) (Kaillera_Keys), 2);
		Update_Controllers_Net(num_player);
		Update_Frame();
		vdraw_flip();
	}
	return 1;
}
#endif

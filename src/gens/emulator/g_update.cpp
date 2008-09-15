/**
 * Gens: Update_Emulation functions.
 */

#include <time.h>

#include "g_update.hpp"
#include "gens.hpp"
#include "g_main.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "ui/gens_ui.hpp"

#include "sdllayer/g_sdlsound.h"

#ifndef NULL
#define NULL 0
#endif

clock_t Last_Time = 0;
clock_t New_Time = 0;
clock_t Used_Time = 0;
int Sleep_Time;

/**
 * Reset_Update_Timers(): Reset the update timers.
 */
void Reset_Update_Timers(void)
{
	Last_Time = GetTickCount ();
	New_Time = 0;
	Used_Time = 0;
}


int Update_Emulation(void)
{
	static int Over_Time = 0;
	int current_div;

	if (Frame_Skip != -1)
	{
		if (audio->enabled())
		{
			audio->writeSoundBuffer(NULL);
		}

		input->updateControllers();

		if (Frame_Number++ < Frame_Skip)
		{
			Update_Frame_Fast();
		}
		else
		{
			Frame_Number = 0;
			Update_Frame();
			draw->flip();
		}
	}
	else
	{
		if (audio->enabled())
		{
			// This does auto-frame skip in a fairly dodgy way -
			// only updating the frame when we have 'lots' in
			// the audio buffer. Hence the audio is a couple of
			// cycles ahead of the graphics.
			
			audio->writeSoundBuffer(NULL);
			while (!audio->lotsInAudioBuffer())
			{
				Update_Frame_Fast();
				audio->writeSoundBuffer(NULL);
			}

			input->updateControllers();
			Update_Frame();
			draw->flip();
		} //If sound is enabled
		
		else
		{
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
				input->updateControllers();
				Update_Frame_Fast();
			}

			if (Frame_Number)
			{
				input->updateControllers();
				Update_Frame();
				draw->flip();
			}
			else
			{
				GensUI::sleep(Sleep_Time);
			}
		} //If sound is not enabled
		
	}
	
	if (ice >= 1)
	{
		if (ice >= 3)
			ice = 1;
#if GENS_BYTE_ORDER == GENS_LIL_ENDIAN
		if (Ram_68k[0xFFB0] == 7)
#else // GENS_BYTE_ORDER == GENS_BIG_ENDIAN
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
	input->updateControllers();
	Update_Frame();
	draw->flip();
	
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
		draw->flip();
}
	return 1;
}
#endif

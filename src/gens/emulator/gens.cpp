/***************************************************************************
 * Gens: Common emulation code.                                            *
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
#include <math.h>

#include "gens.hpp"
#include "g_main.hpp"

// Memory
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"

// Sound
#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"

// CPUs
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/z80/cpu_z80.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"

// VDP
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_32x.h"

// Miscellaneous
#include "audio/audio.h"

int Frame_Skip;
int Frame_Number;


void Init_Tab(void)
{
	int x, y, dep;
	
	for (x = 0; x < 1024; x++)
	{
		for (y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0x3FC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 9)] = (unsigned short) dep;
		}
	}
	
	for (x = 0; x < 512; x++)
	{
		for (y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0x1FC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 8) + 0x8000] =
				(unsigned short) (dep + 0x8000);
		}
	}
	
	for (x = 0; x < 256; x++)
	{
		for (y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0xFC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xC000] =
				(unsigned short) (dep + 0xC000);
		}
	}
	
	for (x = 0; x < 256; x++)
	{
		for (y = 0; y < 32; y++)
		{
			dep = (x & 3) + (((x & 0xFC) >> 2) << 7);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xE000] =
				(unsigned short) (dep + 0xE000);
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xF000] =
				(unsigned short) (dep + 0xF000);
		}
	}
	
	for (x = 0; x < 512; x++)
		Z80_M68K_Cycle_Tab[x] = (int) ((double) x * 7.0 / 15.0);
}


void Check_Country_Order(void)
{
	if ((Country_Order[0] == Country_Order[1])
	    || (Country_Order[0] == Country_Order[2])
	    || (Country_Order[1] == Country_Order[2])
	    || (Country_Order[0] == Country_Order[2]) || (Country_Order[0] > 2)
	    || (Country_Order[0] < 0) || (Country_Order[1] > 2)
	    || (Country_Order[1] < 0) || (Country_Order[2] > 2)
	    || (Country_Order[2] < 0))
	{
		Country_Order[0] = 0;
		Country_Order[1] = 1;
		Country_Order[2] = 2;
	}
}


/**
 * Set_CPU_Freq(): Sets the CPU frequencies.
 * @param system System: 0 == MD only, 1 == MCD, 2 == 32X
 */
void Set_Clock_Freq(const int system)
{
	if (CPU_Mode)
	{
		// PAL
		CPL_Z80 = (int)rint((((double)CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = (int)rint((((double)CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 = (int)rint(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
					(double)MSH2_Speed) / 100.0);
		CPL_SSH2 = (int)rint(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
					(double)SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;
		
		if (system == 1) // SegaCD
		{
			CD_Access_Timer = 2080;
			Timer_Step = 136752;
		}
		
		if (system == 2) // 32X
			_32X_VDP.Mode &= ~0x8000;
		
		YM2612_Init(CLOCK_PAL / 7, audio_get_sound_rate(), YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, audio_get_sound_rate());
	}
	else
	{
		// NTSC
		CPL_Z80 = (int)rint((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = (int)rint((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		CPL_MSH2 = (int)rint(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
					(double)MSH2_Speed) / 100.0);
		CPL_SSH2 = (int)rint(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
					(double)SSH2_Speed) / 100.0);
		
		VDP_Num_Lines = 262;
		VDP_Status &= 0xFFFE;
		
		if (system == 1) // SegaCD
		{
			CD_Access_Timer = 2096;
			Timer_Step = 135708;
		}
		
		if (system == 2) // 32X
			_32X_VDP.Mode |= 0x8000;
		
		YM2612_Init(CLOCK_NTSC / 7, audio_get_sound_rate(), YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, audio_get_sound_rate());
	}
	
	if (system == 2) // 32X
		_32X_VDP.State |= 0x2000;
}

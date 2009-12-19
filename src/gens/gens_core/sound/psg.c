/***************************************************************************
 * Gens: TI SN76489 (PSG) emulator.                                        *
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

/***********************************************************/
/*                                                         */
/* PSG.C : SN76489 emulator                                */
/*                                                         */
/* Noise define constantes taken from MAME                 */
/*                                                         */
/* This source is a part of Gens project                   */
/* Written by Stéphane Dallongeville (gens@consolemul.com) */
/* Copyright (c) 2002 by Stéphane Dallongeville            */
/*                                                         */
/***********************************************************/

#include "psg.h"

/* C includes. */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifndef PURE
#ifdef __GNUC__
#define PURE __attribute__ ((pure))
#else
#define PURE
#endif /* __GNUC__ */
#endif /* PURE */

/** Defines **/

#ifndef PI
#define PI 3.14159265358979323846
#endif

// Change MAX_OUTPUT to change PSG volume (default = 0x7FFF)

#define MAX_OUTPUT 0x4FFF

// NOTE: SMS/GG/MD all have 16-bit LFSRs.
// SG-1000 has a 15-bit LFSR.
#define LFSR_MASK_WHITE		0x0009
#define LFSR_MASK_PERIODIC	0x0001
#define LFSR_INIT		0x8000


/**
 * psg_chip_t: PSG chip instance.
 */
typedef struct _psg_chip_t
{
	int Current_Channel;
	int Current_Register;
	int Register[8];
	unsigned int Counter[4];
	unsigned int CntStep[4];
	int Volume[4];
	
	/* Noise channel variables. */
	unsigned int LFSR_Mask;		// Linear Feedback Shift Register mask.
	unsigned int LFSR;		// Linear Feedback Shift Register contents.
} psg_chip_t;


/** Variables **/

static unsigned int PSG_Step_Table[1024];
static unsigned int PSG_Volume_Table[16];
static unsigned int PSG_Noise_Step_Table[4];

/* PSG chip instance. */
psg_chip_t PSG;


/** Gens-specific externs and variables **/

/* Message logging. */
#include "macros/log_msg.h"

/* GSX v7 savestate functionality. */
#include "util/file/gsx_v7.h"
#include "libgsft/gsft_byteswap.h"

/* GYM dumping. */
#include "util/sound/gym.hpp"

#include "audio/audio.h"
extern int VDP_Current_Line;

int PSG_Enable;
int PSG_Len = 0;

// Pointers to segment buffers.
// PSG_Buf[0] == pointer to an element in Seg_L[]
// PSG_Buf[1] == pointer to an element in Seg_R[]
int *PSG_Buf[2];


/** Functions **/


/**
 * PSG_Write(): Write a value to the PSG.
 * @param data
 */
void PSG_Write(int data)
{
	if (GYM_Dumping)
		gym_dump_update(3, (uint8_t)data, 0);
	
	if (data & 0x80)
	{
		// LATCH/DATA byte.
		PSG.Current_Register = (data & 0x70) >> 4;
		PSG.Current_Channel = PSG.Current_Register >> 1;
		
		// Save the data value in the register.
		PSG.Register[PSG.Current_Register] =
			(PSG.Register[PSG.Current_Register] & 0x3F0) | (data & 0x0F);
	}
	else
	{
		// DATA byte.
		if (!(PSG.Current_Register & 1) && PSG.Current_Channel != 3)
		{
			// TONE channel: Upper 6 bits.
			PSG.Register[PSG.Current_Register] =
				(PSG.Register[PSG.Current_Register] & 0x0F) | ((data & 0x3F) << 4);
		}
		else
		{
			// NOISE channel or Volume Register: Lower 4 bits.
			PSG.Register[PSG.Current_Register] = (data & 0x0F);
		}
	}
	
	if (PSG.Current_Register & 1)
	{
		// Volume register.
		PSG_Special_Update();
		PSG.Volume[PSG.Current_Channel] = PSG_Volume_Table[data & 0x0F];
		
		LOG_MSG(psg, LOG_MSG_LEVEL_DEBUG1,
			"channel %d    volume = %.8X",
			PSG.Current_Channel,
			PSG.Volume[PSG.Current_Channel]);
	}
	else
	{
		// Frequency
		PSG_Special_Update();
		
		if (PSG.Current_Channel != 3)
		{
			// Normal channel
			PSG.CntStep[PSG.Current_Channel] =
				PSG_Step_Table[PSG.Register[PSG.Current_Register]];
			
			if ((PSG.Current_Channel == 2) && ((PSG.Register[6] & 3) == 3))
				PSG.CntStep[3] = PSG.CntStep[2] >> 1;
			
			LOG_MSG(psg, LOG_MSG_LEVEL_DEBUG1,
				"channel %d    step = %.8X",
				PSG.Current_Channel,
				PSG.CntStep[PSG.Current_Channel]);
		}
		else
		{
			// Noise channel
			PSG.LFSR = LFSR_INIT;
			PSG_Noise_Step_Table[3] = PSG.CntStep[2] >> 1;
			PSG.CntStep[3] = PSG_Noise_Step_Table[data & 3];
			
			// Check if we should use white noise or periodic noise.
			if (data & 4)
				PSG.LFSR_Mask = LFSR_MASK_WHITE;
			else
				PSG.LFSR_Mask = LFSR_MASK_PERIODIC;
			
			LOG_MSG(psg, LOG_MSG_LEVEL_DEBUG1,
				"channel N    type = %.2X", data);
		}
	}
}


/** parity16() and LFSR16_Shift() from http://www.smspower.org/dev/docs/wiki/Sound/PSG#noisegeneration */

/**
 * parity16(): Get the parity of a 16-bit value.
 * @param n Value to check.
 * @return Parity.
 */
static inline unsigned int PURE parity16(unsigned int n)
{
	n ^= n >> 8;
	n ^= n >> 4;
	n ^= n >> 2;
	n ^= n >> 1;
	return (n & 1);
}


/**
 * LFSR16_Shift(): Shift the Linear Feedback Shift Register. (16-bit LFSR)
 * @param LFSR Current LFSR contents.
 * @param LFSR_Mask LFSR mask.
 * @return Shifted LFSR value.
 */
static inline unsigned int PURE LFSR16_Shift(unsigned int LFSR, unsigned int LFSR_Mask)
{
	return (LFSR >> 1) |
		(((LFSR_Mask > 1)
			? parity16(LFSR & LFSR_Mask)
			: LFSR_Mask) << 15);
}


/**
 * PSG_Update(): Update the PSG audio output using square waves.
 * @param buffer
 * @param length
 */
void PSG_Update(int **buffer, int length)
{
	int i, j;
	int cur_cnt, cur_step, cur_vol;
	
	// Channels 0-2 (in reverse order)
	for (j = 2; j >= 0; j--)
	{
		// Get the volume for this channel.
		cur_vol = PSG.Volume[j];
		
		if (cur_vol != 0)
		{
			// Current channel's volume is non-zero.
			// Apply the appropriate square wave.
			cur_step = PSG.CntStep[j];
			if (cur_step < 0x10000)
			{
				// Current channel's tone is audible.
				cur_cnt = PSG.Counter[j];
				
				for (i = 0; i < length; i++)
				{
					cur_cnt += cur_step;
					
					if (cur_cnt & 0x10000)
					{
						// Overflow. Apply +1 tone.
						buffer[0][i] += cur_vol;
						buffer[1][i] += cur_vol;
					}
				}
				
				// Update the counter for this channel.
				PSG.Counter[j] = cur_cnt;
			}
			else
			{
				// Current channel's tone is not audible.
				// Always apply a +1 tone.
				for (i = 0; i < length; i++)
				{
					buffer[0][i] += cur_vol;
					buffer[1][i] += cur_vol;
				}
				
				// Update the counter for this channel.
				PSG.Counter[j] += PSG.CntStep[j] * length;
			}
		}
		else
		{
			// Current channel's volume is zero.
			// Simply increase the channel's counter.
			PSG.Counter[j] += PSG.CntStep[j] * length;
		}
	}
	
	// Channel 3 - Noise
	if ((cur_vol = PSG.Volume[3]))
	{
		cur_cnt = PSG.Counter[3];
		cur_step = PSG.CntStep[3];
		
		for (i = 0; i < length; i++)
		{
			cur_cnt += cur_step;
			
			if (PSG.LFSR & 1)
			{
				buffer[0][i] += cur_vol;
				buffer[1][i] += cur_vol;
			}
			
			// Check if the LFSR should be shifted.
			if (cur_cnt & 0x10000)
			{
				cur_cnt &= 0xFFFF;
				PSG.LFSR = LFSR16_Shift(PSG.LFSR, PSG.LFSR_Mask);
			}
		}
		
		PSG.Counter[3] = cur_cnt;
	}
	else
	{
		PSG.Counter[3] += PSG.CntStep[3] * length;
	}
}


/**
 * PSG_Init(): Initialize the PSG chip.
 * @param clock Clock frequency.
 * @param rate Sound rate.
 */
void PSG_Init(int clock, int rate)
{
	int i;
	double out;
	
	// Step calculation
	for (i = 1; i < 1024; i++)
	{
		out = (double)(clock) / (double)(i << 4);	// out = frequency
		out /= (double)(rate);
		out *= 65536.0;
		
		PSG_Step_Table[i] = (unsigned int)out;
	}
	PSG_Step_Table[0] = PSG_Step_Table[1];
	
	// Step calculation [noise table]
	for (i = 0; i < 3; i++)
	{
		out = (double)(clock) / (double)(1 << (9 + i));
		out /= (double)(rate);
		out *= 65536.0;
		
		PSG_Noise_Step_Table[i] = (unsigned int)out;
	}
	PSG_Noise_Step_Table[3] = 0;
	
	// Volume table
	out = (double)MAX_OUTPUT / 3.0;
	for (i = 0; i < 15; i++)
	{
		PSG_Volume_Table[i] = (unsigned int)out;
		out /= 1.258925412;	// = 10 ^ (2/20) = 2dB
	}
	PSG_Volume_Table[15] = 0;
	
	// Clear PSG registers.
	PSG.Current_Register = 0;
	PSG.Current_Channel = 0;
	PSG.LFSR = 0;		// TODO: Should this be LFSR_INIT?
	PSG.LFSR_Mask = 0;	// TODO: Should this be LFSR_MASK_WHITE or LFSR_MASK_PERIODIC?
	
	for (i = 0; i < 4; i++)
	{
		PSG.Volume[i] = 0;
		PSG.Counter[i] = 0;
		PSG.CntStep[i] = 0;
	}
	
	// Initialize the PSG state.
	static const uint32_t psg_state_init[8] = {0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F};
	PSG_Restore_State(psg_state_init);
}


/**
 * PSG_Save_State(): Save the PSG state.
 * @param buf 8-DWORD array to save the state to.
 */
void PSG_Save_State(uint32_t *buf)
{
	int i;
	
	for (i = 0; i < 8; i++)
		buf[i] = PSG.Register[i];
}


/**
 * PSG_Restore_State(): Restore the PSG state.
 * @param buf 8-DWORD array to load the state from.
 */
void PSG_Restore_State(const uint32_t *buf)
{
	int i;
	
	for (i = 0; i < 8; i++)
	{
		PSG_Write(0x80 | (i << 4) | (buf[i] & 0xF));
		
		// Only write DATA bytes for tone registers.
		// Don't write DATA bytes for volume or noise registers.
		if (!(i & 1) && i < 6)
			PSG_Write((buf[i] >> 4) & 0x3F);
	}
}


/** Gens-specific code **/


int PSG_Get_Reg(int regID)
{
	if (regID < 0 || regID >= 8)
		return -1;
	
	return PSG.Register[regID];
}


/**
 * PSG_Special_Update(): Update the PSG buffer.
 */
void PSG_Special_Update(void)
{
	if (!(PSG_Len && PSG_Enable))
		return;
	
	PSG_Update(PSG_Buf, PSG_Len);
	
	// NOTE: Seg_L and Seg_R are arrays. This is pointer arithmetic.
	PSG_Buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line + 1][0];
	PSG_Buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line + 1][0];
	PSG_Len = 0;
}


/**
 * GSX v7 PSG save/restore functions.
 * Ported from Gens Rerecording.
 */


/**
 * PSG_Save_State_GSX_v7(): Restore the PSG state. (GSX v7)
 * @param save GSX v7 PSG struct to save to.
 */
void PSG_Save_State_GSX_v7(struct _gsx_v7_psg *save)
{
	save->current_channel	= cpu_to_le32(PSG.Current_Channel);
	save->current_reg	= cpu_to_le32(PSG.Current_Register);
	
	save->reg[0]		= cpu_to_le32(PSG.Register[0]);
	save->reg[1]		= cpu_to_le32(PSG.Register[1]);
	save->reg[2]		= cpu_to_le32(PSG.Register[2]);
	save->reg[3]		= cpu_to_le32(PSG.Register[3]);
	save->reg[4]		= cpu_to_le32(PSG.Register[4]);
	save->reg[5]		= cpu_to_le32(PSG.Register[5]);
	save->reg[6]		= cpu_to_le32(PSG.Register[6]);
	save->reg[7]		= cpu_to_le32(PSG.Register[7]);
	
	save->counter[0]	= cpu_to_le32(PSG.Counter[0]);
	save->counter[1]	= cpu_to_le32(PSG.Counter[1]);
	save->counter[2]	= cpu_to_le32(PSG.Counter[2]);
	save->counter[3]	= cpu_to_le32(PSG.Counter[3]);
	
	save->cntstep[0]	= cpu_to_le32(PSG.CntStep[0]);
	save->cntstep[1]	= cpu_to_le32(PSG.CntStep[1]);
	save->cntstep[2]	= cpu_to_le32(PSG.CntStep[2]);
	save->cntstep[3]	= cpu_to_le32(PSG.CntStep[3]);
	
	save->volume[0]		= cpu_to_le32(PSG.Volume[0]);
	save->volume[1]		= cpu_to_le32(PSG.Volume[1]);
	save->volume[2]		= cpu_to_le32(PSG.Volume[2]);
	save->volume[3]		= cpu_to_le32(PSG.Volume[3]);
	
	save->noise_type	= cpu_to_le32(PSG.LFSR_Mask == LFSR_MASK_PERIODIC ? 0x08000 : 0x12000);
	save->noise		= cpu_to_le32(PSG.LFSR);
}


/**
 * PSG_Restore_State_GSX_v7(): Restore the PSG state. (GSX v7)
 * @param save GSX v7 PSG struct to restore from.
 */
void PSG_Restore_State_GSX_v7(struct _gsx_v7_psg *save)
{
	PSG.Current_Channel	= le32_to_cpu(save->current_channel);
	PSG.Current_Register	= le32_to_cpu(save->current_reg);
	
	PSG.Register[0]		= le32_to_cpu(save->reg[0]);
	PSG.Register[1]		= le32_to_cpu(save->reg[1]);
	PSG.Register[2]		= le32_to_cpu(save->reg[2]);
	PSG.Register[3]		= le32_to_cpu(save->reg[3]);
	PSG.Register[4]		= le32_to_cpu(save->reg[4]);
	PSG.Register[5]		= le32_to_cpu(save->reg[5]);
	PSG.Register[6]		= le32_to_cpu(save->reg[6]);
	PSG.Register[7]		= le32_to_cpu(save->reg[7]);
	
	PSG.Counter[0]		= le32_to_cpu(save->counter[0]);
	PSG.Counter[1]		= le32_to_cpu(save->counter[1]);
	PSG.Counter[2]		= le32_to_cpu(save->counter[2]);
	PSG.Counter[3]		= le32_to_cpu(save->counter[3]);
	
	PSG.CntStep[0]		= le32_to_cpu(save->cntstep[0]);
	PSG.CntStep[1]		= le32_to_cpu(save->cntstep[1]);
	PSG.CntStep[2]		= le32_to_cpu(save->cntstep[2]);
	PSG.CntStep[3]		= le32_to_cpu(save->cntstep[3]);
	
	PSG.Volume[0]		= le32_to_cpu(save->volume[0]);
	PSG.Volume[1]		= le32_to_cpu(save->volume[1]);
	PSG.Volume[2]		= le32_to_cpu(save->volume[2]);
	PSG.Volume[3]		= le32_to_cpu(save->volume[3]);
	
	PSG.LFSR_Mask		= (le32_to_cpu(save->noise_type) == 0x08000
					? LFSR_MASK_PERIODIC
					: LFSR_MASK_WHITE);
	PSG.LFSR		= le32_to_cpu(save->noise);
}


/** end **/

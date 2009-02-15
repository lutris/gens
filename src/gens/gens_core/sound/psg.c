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

// Debug messages.
#include "macros/debug_msg.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "psg.h"

/* GSX v7 savestate functionality. */
#include "util/file/gsx_v7.h"
#include "gens_core/misc/byteswap.h"

/** Defines **/

#ifndef PI
#define PI 3.14159265358979323846
#endif

// Change MAX_OUTPUT to change PSG volume (default = 0x7FFF)

#define MAX_OUTPUT 0x4FFF

#define W_NOISE 0x12000
#define P_NOISE 0x08000

//#define NOISE_DEF 0x0f35
//#define NOISE_DEF 0x0001
#define NOISE_DEF 0x4000


/** Variables **/

static unsigned int PSG_SIN_Table[16][512];
static unsigned int PSG_Step_Table[1024];
static unsigned int PSG_Volume_Table[16];
static unsigned int PSG_Noise_Step_Table[4];

// PSG save buffer
unsigned int PSG_Save[8];

struct _psg PSG;


/** Gens-specific externs and variables **/
#include "audio/audio.h"
extern int VDP_Current_Line;
extern int GYM_Dumping;


// TODO: Include gym.h instead of declaring this here.
int Update_GYM_Dump(char v0, char v1, char v2);


int PSG_Enable;
int PSG_Improv;
int PSG_Len = 0;

// Pointers to segment buffers.
// PSG_Buf[0] == pointer to an element in Seg_L[]
// PSG_Buf[1] == pointer to an element in Seg_R[]
int *PSG_Buf[2];


/** Functions **/


void PSG_Write(int data)
{
	if (GYM_Dumping)
		Update_GYM_Dump((unsigned char)3,
				(unsigned char)data,
				(unsigned char)0);
	
	if (data & 0x80)
	{
		PSG.Current_Register = (data & 0x70) >> 4;
		PSG.Current_Channel = PSG.Current_Register >> 1;
		
		data &= 0x0F;
		
		PSG.Register[PSG.Current_Register] =
			(PSG.Register[PSG.Current_Register] & 0x3F0) | data;
		
		if (PSG.Current_Register & 1)
		{
			// Volume
			PSG_Special_Update();
			PSG.Volume[PSG.Current_Channel] = PSG_Volume_Table[data];
			
			DEBUG_MSG(psg, 1, "channel %d    volume = %.8X",
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
				
				DEBUG_MSG(psg, 1, "channel %d    step = %.8X",
					  PSG.Current_Channel,
					  PSG.CntStep[PSG.Current_Channel]);
			}
			else
			{
				// Noise channel
				PSG.Noise = NOISE_DEF;
				PSG_Noise_Step_Table[3] = PSG.CntStep[2] >> 1;
				PSG.CntStep[3] = PSG_Noise_Step_Table[data & 3];
				
				if (data & 4)
					PSG.Noise_Type = W_NOISE;
				else
					PSG.Noise_Type = P_NOISE;
				
				DEBUG_MSG(psg, 1, "channel N    type = %.2X", data);
			}
		}
	}
	else
	{
		if (!(PSG.Current_Register & 1))
		{
			// Frequency
			if (PSG.Current_Channel != 3)
			{
				PSG_Special_Update();
				
				PSG.Register[PSG.Current_Register] =
					(PSG.Register[PSG.Current_Register] & 0x0F) | ((data & 0x3F) << 4);
				
				PSG.CntStep[PSG.Current_Channel] =
					PSG_Step_Table[PSG.Register[PSG.Current_Register]];
				
				if ((PSG.Current_Channel == 2) && ((PSG.Register[6] & 3) == 3))
					PSG.CntStep[3] = PSG.CntStep[2] >> 1;
				
				DEBUG_MSG(psg, 1, "channel %d    step = %.8X",
					  PSG.Current_Channel,
					  PSG.CntStep[PSG.Current_Channel]);
			}
		}
	}
}


void PSG_Update_SIN(int **buffer, int length)
{
	int i, j, out;
	int cur_cnt, cur_step, cur_vol;
	unsigned int *sin_t;
	
	// Channels 0-2 (in reverse order)
	for (j = 2; j >= 0; j--)
	{
		if (PSG.Volume[j])
		{
			cur_cnt = PSG.Counter[j];
			cur_step = PSG.CntStep[j];
			sin_t = PSG_SIN_Table[PSG.Register[(j << 1) + 1]];
			
			for (i = 0; i < length; i++)
			{
				out = sin_t[(cur_cnt = (cur_cnt + cur_step) & 0x1FFFF) >> 8];
				
				buffer[0][i] += out;
				buffer[1][i] += out;
			}
			
			PSG.Counter[j] = cur_cnt;
		}
		else
		{
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
			
			if (PSG.Noise & 1)
			{
				buffer[0][i] += cur_vol;
				buffer[1][i] += cur_vol;
				
				if (cur_cnt & 0x10000)
				{
					cur_cnt &= 0xFFFF;
					PSG.Noise = (PSG.Noise ^ PSG.Noise_Type) >> 1;
				}
			}
			else if (cur_cnt & 0x10000)
			{
				cur_cnt &= 0xFFFF;
				PSG.Noise >>= 1;
			}
		}
		
		PSG.Counter[3] = cur_cnt;
	}
	else
	{
		PSG.Counter[3] += PSG.CntStep[3] * length;
	}
}


void PSG_Update(int **buffer, int length)
{
	int i, j;
	int cur_cnt, cur_step, cur_vol;
	
	// Channels 0-2 (in reverse order)
	for (j = 2; j >= 0; j--)
	{
		if ((cur_vol = PSG.Volume[j]))
		{
			if ((cur_step = PSG.CntStep[j]) < 0x10000)
			{
				cur_cnt = PSG.Counter[j];
					
				for (i = 0; i < length; i++)
				{
					if ((cur_cnt += cur_step) & 0x10000)
					{
						buffer[0][i] += cur_vol;
						buffer[1][i] += cur_vol;
					}
				}
					
				PSG.Counter[j] = cur_cnt;
			}
			else
			{
				for (i = 0; i < length; i++)
				{
					buffer[0][i] += cur_vol;
					buffer[1][i] += cur_vol;
				}
			}
		}
		else
		{
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
			
			if (PSG.Noise & 1)
			{
				buffer[0][i] += cur_vol;
				buffer[1][i] += cur_vol;
				
				if (cur_cnt & 0x10000)
				{
					cur_cnt &= 0xFFFF;
					PSG.Noise = (PSG.Noise ^ PSG.Noise_Type) >> 1;
				}
			}
			else if (cur_cnt & 0x10000)
			{
				cur_cnt &= 0xFFFF;
				PSG.Noise >>= 1;
			}
		}
		
		PSG.Counter[3] = cur_cnt;
	}
	else
	{
		PSG.Counter[3] += PSG.CntStep[3] * length;
	}
}


void PSG_Init(int clock, int rate)
{
	int i, j;
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
	
/*
	// Old SIN table calculation
	
	for (i = 0; i < 256; i++)
	{
		out = (i + 1.0) / 256.0;
		
		for (j = 0; j < 16; j++)
		{
			PSG_SIN_Table[j][i] = (unsigned int)(out * (double)PSG_Volume_Table[j]);
		}
	}

	for (i = 0; i < 256; i++)
	{
		out = 1.0 - ((i + 1.0) / 256.0);

		for (j = 0; j < 16; j++)
		{
			PSG_SIN_Table[j][i + 256] = (unsigned int)(out * (double)PSG_Volume_Table[j]);
		}
	}
*/
	
	// SIN table calculation
	for (i = 0; i < 512; i++)
	{
		out = sin((2.0 * PI) * ((double)(i) / 512.0));
		out = sin((2.0 * PI) * ((double)(i) / 512.0));
		
		for (j = 0; j < 16; j++)
		{
			PSG_SIN_Table[j][i] =
				(unsigned int)(out * (double)PSG_Volume_Table[j]);
		}
	}
	
	// Clear PSG registers.
	PSG.Current_Register = 0;
	PSG.Current_Channel = 0;
	PSG.Noise = 0;
	PSG.Noise_Type = 0;
	
	for (i = 0; i < 4; i++)
	{
		PSG.Volume[i] = 0;
		PSG.Counter[i] = 0;
		PSG.CntStep[i] = 0;
	}
	
	// Initialize the PSG state.
	for (i = 0; i < 8; i += 2)
	{
		PSG_Save[i] = 0;
		PSG_Save[i + 1] = 0x0F;	// volume = OFF
	}
	PSG_Restore_State();		// Reset
}


/**
 * PSG_Save_State(): Copy PSG.Register[] to PSG_Save[].
 */
void PSG_Save_State(void)
{
	int i;
	
	for (i = 0; i < 8; i++)
		PSG_Save[i] = PSG.Register[i];
}


/**
 * PSG_Restore_State(): Restore the PSG registers from PSG_Save[].
 */
void PSG_Restore_State(void)
{
	int i;
	
	for (i = 0; i < 8; i++)
	{
		PSG_Write(0x80 | (i << 4) | (PSG_Save[i] & 0xF));
		PSG_Write((PSG_Save[i] >> 4) & 0x3F);
	}
}


/** Gens-specific code **/


/**
 * PSG_Special_Update(): Update the PSG buffer.
 * Calls the appropriate function depending on the "PSG Improved" setting.
 */
void PSG_Special_Update(void)
{
	if (PSG_Len && PSG_Enable)
	{
		if (PSG_Improv)
			PSG_Update_SIN(PSG_Buf, PSG_Len);
		else
			PSG_Update(PSG_Buf, PSG_Len);
		
		// NOTE: Seg_L and Seg_R are arrays. This is pointer arithmetic.
		PSG_Buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line + 1][0];
		PSG_Buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line + 1][0];
		PSG_Len = 0;
	}
}


// Full PSG save/restore functions from Gens Rerecording.

void PSG_Save_State_Full(struct _gsx_v7_psg *save)
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
	
	save->noise_type	= cpu_to_le32(PSG.Noise_Type);
	save->noise		= cpu_to_le32(PSG.Noise);
}

void PSG_Restore_State_Full(struct _gsx_v7_psg *save)
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
	
	PSG.Noise_Type		= le32_to_cpu(save->noise_type);
	PSG.Noise		= le32_to_cpu(save->noise);
}


/** end **/


// Symbol aliases for cross-OS asm compatibility.
void _PSG_Write(int data)
	__attribute__ ((weak, alias ("PSG_Write")));

#include "gens.hpp"
#include "g_main.hpp"

// Memory
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"

// Sound
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"

// CPUs
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/z80/cpu_z80.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "mdZ80/mdZ80.h"

// VDP
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

// Miscellaneous
#include "gens_core/io/io.h"

#include "macros/math_m.h"
#include "audio/audio.h"
#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

int Debug;
int Frame_Skip;
int Frame_Number;
int DAC_Improv;


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
		CPL_Z80 = roundDouble((((double)CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = roundDouble((((double)CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 = roundDouble(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
					(double)MSH2_Speed) / 100.0);
		CPL_SSH2 = roundDouble(((((((double)CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) *
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
		CPL_Z80 = roundDouble((((double)CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = roundDouble((((double)CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		CPL_MSH2 = roundDouble(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
					(double)MSH2_Speed) / 100.0);
		CPL_SSH2 = roundDouble(((((((double)CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) *
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


/**
 * gens_do_MD_frame(): Do an MD frame.
 * @param VDP If true, VDP is enabled.
 */
static inline int __attribute__((always_inline)) gens_do_MD_frame(bool VDP)
{
	int *buf[2];
	int HInt_Counter;
	
	// Set the number of visible lines.
	SET_VISIBLE_LINES;
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	mdZ80_clear_odo(&M_Z80);
	
	// TODO: Send "Before Frame" event to registered MDP event handlers.
	//Patch_Codes();
	
	VRam_Flag = 1;
	
	VDP_Status &= 0xFFF7;		// Clear V Blank
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	
	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step H interrupt
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		
		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (DMAT_Length)
			main68k_addCycles(Update_DMA());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		main68k_exec (Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line();
		}
		
		if (VDP)
		{
			// VDP is enabled.
			Render_Line();
		}
		
		main68k_exec(Cycles_M68K);
		Z80_EXEC(0);
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	
	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (DMAT_Length)
		main68k_addCycles(Update_DMA());
	
	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line();
	}
	
	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	main68k_exec(Cycles_M68K - 360);
	Z80_EXEC(168);
	
	VDP_Status &= 0xFFFB;		// HBlank = 0
	VDP_Status |= 0x0080;		// V Int happened
	
	VDP_Int |= 0x8;
	Update_IRQ_Line();
	mdZ80_interrupt(&M_Z80, 0xFF);
	
	main68k_exec(Cycles_M68K);
	Z80_EXEC(0);
	
	for (VDP_Current_Line++;
	     VDP_Current_Line < VDP_Num_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		
		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (DMAT_Length)
			main68k_addCycles(Update_DMA());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		main68k_exec(Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;	// HBlank = 0
		
		main68k_exec(Cycles_M68K);
		Z80_EXEC(0);
	}
	
	PSG_Special_Update();
	YM2612_Special_Update();
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (audio_get_wav_dumping())
		audio_wav_dump_update();
	if (GYM_Dumping)
		Update_GYM_Dump((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);
	
	return 1;
}


int Do_Genesis_Frame_No_VDP(void)
{
	return gens_do_MD_frame(false);
}
int Do_Genesis_Frame(void)
{
	return gens_do_MD_frame(true);
}

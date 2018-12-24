/**
 * Gens: Sega 32X initialization and main loop code.
 */

#include <string.h>

#include "gens.hpp"
#include "g_32x.hpp"
#include "g_md.hpp"
#include "g_main.hpp"
#include "g_update.hpp"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_32x.h"
#include "gens_core/mem/mem_sh2.h"

#include "gens_core/sound/ym2612.hpp"
#include "gens_core/sound/psg.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/z80/cpu_z80.h"
#include "mdZ80/mdZ80.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"
#include "gens_core/io/io.h"
#include "util/file/save.hpp"
#include "gens_core/sound/pwm.h"

#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

#include "libgsft/gsft_byteswap.h"

#include "ui/gens_ui.hpp"

// Audio Handler.
#include "audio/audio.h"

// MDP Event Manager.
#include "plugins/eventmgr.hpp"

// 32X 32-bit color functions
#include "gens_core/vdp/vdp_32x_32bit.h"

// Reverse-engineered 32X firmware.
#include "fw/bios32xg.bin.h"
#include "fw/bios32xm.bin.h"
#include "fw/bios32xs.bin.h"


#define SH2_EXEC(cycM, cycS)		\
do {					\
	SH2_Exec(&M_SH2, cycM);		\
	SH2_Exec(&S_SH2, cycS);		\
} while (0)


/**
 * Init_32X(): Initialize the 32X with the specified ROM image.
 * @param MD_ROM ROM image struct.
 * @return 1 if successful; 0 if an error occurred.
 */
int Init_32X(ROM_t* MD_ROM)
{
	FILE *f;
	int i;
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	// Clear the firmware buffers.
	memset(&_32X_Genesis_Rom[0], 0x00, sizeof(_32X_Genesis_Rom));
	memset(&_32X_MSH2_Rom[0], 0x00, sizeof(_32X_MSH2_Rom));
	memset(&_32X_SSH2_Rom[0], 0x00, sizeof(_32X_SSH2_Rom));

	// Read the 32X MC68000 firmware. (usually "32X_G_BIOS.BIN")
	ROM_ByteSwap_State &= ~ROM_BYTESWAPPED_32X_FW_68K;
	if ((f = fopen(BIOS_Filenames._32X_MC68000, "rb")))
	{
		// External firmware file opened.
		fread(&_32X_Genesis_Rom[0], 1, sizeof(_32X_Genesis_Rom), f);
		fclose(f);
		be16_to_cpu_array(&_32X_Genesis_Rom[0], sizeof(_32X_Genesis_Rom));
	}
	else
	{
		// Use the reverse-engineered firmware.
		memcpy(&_32X_Genesis_Rom[0], &cbob_bios32xg_bin[0], sizeof(cbob_bios32xg_bin));
		be16_to_cpu_array(&_32X_Genesis_Rom[0], sizeof(cbob_bios32xg_bin));
	}
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_32X_FW_68K;
	
	// Read the Master SH2 firmware. (usually "32X_M_BIOS.BIN")	
	ROM_ByteSwap_State &= ~ROM_BYTESWAPPED_32X_FW_MSH2;
	if ((f = fopen(BIOS_Filenames._32X_MSH2, "rb")))
	{
		// External firmware file opened.
		fread(&_32X_MSH2_Rom[0], 1, sizeof(_32X_MSH2_Rom), f);
		fclose(f);
		le16_to_cpu_array(&_32X_MSH2_Rom[0], sizeof(_32X_MSH2_Rom));
	}
	else
	{
		// Use the reverse-engineered firmware.
		memcpy(&_32X_MSH2_Rom[0], &cbob_bios32xm_bin[0], sizeof(cbob_bios32xm_bin));
		le16_to_cpu_array(&_32X_MSH2_Rom[0], sizeof(cbob_bios32xm_bin));
	}
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_32X_FW_MSH2;
	
	// Read the Slave SH2 firmware. (usually "32X_S_BIOS.BIN")
	ROM_ByteSwap_State &= ~ROM_BYTESWAPPED_32X_FW_SSH2;
	if ((f = fopen(BIOS_Filenames._32X_SSH2, "rb")))
	{
		// External firmware file opened.
		fread(&_32X_SSH2_Rom[0], 1, sizeof(_32X_SSH2_Rom), f);
		fclose(f);
		le16_to_cpu_array(&_32X_SSH2_Rom[0], sizeof(_32X_SSH2_Rom));
	}
	else
	{
		// Use the reverse-engineered firmware.
		memcpy(&_32X_SSH2_Rom[0], &cbob_bios32xs_bin[0], sizeof(cbob_bios32xs_bin));
		le16_to_cpu_array(&_32X_SSH2_Rom[0], sizeof(cbob_bios32xs_bin));
	}
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_32X_FW_SSH2;
	
	Flag_Clr_Scr = 1;
	Settings.Paused = Frame_Number = 0;
	SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
	Controller_1_COM = Controller_2_COM = 0;
#ifdef GENS_DEBUGGING
	STOP_DEBUGGING();
#endif
	
#if 0	// TODO: Replace with MDP "exclusive mode" later.
	if (!Kaillera_Client_Running)
#endif
	Init_Genesis_SRAM(MD_ROM);
	
	// Check what country code should be used.
	// TODO: Get rid of magic numbers.
	switch (Country)
	{
		default:
		case -1: // Autodetection.
			Detect_Country_Genesis(MD_ROM);
			break;
		
		case 0: // Japan (NTSC)
			Game_Mode = 0;
			CPU_Mode = 0;
			break;
		
		case 1: // US (NTSC)
			Game_Mode = 1;
			CPU_Mode = 0;
			break;
		
		case 2: // Europe (PAL)
			Game_Mode = 1;
			CPU_Mode = 1;
			break;
		
		case 3: // Japan (PAL)
			Game_Mode = 0;
			CPU_Mode = 1;
			break;
	}
	
	VDP_Num_Vis_Lines = 224;
	Gen_Version = 0x20 + 0x0;	// Version de la megadrive (0x0 - 0xF)
	
	// Two copies of the ROM are needed, one for the 68000 and one for the SH2s.
	// SH2 is little-endian, 68000 is big endian.
	// Thus, they need to be byteswapped differently.
	
	// First, copy the ROM to the 32X ROM section.
	memcpy(_32X_Rom, Rom_Data, 4 * 1024 * 1024);
	
	// Byteswap the 68000 ROM data from big-endian (MC68000) to host-endian.
	ROM_ByteSwap_State &= ~ROM_BYTESWAPPED_MD_ROM;
	be16_to_cpu_array(Rom_Data, Rom_Size);
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_MD_ROM;
	
	// Byteswap the SH2 ROM data from little-endian (SH2) to host-endian.
	ROM_ByteSwap_State &= ~ROM_BYTESWAPPED_32X_ROM;
	le16_to_cpu_array(_32X_Rom, Rom_Size);
	ROM_ByteSwap_State |= ROM_BYTESWAPPED_32X_ROM;
	
	// Reset all CPUs and other components.
	MSH2_Reset();
	SSH2_Reset();
	M68K_Reset(1);
	Z80_Reset();
	VDP_Reset();
	_32X_VDP_Reset();
	_32X_Set_FB();
	PWM_Init();
	
	// Initialize the controller state.
	Init_Controllers();
	
	// Set clock rates depending on the CPU mode (NTSC / PAL).
	Set_Clock_Freq(2);
	
	// If auto-fix checksum is enabled, fix the ROM checksum.
	if (Auto_Fix_CS)
		ROM::fixChecksum();
	
	// Initialize sound.
	if (audio_get_enabled())
	{
		audio_end();
		
		if (audio_init(AUDIO_BACKEND_DEFAULT))
			audio_set_enabled(false);
		else
		{
			if (audio_play_sound)
				audio_play_sound();
		}
	}
	
	Reset_Update_Timers();
	
	// Set the appropriate frame update function pointers.
	Update_Frame = Do_32X_Frame;
	Update_Frame_Fast = Do_32X_Frame_No_VDP;
	
	// We patch the Master SH2 bios with ROM bios
	// this permit 32X games with older BIOS version to run correctly
	// Ecco 32X demo needs it
	for (i = 0; i < 0x400; i++)
		_32X_MSH2_Rom[i + 0x36C] = _32X_Rom[i + 0x400];
	
	return 1;
}


/**
 * Reset_32X(): Resets the 32X.
 */
void Reset_32X(void)
{
	int i;
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	Settings.Paused = 0;
	Controller_1_COM = Controller_2_COM = 0;
	_32X_ADEN = _32X_RES = _32X_FM = _32X_RV = 0;
	
	// If the ROM size is smaller than 2MB, enable SRAM.
	// Otherwise, disable SRAM.
	// TODO: Instead of hardcoding 2MB, use SRAM_Start.
	if (Rom_Size <= (2 * 1024 * 1024))
	{
		SRAM_ON = 1;
		SRAM_Write = 1;
	}
	else
	{
		SRAM_ON = 0;
		SRAM_Write = 0;
	}
	
	// Reset all CPUs and other components.
	MSH2_Reset();
	SSH2_Reset();
	M68K_Reset(1);
	Z80_Reset();
	VDP_Reset();
	_32X_VDP_Reset();
	_32X_Set_FB();
	YM2612_Reset();
	PWM_Init();
	
	// Initialize the controller state.
	Init_Controllers();
	
	if (CPU_Mode)
	{
		VDP_Status |= 1;
		_32X_VDP.Mode &= ~0x8000;
	}
	else
	{
		VDP_Status &= ~1;
		_32X_VDP.Mode |= 0x8000;
	}
	
	_32X_VDP.State |= 0x2000;
	
	if (Auto_Fix_CS)
		ROM::fixChecksum();
	
	// We patch the Master SH2 bios with ROM bios
	// this permit 32X games with older BIOS version to run correctly
	// Ecco 32X demo needs it
	
	for (i = 0; i < 0x400; i++)
		_32X_MSH2_Rom[i + 0x36C] = _32X_Rom[i + 0x400];
}


/**
 * Do_32X_VDP_Only(): Updates the Genesis and 32X VDP.
 * @return 0 if successful. TODO: Remove unnecessary return values.
 */
int Do_32X_VDP_Only(void)
{
	// Set the number of visible lines.
	VDP_SET_VISIBLE_LINES();
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		Render_Line_32X();
		Post_Line_32X();
	}

	return 0;
}


/**
 * T_gens_do_32X_frame(): Do a 32X frame.
 * @param VDP If true, VDP is updated.
 */
template<bool VDP>
static inline int T_gens_do_32X_frame(void)
{
	int i, j, k, l, p_i, p_j, p_k, p_l, *buf[2];
	int HInt_Counter, HInt_Counter_32X;
	int CPL_PWM;
	
	// Set the number of visible lines.
	VDP_SET_VISIBLE_LINES();
	
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	CPL_PWM = CPL_M68K * 3;
	
	PWM_Cycles = Cycles_SSH2 = Cycles_MSH2 = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	
	main68k_tripOdometer();
	mdZ80_clear_odo(&M_Z80);
	SH2_Clear_Odo(&M_SH2);
	SH2_Clear_Odo(&S_SH2);
	PWM_Clear_Timer();
	
	// Raise the MDP_EVENT_PRE_FRAME event.
	EventMgr::RaiseEvent(MDP_EVENT_PRE_FRAME, NULL);
	
	VRam_Flag = 1;
	
	VDP_Status &= 0xFFF7;		// Clear V Blank
	if (VDP_Reg.Set4 & 0x2)
		VDP_Status ^= 0x0010;
	_32X_VDP.State &= ~0x8000;
	
	HInt_Counter = VDP_Reg.H_Int;	// Hint_Counter = step d'interruption H
	HInt_Counter_32X = _32X_HIC;
	
	p_i = 84;
	p_j = (p_i * CPL_MSH2) / CPL_M68K;
	p_k = (p_i * CPL_SSH2) / CPL_M68K;
	p_l = p_i * 3;
	
	for (VDP_Current_Line = 0;
	     VDP_Current_Line < VDP_Num_Vis_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		
		i = Cycles_M68K + (p_i * 2);
		j = Cycles_MSH2 + (p_j * 2);
		k = Cycles_SSH2 + (p_k * 2);
		l = PWM_Cycles + (p_l * 2);
		
		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_MSH2 += CPL_MSH2;
		Cycles_SSH2 += CPL_SSH2;
		Cycles_Z80 += CPL_Z80;
		PWM_Cycles += CPL_PWM;
		if (DMAT_Length)
			main68k_addCycles(Update_DMA());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		_32X_VDP.State |= 0x6000;
		
		main68k_exec(i - p_i);
		SH2_EXEC(j - p_j, k - p_k);
		PWM_Update_Timer (l - p_l);
		
		VDP_Status &= ~0x0004;	// HBlank = 0
		_32X_VDP.State &= ~0x6000;
		
		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			VDP_Update_IRQ_Line();
		}
		
		if (--HInt_Counter_32X < 0)
		{
			HInt_Counter_32X = _32X_HIC;
			if (_32X_MINT & 0x04)
				SH2_Interrupt(&M_SH2, 10);
			if (_32X_SINT & 0x04)
				SH2_Interrupt(&S_SH2, 10);
		}
		
		if (VDP)
		{
			// VDP needs to be updated.
			Render_Line_32X();
			Post_Line_32X();
		}
		
		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			SH2_EXEC(j, k);
			PWM_Update_Timer(l);
			i += p_i;
			j += p_j;
			k += p_k;
			l += p_l;
		}
		
		main68k_exec(Cycles_M68K);
		SH2_EXEC(Cycles_MSH2, Cycles_SSH2);
		PWM_Update_Timer(PWM_Cycles);
		
		Z80_EXEC(0);
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	PWM_Update (buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	
	i = Cycles_M68K + p_i;
	j = Cycles_MSH2 + p_j;
	k = Cycles_SSH2 + p_k;
	l = PWM_Cycles + p_l;
	
	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_MSH2 += CPL_MSH2;
	Cycles_SSH2 += CPL_SSH2;
	Cycles_Z80 += CPL_Z80;
	PWM_Cycles += CPL_PWM;
	if (DMAT_Length)
		main68k_addCycles(Update_DMA());
	
	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		VDP_Update_IRQ_Line();
	}
	
	if (--HInt_Counter_32X < 0)
	{
		HInt_Counter_32X = _32X_HIC;
		if (_32X_MINT & 0x04)
			SH2_Interrupt(&M_SH2, 10);
		if (_32X_SINT & 0x04)
			SH2_Interrupt(&S_SH2, 10);
	}
	
	VDP_Status |= 0x000C;		// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	_32X_VDP.State |= 0xE000;	// VBlank = 1, HBlank = 1, PEN = 1
	
	if (_32X_VDP.State & 0x10000)
		_32X_VDP.State |= 1;
	else
		_32X_VDP.State &= ~1;
	
	_32X_Set_FB();
	
	while (i < (Cycles_M68K - 360))
	{
		main68k_exec(i);
		SH2_EXEC(j, k);
		PWM_Update_Timer(l);
		i += p_i;
		j += p_j;
		k += p_k;
		l += p_l;
	}
	
	main68k_exec(Cycles_M68K - 360);
	Z80_EXEC(168);
	
	VDP_Status &= ~0x0004;		// HBlank = 0
	_32X_VDP.State &= ~0x4000;
	VDP_Status |= 0x0080;		// V Int happened
	
	VDP_Int |= 0x8;
	VDP_Update_IRQ_Line();
	
	if (_32X_MINT & 0x08)
		SH2_Interrupt(&M_SH2, 12);
	if (_32X_SINT & 0x08)
		SH2_Interrupt(&S_SH2, 12);
	
	mdZ80_interrupt(&M_Z80, 0xFF);
	
	while (i < Cycles_M68K)
	{
		main68k_exec(i);
		SH2_EXEC(j, k);
		PWM_Update_Timer(l);
		i += p_i;
		j += p_j;
		k += p_k;
		l += p_l;
	}
	
	main68k_exec(Cycles_M68K);
	SH2_EXEC(Cycles_MSH2, Cycles_SSH2);
	PWM_Update_Timer(PWM_Cycles);
	
	Z80_EXEC(0);
	
	for (VDP_Current_Line++;
	     VDP_Current_Line < VDP_Num_Lines;
	     VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		
		i = Cycles_M68K + (p_i * 2);
		j = Cycles_MSH2 + (p_j * 2);
		k = Cycles_SSH2 + (p_k * 2);
		l = PWM_Cycles + (p_l * 2);
		
		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_MSH2 += CPL_MSH2;
		Cycles_SSH2 += CPL_SSH2;
		Cycles_Z80 += CPL_Z80;
		PWM_Cycles += CPL_PWM;
		if (DMAT_Length)
			main68k_addCycles(Update_DMA());
		
		VDP_Status |= 0x0004;	// HBlank = 1
		_32X_VDP.State |= 0x6000;
		
		main68k_exec (i - p_i);
		SH2_EXEC(j - p_j, k - p_k);
		PWM_Update_Timer (l - p_l);
		
		VDP_Status &= ~0x0004;	// HBlank = 0
		_32X_VDP.State &= ~0x6000;
		
		if (--HInt_Counter_32X < 0)
		{
			HInt_Counter_32X = _32X_HIC;
			if ((_32X_MINT & 0x04) && (_32X_MINT & 0x80))
				SH2_Interrupt(&M_SH2, 10);
			if ((_32X_SINT & 0x04) && (_32X_SINT & 0x80))
				SH2_Interrupt(&S_SH2, 10);
		}
		
		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			SH2_EXEC(j, k);
			PWM_Update_Timer(l);
			i += p_i;
			j += p_j;
			k += p_k;
			l += p_l;
		}
		
		main68k_exec(Cycles_M68K);
		SH2_EXEC(Cycles_MSH2, Cycles_SSH2);
		PWM_Update_Timer(PWM_Cycles);
		
		Z80_EXEC(0);
	}
	
	PSG_Special_Update();
	YM2612_Special_Update();
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		wav_dump_update();
	if (GYM_Dumping)
		gym_dump_update(0, 0, 0);
	
	// Raise the MDP_EVENT_POST_FRAME event.
	mdp_event_post_frame_t post_frame;
	if (bppMD == 32)
		post_frame.md_screen = &MD_Screen32[8];
	else
		post_frame.md_screen = &MD_Screen[8];
	post_frame.width = (vdp_isH40() ? 320 : 256);
	post_frame.height = VDP_Num_Vis_Lines;
	post_frame.pitch = 336;
	post_frame.bpp = bppMD;
	
	EventMgr::RaiseEvent(MDP_EVENT_POST_FRAME, &post_frame);
	
	return 1;
}


int Do_32X_Frame_No_VDP(void)
{
	return T_gens_do_32X_frame<false>();
}
int Do_32X_Frame(void)
{
	return T_gens_do_32X_frame<true>();
}

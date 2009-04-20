/***************************************************************************
 * Gens: GSX v7 Savestate Structs.                                         *
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

#ifndef GENS_GSX_V7_H
#define GENS_GSX_V7_H

// Make sure structs are packed.
#ifdef PACKED
#undef PACKED
#endif

#include <stdint.h>

#define PACKED __attribute__ ((packed))

// GSX v7 savestate structs.
// All integer data types are stored in little-endian.

/**
 * gsx_v7_ym2612_slot: GSX v7 YM2612 slot struct definition.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_ym2612_slot
{
	unsigned int DT; // Detune parameter.
	int MUL;	// "Multiple Frequency" parameter.
	int TL;		// Total Level == volume when the envelope is at the highest point.
	int TLL;	// Total Level, adjusted.
	int SLL;	// Sustain level, adjusted. Volume where the envelope ends its first phase of regression.
	int KSR_S;	// Key Scale Rate Shift = factor to take account of KSL in the variations of the envelope.
	int KSR;	// Key Scale Rate = value calculated in relation to the current frequency. It will influence
				// the parameters of the envelope based on the attack, decay... as in reality!
	int SEG;	// Envelope type. (SSG?)
	
	// In the Gens v7 savestate extension, these are normalized to valies between 0-63.
	// They are converted back to pointers by ym2612.c.
	unsigned int AR; // Attack Rate (table pointer offset)		= AR[KSR]
	unsigned int DR; // Decay Rate (table pointer offset)		= DR[KSR]
	unsigned int SR; // Sustain Rate (table pointer offset)		= SR[KSR]
	unsigned int RR; // Release Rate (table pointer offset)		= RR[KSR]
	
	int Fcnt;	// Frequency count = frequency meter to determine the current amplitude. (SIN[Finc >> 16])
	int Finc;	// Frequency step = pas d'incrémentation du compteur-fréquence
				// plus le pas est grand, plus la fréquence est aïgu (ou haute)
	
	int Ecurp;	// Envelope current phase = current phase of the ADSR envelope we are on, for example
				// attack phase or decay phase... we will call a function to update the current
				// envelope based on thi value.
	
	int Ecnt;	// Envelope counter = Current position in the envelope.
	int Einc;	// Current envelope step.
	int Ecmp;	// Envelope counter limit for the next phase.
	
	int EincA;	// Envelope step for Attack = increments the counter during attack phase that is equal to AR[KSR].
	int EincD;	// Envelope step for Attack = increments the counter during decay phase that is equal to DR[KSR].
	int EincS;	// Envelope step for Attack = increments the counter during sustain phase that is equal to SR[KSR].
	int EincR;	// Envelope step for Attack = increments the counter during release phase that is equal to RR[KSR].
	
	// NOTE: This seems to be unused...
	int OUTp;	// Pointer of SLOT output = pointer to connect the output of this slot at the entrance to
				// another slot. [TODO: Check this!]
	
	int INd;	// Input data of the slot = data entry slot
	int ChgEnM;	// Change envelope mask.
	int AMS;	// AMS depth level of this SLOT = degree of amplitude modulation by the LFO.
	int AMSon;	// AMS enable flag.
} gsx_v7_ym2612_slot;
#pragma pack()

/**
 * gsx_v7_ym2612_channel: GSX v7 YM2612 channel struct definition.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_ym2612_channel
{
	int S0_OUT[4];		// Previous Slot 0 output. (for feedback)
	int Old_OUTd;		// Previous channel output. (mixed)
	int OUTd;		// Current channel output. (mixed)
	int LEFT;		// LEFT enable flag.
	int RIGHT;		// RIGHT enable flag.
	int ALGO;		// Algorithm. (Determines the connections between operators.)
	int FB;			// Shift count of self feedback. (Degree of feedback from Slot 1.)
	int FMS;		// Frequency Modulation Sensitivity = degree of channel modulation of the frequency by LFO.
	int AMS;		// Amplitude Modulation Sensitivity = degree of channel modulation of the amplitude by LFO.
	int FNUM[4];		// High frequency of the channel. (+ 3 for the special mode)
	int FOCT[4];		// Octave of the channel. (+ 3 for the special mode)
	int KC[4];		// Key Code = value based on the frequency. (ee slots for KSR, KSR == KC >> KSR_S)
	gsx_v7_ym2612_slot slot[4];	// Slot operators. (4 slots per channel)
	int FFlag;		// Frequency step recalculation flag.
} gsx_v7_ym2612_channel;
#pragma pack()

/**
 * gsx_v7_ym2612: GSX v7 YM2612 struct definition.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_ym2612
{
	int clock_freq;		// YM2612 clock frequency. (Hz)
	int sample_rate;	// Sample rate. (11025, 22050, 44100)
	int timer_base;		// Timer base calculation.
	int status;		// YM2612 status. (timer overflow)
	int OPNA_addr;		// OPNA address.
	int OPNB_addr;		// OPNB address.
	int LFOcnt;		// LFO counter.
	int LFOinc;		// LFO step counter.
	
	int timerA;		// TimerA limit.
	int timerAL;
	int timerAcnt;		// TimerA counter.
	int timerB;		// TimerB limit.
	int timerBL;
	int timerBcnt;		// TimerB counter.
	int mode;		// Mode. (3 == normal; 6 == special) [TODO: Check this!]
	int dac_enabled;	// DAC enabled flag.
	int dac_data;		// DAC data.
	
	int reserved1;
	double frequency_base;	// Frequency base. Calculated from the sample rate.
	
	unsigned int interp_cnt;	// Interpolation counter.
	unsigned int interp_step;	// Interpolation step.
	
	gsx_v7_ym2612_channel	channels[6];	// YM2612 channels.
	
	unsigned int reg[2][0x100];	// YM2612 registers.
} gsx_v7_ym2612;
#pragma pack()


/**
 * gsx_v7_psg: GSX v7 PSG struct definition.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_psg
{
	int current_channel;
	int current_reg;
	int reg[8];
	unsigned int counter[4];
	unsigned int cntstep[4];
	int volume[4];
	unsigned int noise_type;
	unsigned int noise;
} gsx_v7_psg;
#pragma pack()


/**
 * gsx_v7_z80_reg: Z80 registers.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_z80_reg
{
	uint16_t FA;	// Stored backwards for some reason.
	uint8_t  reserved_AF;
	uint8_t  FXY;
	
	uint16_t BC;
	uint16_t reserved_BC;
	
	uint16_t DE;
	uint16_t reserved_DE;
	
	uint16_t HL;
	uint16_t reserved_HL;
	
	uint16_t IX;
	uint16_t reserved_IX;
	
	uint16_t IY;
	uint16_t reserved_IY;
	
	// This was originally "PC", but mdZ80 stores the program counter
	// as an x86 pointer, not as a Z80 pointer. So, it is written as
	// 0xDEADBEEF (big-endian) in the Gens v7 savestate extension,
	// and the original Gens v5 Z80 PC is used instead.
	uint32_t DEADBEEF_1;	// be32
	
	uint16_t SP;
	uint16_t reserved_SP;
	
	uint16_t FA2;	// Stored backwards for some reason.
	uint8_t  reserved_AF2;
	uint8_t  FXY2;
	
	uint16_t BC2;
	uint16_t reserved_BC2;
	
	uint16_t DE2;
	uint16_t reserved_DE2;
	
	uint16_t HL2;
	uint16_t reserved_HL2;
	
	uint8_t  IFF1;
	uint8_t  IFF2;
	uint16_t reserved_IFF;
	
	uint16_t R;
	uint16_t reserved_R;
	
	uint8_t I;
	uint8_t IM;
	uint8_t IntVect;
	uint8_t IntLine;
	
	uint32_t status;
	
	// This was originally "BasePC", but mdZ80 stores the program counter
	// as an x86 pointer, not as a Z80 pointer. So, it is written as
	// 0xDEADBEEF (big-endian) in the Gens v7 savestate extension,
	// and the original Gens v5 Z80 PC is used instead.
	uint32_t DEADBEEF_2;	// be32
	
	uint32_t TmpSav0;
	uint32_t TmpSav1;
	
	uint32_t CycleCnt;
	uint32_t CycleTD;
	uint32_t CycleIO;
	uint32_t CycleSup;
	
	// Other variables that aren't in the same order as mdZ80,
	// but are used by the Gens v7 savestate format.
	uint32_t RetIC;
	uint32_t IntAckC;
} gsx_v7_z80_reg;
#pragma pack()


/**
 * gsx_v7_mc68000_reg: MC68000 registers.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_mc68000_reg
{
	uint32_t dreg[8];
	uint32_t areg[8];
	uint32_t asp;
	uint32_t pc;
	uint32_t odometer;
	uint8_t  interrupts[8];
	uint16_t sr;
} gsx_v7_mc68000_reg;
#pragma pack()


/**
 * gsx_v7_controller_port: Controller port information.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_controller_port
{
	uint32_t state;
	uint32_t COM;
	uint32_t counter;
	uint32_t delay;
} gsx_v7_controller_port;
#pragma pack()

/**
 * gsx_v7_controller_status: Controller status information.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_controller_status
{
	uint32_t type;
	uint32_t up;
	uint32_t down;
	uint32_t left;
	uint32_t right;
	uint32_t start;
	uint32_t mode;
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t X;
	uint32_t Y;
	uint32_t Z;
} gsx_v7_controller_status;
#pragma pack()

/**
 * gsx_v7_controllers: Controller status.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_controllers
{
	gsx_v7_controller_port		port1;
	gsx_v7_controller_status	player1;
	
	gsx_v7_controller_port		port2;
	gsx_v7_controller_status	player2;
	
	gsx_v7_controller_status	player1B;
	gsx_v7_controller_status	player1C;
	gsx_v7_controller_status	player1D;
	
	gsx_v7_controller_status	player2B;
	gsx_v7_controller_status	player2C;
	gsx_v7_controller_status	player2D;
} gsx_v7_controllers;
#pragma pack()


/**
 * gsx_v7_sram: Save RAM.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_sram
{
	uint8_t  sram[64 * 1024];
	uint32_t sram_start;
	uint32_t sram_end;
	uint32_t sram_on;
	uint32_t sram_write;
	uint32_t sram_custom;
} gsx_v7_sram;
#pragma pack()


/**
 * gsx_v7_starscream_extra: Extra Starscream 68000 information.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v7_starscream_extra
{
	uint32_t cycles_needed;
	uint32_t cycles_leftover;
	uint32_t fetch_region_start;
	uint32_t fetch_region_end;
	
	uint8_t  xflag;
	uint8_t  execinfo;
	uint8_t  trace_trickybit;
	uint8_t  filler;
	
	uint32_t io_cycle_counter;
	uint32_t io_fetchbase;
	uint32_t io_fetchbased_pc;
	uint32_t access_address;
	uint32_t save_01;
	uint32_t save_02;
} gsx_v7_starscream_extra;
#pragma pack()


/**
 * gsx_struct_md_v7_t: Gens v7 savestate extensions. (MD portion)
 */
#pragma pack(1)
typedef struct PACKED _gsx_struct_md_v7_t
{
	gsx_v7_ym2612		ym2612;
	gsx_v7_psg		psg;
	gsx_v7_z80_reg		z80_reg;
	gsx_v7_mc68000_reg	mc68000_reg;
	
	gsx_v7_controllers	controllers;
	
	// Miscellaneous.
	uint32_t vdp_status;
	uint32_t vdp_int;
	uint32_t vdp_current_line;
	uint32_t vdp_num_lines;
	uint32_t vdp_num_vis_lines;
	uint32_t dmat_length;
	uint32_t dmat_type;
	uint32_t lag_count;	// Gens Rerecording
	uint32_t vram_flag;
	
	// Color RAM.
	uint8_t  cram[256*2];
	
	// Save RAM.
	gsx_v7_sram		sram;
	
	// More miscellaneous.
	uint32_t bank_m68k;
	uint32_t s68k_state;
	uint32_t z80_state;
	uint32_t last_bus_req_cnt;
	uint32_t last_bus_req_st;
	uint32_t fake_fetch;
	uint32_t game_mode;
	uint32_t cpu_mode;
	uint32_t cpl_m68k;
	uint32_t cpl_s68k;
	uint32_t cpl_z80;
	uint32_t cycles_s68k;
	uint32_t cycles_m68k;
	uint32_t cycles_z80;
	uint32_t gen_mode;
	uint32_t gen_version;
	
	// H Counter Table.
	uint8_t  h_counter_table[512][2];
	
	// VDP registers.
	uint32_t vdp_reg[26];
	
	// VDP control.
	uint32_t vdp_ctrl[7];
	
	// Extra Starscream 68000 information.
	gsx_v7_starscream_extra		starscream_extra;
} gsx_struct_md_v7_t;
#pragma pack()

#endif /* GENS_GSX_V7_H */

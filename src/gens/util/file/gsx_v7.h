/***************************************************************************
 * Gens: GSX v7 Savestate Structs.                                         *
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

#ifndef GENS_GSX_V7_H
#define GENS_GSX_V7_H

// Make sure structs are packed.
#ifdef PACKED
#undef PACKED
#endif

#include <stdint.h>

#define PACKED __attribute__ ((__packed__))

// GSX v7 savestate structs.
// All integer data types are stored in little-endian.

/**
 * gsx_v7_ym2612_slot: GSX v7 YM2612 slot struct definition.
 */
typedef struct PACKED _gsx_v7_ym2612_slot
{
	int DT;		// Detune parameter.
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
	int AR;		// Attack Rate (table pointer)	= AR[KSR]
	int DR;		// Decay Rate (table pointer)	= DR[KSR]
	int SR;		// Sustain Rate (table pointeR)	= SR[KSR]
	int RR;		// Release Rate (table pointer)	= RR[KSR]
	
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

/**
 * gsx_v7_ym2612_channel: GSX v7 YM2612 channel struct definition.
 */
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

/**
 * gsx_v7_ym2612: GSX v7 YM2612 struct definition.
 */
typedef struct PACKED _gsx_v7_ym2612
{
	int clock_freq;		// YM2612 clock frequency. (Hz)
	int sample_rate;		// Sample rate. (11025, 22050, 44100)
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


/**
 * gsx_v7_psg: GSX v7 PSG struct definition.
 */
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


/**
 * gsx_v7_z80_reg: Z80 registers.
 */
typedef struct PACKED _gsx_v7_z80_reg
{
	uint16_t FA;	// Stored backwards for some reason.
	uint8_t  reserved_AF;
	uint8_t  FXY;
	
	uint16_t BC;
	uint16_t reservedBC;
	
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
	// 0xDEADBEEF (big-endian) in the Gens v7 savestate.
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
	// 0xDEADBEEF (big-endian) in the Gens v7 savestate.
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


/**
 * gsx_v7_mc68000_reg: MC68000 registers.
 */
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


#endif /* GENS_GSX_V7_H */

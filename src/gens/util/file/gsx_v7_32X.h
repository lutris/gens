/***************************************************************************
 * Gens: GSX v7 Savestate Structs. (32X)                                   *
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

#ifndef GENS_GSX_V7_32X_H
#define GENS_GSX_V7_32X_H

// Make sure structs are packed.
#ifdef PACKED
#undef PACKED
#endif

#include <stdint.h>

#define PACKED __attribute__ ((packed))

// GSX v7 32X savestate structs.
// All integer data types are stored in little-endian.

#pragma pack(1)
typedef struct PACKED _gsx_v7_32X_cpu
{
	uint8_t		Cache[0x1000];
	uint32_t	R[0x10];
	
	struct _SR
	{
		uint8_t T;
		uint8_t S;
		uint8_t IMask;
		uint8_t MQ;
	} SR;
	
	struct _INT
	{
		uint8_t Vect;
		uint8_t Prio;
		uint8_t res1;
		uint8_t res2;
	} INT;
	
	uint32_t	GBR;
	uint32_t	VBR;
	
	uint8_t		INT_QUEUE[0x20];
	
	uint32_t	MACH;
	uint32_t	MACL;
	uint32_t	PR;
	uint32_t	PC;
	
	uint32_t	Status;
	uint32_t	Base_PC;	// Yes, this again. -_-
	uint32_t	Fetch_Start;
	uint32_t	Fetch_End;
	
	uint32_t	DS_Inst;
	uint32_t	DS_PC;
	
	uint32_t	Odometer;
	uint32_t	Cycle_TD;
	uint32_t	Cycle_IO;
	uint32_t	Cycle_Sup;
	
	uint8_t		IO_Reg[0x200];
	
	uint32_t	DVCR;
	uint32_t	DVSR;
	uint32_t	DVDNTH;
	uint32_t	DVDNTL;
	
	uint8_t		DRCR0;
	uint8_t		DRCR1;
	uint8_t		DREQ0;
	uint8_t		DREQ1;
	
	uint32_t	DMAOR;
	
	uint32_t	SAR0;
	uint32_t	DAR0;
	uint32_t	TCR0;
	uint32_t	CHCR0;
	
	uint32_t	SAR1;
	uint32_t	DAR1;
	uint32_t	TCR1;
	uint32_t	CHCR1;
	
	uint32_t	VCRDIV;
	uint32_t	VCRDMA0;
	uint32_t	VCRDMA1;
	uint32_t	VCRWDT;
	
	uint32_t	IPDIV;
	uint32_t	IPDMA;
	uint32_t	IPWDT;
	uint32_t	IPBSC;
	
	uint32_t	BARA;
	uint32_t	BAMRA;
	
	uint8_t		WDT_Tab[8];
	uint32_t	WDTCNT;
	uint8_t		WDT_Sft;
	uint8_t		WDTSR;
	uint8_t		WDTRST;
	
	uint8_t		FRT_Tab[4];
	uint32_t	FRTCNT;
	uint32_t	FRTOCRA;
	uint32_t	FRTOCRB;
	
	uint8_t		FRTTIER;
	uint8_t		DUPE1_FRTCSR;	// Same as FRTCSR.
	uint8_t		FRTTCR;
	uint8_t		FRTTOCR;
	uint32_t	FRTICR;
	uint32_t	FRT_Sft;
	uint32_t	BCR1;
	uint8_t		FRTCSR;
} gsx_v7_32X_cpu;
#pragma pack()

#pragma pack(1)
typedef struct PACKED _gsx_v7_32X
{
	gsx_v7_32X_cpu	cpu[2];
	
	uint8_t		_32x_ram[256 * 1024];
	uint8_t		msh2_reg[0x40];
	uint8_t		ssh2_reg[0x40];
	uint8_t		sh2_vdp_reg[0x10];
	
	uint8_t		_32x_comm[0x10];
	uint8_t		_32x_aden;
	uint8_t		_32x_res;
	uint8_t		_32x_fm;
	uint8_t		_32x_rv;
	
	uint32_t	_32x_dreq_st;
	uint32_t	_32x_dreq_src;
	uint32_t	_32x_dreq_dst;
	uint32_t	_32x_dreq_len;
	
	uint16_t	_32x_fifo_A[4];
	uint16_t	_32x_fifo_B[4];
	
	uint32_t	_32x_fifo_block;
	uint32_t	_32x_fifo_read;
	uint32_t	_32x_fifo_write;
	
	uint8_t		_32x_mint;
	uint8_t		_32x_sint;
	uint8_t		_32x_hic;
	
	uint32_t	cpl_ssh2;
	uint32_t	cpl_msh2;
	int		cycles_msh2;
	int		cycles_ssh2;
	
	struct _vdp
	{
		uint32_t	mode;
		uint32_t	state;
		uint32_t	af_data;
		uint32_t	af_st;
		uint32_t	af_len;
		uint32_t	af_line;
	} vdp;
	
	uint8_t		vdp_ram[256 * 1024];
	uint16_t	vdp_cram[256];
	
	uint32_t	set_sr_table[0x400];
	int		bank_sh2;
	
	uint16_t	pwm_fifo_R[8];
	uint16_t	pwm_fifo_L[8];
	uint32_t	pwm_rp_R;
	uint32_t	pwm_wp_R;
	uint32_t	pwm_rp_L;
	uint32_t	pwm_wp_L;
	uint32_t	pwm_cycles;
	uint32_t	pwm_cycle;
	uint32_t	pwm_cycle_cnt;
	uint32_t	pwm_int;
	uint32_t	pwm_int_cnt;
	uint32_t	pwm_mode;
	uint32_t	pwm_out_R;
	uint32_t	pwm_out_L;
	
	uint8_t		rom_header[1024];
	uint8_t		_32x_msh2_rom[2 * 1024];
	uint8_t		_32x_ssh2_rom[1 * 1024];
} gsx_v7_32X;
#pragma pack()

#endif /* GENS_GSX_V7_32X_H */

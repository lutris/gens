/***************************************************************************
 * Gens: GSX Savestate Struct Definitions.                                 *
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

#ifndef GENS_GSX_STRUCT_H
#define GENS_GSX_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Make sure structs are packed.
#define PACKED __attribute__ ((__packed__))

// Gens v7 savestate extensions.
#include "gsx_v7.h"

// GSX savestate structs.
// All integer data types are stored in little-endian.

/**
 * gsx_struct_md_t: Gens v5 savestate data.
 * This struct also contains the GSX header.
 */
typedef struct PACKED _gsx_struct_md_t
{
	// [0x00] Header.
	struct PACKED
	{
		unsigned char magic[0x05];	// Magic Number. {'G', 'S', 'T', 0x40, 0xE0}
		uint8_t reserved1[0x3B];
	} header;
	
	// [0x40] VDP control data. [Gens/Kega specific.]
	struct
	{
		uint32_t ctrl_data;		// Last VDP control data written.
		uint8_t  write_flag_2;		// Second write flag. (1 for second write) (Ctrl.Flag)
		uint8_t  dma_fill_flag;		// DMA fill flag. (1 means next data write will cause a DMA fill) ((Ctrl.DMA >> 2) & 1)
		uint16_t ctrl_access;		// VDP control access data. (Gens Rerecording; added by Nitsuja)
		uint32_t write_address;		// VDP write address.
		uint32_t reserved;		// Reserved.
	} vdp_ctrl;
	
	// [0x50] Version ID.
	struct {
		uint8_t version;		// Savestate version.
		uint8_t emulator;		// Emulator ID.
		uint8_t reserved2[0x0E];
	} version;
	
	uint16_t psg[0x08];		// [0x60] PSG register data.
	uint8_t reserved1[0x10];	// [0x70] Reserved.
	
	// [0x80] MC68000 registers.
	struct PACKED
	{
		uint32_t dreg[8];	// Data registers. (D0-D7)
		uint32_t areg[8];	// Address registers. (A0-A7)
		uint8_t  reserved1[8];
		uint32_t pc;		// Program counter.
		uint32_t reserved2;	// Reserved.
		uint16_t sr;		// Status register.
		
		// Stack pointers. (0xD2, 0xD6)
		uint32_t usp;	// User stack pointer.
		uint32_t ssp;	// Supervisor stack pointer.
	} mc68000_reg;
	
	uint8_t  reserved2[0x20];	// [0xDA] Reserved.
	uint8_t  vdp_reg[0x18];		// [0xFA] VDP registers.
	uint8_t  cram[0x80];		// [0x112] CRAM (Color RAM).
	uint8_t  vsram[0x50];		// [0x192] VSRAM (Vertical Scroll RAM).
	uint16_t reserved3;		// [0x1E2] Reserved.
	uint8_t  ym2612[0x200];		// [0x1E4] YM2612 registers.
	uint8_t  reserved4[0x20];	// [0x3E4] Reserved.
	
	// [0x404] Z80 registers.
	struct PACKED
	{
		uint16_t AF;
		uint8_t  FXY; // Gens Rerecording addition.
		uint8_t  reserved1;
		uint16_t BC;
		uint16_t reserved2;
		uint16_t DE;
		uint16_t reserved3;
		uint16_t HL;
		uint16_t reserved4;
		uint16_t IX;
		uint16_t reserved5;
		uint16_t IY;
		uint16_t reserved6;
		uint16_t PC;
		uint16_t reserved7;
		uint16_t SP;
		uint16_t reserved8;
		uint16_t AF2;
		uint16_t reserved9;
		uint16_t BC2;
		uint16_t reservedA;
		uint16_t DE2;
		uint16_t reservedB;
		uint16_t HL2;
		uint16_t reservedC;
		uint8_t  I;
		uint8_t  reservedD;
		uint8_t  IFF1;
		uint8_t  reservedE;
		uint8_t  state_reset;
		uint8_t  state_busreq;
		uint16_t reservedF;
		uint32_t bank;
	} z80_reg;
	
	uint8_t  reserved5[0x34];	// [0x440] Reserved.
	uint8_t  z80_ram[0x2000];	// [0x474] Z80 RAM.
	uint32_t reserved6;		// [0x2474] Reserved.
	uint8_t  mc68000_ram[0x10000];	// [0x2478] MC68000 RAM. [be16]
	uint8_t  vram[0x10000];		// [0x12478] VRAM. [be16]
	
	// Frame Count. (Gens Rerecording)
	uint32_t frame_count;
} gsx_struct_md_t;


/**
 * gsx_struct_md_v7_t: Gens v7 savestate extensions (MD portion).
 */
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
	uint8_t  h_counter_table[512 * 2];
	
	// VDP registers.
	uint32_t vdp_reg[26];
	
	// VDP control.
	uint32_t vdp_ctrl[7];
	
	// Extra Starscream 68000 information.
	gsx_v7_starscream_extra		starscream_extra;
} gsx_struct_md_v7_t;

#ifdef __cplusplus
}
#endif

#endif /* GENS_GSX_STRUCT_H */

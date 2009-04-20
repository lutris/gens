/***************************************************************************
 * Gens: GSX v6 Savestate Structs.                                         *
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

#ifndef GENS_GSX_V6_H
#define GENS_GSX_V6_H

// Make sure structs are packed.
#ifdef PACKED
#undef PACKED
#endif

#include <stdint.h>

#define PACKED __attribute__ ((packed))

// GSX v6 savestate structs.
// All integer data types are stored in little-endian.

/**
 * gsx_v6_mc68000_reg: MC68000 registers.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v6_mc68000_reg
{
	uint32_t dreg[8];
	uint32_t areg[8];
	uint32_t asp;
	uint32_t pc;
	uint32_t odometer;
	uint8_t  interrupts[8];
	uint16_t sr;
	uint16_t contextfiller00;
} gsx_v6_mc68000_reg;
#pragma pack()

/**
 * gsx_v6_vdp_reg: VDP registers.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v6_vdp_reg
{
	uint32_t DUPE_H_Int;		// Contains H_Int, but isn't used.
	uint32_t Set1;
	uint32_t Set2;
	uint32_t DUPE_Pat_ScrA_Adr;	// Contains Pat_ScrA_Adr, but isn't used.
	uint32_t Pat_ScrA_Adr;
	uint32_t Pat_Win_Adr;
	uint32_t Pat_ScrB_Adr;
	uint32_t Spr_Att_Adr;
	uint32_t Reg6;
	uint32_t BG_Color;
	uint32_t Reg8;
	uint32_t Reg9;
	uint32_t H_Int;
	uint32_t Set3;
	uint32_t Set4;
	uint32_t H_Scr_Adr;
	uint32_t Reg14;
	uint32_t Auto_Inc;
	uint32_t Scr_Size;
	uint32_t Win_H_Pos;
	uint32_t Win_V_Pos;
	uint32_t DMA_Length_L;
	uint32_t DMA_Length_H;
	uint32_t DMA_Src_Adr_L;
	uint32_t DMA_Src_Adr_M;
	uint32_t DMA_Src_Adr_H;
	uint32_t DMA_Length;
	uint32_t DMA_Address;
} gsx_v6_vdp_reg;
#pragma pack()

/**
 * gsx_v6_controller_port: Controller port information.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v6_controller_port
{
	uint32_t counter;
	uint32_t delay;
	uint32_t state;
	uint32_t COM;
} gsx_v6_controller_port;
#pragma pack()

/**
 * gsx_v6_controller_status: Controller status information.
 */
#pragma pack(1)
typedef struct PACKED _gsx_v6_controller_status
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
} gsx_v6_controller_status;
#pragma pack()


/**
 * gsx_struct_md_v6_t: Gens v6 savestate extensions. (MD portion)
 */
#pragma pack(1)
typedef struct PACKED _gsx_struct_md_v6_t
{
	gsx_v6_mc68000_reg	mc68000_reg;
	gsx_v6_vdp_reg		vdp_reg;
	
	// Controller port information.
	gsx_v6_controller_port	control_port1;
	gsx_v6_controller_port	control_port2;
	
	uint32_t	memory_control_status;
	uint32_t	cell_conv_tab;
	
	// Controller status information.
	gsx_v6_controller_status	player1;
	gsx_v6_controller_status	player2;
	
	// Miscellaneous.
	uint32_t	dmat_length;
	uint32_t	dmat_type;
	uint32_t	dmat_tmp;
	uint32_t	vdp_current_line;
	uint32_t	DUPE_vdp_num_vis_lines;
	uint32_t	vdp_num_vis_lines;
	uint32_t	bank_m68k;
	uint32_t	s68k_state;
	uint32_t	z80_state;
	uint32_t	last_bus_req_cnt;
	uint32_t	last_bus_req_st;
	uint32_t	fake_fetch;
	uint32_t	game_mode;
	uint32_t	cpu_mode;
	uint32_t	cpl_m68k;
	uint32_t	cpl_s68k;
	uint32_t	cpl_z80;
	uint32_t	cycles_s68k;
	uint32_t	cycles_m68k;
	uint32_t	cycles_z80;
	uint32_t	vdp_status;
	uint32_t	vdp_int;
	uint32_t	vdp_ctrl_write;
	uint32_t	vdp_ctrl_dma_mode;
	uint32_t	vdp_ctrl_dma;
	uint32_t	lag_count;		// Gens Rerecording
	
	uint32_t	vram_flag;
	uint32_t	DUPE1_vdp_reg_dma_length;
	uint32_t	vdp_reg_auto_inc;
	uint32_t	DUPE2_vdp_reg_dma_length;
	
	uint8_t		cram[512];
	uint8_t		h_counter_table[512][2];
	
	uint32_t	vdp_reg_dma_length_l;
	uint32_t	vdp_reg_dma_length_h;
	uint32_t	vdp_reg_dma_src_adr_l;
	uint32_t	vdp_reg_dma_src_adr_m;
	uint32_t	vdp_reg_dma_src_adr_h;
	uint32_t	vdp_reg_dma_length;
	uint32_t	vdp_reg_dma_address;
} gsx_struct_md_v6_t;
#pragma pack()

#endif /* GENS_GSX_V6_H */

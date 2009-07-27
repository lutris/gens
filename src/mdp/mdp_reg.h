/***************************************************************************
 * MDP: Mega Drive Plugins - Register Defines.                             *
 *                                                                         *
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

#ifndef __MDP_REG_H
#define __MDP_REG_H

#include "mdp_fncall.h"
#include "mdp_stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/** MDP_REG_IC: Register IC IDs. **/
typedef enum _MDP_REG_IC
{
	/*! BEGIN: MDP v1.0 register IC IDs. !*/
	MDP_REG_IC_NULL		= 0,
	MDP_REG_IC_M68K		= 1,
	MDP_REG_IC_VDP		= 2,
	MDP_REG_IC_YM2612	= 3,
	MDP_REG_IC_PSG		= 4,
	MDP_REG_IC_Z80		= 5,
	/*! END: MDP v1.0 register IC IDs. !*/
	
#if 0
	/* TODO: These will be implemented in a future version of MDP. */
	MDP_REG_IC_S68K		= 6,
	MDP_REG_IC_MSH2		= 7,
	MDP_REG_IC_SSH2		= 8,
#endif
	
	MDP_REG_IC_MAX
} MDP_REG_IC;


/* MDP_REG_68K: 68K register IDs. */
typedef enum _MDP_REG_68K
{
	MDP_REG_68K_D0		= 0,	/* DWORD */
	MDP_REG_68K_D1		= 1,	/* DWORD */
	MDP_REG_68K_D2		= 2,	/* DWORD */
	MDP_REG_68K_D3		= 3,	/* DWORD */
	MDP_REG_68K_D4		= 4,	/* DWORD */
	MDP_REG_68K_D5		= 5,	/* DWORD */
	MDP_REG_68K_D6		= 6,	/* DWORD */
	MDP_REG_68K_D7		= 7,	/* DWORD */
	MDP_REG_68K_A0		= 8,	/* DWORD */
	MDP_REG_68K_A1		= 9,	/* DWORD */
	MDP_REG_68K_A2		= 10,	/* DWORD */
	MDP_REG_68K_A3		= 11,	/* DWORD */
	MDP_REG_68K_A4		= 12,	/* DWORD */
	MDP_REG_68K_A5		= 13,	/* DWORD */
	MDP_REG_68K_A6		= 14,	/* DWORD */
	MDP_REG_68K_A7		= 15,	/* DWORD */
	MDP_REG_68K_SP		= 15,	/* DWORD (alias for A7) */
	MDP_REG_68K_ASP		= 16,	/* DWORD */
	MDP_REG_68K_PC		= 17,	/* DWORD */
	MDP_REG_68K_SR		= 18	/* WORD */
} MDP_REG_68K;

#pragma pack(1)
/* mdp_reg_68k_t: 68K register struct for reg_get_all() / reg_set_all() */
typedef struct PACKED _mdp_reg_68k_t
{
	uint32_t dreg[8];
	uint32_t areg[8];
	uint32_t asp;
	uint32_t pc;
	uint16_t sr;
} mdp_reg_68k_t;
#pragma pack()


/* MDP_REG_VDP: VDP register IDs. */
typedef enum _MDP_REG_VDP
{
	MDP_REG_VDP_MODE_SET1		= 0,	/* BYTE */
	MDP_REG_VDP_MODE_SET2		= 1,	/* BYTE */
	MDP_REG_VDP_PAT_SCRA_ADDR	= 2,	/* BYTE */
	MDP_REG_VDP_PAT_WIN_ADDR	= 3,	/* BYTE */
	MDP_REG_VDP_PAT_SCRB_ADDR	= 4,	/* BYTE */
	MDP_REG_VDP_SPR_ATT_ADDR	= 5,	/* BYTE */
	MDP_REG_VDP_REG6		= 6,	/* BYTE */
	MDP_REG_VDP_BG_COLOR		= 7,	/* BYTE */
	MDP_REG_VDP_REG8		= 8,	/* BYTE */
	MDP_REG_VDP_REG9		= 9,	/* BYTE */
	MDP_REG_VDP_H_INT		= 10,	/* BYTE */
	MDP_REG_VDP_MODE_SET3		= 11,	/* BYTE */
	MDP_REG_VDP_MODE_SET4		= 12,	/* BYTE */
	MDP_REG_VDP_H_SCR_ADDR		= 13,	/* BYTE */
	MDP_REG_VDP_REG14		= 14,	/* BYTE */
	MDP_REG_VDP_AUTO_INC		= 15,	/* BYTE */
	MDP_REG_VDP_SCR_SIZE		= 16,	/* BYTE */
	MDP_REG_VDP_WIN_HPOS		= 17,	/* BYTE */
	MDP_REG_VDP_WIN_VPOS		= 18,	/* BYTE */
	MDP_REG_VDP_DMA_LEN_L		= 19,	/* BYTE */
	MDP_REG_VDP_DMA_LEN_H		= 20,	/* BYTE */
	MDP_REG_VDP_DMA_SRC_L		= 21,	/* BYTE */
	MDP_REG_VDP_DMA_SRC_M		= 22,	/* BYTE */
	MDP_REG_VDP_DMA_SRC_H		= 23	/* BYTE */
} MDP_REG_VDP;

#pragma pack(1)
/* mdp_reg_vdp_t: VDP register struct for reg_get_all() / reg_set_all() */
typedef struct PACKED _mdp_reg_vdp_t
{
	union
	{
		uint8_t data[24];
		struct
		{
			uint8_t mode_set1;
			uint8_t mode_set2;
			uint8_t pat_scra_addr;
			uint8_t pat_win_addr;
			uint8_t pat_scrb_addr;
			uint8_t spr_att_addr;
			uint8_t reg6;
			uint8_t bg_color;
			uint8_t reg8;
			uint8_t reg9;
			uint8_t h_int;
			uint8_t mode_set3;
			uint8_t mode_set4;
			uint8_t h_scr_addr;
			uint8_t reg14;
			uint8_t auto_inc;
			uint8_t scr_size;
			uint8_t win_hpos;
			uint8_t win_vpos;
			uint8_t dma_len_l;
			uint8_t dma_len_h;
			uint8_t dma_src_l;
			uint8_t dma_src_m;
			uint8_t dma_src_h;
		} regs;
	};
} mdp_reg_vdp_t;
#pragma pack()


/* Macro for YM2612 register IDs. */
/* YM2612 has two banks of 256 registers. */
#define MDP_REG_YM2612_GET_ID(bank, reg)	((uint32_t)(((bank & 1) << 8) | (reg & 0xFF)))
#define MDP_REG_YM2612_GET_BANK(id)		((id >> 8) & 1)
#define MDP_REG_YM2612_GET_REG(id)		(id & 0xFF)

#pragma pack(1)
/* mdp_reg_ym2612_t: YM2612 register struct for reg_get_all() / reg_set_all() */
typedef struct PACKED _mdp_reg_ym2612_t
{
	uint8_t regs[2][0x100];
} mdp_reg_ym2612_t;
#pragma pack()


#pragma pack(1)
/* mdp_reg_psg_t: PSG register struct for reg_get_all() / reg_set_all() */
typedef struct PACKED _mdp_reg_psg_t
{
	uint32_t regs[8];
} mdp_reg_psg_t;
#pragma pack()


/* MDP_REG_Z80: Z80 register IDs. */
typedef enum _MDP_REG_Z80
{
	/* Main registers: BYTE */
	MDP_REG_Z80_A		= 0,	/* BYTE */
	MDP_REG_Z80_F		= 1,	/* BYTE */
	MDP_REG_Z80_B		= 2,	/* BYTE */
	MDP_REG_Z80_C		= 3,	/* BYTE */
	MDP_REG_Z80_D		= 4,	/* BYTE */
	MDP_REG_Z80_E		= 5,	/* BYTE */
	MDP_REG_Z80_H		= 6,	/* BYTE */
	MDP_REG_Z80_L		= 7,	/* BYTE */
	MDP_REG_Z80_IXH		= 8,	/* BYTE */
	MDP_REG_Z80_IXL		= 9,	/* BYTE */
	MDP_REG_Z80_IYH		= 10,	/* BYTE */
	MDP_REG_Z80_IYL		= 11,	/* BYTE */
	
	/* Main registers: WORD */
	MDP_REG_Z80_AF		= 12,	/* WORD */
	MDP_REG_Z80_BC		= 13,	/* WORD */
	MDP_REG_Z80_DE		= 14,	/* WORD */
	MDP_REG_Z80_HL		= 15,	/* WORD */
	MDP_REG_Z80_IX		= 16,	/* WORD */
	MDP_REG_Z80_IY		= 17,	/* WORD */
	
	/* Shadow registers: BYTE */
	MDP_REG_Z80_A2		= 18,	/* BYTE */
	MDP_REG_Z80_F2		= 19,	/* BYTE */
	MDP_REG_Z80_B2		= 20,	/* BYTE */
	MDP_REG_Z80_C2		= 21,	/* BYTE */
	MDP_REG_Z80_D2		= 22,	/* BYTE */
	MDP_REG_Z80_E2		= 23,	/* BYTE */
	MDP_REG_Z80_H2		= 24,	/* BYTE */
	MDP_REG_Z80_L2		= 25,	/* BYTE */
	MDP_REG_Z80_IXH2	= 26,	/* BYTE */
	MDP_REG_Z80_IXL2	= 27,	/* BYTE */
	MDP_REG_Z80_IYH2	= 28,	/* BYTE */
	MDP_REG_Z80_IYL2	= 29,	/* BYTE */
	
	/* Shadow registers: WORD */
	MDP_REG_Z80_AF2		= 30,	/* WORD */
	MDP_REG_Z80_BC2		= 31,	/* WORD */
	MDP_REG_Z80_DE2		= 32,	/* WORD */
	MDP_REG_Z80_HL2		= 33,	/* WORD */
	
	/* Other registers. */
	MDP_REG_Z80_PC		= 34,	/* WORD */
	MDP_REG_Z80_SP		= 35,	/* WORD */
	MDP_REG_Z80_I		= 36,	/* BYTE */
	MDP_REG_Z80_R		= 37,	/* BYTE */
	MDP_REG_Z80_IM		= 38,	/* Interrupt Mode: 0, 1, or 2. */
	MDP_REG_Z80_IFF		= 39,	/* Bitfield: Bit 0 == IFF1, Bit 1 == IFF2 */
} MDP_REG_Z80;


#if MDP_BYTEORDER == MDP_LIL_ENDIAN
#define Z80_8BIT_REG_STRUCT(H, L)	\
	struct				\
	{				\
		uint8_t L;		\
		uint8_t H;		\
	};
#else /* MDP_BYTEORDER == MDP_BIG_ENDIAN */
#define Z80_8BIT_REG_STRUCT(H, L)	\
	struct				\
	{				\
		uint8_t H;		\
		uint8_t L;		\
	};
#endif

#pragma pack(1)
/* mdp_reg_z80_t: Z80 register struct for reg_get_all() / reg_set_all() */
typedef struct PACKED _mdp_reg_z80_t
{
	/* Main registers. */
	union
	{
		Z80_8BIT_REG_STRUCT(A, F);
		uint16_t AF;
	};
	union
	{
		Z80_8BIT_REG_STRUCT(B, C);
		uint16_t BC;
	};	
	union
	{
		Z80_8BIT_REG_STRUCT(D, E);
		uint16_t DE;
	};	
	union
	{
		Z80_8BIT_REG_STRUCT(H, L);
		uint16_t HL;
	};
	union
	{
		Z80_8BIT_REG_STRUCT(IXH, IXL);
		uint16_t IX;
	};
	union
	{
		Z80_8BIT_REG_STRUCT(IYH, IYL);
		uint16_t IY;
	};
	
	/* Shadow registers. */
	union
	{
		Z80_8BIT_REG_STRUCT(A2, F2);
		uint16_t AF2;
	};
	union
	{
		Z80_8BIT_REG_STRUCT(B2, C2);
		uint16_t BC2;
	};	
	union
	{
		Z80_8BIT_REG_STRUCT(D2, E2);
		uint16_t DE2;
	};	
	union
	{
		Z80_8BIT_REG_STRUCT(H2, L2);
		uint16_t HL2;
	};
	
	/* Other registers. */
	uint16_t PC;
	uint16_t SP;
	uint8_t   I;
	uint8_t   R;
	uint8_t  IM;
	uint8_t IFF;	/* Bitfield: Bit 0 == IFF1, Bit 1 == IFF2 */
} mdp_reg_z80_t;
#pragma pack()


#ifdef __cplusplus
}
#endif

#endif /* __MDP_MEM_H */

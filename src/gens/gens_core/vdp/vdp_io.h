/***************************************************************************
 * Gens: VDP I/O functions.                                                *
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

#ifndef GENS_VDP_IO_H
#define GENS_VDP_IO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>

// Needed for inline functions.
#include "gens_core/mem/mem_m68k.h"
#include "util/file/rom.hpp"
#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Constant data.
extern const uint32_t CD_Table[64];
extern const uint8_t  DMA_Timing_Table[16];

// System status.
extern int Genesis_Started;
extern int SegaCD_Started;
extern int _32X_Started;

/**
 * VDP_Reg_t: VDP Register struct.
 */
typedef struct
{
	union
	{
		uint8_t reg[24];
		struct
		{
			uint8_t Set1;
			uint8_t Set2;
			uint8_t Pat_ScrA_Adr;
			uint8_t Pat_Win_Adr;
			uint8_t Pat_ScrB_Adr;
			uint8_t Spr_Att_Adr;
			uint8_t Reg6;
			uint8_t BG_Color;
			uint8_t Reg8;
			uint8_t Reg9;
			uint8_t H_Int;
			uint8_t Set3;
			uint8_t Set4;
			uint8_t H_Scr_Adr;
			uint8_t Reg14;
			uint8_t Auto_Inc;
			uint8_t Scr_Size;
			uint8_t Win_H_Pos;
			uint8_t Win_V_Pos;
			uint8_t DMA_Length_L;
			uint8_t DMA_Length_H;
			uint8_t DMA_Src_Adr_L;
			uint8_t DMA_Src_Adr_M;
			uint8_t DMA_Src_Adr_H;
		};
	};
	
	// These two variables are internal to Gens.
	// They don't map to any actual VDP registers.
	unsigned int DMA_Length;
	unsigned int DMA_Address;
	
	// DMAT variables.
	unsigned int DMAT_Tmp;
	unsigned int DMAT_Length;
	unsigned int DMAT_Type;
} VDP_Reg_t;
extern VDP_Reg_t VDP_Reg;

/**
 * VDP_Ctrl_t: VDP Control struct.
 */
typedef union
{
	int reg[7];
	uint32_t ureg[7];
	struct
	{
		unsigned int Flag;
		unsigned int Data;
		unsigned int Write;
		unsigned int Access;
		unsigned int Address;
		unsigned int DMA_Mode;
		unsigned int DMA;
	};
} VDP_Ctrl_t;
extern VDP_Ctrl_t VDP_Ctrl;

typedef union
{
	uint8_t  u8[64*1024];
	uint16_t u16[(64*1024)>>1];
	uint32_t u32[(64*1024)>>2];
} VDP_VRam_t;
extern VDP_VRam_t VRam;
 
typedef union
{
	uint8_t  u8[64<<1];
	uint16_t u16[64];
	uint32_t u32[64>>1];
} VDP_CRam_t;
extern VDP_CRam_t CRam;

extern uint32_t VSRam_Over[8];
extern union
{
	uint8_t  u8[128<<1];
	uint16_t u16[128];
} VSRam;

extern uint8_t  H_Counter_Table[512][2];

extern int VDP_Current_Line;
extern int VDP_Num_Lines;
extern int VDP_Num_Vis_Lines;
extern int VDP_Int;
extern int VDP_Status;

// Flags.
typedef union
{
	unsigned int flags;
	struct
	{
		unsigned int VRam	:1;	// VRam was modified. (Implies VRam_Spr.)
		unsigned int VRam_Spr	:1;	// Sprite Attribute Table was modified.
		unsigned int CRam	:1;	// CRam was modified.
	};
} VDP_Flags_t;
extern VDP_Flags_t VDP_Flags;

// Set this to 1 to enable zero-length DMA requests.
// Default is 0. (hardware-accurate)
extern int Zero_Length_DMA;

// VDP address pointers.
extern uint16_t *ScrA_Addr;
extern uint16_t *ScrB_Addr;
extern uint8_t *Win_Addr;
extern uint8_t *Spr_Addr;
extern uint16_t *H_Scroll_Addr;

// VDP convenience values: Horizontal.
// NOTE: These must be signed for VDP arithmetic to work properly!
extern int H_Cell;
extern int H_Win_Mul;
extern int H_Pix;
extern int H_Pix_Begin;

// VDP convenience values: Scroll.
extern unsigned int V_Scroll_MMask;
extern unsigned int H_Scroll_Mask;

extern unsigned int H_Scroll_CMul;
extern unsigned int H_Scroll_CMask;
extern unsigned int V_Scroll_CMask;

// TODO: Eliminate these.
extern int Win_X_Pos;
extern int Win_Y_Pos;

void     VDP_Reset(void);
uint8_t  VDP_Read_H_Counter(void);
uint8_t  VDP_Read_V_Counter(void);
uint16_t VDP_Read_Status(void);
uint16_t VDP_Read_Data(void);

/* Functions that need to be ported to C. */
int Write_Byte_VDP_Data(uint8_t  Data);
int Write_Word_VDP_Data(uint16_t Data);
int Write_VDP_Ctrl(uint16_t Data);
void VDP_Set_Reg(int reg_num, uint8_t val);
unsigned int Update_DMA(void);

uint8_t VDP_Int_Ack(void);
void VDP_Update_IRQ_Line(void);


/** Inline VDP functions. **/

/**
 * vdp_isH40(): Determine if the current horiontal resolution is H40.
 * @return Zero if H32 (256); non-zero if H40 (320).
 */
static inline int vdp_isH40(void)
{
	// Default when no game is loaded is 1. (320x224)
	int rval = 0;
	if ((VDP_Reg.Set4 & 0x1) || !Game)
		rval = 1;
#ifdef GENS_DEBUGGER
	if (debug_mode != DEBUG_NONE)
		rval = 1;
#endif
#if 0
	if (!FrameCount)
		rval = 1;
#endif
	return rval;
}

/**
 * vdp_isH40(): Determine if the current horiontal resolution is H40.
 * @return Zero if V28 (224); non-zero if V30 (240).
 */
static inline int vdp_isV30(void)
{
	// Default when no game is loaded is 0. (320x224)
	int rval = 0;
	if (((CPU_Mode == 1) && (VDP_Reg.Set2 & 0x8)) && Game)
		rval = 1;
#ifdef GENS_DEBUGGER
	if (debug_mode != DEBUG_NONE)
		rval = 1;
#endif
#if 0
	if (!FrameCount)
		rval = 1;
#endif
	return rval;
}

#ifdef __cplusplus
}
#endif

/** VDP macros. **/

/**
 * VDP_SET_VISIBLE_LINES(): Sets the number of visible lines, depending on CPU mode and VDP setting.
 * Possible options:
 * - Normal: 224 lines. (V28)
 * - If PAL and Set2.M2 (bit 3) is set, 240 lines. (V30)
 */
#define VDP_SET_VISIBLE_LINES()				\
do {							\
	if ((CPU_Mode == 1) && (VDP_Reg.Set2 & 0x8))	\
		VDP_Num_Vis_Lines = 240;		\
	else						\
		VDP_Num_Vis_Lines = 224;		\
} while (0)

#endif /* GENS_VDP_IO_H */

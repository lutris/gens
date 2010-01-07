/***************************************************************************
 * Gens: VDP I/O functions.                                                *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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
			/**
			 * Mode 5 (MD) registers.
			 * DISP == Display Enable. (1 == on; 0 == off)
			 * IE0 == Enable V interrupt. (1 == on; 0 == off)
			 * IE1 == Enable H interrupt. (1 == on; 0 == off)
			 * IE2 == Enable external interrupt. (1 == on; 0 == off)
			 * PSEL == Palette Select; if clear, masks high two bits of each CRam color component.
			 * M1 == DMA Enable. (1 == on; 0 == off)
			 * M2 == V cell mode. (1 == V30 [PAL only]; 0 == V28)
			 * M3 == HV counter latch. (1 == stop HV counter; 0 == enable read, H, V counter)
			 * M4 == Mode 4/5 toggle; set for Mode 5, clear for Mode 4.
			 * VSCR == V Scroll mode. (0 == full; 1 == 2-cell)
			 * HSCR/LSCR == H Scroll mode. (00 == full; 01 == invalid; 10 == 1-cell; 11 == 1-line)
			 * RS0/RS1 == H cell mode. (11 == H40; 00 == H32; others == invalid)
			 * LSM1/LSM0 == Interlace mode. (00 == normal; 01 == interlace mode 1; 10 == invalid; 11 == interlace mode 2)
			 * S/TE == Highlight/Shadow enable. (1 == on; 0 == off)
			 * VSZ1/VSZ2 == Vertical scroll size. (00 == 32 cells; 01 == 64 cells; 10 == invalid; 11 == 128 cells)
			 * HSZ1/HSZ2 == Vertical scroll size. (00 == 32 cells; 01 == 64 cells; 10 == invalid; 11 == 128 cells)
			 */
			uint8_t Set1;		// Mode Set 1.  [   0    0    0  IE1    0 PSEL   M3    0]
			uint8_t Set2;		// Mode Set 2.  [   0 DISP  IE0   M1   M2   M4    0    0]
			uint8_t Pat_ScrA_Adr;	// Pattern name table base address for Scroll A.
			uint8_t Pat_Win_Adr;	// Pattern name table base address for Window.
			uint8_t Pat_ScrB_Adr;	// Pattern name table base address for Scroll B.
			uint8_t Spr_Att_Adr;	// Sprite Attribute Table base address.
			uint8_t Reg6;		// unused
			uint8_t BG_Color;	// Background color.
			uint8_t Reg8;		// unused
			uint8_t Reg9;		// unused
			uint8_t H_Int;		// H Interrupt.
			uint8_t Set3;		// Mode Set 3.  [   0    0    0    0  IE2 VSCR HSCR LSCR]
			uint8_t Set4;		// Mode Set 4.  [ RS0    0    0    0 S/TE LSM1 LSM0  RS1]
			uint8_t H_Scr_Adr;	// H Scroll Data Table base address.
			uint8_t Reg14;		// unused
			uint8_t Auto_Inc;	// Auto Increment.
			uint8_t Scr_Size;	// Scroll Size. [   0    0 VSZ1 VSZ0    0    0 HSZ1 HSZ0]
			uint8_t Win_H_Pos;	// Window H position.
			uint8_t Win_V_Pos;	// Window V position.
			uint8_t DMA_Length_L;	// DMA Length Counter Low.
			uint8_t DMA_Length_H;	// DMA Length Counter High.
			uint8_t DMA_Src_Adr_L;	// DMA Source Address Low.
			uint8_t DMA_Src_Adr_M;	// DMA Source Address Mid.
			uint8_t DMA_Src_Adr_H;	// DMA Source Address High.
		} m5;
		struct
		{
			/**
			 * Mode 4 (SMS) registers.
			 * NOTE: Mode 4 is currently not implemented.
			 * This is here for future use.
			 */
			uint8_t Set1;		// Mode Set 1.
			uint8_t Set2;		// Mode Set 2.
			uint8_t NameTbl_Addr;	// Name table base address.
			uint8_t ColorTbl_Addr;	// Color table base address.
			uint8_t	Pat_BG_Addr;	// Background Pattern Generator base address.
			uint8_t Spr_Att_Addr;	// Sprite Attribute Table base address.
			uint8_t Spr_Pat_addr;	// Sprite Pattern Generator base address.
			uint8_t BG_Color;	// Background color.
			uint8_t H_Scroll;	// Horizontal scroll.
			uint8_t V_Scroll;	// Vertical scroll.
			uint8_t H_Int;		// H Interrupt.
		} m4;
	};
	
	// These two variables are internal to Gens.
	// They don't map to any actual VDP registers.
	unsigned int DMA_Length;
	unsigned int DMA_Address;
	
	// DMAT variables.
	unsigned int DMAT_Tmp;
	unsigned int DMAT_Length;
	unsigned int DMAT_Type;
	
	// VDP address pointers.
	// These are relative to VRam[] and are based on register values.
	uint16_t *ScrA_Addr;
	uint16_t *ScrB_Addr;
	uint16_t *Win_Addr;
	uint16_t *Spr_Addr;
	uint16_t *H_Scroll_Addr;
	
	// VDP convenience values: Horizontal.
	// NOTE: These must be signed for VDP arithmetic to work properly!
	int H_Cell;
	int H_Win_Mul;
	int H_Pix;
	int H_Pix_Begin;
	
	// VDP convenience values: Scroll.
	unsigned int V_Scroll_MMask;
	unsigned int H_Scroll_Mask;
	
	unsigned int H_Scroll_CMul;
	unsigned int H_Scroll_CMask;
	unsigned int V_Scroll_CMask;
	
	// TODO: Eliminate these.
	int Win_X_Pos;
	int Win_Y_Pos;
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

// TODO:
// - Eliminate VSRam_Over[].
// - Shrink VSRam[] to 80 bytes (40 words).
extern uint32_t VSRam_Over[8];
extern union
{
	uint8_t  u8[128<<1];	// Only 80 bytes on the actual system!
	uint16_t u16[128];	// Only 40 words on the actual system!
} VSRam;

extern uint8_t  H_Counter_Table[512][2];

extern int VDP_Int;
extern int VDP_Status;

// VDP line counters.
// NOTE: Gens/GS currently uses 312 lines for PAL. It should use 313!
typedef struct _VDP_Lines_t
{
	/** Total lines using NTSC/PAL line numbering. **/
	struct
	{
		unsigned int Total;	// Total number of lines on the display. (262, 313)
		unsigned int Current;	// Current display line.
	} Display;
	
	/** Visible lines using VDP line numbering. **/
	struct
	{
		int Total;	// Total number of visible lines. (192, 224, 240)
		int Current;	// Current visible line. (May be negative for top border.)
		int Border_Size;	// Size of the border. (192 lines == 24; 224 lines == 8)
		int NTSC_V30_Roll;	// Current NTSC V30 roll offset.
	} Visible;
} VDP_Lines_t;
extern VDP_Lines_t VDP_Lines;

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

void     VDP_Reset(void);
uint8_t  VDP_Read_H_Counter(void);
uint8_t  VDP_Read_V_Counter(void);
uint16_t VDP_Read_Status(void);
uint16_t VDP_Read_Data(void);

unsigned int VDP_Update_DMA(void);

void VDP_Write_Data_Byte(uint8_t  data);
void VDP_Write_Data_Word(uint16_t data);

/* Functions that need to be ported to C. */
void Write_VDP_Ctrl(uint16_t Data);
void VDP_Set_Reg(int reg_num, uint8_t val);

uint8_t VDP_Int_Ack(void);
void VDP_Update_IRQ_Line(void);


/** Inline VDP functions. **/

/**
 * vdp_getHPix(): Get the current horizontal resolution.
 * This should only be used for non-VDP code.
 * VDP code should access VDP_Reg.H_Pix directly.
 * @return Horizontal resolution, in pixels.
 */
static inline int vdp_getHPix(void)
{
	// Default when no game is loaded is 320. (320x224)
#ifdef GENS_DEBUGGER
	if (!Game || debug_mode != DEBUG_NONE)
		return 320;
#else
	if (!Game)
		return 320;
#endif
	
#if 0
	if (!FrameCount)
		rval = 1;
#endif
	
	// Game is running. Return VDP_Reg.H_Pix.
	return VDP_Reg.H_Pix;
}


/**
 * vdp_getVPix(): Get the current vertical resolution.
 * This should only be used for non-VDP code.
 * VDP code should access VDP_Reg.Set2 directly.
 * @return Vertical resolution, in pixels.
 */
static inline int vdp_getVPix(void)
{
	// Default when no game is loaded is 224. (320x224)
#ifdef GENS_DEBUGGER
	if (!Game || debug_mode != DEBUG_NONE)
		return 224;
#else
	if (!Game)
		return 224;
#endif
	
#if 0
	if (!FrameCount)
	rval = 1;
#endif
	
	return VDP_Lines.Visible.Total;
}

#ifdef __cplusplus
}
#endif

/** VDP macros. **/

/**
 * VDP_SET_VISIBLE_LINES(): Sets the number of visible lines, depending on CPU mode and VDP setting.
 * Possible options:
 * - Mode 4 (SMS): 192 lines. (TODO)
 * - Mode 5, V28: 224 lines.
 * - Mode 5, V30: 240 lines. (NTSC V30 results in a rolling picture.)
 */
static inline void VDP_Set_Visible_Lines(void)
{
	// CPU_Mode: 0 == NTSC; 1 == PAL
	// TODO: Remove the "+1" for PAL once Gens/GS uses 313 lines instead of 312.
	if (VDP_Reg.m5.Set2 & 0x08)
	{
		// V30.
		VDP_Lines.Visible.Total = 240;
		VDP_Lines.Visible.Current = ((CPU_Mode == 1) ? -43+1 : 0);
		VDP_Lines.Visible.Border_Size = 0;
	}
	else
	{
		// V28.
		VDP_Lines.Visible.Total = 224;
		VDP_Lines.Visible.Current = ((CPU_Mode == 1) ? -51+1 : -24);
		VDP_Lines.Visible.Border_Size = 8;
	}
	
	// TODO: V24 (192 lines)
#if 0
	{
		VDP_Lines.Visible.Total = 192;
		VDP_Lines.Visible.Current = ((CPU_Mode == 1) ? -67+1 : -40);
		VDP_Lines.Visible.Border_Size = 24;
	}
#endif
}

#endif /* GENS_VDP_IO_H */

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

#include "vdp_io.h"
#include "vdp_rend.h"

// Message logging.
#include "macros/log_msg.h"

// Starscream 68000 core.
#include "gens_core/cpu/68k/star_68k.h"
#include "gens_core/mem/mem_s68k.h"	/* For Ram_Word_State. */

// C includes.
#include <string.h>

// VDP registers and control.
VDP_Reg_t VDP_Reg;
VDP_Ctrl_t VDP_Ctrl;
unsigned int VDP_Mode;

// Memory variables.
VDP_VRam_t VRam;
VDP_CRam_t CRam;
VSRam_t VSRam;

// Miscellaneous. (TODO: Determine if these should be signed or unsigned.)
int VDP_Status;
int VDP_Int;

// VDP line counters.
VDP_Lines_t VDP_Lines;

// Flags.
VDP_Flags_t VDP_Flags;

// Enable zero-length DMA.
int Zero_Length_DMA;


// Tables.
uint8_t H_Counter_Table[512][2];

const uint32_t CD_Table[64] =
{
	0x0005, 0x0009, 0x0000, 0x000A,	// bits 0-2  = Location (0x00:WRONG, 0x01:VRAM, 0x02:CRAM, 0x03:VSRAM)
	0x0007, 0x000B, 0x0000, 0x0000,	// bits 3-4  = Access   (0x00:WRONG, 0x04:READ, 0x08:WRITE)
	0x0006, 0x0000, 0x0000, 0x0000,	// bits 8-11 = DMA MEM TO VRAM (0x0000:NO DMA, 0x0100:MEM TO VRAM, 0x0200: MEM TO CRAM, 0x0300: MEM TO VSRAM)
	0x0000, 0x0000, 0x0000, 0x0000,	// bits 12   = DMA VRAM FILL (0x0000:NO DMA, 0x0400:VRAM FILL)
	
	0x0005, 0x0009, 0x0000, 0x000A,	// bits 13   = DMA VRAM COPY (0x0000:NO DMA, 0x0800:VRAM COPY)
	0x0007, 0x000B, 0x0000, 0x0000,
	0x0006, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	
	0x0005, 0x0509, 0x0000, 0x020A,
	0x0007, 0x030B, 0x0000, 0x0000,
	0x0006, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	
	/*
	0x0800, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	*/
	
	0x0800, 0x0100, 0x0000, 0x0200,
	0x0000, 0x0300, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000
};


/**
 * DMA_Timing_Table[][]: Maximum number of DMA transfers per line.
 */
static const uint8_t DMA_Timing_Table[4][4] =
{
	/* Format: H32 active, H32 blanking, H40 active, H40 blanking */
	{8,    83,   9, 102},	/* 68K to VRam (1 word == 2 bytes) */
	{16,  167,  18, 205},	/* 68K to CRam or VSRam */
	{15,  166,  17, 204},	/* VRam Fill */
	{8,    83,   9, 102},	/* VRam Copy */
};


// System status.
int Genesis_Started = 0;
int SegaCD_Started = 0;
int _32X_Started = 0;


/**
 * VDP_Reset(): Reset the VDP.
 */
void VDP_Reset(void)
{
	// Clear MD_Screen and MD_Screen32.
	memset(&MD_Screen, 0x00, sizeof(MD_Screen));
	memset(&MD_Screen32, 0x00, sizeof(MD_Screen32));
	
	// Clear VRam, CRam, and VSRam.
	// (This also clears the VSRam overflow area.)
	memset(&VRam, 0x00, sizeof(VRam));
	memset(&CRam, 0x00, sizeof(CRam));
	memset(&VSRam, 0x00, sizeof(VSRam));
	
	// Clear MD_Palette and MD_Palette32.
	if (!(VDP_Layers & VDP_LAYER_PALETTE_LOCK))
	{
		memset(&MD_Palette, 0x00, sizeof(MD_Palette));
		memset(&MD_Palette32, 0x00, sizeof(MD_Palette32));
	}
	
	// Sprite arrays.
	memset(&Sprite_Struct, 0x00, sizeof(Sprite_Struct));
	memset(&Sprite_Visible, 0x00, sizeof(Sprite_Visible));
	
	/**
	 * VDP registers.
	 * Default register values:
	 * - 0x01 (Mode1):   0x04 (H_Int off, Mode 5 [MD])
	 * - 0x0A (H_Int):   0xFF (disabled).
	 * - 0x0C (Mode4):   0x81 (H40, S/H off, no interlace)
	 * - 0x0F (AutoInc): 0x02 (auto-increment by 2 on memory access)
	 * All other registers are set to 0x00 by default.
	 */
	static const uint8_t vdp_reg_init[24] =
	{
		0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xFF, 0x00, 0x81, 0x00, 0x00, 0x02,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	
	unsigned int reg;
	for (reg = 0; reg < sizeof(vdp_reg_init); reg++)
	{
		VDP_Set_Reg(reg, vdp_reg_init[reg]);
	}
	
	// Reset the non-register parts of the VDP register struct.
	VDP_Reg.DMA_Length = 0;
	VDP_Reg.DMA_Address = 0;
	VDP_Reg.DMAT_Tmp = 0;
	VDP_Reg.DMAT_Length = 0;
	VDP_Reg.DMAT_Type = 0;
	
	// Other variables.
	VDP_Status = 0x0200;
	VDP_Int = 0;
	
	// VDP Control struct.
	VDP_Ctrl.Flag = 0;
	VDP_Ctrl.Data.d = 0;
	VDP_Ctrl.Write = 0;
	VDP_Ctrl.Access = 0;
	VDP_Ctrl.Address = 0;
	VDP_Ctrl.DMA_Mode = 0;
	VDP_Ctrl.DMA = 0;
	
	// Set the CRam and VRam flags.
	VDP_Flags.CRam = 1;
	VDP_Flags.VRam = 1;
	
	// Initialize the Horizontal Counter table.
	unsigned int hc;
	unsigned int hc_val;
	for (hc = 0; hc < 512; hc++)
	{
		// H32
		hc_val = ((hc * 170) / 488) - 0x18;
		H_Counter_Table[hc][0] = (unsigned char)hc_val;
		
		// H40
		hc_val = ((hc * 205) / 488) - 0x1C;
		H_Counter_Table[hc][1] = (unsigned char)hc_val;
	}
}


uint8_t VDP_Int_Ack(void)
{
	if ((VDP_Reg.m5.Set2 & 0x20) && (VDP_Int & 0x08))
	{
		// VBlank interrupt acknowledge.
		VDP_Int &= ~0x08;
		
		uint8_t rval_mask = VDP_Reg.m5.Set1;
		rval_mask &= 0x10;
		rval_mask >>= 2;
		
		return ((VDP_Int) & rval_mask);
	}
	
	// Reset the interrupt counter.
	VDP_Int = 0;
	return 0;
}


void VDP_Update_IRQ_Line(void)
{
	// TODO: HBlank interrupt should take priority over VBlank interrupt.
	if ((VDP_Reg.m5.Set2 & 0x20) && (VDP_Int & 0x08))
	{
		// VBlank interrupt.
		main68k_interrupt(6, -1);
		return;
	}
	else if ((VDP_Reg.m5.Set1 & 0x10) && (VDP_Int & 0x04))
	{
		// HBlank interrupt.
		main68k_interrupt(4, -1);
		return;
	}
	
	// No VDP interrupts.
	main68k_context.interrupts[0] &= 0xF0;
}


/**
 * VDP_Set_Visible_Lines(): Sets the number of visible lines, depending on CPU mode and VDP setting.
 */
void VDP_Set_Visible_Lines(void)
{
	// Arrays of values.
	// Indexes: 0 == 192 lines; 1 == 224 lines; 2 == 240 lines.
	static const int VisLines_Total[3] = {192, 224, 240};
	static const int VisLines_Border_Size[3] = {24, 8, 0};
	static const int VisLines_Current_NTSC[3] = {-40, -24, 0};
	static const int VisLines_Current_PAL[3] = {-67+1, -51+1, -43+1};
	
	// Line offset.
	int LineOffset;
	
	// Check the current video mode.
	if (!Game)
	{
		// No game is running. Assume 224 lines.
		LineOffset = 1;
	}
	else if (VDP_Mode & VDP_MODE_M5)
	{
		// Mode 5. Must be either 224 lines or 240 lines.
		if (VDP_Mode & VDP_MODE_M2)
			LineOffset = 2; // 240 lines.
		else
			LineOffset = 1; // 224 lines.
	}
	else
	{
		// Mode 4 or TMS9918 mode.
		// Mode 4 may be 192 lines, 224 lines, or 240 lines.
		// Modes 0-3 may only be 192 lines.
		// TODO: If emulating SMS1, disable 224-line and 240-line modes.
		switch (VDP_Mode)
		{
			case 0x0B:
				// Mode 4: 224 lines.
				LineOffset = 1;
				break;
			case 0x0E:
				// Mode 4: 240 lines.
				LineOffset = 2;
				break;
			default:
				// Modes 0-4: 192 lines.
				LineOffset = 0;
				break;
		}
	}
	
	LineOffset = 0;
	VDP_Lines.Visible.Total = VisLines_Total[LineOffset];
	VDP_Lines.Visible.Border_Size = VisLines_Border_Size[LineOffset];
	VDP_Lines.Visible.Current = ((CPU_Mode == 1)
					? VisLines_Current_PAL[LineOffset]
					: VisLines_Current_NTSC[LineOffset]);
}


/**
 * VDP_Update_Mode(): Update VDP_Mode.
 */
static inline void VDP_Update_Mode(void)
{
	register uint8_t Set1 = VDP_Reg.m5.Set1;
	register uint8_t Set2 = VDP_Reg.m5.Set2;
	VDP_Mode = ((Set2 & 0x10) >> 4) |	// M1
		   ((Set2 & 0x08) >> 2) |	// M2
		   ((Set1 & 0x06) << 1) |	// M3, M4/PSEL
		   ((Set2 & 0x04) << 2);	// M5
	
	// CRam needs to be updated.
	// TODO: Only update if VDP_Mode is changed.
	VDP_Flags.CRam = 1;
}


/**
 * Scroll_Size_t: Convenience enum for dealing with scroll plane sizes.
 */
typedef enum
{
	V32_H32 = 0, V32_H64,  V32_HXX,  V32_H128,
	V64_H32,     V64_H64,  V64_HXX,  V64_H128,
	VXX_H32,     VXX_H64,  VXX_HXX,  VXX_H128,
	V128_H32,    V128_H64, V128_HXX, V128_H128
} Scroll_Size_t;

/**
 * VDP_Set_Reg(): Set the value of a register. (Mode 5 only!)
 * @param reg_num Register number.
 * @param val New value for the register.
 */
void VDP_Set_Reg(int reg_num, uint8_t val)
{
	if (reg_num < 0 || reg_num >= 24)
		return;
	
	// Save the new register value.
	VDP_Reg.reg[reg_num] = val;
	
	// Temporary value for calculation.
	unsigned int tmp;
	
	// Update things affected by the register.
	switch (reg_num)
	{
		case 0:
			// Mode Set 1.
			VDP_Update_IRQ_Line();
			
			// Update the VDP mode.
			VDP_Update_Mode();
			break;
		
		case 1:
			// Mode Set 2.
			VDP_Update_IRQ_Line();
			
			if (!(VDP_Reg.m5.Set2 & 0x08))
			{
				// V28 mode. Reset the NTSC V30 roll values.
				VDP_Lines.NTSC_V30.Offset = 0;
				VDP_Lines.NTSC_V30.VBlank = 0;
			}
			
			// Update the VDP mode.
			VDP_Update_Mode();
			break;
		
		case 2:
			// Scroll A base address.
			tmp = (val & 0x38) << 10;
			VDP_Reg.ScrA_Addr = &VRam.u16[tmp>>1];
			break;
		
		case 3:
			// Window base address.
			if (VDP_Reg.m5.Set4 & 0x01)	// Check for H40 mode. (TODO: Test 0x81 instead?)
				tmp = (val & 0x3C) << 10;	// H40.
			else
				tmp = (val & 0x3E) << 10;	// H32.
			
			VDP_Reg.Win_Addr = &VRam.u16[tmp>>1];
			break;
		
		case 4:
			// Scroll B base address.
			tmp = (val & 0x07) << 13;
			VDP_Reg.ScrB_Addr = &VRam.u16[tmp>>1];
			break;
		
		case 5:
			// Sprite Attribute Table base address.
			if (VDP_Reg.m5.Set4 & 0x01)	// Check for H40 mode. (TODO: Test 0x81 instead?)
				tmp = (val & 0x7E) << 9;
			else
				tmp = (val & 0x7F) << 9;
			
			VDP_Reg.Spr_Addr = &VRam.u16[tmp>>1];
			
			// Update the Sprite Attribute Table.
			// TODO: Only set this if the actual value has changed.
			VDP_Flags.VRam_Spr = 1;
			break;
		
		case 7:
			// Background Color.
			// TODO: Only set this if the actual value has changed.
			VDP_Flags.CRam = 1;
			break;
		
		case 11:
		{
			// Mode Set 3.
			static const unsigned int Size_V_Scroll[4] = {255, 511, 255, 1023};
			static const unsigned int H_Scroll_Mask_Table[4] = {0x0000, 0x0007, 0x01F8, 0x1FFF};
			
			// Check the Vertical Scroll mode. (Bit 3)
			// 0: Full scrolling. (Mask == 0)
			// 1: 2CELL scrolling. (Mask == 0x7E)
			VDP_Reg.V_Scroll_MMask = ((val & 4) ? 0x7E : 0);
			
			// Horizontal Scroll mode
			VDP_Reg.H_Scroll_Mask = H_Scroll_Mask_Table[val & 3];
			
			break;
		}
		
		case 12:
			// Mode Set 4.
			
			// This register has the Shadow/Highlight setting,
			// so set the CRam Flag to force a CRam update.
			// TODO: Only set this if the actual value has changed.
			VDP_Flags.CRam = 1;
			
			if (val & 0x81)		// TODO: Original asm tests 0x81. Should this be done for other H40 tests?
			{
				// H40 mode.
				VDP_Reg.H_Cell = 40;
				VDP_Reg.H_Win_Mul = 6;
				VDP_Reg.H_Pix = 320;
				VDP_Reg.H_Pix_Begin = 0;
				
				// Check the window horizontal position.
				VDP_Reg.Win_X_Pos = ((VDP_Reg.m5.Win_H_Pos & 0x1F) * 2);
				if (VDP_Reg.Win_X_Pos > 40)
					VDP_Reg.Win_X_Pos = 40;
				
				// Update the Window base address.
				tmp = (VDP_Reg.m5.Pat_Win_Adr & 0x3C) << 10;
				VDP_Reg.Win_Addr = &VRam.u16[tmp>>1];
				
				// Update the Sprite Attribute Table base address.
				tmp = (VDP_Reg.m5.Spr_Att_Adr & 0x7E) << 9;
				VDP_Reg.Spr_Addr = &VRam.u16[tmp>>1];
			}
			else
			{
				// H32 mode.
				VDP_Reg.H_Cell = 32;
				VDP_Reg.H_Win_Mul = 5;
				VDP_Reg.H_Pix = 256;
				VDP_Reg.H_Pix_Begin = 32;
				
				// Check the window horizontal position.
				VDP_Reg.Win_X_Pos = ((VDP_Reg.m5.Win_H_Pos & 0x1F) * 2);
				if (VDP_Reg.Win_X_Pos > 32)
					VDP_Reg.Win_X_Pos = 32;
				
				// Update the Window base address.
				tmp = (VDP_Reg.m5.Pat_Win_Adr & 0x3E) << 10;
				VDP_Reg.Win_Addr = &VRam.u16[tmp>>1];
				
				// Update the Sprite Attribute Table base address.
				tmp = (VDP_Reg.m5.Spr_Att_Adr & 0x7F) << 9;
				VDP_Reg.Spr_Addr = &VRam.u16[tmp>>1];
			}
			
			break;
		
		case 13:
			// H Scroll Table base address.
			tmp = (val & 0x3F) << 10;
			VDP_Reg.H_Scroll_Addr = &VRam.u16[tmp>>1];
			break;
		
		case 16:
		{
			// Scroll Size.
			tmp = (val & 0x3);
			tmp |= (val & 0x30) >> 2;
			
			switch ((Scroll_Size_t)tmp)
			{
				case V32_H32:
				case VXX_H32:
					VDP_Reg.H_Scroll_CMul = 5;
					VDP_Reg.H_Scroll_CMask = 31;
					VDP_Reg.V_Scroll_CMask = 31;
					break;
				
				case V64_H32:
					VDP_Reg.H_Scroll_CMul = 5;
					VDP_Reg.H_Scroll_CMask = 31;
					VDP_Reg.V_Scroll_CMask = 63;
					break;
				
				case V128_H32:
					VDP_Reg.H_Scroll_CMul = 5;
					VDP_Reg.H_Scroll_CMask = 31;
					VDP_Reg.V_Scroll_CMask = 127;
					break;
				
				case V32_H64:
				case VXX_H64:
					VDP_Reg.H_Scroll_CMul = 6;
					VDP_Reg.H_Scroll_CMask = 63;
					VDP_Reg.V_Scroll_CMask = 31;
					break;
				
				case V64_H64:
				case V128_H64:
					VDP_Reg.H_Scroll_CMul = 6;
					VDP_Reg.H_Scroll_CMask = 63;
					VDP_Reg.V_Scroll_CMask = 63;
					break;
				
				case V32_HXX:
				case V64_HXX:
				case VXX_HXX:
				case V128_HXX:
					VDP_Reg.H_Scroll_CMul = 6;
					VDP_Reg.H_Scroll_CMask = 63;
					VDP_Reg.V_Scroll_CMask = 0;
					break;
				
				case V32_H128:
				case V64_H128:
				case VXX_H128:
				case V128_H128:
					VDP_Reg.H_Scroll_CMul = 7;
					VDP_Reg.H_Scroll_CMask = 127;
					VDP_Reg.V_Scroll_CMask = 31;
					break;
				
				default:	// to make gcc shut up
					break;
			}
			
			break;
		}
		
		case 17:
			// Window H position.
			VDP_Reg.Win_X_Pos = ((val & 0x1F) * 2);
			if (VDP_Reg.Win_X_Pos > VDP_Reg.H_Cell)
				VDP_Reg.Win_X_Pos = VDP_Reg.H_Cell;
			break;
			
		case 18:
			// Window V position.
			VDP_Reg.Win_Y_Pos = (val & 0x1F);
			break;
		
		case 19:
			// DMA Length Low.
			VDP_Reg.DMA_Length = (VDP_Reg.DMA_Length & 0xFFFFFF00) | val;
			break;
		
		case 20:
			// DMA Length High.
			VDP_Reg.DMA_Length = (VDP_Reg.DMA_Length & 0xFFFF00FF) | (val << 8);
			break;
		
		case 21:
			// DMA Address Low.
			VDP_Reg.DMA_Address = (VDP_Reg.DMA_Address & 0xFFFFFF00) | val;
			break;
		
		case 22:
			// DMA Address Mid.
			VDP_Reg.DMA_Address = (VDP_Reg.DMA_Address & 0xFFFF00FF) | (val << 8);
			break;
		
		case 23:
			// DMA Address High.
			VDP_Reg.DMA_Address = (VDP_Reg.DMA_Address & 0xFF00FFFF) | ((val & 0x7F) << 16);
			VDP_Ctrl.DMA_Mode = (val & 0xC0);
			break;
		
		default:	// to make gcc shut up
			break;
	}
}


/**
 * VDP_Read_H_Counter(): Read the H Counter.
 * @return H Counter.
 */
uint8_t VDP_Read_H_Counter(void)
{
	unsigned int odo_68K = main68k_readOdometer();
	odo_68K -= (Cycles_M68K - CPL_M68K);
	odo_68K &= 0x1FF;
	
	// H_Counter_Table[][0] == H32.
	// H_Counter_Table[][1] == H40.
	
	// TODO: We're checking both RS0 and RS1 here. Others only check one.
	if (VDP_Reg.m5.Set4 & 0x81)
		return H_Counter_Table[odo_68K][1];
	else
		return H_Counter_Table[odo_68K][0];
}


/**
 * VDP_Read_V_Counter(): Read the V Counter.
 * @return V Counter.
 */
uint8_t VDP_Read_V_Counter(void)
{
	unsigned int odo_68K = main68k_readOdometer();
	odo_68K -= (Cycles_M68K - CPL_M68K);
	odo_68K &= 0x1FF;
	
	unsigned int H_Counter;
	uint8_t bl, bh;		// TODO: Figure out what this actually means.
	
	if (VDP_Reg.m5.Set4 & 0x81)
	{
		// H40
		H_Counter = H_Counter_Table[odo_68K][0];
		bl = 0xA4;
	}
	else
	{
		// H32
		H_Counter = H_Counter_Table[odo_68K][1];
		bl = 0x84;
	}
	
	bh = ((H_Counter <= 0xE0) ? 1 : 0);
	bl = ((H_Counter >= bl) ? 1 : 0);
	bl &= bh;
	
	int V_Counter = VDP_Lines.Visible.Current;
	if (V_Counter < 0)
		V_Counter += VDP_Lines.Display.Total;
	V_Counter += (bl ? 1 : 0);
	
	// TODO: Some of these values are wrong.
	// Rewrite HV handling to match Genesis Plus.
	
	// V_Counter_Overflow depends on PAL/NTSC status.
	// (VDP_Status & 1) == 1 for PAL, 0 for NTSC.
	if (VDP_Status & 1)
	{
		// PAL.
		if (V_Counter >= 0x103)
		{
			// Overflow.
			V_Counter -= 56;
		}
	}
	else
	{
		// NTSC.
		if (V_Counter >= 0xEB)
		{
			// Overflow.
			V_Counter -= 6;
		}
	}
	
	// Check for interlaced mode.
	// NOTE: Bit 1 is LSM0. Should check both LSM1 and LSM0!
	if (VDP_Reg.m5.Set4 & 2)
	{
		// Interlaced mode is enabled.
		uint8_t vc_tmp = (V_Counter & 0xFF);
		vc_tmp = (vc_tmp << 1) | (vc_tmp >> 7);
		return vc_tmp;
	}
	
	// Interlaced mode is not enabled.
	return (uint8_t)(V_Counter & 0xFF);
}


/**
 * VDP_Read_Status(): Read the VDP status register.
 * @return VDP status register.
 */
uint16_t VDP_Read_Status(void)
{
	// Toggle the upper 8 bits of VDP_Status. (TODO: Is this correct?)
	VDP_Status ^= 0xFF00;
	
	// Mask the SOVR ("Sprite Overflow") and C ("Collision between non-zero pixels in two sprites") bits.
	// TODO: Should these be masked? This might be why some games are broken...
	VDP_Status &= ~(0x0040 | 0x0020);
	
	// Check if we're currently in VBlank.
	if (!(VDP_Status & 0x0008))
	{
		// Not in VBlank. Mask the F bit. ("Vertical Interrupt Happened")
		VDP_Status &= ~0x0080;
	}
	
	// If the Display is disabled, OR the result with 0x0008.
	if (VDP_Reg.m5.Set2 & 0x40)
		return VDP_Status;
	else
		return (VDP_Status | 0x0008);
}


/**
 * VDP_Read_Data(): Read data from the VDP.
 * @return Data.
 */
uint16_t VDP_Read_Data(void)
{
	// TODO: Test this function.
	// I can't find any MD games that read data from the VDP.
	LOG_MSG(vdp_io, LOG_MSG_LEVEL_DEBUG2,
		"VDP_Ctrl.Access == %d", VDP_Ctrl.Access);
	
	// Clear the VDP control flag.
	// (It's set when the address is set.)
	VDP_Ctrl.Flag = 0;
	
	uint16_t data;
	
	// Check the access mode.
	switch (VDP_Ctrl.Access)
	{
		case 5:
			// VRam Read.
			data = VRam.u16[(VDP_Ctrl.Address & 0xFFFE) >> 1];
			VDP_Ctrl.Address += VDP_Reg.m5.Auto_Inc;
			break;
		
		case 6:
			// CRam Read.
			data = CRam.u16[(VDP_Ctrl.Address & 0x7E) >> 1];
			VDP_Ctrl.Address += VDP_Reg.m5.Auto_Inc;
			break;
		
		case 7:
			// VSRam Read.
			data = VSRam.u16[(VDP_Ctrl.Address & 0x7E) >> 1];
			VDP_Ctrl.Address += VDP_Reg.m5.Auto_Inc;
			break;
		
		default:
			// Invalid read specification.
			data = 0;
			break;
	}
	
	return data;
}


/**
 * VDP_Update_DMA(): Update the DMA state.
 * @return Number of cycles used.
 */
unsigned int VDP_Update_DMA(void)
{
	// DMA transfer rate depends on the following:
	// - Horizontal resolution. (H32/H40)
	// - DMA type.
	// - Are we in VBlank or active drawing?
	
	// DMA_Timing_Table offset:
	// [DMAT1 DMAT0][HRES VBLANK]
	// * DMAT1/DMAT0 == DMA type.
	// * HRES == horizontal resolution.
	// * VBLANK == vertical blanking.
	
	// Horizontal resolution.
	// TODO: Use both RS0/RS1, not just RS1.
	unsigned int offset = ((VDP_Reg.m5.Set4 & 1) * 2);
	
	// Check if we're in VBlank or the VDP is disabled.
	if (VDP_Lines.Visible.Current < 0 ||
	    VDP_Lines.Visible.Current >= VDP_Lines.Visible.Current ||
	    (!(VDP_Reg.m5.Set2 & 0x40)))
	{
		// In VBlank, or VDP is disabled.
		offset |= 1;
	}
	
	// Cycles elapsed is based on M68K cycles per line.
	unsigned int cycles = CPL_M68K;
	
	// Get the DMA transfer rate.
	const uint8_t timing = DMA_Timing_Table[VDP_Reg.DMAT_Type & 3][offset];
	if (VDP_Reg.DMAT_Length > timing)
	{
		// DMA is not finished.
		VDP_Reg.DMAT_Length -= timing;
		if (VDP_Reg.DMAT_Type & 2)
		{
			// DMA to CRam or VSRam.
			// Return 0.
			return 0;
		}
		
		// DMA to VRam.
		// Return the total number of cycles.
		return cycles;
	}
	
	// DMA is finished. Do some processing.
	unsigned int len_tmp = (VDP_Reg.DMAT_Length + timing);
	VDP_Reg.DMAT_Length = 0;
	
	// Calculate the new cycles value.
	// (NOTE: I have no idea how this formula was created.)
	//cycles = (((cycles << 16) / timing) * len_tmp) >> 16;
	cycles <<= 16;
	cycles /= timing;
	cycles *= len_tmp;
	cycles >>= 16;
	
	// Clear the DMA Busy flag.
	VDP_Status &= ~0x0002;
	
	if (VDP_Reg.DMAT_Type & 2)
	{
		// DMA to CRam or VSRam.
		// Return 0.
		return 0;
	}
	
	// DMA to VRam.
	// Return the total number of cycles.
	return cycles;
}


/**
 * VDP_Write_Data_Byte(): Write data to the VDP. (8-bit)
 * @param data 8-bit data.
 */
void VDP_Write_Data_Byte(uint8_t data)
{
	// Note that in Mode 5, the VDP requires 16-bit data.
	// 8-bit writes will result in the data byte being mirrored
	// for both high-byte and low-byte.
	//
	// The following two instructions are equivalent.
	// move.b   #$58, ($C00000)
	// move.w #$5858, ($C00000)
	
	VDP_Write_Data_Word(data | (data << 8));
}


/**
 * DMA_Fill(): Perform a DMA Fill operation. (Called from VDP_Write_Data_Word().)
 * @param data 16-bit data.
 */
static void DMA_Fill(uint16_t data)
{
	// Set the VRam flag.
	VDP_Flags.VRam = 1;
	
	// Get the values. (length is in bytes)
	// NOTE: DMA Fill uses *bytes* for length, not words!
	uint16_t address = (VDP_Ctrl.Address & 0xFFFF);
	unsigned int length = (VDP_Reg.DMA_Length & 0xFFFF);
	if (length == 0)
	{
		// DMA length is 0. Set it to 65,536 words.
		// TODO: This was actually not working in the asm,
		// since I was testing for zero after an or/mov, expecting
		// the mov to set flags. mov doesn't set flags!
		// So I'm not sure if this is right or not.
		length = 65536;
	}
	
	// Set the DMA Busy flag.
	VDP_Status |= 0x0002;
	
	// TODO: Although we decrement DMAT_Length correctly based on
	// DMA cycles per line, we fill everything immediately instead
	// of filling at the correct rate.
	// Perhaps this should be combined with DMA_LOOP.
	VDP_Reg.DMA_Length = 0;	// Clear the DMA length.
	VDP_Ctrl.DMA = 0;	// Clear the DMA mode.
	
	// Set DMA type and length.
	VDP_Reg.DMAT_Type = 0x02;	// DMA Fill.
	VDP_Reg.DMAT_Length = length;
	
	// TODO: I don't think this algorithm is correct.
	// It matches the original asm, but does not match the Genesis Software Manual
	// for either even VRam addresses or odd VRam addresses.
	
	// Write the low byte first.
	// TODO: Endianness conversions.
	VRam.u8[address ^ 1] = (data & 0xFF);
	
	// Fill the data.
	const uint8_t fill_hi = (data >> 8) & 0xFF;
	do
	{
		VRam.u8[address] = fill_hi;
		address += VDP_Reg.m5.Auto_Inc;
	} while (--length != 0);
	
	// Save the new address.
	VDP_Ctrl.Address = (address & 0xFFFF);
}

/**
 * VDP_Write_Data_Word(): Write data to the VDP. (16-bit)
 * @param data 16-bit data.
 */
void VDP_Write_Data_Word(uint16_t data)
{
	// Clear the VDP data latch.
	VDP_Ctrl.Flag = 0;
	
	if (VDP_Ctrl.DMA & 0x04)
	{
		// DMA Fill operation is in progress.
		DMA_Fill(data);
		return;
	}
	
	// Check the access mode.
	uint32_t address = VDP_Ctrl.Address;
	switch (VDP_Ctrl.Access)
	{
		case 9:
			// VRam Write.
			VDP_Flags.VRam = 1;
			address &= 0xFFFF;	// VRam is 64 KB. (32 Kwords)
			if (address & 0x0001)
			{
				// Odd address.
				// VRam writes are only allowed at even addresses.
				// The VDP simply masks the low bit of the address
				// and swaps the high and low bytes before writing.
				address &= ~0x0001;
				data = (data << 8 | data >> 8);
			}
			
			// Write the word to VRam.
			VRam.u16[address>>1] = data;
			
			// Increment the address register.
			VDP_Ctrl.Address += VDP_Reg.m5.Auto_Inc;
			break;
		
		case 10:
			// CRam Write.
			// TODO: According to the Genesis Software Manual, writing at
			// odd addresses results in "interesting side effects".
			// Those side effects aren't listed, so we're just going to
			// mask the LSB for now.
			VDP_Flags.CRam = 1;
			address &= 0x7E;	// CRam is 128 bytes. (64 words)
			
			// Write the word to CRam.
			CRam.u16[address>>1] = data;
			
			// Increment the address register.
			VDP_Ctrl.Address += VDP_Reg.m5.Auto_Inc;
			break;
		
		case 11:
			// VSRam Write.
			// TODO: The Genesis Software Manual doesn't mention what happens
			// with regards to odd address writes for VSRam.
			// TODO: VSRam is 80 bytes, but we're allowing a maximum of 128 bytes here...
			VDP_Flags.CRam = 1;
			address &= 0x7E;	// VSRam is 80 bytes. (40 words)
			
			// Write the word to VSRam.
			VSRam.u16[address>>1] = data;
			
			// Increment the address register.
			VDP_Ctrl.Address += VDP_Reg.m5.Auto_Inc;
			break;
		
		default:
			// Invalid write specification.
			break;
	}
}


typedef enum
{
	DMA_DEST_VRAM	= 1,
	DMA_DEST_CRAM	= 2,
	DMA_DEST_VSRAM	= 3,
} DMA_Dest_t;

typedef enum
{
	DMA_SRC_ROM			= 0,
	DMA_SRC_M68K_RAM		= 1,
	DMA_SRC_PRG_RAM			= 2,
	DMA_SRC_WORD_RAM_2M		= 3,
	DMA_SRC_WORD_RAM_1M_0		= 5,
	DMA_SRC_WORD_RAM_1M_1		= 6,
	DMA_SRC_WORD_RAM_CELL_1M_0	= 7,
	DMA_SRC_WORD_RAM_CELL_1M_1	= 8,
} DMA_Src_t;

#define DMA_TYPE(src, dest) (((int)(src) << 2) | ((int)(dest)))


/**
 * T_DMA_Loop(): DMA copy loop.
 * @param src_component Source component.
 * @param dest_component Destination component.
 * @param src_address Source address.
 * @param length Length.
 */
template<DMA_Src_t src_component, DMA_Dest_t dest_component>
static inline void T_DMA_Loop(unsigned int src_address, unsigned int dest_address, int length)
{
	LOG_MSG(vdp_io, LOG_MSG_LEVEL_DEBUG2,
		"<%d, %d> src_address == 0x%06X, dest_address == 0x%04X, length == %d",
		src_component, dest_component, src_address, dest_address, length);
	
	// Save the DMA length for timing purposes.
	VDP_Reg.DMAT_Length = length;
	
	// Mask the source address, depending on type.
	switch (src_component)
	{
		case DMA_SRC_ROM:
			src_address &= 0x3FFFFE;
			break;
		
		case DMA_SRC_M68K_RAM:
			src_address &= 0xFFFE;
			break;
		
		case DMA_SRC_PRG_RAM:
			src_address = ((src_address & 0x1FFFE) + Bank_M68K);
			break;
		
		case DMA_SRC_WORD_RAM_2M:
			src_address -= 2;	// TODO: What is this for?
			src_address &= 0x3FFFE;
			break;
		
		case DMA_SRC_WORD_RAM_1M_0:
		case DMA_SRC_WORD_RAM_1M_1:
		case DMA_SRC_WORD_RAM_CELL_1M_0:
		case DMA_SRC_WORD_RAM_CELL_1M_1:
			src_address -= 2;	// TODO: What is this for?
			src_address &= 0x1FFFE;
			break;
		
		default:	// to make gcc shut up
			break;
	}
	
	// Determine if any flags should be set.
	switch (dest_component)
	{
		case DMA_DEST_VRAM:
			VDP_Flags.VRam = 1;
			VDP_Reg.DMAT_Type = 0;
			break;
		
		case DMA_DEST_CRAM:
			VDP_Flags.CRam = 1;
			VDP_Reg.DMAT_Type = 1;
			break;
		
		case DMA_DEST_VSRAM:
			VDP_Reg.DMAT_Type = 1;
			break;
		
		default:	// to make gcc shut up
			break;
	}
	
	VDP_Ctrl.DMA = 0;
	
	// src_base_address is used to ensure 128 KB wrapping.
	unsigned int src_base_address;
	if (src_component != DMA_SRC_M68K_RAM)
		src_base_address = (src_address & 0xFE0000);
	
	do
	{
		// Get the word.
		uint16_t w;
		switch (src_component)
		{
			case DMA_SRC_ROM:
				w = Rom_Data.u16[src_address >> 1];
				break;
			
			case DMA_SRC_M68K_RAM:
				w = Ram_68k.u16[src_address >> 1];
				break;
			
			case DMA_SRC_PRG_RAM:
				// TODO: This is untested!
				w = ((uint16_t*)Ram_Prg)[src_address >> 1];
				break;
			
			case DMA_SRC_WORD_RAM_2M:
				w = ((uint16_t*)Ram_Word_2M)[src_address >> 1];
				break;
			
			case DMA_SRC_WORD_RAM_1M_0:
				// TODO: This is untested!
				w = ((uint16_t*)Ram_Word_1M)[src_address >> 1];
				break;
			
			case DMA_SRC_WORD_RAM_1M_1:
				// TODO: This is untested!
				w = ((uint16_t*)Ram_Word_1M)[(src_address + 0x20000) >> 1];
				break;
			
			case DMA_SRC_WORD_RAM_CELL_1M_0:
				// TODO: This is untested!
				// Cell conversion is required.
				w = Cell_Conv_Tab[src_address >> 1];
				w = ((uint16_t*)Ram_Word_1M)[w];
				break;
			
			case DMA_SRC_WORD_RAM_CELL_1M_1:
				// TODO: This is untested!
				// Cell conversion is required.
				w = Cell_Conv_Tab[src_address >> 1];
				w = ((uint16_t*)Ram_Word_1M)[w + (0x20000 >> 1)];
				break;
			
			default:	// to make gcc shut up
				break;
		}
		
		// Increment the source address.
		// TODO: The 128 KB wrapping causes garbage on TmEE's mmf.bin (correct),
		// but the garbage doesn't match Kega Fusion.
		if (src_component == DMA_SRC_M68K_RAM)
			src_address = ((src_address + 2) & 0xFFFF);
		else
			src_address = (((src_address + 2) & 0x1FFFF) | src_base_address);
		
		// Write the word.
		switch (dest_component)
		{
			case DMA_DEST_VRAM:
				if (dest_address & 1)
					w = (w << 8 | w >> 8);
				VRam.u16[dest_address >> 1] = w;
				break;
			
			case DMA_DEST_CRAM:
				CRam.u16[dest_address >> 1] = w;
				break;
			
			case DMA_DEST_VSRAM:
				VSRam.u16[dest_address >> 1] = w;
				break;
			
			default:	// to make gcc shut up
				break;
		}
		
		dest_address = ((dest_address + VDP_Reg.m5.Auto_Inc) & 0xFFFF);
		
		// Check for CRam or VSRam destination overflow.
		if (dest_component == DMA_DEST_CRAM ||
		    dest_component == DMA_DEST_VSRAM)
		{
			if (dest_address >= 0x80)
			{
				// CRam/VSRam overflow!
				length--;	// for this word
				break;
			}
		}
	} while (--length != 0);
	
	// Save the new destination address.
	VDP_Ctrl.Address = dest_address;
	
	// If any bytes weren't copied, subtract it from the saved length.
	VDP_Reg.DMAT_Length -= length;
	if (VDP_Reg.DMAT_Length <= 0)
	{
		// No DMA left!
		VDP_Status &= ~0x0002;
		return;
	}
	
	// Save the new source address.
	// NOTE: The new DMA_Address is the wrapped version.
	// The old asm code saved the unwrapped version.
	// Ergo, it simply added length to DMA_Address.
	VDP_Reg.DMA_Address += VDP_Reg.DMAT_Length;
	VDP_Reg.DMA_Address &= 0x7FFFFF;
	
	// Update DMA.
	VDP_Update_DMA();
	main68k_releaseCycles();
}


extern "C" {
void VDP_Do_DMA_asm(DMA_Dest_t dest_component, unsigned int src_address, unsigned int dest_address,
		    int length, unsigned int auto_inc, int src_component);
void VDP_Do_DMA_COPY_asm(unsigned int src_address, unsigned int dest_address, int length, unsigned int auto_inc);
}

/**
 * VDP_Write_Ctrl(): Write a control word to the VDP.
 * @param data Control word.
 */
void VDP_Write_Ctrl(uint16_t data)
{
	// TODO: Check endianness with regards to the control words. (Wordswapping!)
	
	// Check if this is the first or second control word.
	if (!VDP_Ctrl.Flag)
	{
		// First control word.
		// Check if this is an actual control word or a register write.
		if ((data & 0xC000) == 0x8000)
		{
			// Register write.
			VDP_Ctrl.Access = 5;	// TODO: What does this mean?
			VDP_Ctrl.Address = 0;	// Reset the address counter.
			
			const int reg = (data >> 8) & 0x1F;
			VDP_Set_Reg(reg, (data & 0xFF));
			return;
		}
		
		// Control word.
		VDP_Ctrl.Data.w[0] = data;
		VDP_Ctrl.Flag = 1;		// Latch the first control word.
		
		// Determine the VDP address.
		VDP_Ctrl.Address = (data & 0x3FFF);
		VDP_Ctrl.Address |= ((VDP_Ctrl.Data.w[1] & 0x3) << 14);
		
		// Determine the destination.
		unsigned int CD_Offset = ((data >> 14) & 0x3);
		CD_Offset |= ((VDP_Ctrl.Data.w[1] & 0xF0) >> 2);
		VDP_Ctrl.Access = (CD_Table[CD_Offset] & 0xFF);
		return;
	}
	
	// Second control word.
	VDP_Ctrl.Data.w[1] = data;
	VDP_Ctrl.Flag = 0;		// Clear the latch.
	
	// Determine the VDP address.
	VDP_Ctrl.Address = (VDP_Ctrl.Data.w[0] & 0x3FFF);
	VDP_Ctrl.Address |= ((data & 3) << 14);
	
	// Determine the destination.
	unsigned int CD_Offset = ((VDP_Ctrl.Data.w[0] >> 14) & 0x3);
	CD_Offset |= ((data & 0xF0) >> 2);
	unsigned int CD = CD_Table[CD_Offset];
	VDP_Ctrl.Access = (CD & 0xFF);
	
	if (!(CD & 0xFF00))
	{
		// No DMA is needed.
		return;
	}
	
	/** Perform a DMA operation. **/
	
	// Check if DMA is enabled.
	if (!(VDP_Reg.m5.Set2 & 0x10))
	{
		// DMA is disabled.
		VDP_Ctrl.DMA = 0;
		return;
	}
	
	// DMA access mode is the high byte in the CD_Table[] word.
	CD >>= 8;
	
	// Check for DMA FILL.
	if ((CD & 0x04) && (VDP_Ctrl.DMA_Mode == 0x80))
	{
		// DMA FILL.
		VDP_Ctrl.DMA = (CD & 0xFF);
		return;
	}
	
	// Determine the DMA destination.
	DMA_Dest_t dest_component = (DMA_Dest_t)(CD & 0x03);	// 0 == invalid; 1 == VRam; 2 == CRam; 3 == VSRam
	
	// Get the DMA addresses.
	unsigned int src_address = VDP_Reg.DMA_Address;			// Src Address / 2
	unsigned int dest_address = (VDP_Ctrl.Address & 0xFFFF);	// Dest Address
	
	// Check for CRam or VSRam destination overflow.
	if (dest_component == DMA_DEST_CRAM ||
	    dest_component == DMA_DEST_VSRAM)
	{
		if (dest_address >= 0x80)
		{
			// CRam/VSRam overflow! Don't do anything.
			VDP_Ctrl.DMA = 0;
			return;
		}
	}
	
	int length = (VDP_Reg.DMA_Length & 0xFFFF);
	if (length == 0)
	{
		// DMA length is zero.
		if (Zero_Length_DMA)
		{
			// Zero-Length DMA transfers are enabled.
			// Ignore this request.
			VDP_Ctrl.DMA = 0;
			return;
		}
		
		// Zero-Length DMA trnasfers are disabled.
		// The MD VDP decrements the DMA length counter before checking if it has
		// reached zero. So, doing a zero-length DMA request will actually do a
		// DMA request for 65,536 words.
		length = 0x10000;
	}
	
	// Check for DMA COPY.
	if (VDP_Ctrl.DMA_Mode == 0xC0)
	{
		// DMA COPY.
		src_address &= 0xFFFF;
		VDP_Status |= 0x0002;	// Set the DMA BUSY bit.
		VDP_Reg.DMA_Length = 0;
		VDP_Reg.DMAT_Length = length;
		VDP_Reg.DMAT_Type = 0x3;
		VDP_Flags.VRam = 1;
		
		// TODO: Is this correct with regards to endianness?
		do
		{
			VRam.u8[dest_address] = VRam.u8[src_address];
			
			// Increment the addresses.
			src_address = ((src_address + 1) & 0xFFFF);
			dest_address = ((dest_address + VDP_Reg.m5.Auto_Inc) & 0xFFFF);
		} while (--length != 0);
		
		// Save the new addresses.
		VDP_Reg.DMA_Address = src_address;
		VDP_Ctrl.Address = dest_address;
		//VDP_Do_DMA_COPY_asm(src_address, dest_address, length, VDP_Reg.m5.Auto_Inc);
		return;
	}
	
	if (VDP_Ctrl.DMA_Mode & 0x80)
	{
		// TODO: What does this mean?
		VDP_Ctrl.DMA = 0;
		return;
	}
	
	// Multiply the source address by two to get the real source address.
	src_address *= 2;
	
	// Determine the source component.
	DMA_Src_t src_component;
	int WRam_Mode;
	if (src_address < Rom_Size)
	{
		// Main ROM.
		src_component = DMA_SRC_ROM;
		goto DMA_Src_OK;
	}
	if (!SegaCD_Started)
	{
		// SegaCD is not started. Assume M68K RAM.
		// TODO: This includes invalid addresses!
		src_component = DMA_SRC_M68K_RAM;
		goto DMA_Src_OK;
	}
	
	// SegaCD is started.
	if (src_address >= 0x240000)
	{
		// Assume M68K RAM.
		// TODO: This includes invalid addresses!
		src_component = DMA_SRC_M68K_RAM;
		goto DMA_Src_OK;
	}
	else if (src_address < 0x40000)
	{
		// Program RAM.
		src_component = DMA_SRC_PRG_RAM;
		goto DMA_Src_OK;
	}
	
	// Word RAM. Check the Word RAM state to determine the mode.
	// TODO: Determine how this works.
	WRam_Mode = (Ram_Word_State & 0x03) + 3;
	if (WRam_Mode < 5 || src_address < 0x220000)
	{
		src_component = (DMA_Src_t)WRam_Mode;
		goto DMA_Src_OK;
	}
	src_component = (DMA_Src_t)(WRam_Mode + 2);
	
DMA_Src_OK:
	
	// Set the DMA BUSY bit.
	VDP_Status |= 0x0002;
	
	switch (DMA_TYPE(src_component, dest_component))
	{
		case DMA_TYPE(DMA_SRC_ROM, DMA_DEST_VRAM):
			T_DMA_Loop<DMA_SRC_ROM, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_ROM, DMA_DEST_CRAM):
			T_DMA_Loop<DMA_SRC_ROM, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_ROM, DMA_DEST_VSRAM):
			T_DMA_Loop<DMA_SRC_ROM, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_M68K_RAM, DMA_DEST_VRAM):
			T_DMA_Loop<DMA_SRC_M68K_RAM, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_M68K_RAM, DMA_DEST_CRAM):
			T_DMA_Loop<DMA_SRC_M68K_RAM, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_M68K_RAM, DMA_DEST_VSRAM):
			T_DMA_Loop<DMA_SRC_M68K_RAM, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_PRG_RAM, DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_PRG_RAM, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_PRG_RAM, DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_PRG_RAM, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_PRG_RAM, DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_PRG_RAM, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_2M, DMA_DEST_VRAM):
			T_DMA_Loop<DMA_SRC_WORD_RAM_2M, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_2M, DMA_DEST_CRAM):
			T_DMA_Loop<DMA_SRC_WORD_RAM_2M, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_2M, DMA_DEST_VSRAM):
			T_DMA_Loop<DMA_SRC_WORD_RAM_2M, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_0, DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_0, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_0, DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_0, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_0, DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_0, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_1, DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_1, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_1, DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_1, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_1M_1, DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_1M_1, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_0, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_VRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_VRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_CRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_CRAM>(src_address, dest_address, length);
			break;
		
		case DMA_TYPE(DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_VSRAM):
			// TODO: This is untested!
			T_DMA_Loop<DMA_SRC_WORD_RAM_CELL_1M_1, DMA_DEST_VSRAM>(src_address, dest_address, length);
			break;
		
		default:
			// Invalid DMA mode.
			VDP_Ctrl.DMA = 0;
			break;
	}
	
	return;
}

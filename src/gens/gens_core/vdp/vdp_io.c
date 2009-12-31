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

#include "vdp_io.h"
#include "vdp_rend.h"

// Starscream 68000 core.
#include "gens_core/cpu/68k/star_68k.h"

// C includes.
#include <string.h>


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


const uint8_t DMA_Timing_Table[] =
{
	/*
	83,  167, 166,  83,
	102, 205, 204, 102,
	8,    16,  15,   8,
	9,    18,  17,   9
	*/
	
	/*
	92,  167, 166,  83,
	118, 205, 204, 102,
	9,    16,  15,   8,
	10,   18,  17,   9
	*/
	
	83,  167, 166,  83,
	102, 205, 204, 102,
	8,    16,  15,   8,
	9,    18,  17,   9
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
	
	// Reset the non-register parts of the VDP registers.
	VDP_Reg.DMA_Length = 0;
	VDP_Reg.DMA_Address = 0;
	
	// Other variables.
	VDP_Status = 0x0200;
	VDP_Int = 0;
	DMAT_Tmp = 0;
	DMAT_Length = 0;
	DMAT_Type = 0;
	Ctrl.Flag = 0;
	Ctrl.Data = 0;
	Ctrl.Write = 0;
	Ctrl.Access = 0;
	Ctrl.Address = 0;
	Ctrl.DMA_Mode = 0;
	Ctrl.DMA = 0;
	
	// Set the CRam and VRam flags.
	CRam_Flag = 1;
	VRam_Flag = 1;
	
	// Initialize the Horizontal Counter table.
	unsigned int hc;
	unsigned int hc_val;
	for (hc = 0; hc < 512; hc++)
	{
		hc_val = ((hc * 170) / 488) - 0x18;
		H_Counter_Table[hc][0] = (unsigned char)hc_val;
		
		hc_val = ((hc * 205) / 488) - 0x1C;
		H_Counter_Table[hc][1] = (unsigned char)hc_val;
	}
	
	// Clear the VSRam overflow area.
	memset(&VSRam_Over, 0x00, sizeof(VSRam_Over));
}


uint8_t VDP_Int_Ack(void)
{
	if ((VDP_Reg.Set2 & 0x20) && (VDP_Int & 0x08))
	{
		// VBlank interrupt acknowledge.
		VDP_Int &= ~0x08;
		
		uint8_t rval_mask = VDP_Reg.Set1;
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
	if ((VDP_Reg.Set2 & 0x20) && (VDP_Int & 0x08))
	{
		// VBlank interrupt.
		main68k_interrupt(6, -1);
		return;
	}
	else if ((VDP_Reg.Set1 & 0x10) && (VDP_Int & 0x04))
	{
		// HBlank interrupt.
		main68k_interrupt(4, -1);
		return;
	}
	
	// No VDP interrupts.
	main68k_context.interrupts[0] &= 0xF0;
}


// The following are pointers from vdp_io_x86.asm.
extern uint8_t *ScrA_Addr;
extern uint8_t *Win_Addr;
extern uint8_t *ScrB_Addr;
extern uint8_t *Spr_Addr;
extern uint8_t *H_Scroll_Addr;

// The following are internal scroll settings.
extern int V_Scroll_MMask;
extern int H_Scroll_Mask;

extern int H_Scroll_CMul;
extern int H_Scroll_CMask;
extern int V_Scroll_CMask;

// The following are internal H cell settings.
extern int H_Cell;
extern int H_Win_Mul;
extern int H_Pix;
extern int H_Pix_Begin;

// The following are internal window settings that should be eliminated.
extern int Win_X_Pos;
extern int Win_Y_Pos;

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
			
			// VDP register 0, bit 2: Palette Select
			// If cleared, only the LSBs of each CRAM component is used.
			CRam_Flag = 1;
			break;
		
		case 1:
			// Mode Set 2.
			VDP_Update_IRQ_Line();
			break;
		
		case 2:
			// Scroll A base address.
			tmp = (val & 0x38) << 10;
			ScrA_Addr = &VRam.u8[tmp];
			break;
		
		case 3:
			// Window base address.
			if (VDP_Reg.Set4 & 0x01)	// Check for H40 mode. (TODO: Test 0x81 instead?)
				tmp = (val & 0x3C) << 10;	// H40.
			else
				tmp = (val & 0x3E) << 10;	// H32.
			
			Win_Addr = &VRam.u8[tmp];
			break;
		
		case 4:
			// Scroll B base address.
			tmp = (val & 0x07) << 13;
			ScrB_Addr = &VRam.u8[tmp];
			break;
		
		case 5:
			// Sprite Attribute Table base address.
			if (VDP_Reg.Set4 & 0x01)	// Check for H40 mode. (TODO: Test 0x81 instead?)
				tmp = (val & 0x7E) << 9;
			else
				tmp = (val & 0x7F) << 9;
			
			Spr_Addr = &VRam.u8[tmp];
			VRam_Flag |= 2;		// Spriteshave changed.
			break;
		
		case 7:
			// Background Color.
			CRam_Flag = 1;
			break;
		
		case 11:
		{
			// Mode Set 3.
			static const unsigned int Size_V_Scroll[4] = {255, 511, 255, 1023};
			static const unsigned int H_Scroll_Mask_Table[4] = {0x0000, 0x0007, 0x01F8, 0x1FFF};
			
			// Check the Vertical Scroll mode. (Bit 3)
			// 0: Full scrolling. (Mask == 0)
			// 1: 2CELL scrolling. (Mask == 0x7E)
			V_Scroll_MMask = ((val & 4) ? 0x7E : 0);
			
			// Horizontal Scroll mode
			H_Scroll_Mask = H_Scroll_Mask_Table[val & 3];
			
			break;
		}
		
		case 12:
			// Mode Set 4.
			
			// This register has the Shadow/Highlight setting,
			// so set the CRam Flag to force a CRam update.
			CRam_Flag = 1;
			
			if (val & 0x81)		// TODO: Original asm tests 0x81. Should this be done for other H40 tests?
			{
				// H40 mode.
				H_Cell = 40;
				H_Win_Mul = 6;
				H_Pix = 320;
				H_Pix_Begin = 0;
				
				// Check the window horizontal position.
				// TODO: Eliminate Win_X_Pos. (Just use VDP_Reg.Win_H_Pos directly.)
				if ((VDP_Reg.Win_H_Pos & 0x1F) > 40)
					Win_X_Pos = 40;
				
				// Update the Window base address.
				tmp = (VDP_Reg.Pat_Win_Adr & 0x3C) << 10;
				Win_Addr = &VRam.u8[tmp];
				
				// Update the Sprite Attribute Table base address.
				tmp = (VDP_Reg.Spr_Att_Adr & 0x7E) << 9;
				Spr_Addr = &VRam.u8[tmp];
			}
			else
			{
				// H32 mode.
				H_Cell = 32;
				H_Win_Mul = 5;
				H_Pix = 256;
				H_Pix_Begin = 32;
				
				// Check the window horizontal position.
				// TODO: Eliminate Win_X_Pos. (Just use VDP_Reg.Win_H_Pos directly.)
				if ((VDP_Reg.Win_H_Pos & 0x1F) > 32)
					Win_X_Pos = 32;
				
				// Update the Window base address.
				tmp = (VDP_Reg.Pat_Win_Adr & 0x3E) << 10;
				Win_Addr = &VRam.u8[tmp];
				
				// Update the Sprite Attribute Table base address.
				tmp = (VDP_Reg.Spr_Att_Adr & 0x7F) << 9;
				Spr_Addr = &VRam.u8[tmp];
			}
			
			break;
		
		case 13:
			// H Scroll Table base address.
			tmp = (val & 0x3F) << 10;
			H_Scroll_Addr = &VRam.u8[tmp];
			break;
		
		case 16:
		{
			// Scroll Size.
			tmp = (val & 0x3);
			tmp |= (val & 0x30) >> 2;
			switch (tmp)
			{
				case 0:		// V32_H32
				case 8:		// VXX_H32
					H_Scroll_CMul = 5;
					H_Scroll_CMask = 31;
					V_Scroll_CMask = 31;
					break;
				
				case 4:		// V64_H32
					H_Scroll_CMul = 5;
					H_Scroll_CMask = 31;
					V_Scroll_CMask = 64;
					break;
				
				case 12:	// V128_H32
					H_Scroll_CMul = 5;
					H_Scroll_CMask = 31;
					V_Scroll_CMask = 127;
					break;
				
				case 1:		// V32_H64
				case 9:		// VXX_H64
					H_Scroll_CMul = 6;
					H_Scroll_CMask = 63;
					V_Scroll_CMask = 31;
					break;
				
				case 5:		// V64_H64
				case 13:	// V128_H64
					H_Scroll_CMul = 6;
					H_Scroll_CMask = 63;
					V_Scroll_CMask = 63;
					break;
				
				case 2:		// V32_HXX
				case 6:		// V64_HXX
				case 10:	// VXX_HXX
				case 14:	// V128_HXX
					H_Scroll_CMul = 6;
					H_Scroll_CMask = 63;
					V_Scroll_CMask = 0;
					break;
				
				case 3:		// V32_H128
				case 7:		// V64_H128
				case 11:	// VXX_H128
				case 15:	// V128_H128
					H_Scroll_CMul = 7;
					H_Scroll_CMask = 127;
					V_Scroll_CMask = 31;
					break;
			}
			
			break;
		}
		
		case 17:
			// Window H position.
			Win_X_Pos = (val & 0x1F);
			if (Win_X_Pos > H_Cell)
				Win_X_Pos = H_Cell;
			break;
			
		case 18:
			// Window V position.
			Win_Y_Pos = (val & 0x1F);
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
			Ctrl.DMA_Mode = (val & 0xC0);
			break;
	}
}

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


const uint32_t Size_V_Scroll[4] = {255, 511, 255, 1023};
const uint32_t H_Scroll_Mask_Table[4] = {0x0000, 0x0007, 0x01F8, 0x1FFF};


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
		Set_VDP_Reg(reg, vdp_reg_init[reg]);
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

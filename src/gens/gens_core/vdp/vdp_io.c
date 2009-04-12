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

// C includes.
#include <string.h>


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
	
	// VDP registers.
	int reg;
	for (reg = 0; reg < 24; reg++)
	{
		Set_VDP_Reg(reg, 0);
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
	
	// Reset VSRam_Over[7].
	// TODO: What does this do, and why only reset the 7th item in the array?
	VSRam_Over[7] = 0;
}

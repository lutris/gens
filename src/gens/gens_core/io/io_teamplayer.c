/***************************************************************************
 * Gens: Controller I/O. (Templayer Adapter)                               *
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

#include "io_teamplayer.h"
#include "io.h"

// C includes.
#include <stdint.h>

// I/O table values.
typedef enum _TBL_TP_IO_VAL
{
	TP_UNDEF = 0,
	
	TP_L0L,		TP_L0H,		TP_H0L,		TP_H0H,
	TP_L1L,		TP_L1H,		TP_H1L,		TP_H1H,
	TP_L2L,		TP_L2H,		TP_H2L,		TP_H2H,
	TP_L3L,		TP_L3H,		TP_H3L,		TP_H3H,
	
	TP_PA_DIR,	TP_PA_ABC,	TP_PA_XYZ,
	TP_PB_DIR,	TP_PB_ABC,	TP_PB_XYZ,
	TP_PC_DIR,	TP_PC_ABC,	TP_PC_XYZ,
	TP_PD_DIR,	TP_PD_ABC,	TP_PD_XYZ
} TBL_TP_IO_VAL;

// I/O tables.
static uint8_t tbl_tp_io[2][256] =
{
	// Teamplayer 1: Default I/O table.
	{TP_L0L,	TP_L0H,		TP_H0L,		TP_H0H,
	 TP_L1L,	TP_L1H,		TP_H1L,		TP_H1H,
	 TP_L2L,	TP_L2H,		TP_H2L,		TP_H2H,
	 TP_UNDEF,	TP_L3H,		TP_UNDEF,	TP_H3H},
	
	// Teamplayer 2: Default I/O table.
	{TP_L0L,	TP_L0H,		TP_H0L,		TP_H0H,
	 TP_L1L,	TP_L1H,		TP_H1L,		TP_H1H,
	 TP_L2L,	TP_L2H,		TP_H2L,		TP_H2H,
	 TP_UNDEF,	TP_L3H,		TP_UNDEF,	TP_H3H},
};


/**
 * RD_Controller_1_TP(): Read a Teamplayer device plugged into Port 1.
 * @return Controller port data.
 */
unsigned char RD_Controller_1_TP(void)
{
	uint32_t eax, ebx;
	
	eax = Controller_1_State;
	ebx = Controller_1_Counter;
	
	if (Controller_2_State & 0x0C)
	{
		// TODO: I believe this is for 4-Way Play.
		return 0;
	}
	
	eax >>= 3;
	ebx &= 0x0F0000;
	eax &= 0x0C;
	ebx >>= 12;
	eax += ebx;
	ebx = eax;
	if (eax & 0x04)
		ebx |= 0x02;
	
	ebx <<= 3;
	ebx &= 0x70;
	
	switch (tbl_tp_io[0][eax / 4])
	{
		case TP_H0H:
			return 0x73;
		
		case TP_L0H:
			return 0x3F;
		
		case TP_H0L:
		case TP_L0L:
		case TP_H1H:
		case TP_L1H:
			return ebx;
		
		case TP_H1L:
		case TP_L1L:
			eax = (Controller_1_Type & 1);
			eax |= (ebx & 0xFFFF);
			return eax;
		
		case TP_H2H:
		case TP_L2H:
			eax = (Controller_1B_Type & 1);
			eax |= (ebx & 0xFFFF);
			return eax;
			
		case TP_H2L:
		case TP_L2L:
			eax = (Controller_1C_Type & 1);
			eax |= (ebx & 0xFFFF);
			return eax;
		
		case TP_H3H:
		case TP_L3H:
			eax = (Controller_1D_Type & 1);
			eax |= (ebx & 0xFFFF);
			return eax;
		
		case TP_PA_DIR:
			// Player A: D-pad.
			eax = ((Controller_1_Buttons & CONTROLLER_RIGHT ? 8 : 0) |
			       (Controller_1_Buttons & CONTROLLER_LEFT  ? 4 : 0) |
			       (Controller_1_Buttons & CONTROLLER_DOWN  ? 2 : 0) |
			       (Controller_1_Buttons & CONTROLLER_UP    ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PA_ABC:
			// Player A: ABC/Start.
			eax = ((Controller_1_Buttons & CONTROLLER_START ? 8 : 0) |
			       (Controller_1_Buttons & CONTROLLER_A     ? 4 : 0) |
			       (Controller_1_Buttons & CONTROLLER_C     ? 2 : 0) |
			       (Controller_1_Buttons & CONTROLLER_B     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PA_XYZ:
			// Player A: XYZ/Mode.
			eax = ((Controller_1_Buttons & CONTROLLER_MODE  ? 8 : 0) |
			       (Controller_1_Buttons & CONTROLLER_X     ? 4 : 0) |
			       (Controller_1_Buttons & CONTROLLER_Y     ? 2 : 0) |
			       (Controller_1_Buttons & CONTROLLER_Z     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PB_DIR:
			// Player B: D-pad.
			eax = ((Controller_1B_Buttons & CONTROLLER_RIGHT ? 8 : 0) |
			       (Controller_1B_Buttons & CONTROLLER_LEFT  ? 4 : 0) |
			       (Controller_1B_Buttons & CONTROLLER_DOWN  ? 2 : 0) |
			       (Controller_1B_Buttons & CONTROLLER_UP    ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PB_ABC:
			// Player B: ABC/Start.
			eax = ((Controller_1B_Buttons & CONTROLLER_START ? 8 : 0) |
			       (Controller_1B_Buttons & CONTROLLER_A     ? 4 : 0) |
			       (Controller_1B_Buttons & CONTROLLER_C     ? 2 : 0) |
			       (Controller_1B_Buttons & CONTROLLER_B     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PB_XYZ:
			// Player B: XYZ/Mode.
			eax = ((Controller_1B_Buttons & CONTROLLER_MODE  ? 8 : 0) |
			       (Controller_1B_Buttons & CONTROLLER_X     ? 4 : 0) |
			       (Controller_1B_Buttons & CONTROLLER_Y     ? 2 : 0) |
			       (Controller_1B_Buttons & CONTROLLER_Z     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PC_DIR:
			// Player C: D-pad.
			eax = ((Controller_1C_Buttons & CONTROLLER_RIGHT ? 8 : 0) |
			       (Controller_1C_Buttons & CONTROLLER_LEFT  ? 4 : 0) |
			       (Controller_1C_Buttons & CONTROLLER_DOWN  ? 2 : 0) |
			       (Controller_1C_Buttons & CONTROLLER_UP    ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PC_ABC:
			// Player C: ABC/Start.
			eax = ((Controller_1C_Buttons & CONTROLLER_START ? 8 : 0) |
			       (Controller_1C_Buttons & CONTROLLER_A     ? 4 : 0) |
			       (Controller_1C_Buttons & CONTROLLER_C     ? 2 : 0) |
			       (Controller_1C_Buttons & CONTROLLER_B     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PC_XYZ:
			// Player C: XYZ/Mode.
			eax = ((Controller_1C_Buttons & CONTROLLER_MODE  ? 8 : 0) |
			       (Controller_1C_Buttons & CONTROLLER_X     ? 4 : 0) |
			       (Controller_1C_Buttons & CONTROLLER_Y     ? 2 : 0) |
			       (Controller_1C_Buttons & CONTROLLER_Z     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PD_DIR:
			// Player D: D-pad.
			eax = ((Controller_1D_Buttons & CONTROLLER_RIGHT ? 8 : 0) |
			       (Controller_1D_Buttons & CONTROLLER_LEFT  ? 4 : 0) |
			       (Controller_1D_Buttons & CONTROLLER_DOWN  ? 2 : 0) |
			       (Controller_1D_Buttons & CONTROLLER_UP    ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PD_ABC:
			// Player D: ABC/Start.
			eax = ((Controller_1D_Buttons & CONTROLLER_START ? 8 : 0) |
			       (Controller_1D_Buttons & CONTROLLER_A     ? 4 : 0) |
			       (Controller_1D_Buttons & CONTROLLER_C     ? 2 : 0) |
			       (Controller_1D_Buttons & CONTROLLER_B     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PD_XYZ:
			// Player D: XYZ/Mode.
			eax = ((Controller_1D_Buttons & CONTROLLER_MODE  ? 8 : 0) |
			       (Controller_1D_Buttons & CONTROLLER_X     ? 4 : 0) |
			       (Controller_1D_Buttons & CONTROLLER_Y     ? 2 : 0) |
			       (Controller_1D_Buttons & CONTROLLER_Z     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_UNDEF:
		default:
			// Unknown state.
			eax = 0x0F;
			eax |= ebx;
			return eax;
	}
}


/**
 * RD_Controller_2_TP(): Read a Teamplayer device plugged into Port 2.
 * @return Controller port data.
 */
unsigned char RD_Controller_2_TP(void)
{
	uint32_t eax, ebx;
	
	eax = Controller_2_State;
	ebx = Controller_2_Counter;
	
	eax >>= 3;
	ebx &= 0x0F0000;
	eax &= 0x0C;
	ebx >>= 12;
	eax += ebx;
	ebx = eax;
	if (eax & 0x04)
		ebx |= 0x02;
	
	ebx <<= 3;
	ebx &= 0x70;
	
	switch (tbl_tp_io[0][eax / 4])
	{
		case TP_H0H:
			return 0x73;
		
		case TP_L0H:
			return 0x3F;
		
		case TP_H0L:
		case TP_L0L:
		case TP_H1H:
		case TP_L1H:
			return ebx;
		
		case TP_H1L:
		case TP_L1L:
			eax = (Controller_2_Type & 1);
			eax |= (ebx & 0xFFFF);
			return eax;
		
		case TP_H2H:
		case TP_L2H:
			eax = (Controller_2B_Type & 1);
			eax |= (ebx & 0xFFFF);
			return eax;
			
		case TP_H2L:
		case TP_L2L:
			eax = (Controller_2C_Type & 1);
			eax |= (ebx & 0xFFFF);
			return eax;
		
		case TP_H3H:
		case TP_L3H:
			eax = (Controller_2D_Type & 1);
			eax |= (ebx & 0xFFFF);
			return eax;
		
		case TP_PA_DIR:
			// Player A: D-pad.
			eax = ((Controller_2_Buttons & CONTROLLER_RIGHT ? 8 : 0) |
			       (Controller_2_Buttons & CONTROLLER_LEFT  ? 4 : 0) |
			       (Controller_2_Buttons & CONTROLLER_DOWN  ? 2 : 0) |
			       (Controller_2_Buttons & CONTROLLER_UP    ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PA_ABC:
			// Player A: ABC/Start.
			eax = ((Controller_2_Buttons & CONTROLLER_START ? 8 : 0) |
			       (Controller_2_Buttons & CONTROLLER_A     ? 4 : 0) |
			       (Controller_2_Buttons & CONTROLLER_C     ? 2 : 0) |
			       (Controller_2_Buttons & CONTROLLER_B     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PA_XYZ:
			// Player A: XYZ/Mode.
			eax = ((Controller_2_Buttons & CONTROLLER_MODE  ? 8 : 0) |
			       (Controller_2_Buttons & CONTROLLER_X     ? 4 : 0) |
			       (Controller_2_Buttons & CONTROLLER_Y     ? 2 : 0) |
			       (Controller_2_Buttons & CONTROLLER_Z     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PB_DIR:
			// Player B: D-pad.
			eax = ((Controller_2B_Buttons & CONTROLLER_RIGHT ? 8 : 0) |
			       (Controller_2B_Buttons & CONTROLLER_LEFT  ? 4 : 0) |
			       (Controller_2B_Buttons & CONTROLLER_DOWN  ? 2 : 0) |
			       (Controller_2B_Buttons & CONTROLLER_UP    ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PB_ABC:
			// Player B: ABC/Start.
			eax = ((Controller_2B_Buttons & CONTROLLER_START ? 8 : 0) |
			       (Controller_2B_Buttons & CONTROLLER_A     ? 4 : 0) |
			       (Controller_2B_Buttons & CONTROLLER_C     ? 2 : 0) |
			       (Controller_2B_Buttons & CONTROLLER_B     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PB_XYZ:
			// Player B: XYZ/Mode.
			eax = ((Controller_2B_Buttons & CONTROLLER_MODE  ? 8 : 0) |
			       (Controller_2B_Buttons & CONTROLLER_X     ? 4 : 0) |
			       (Controller_2B_Buttons & CONTROLLER_Y     ? 2 : 0) |
			       (Controller_2B_Buttons & CONTROLLER_Z     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PC_DIR:
			// Player C: D-pad.
			eax = ((Controller_2C_Buttons & CONTROLLER_RIGHT ? 8 : 0) |
			       (Controller_2C_Buttons & CONTROLLER_LEFT  ? 4 : 0) |
			       (Controller_2C_Buttons & CONTROLLER_DOWN  ? 2 : 0) |
			       (Controller_2C_Buttons & CONTROLLER_UP    ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PC_ABC:
			// Player C: ABC/Start.
			eax = ((Controller_2C_Buttons & CONTROLLER_START ? 8 : 0) |
			       (Controller_2C_Buttons & CONTROLLER_A     ? 4 : 0) |
			       (Controller_2C_Buttons & CONTROLLER_C     ? 2 : 0) |
			       (Controller_2C_Buttons & CONTROLLER_B     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PC_XYZ:
			// Player C: XYZ/Mode.
			eax = ((Controller_2C_Buttons & CONTROLLER_MODE  ? 8 : 0) |
			       (Controller_2C_Buttons & CONTROLLER_X     ? 4 : 0) |
			       (Controller_2C_Buttons & CONTROLLER_Y     ? 2 : 0) |
			       (Controller_2C_Buttons & CONTROLLER_Z     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PD_DIR:
			// Player D: D-pad.
			eax = ((Controller_2D_Buttons & CONTROLLER_RIGHT ? 8 : 0) |
			       (Controller_2D_Buttons & CONTROLLER_LEFT  ? 4 : 0) |
			       (Controller_2D_Buttons & CONTROLLER_DOWN  ? 2 : 0) |
			       (Controller_2D_Buttons & CONTROLLER_UP    ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PD_ABC:
			// Player D: ABC/Start.
			eax = ((Controller_2D_Buttons & CONTROLLER_START ? 8 : 0) |
			       (Controller_2D_Buttons & CONTROLLER_A     ? 4 : 0) |
			       (Controller_2D_Buttons & CONTROLLER_C     ? 2 : 0) |
			       (Controller_2D_Buttons & CONTROLLER_B     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_PD_XYZ:
			// Player D: XYZ/Mode.
			eax = ((Controller_2D_Buttons & CONTROLLER_MODE  ? 8 : 0) |
			       (Controller_2D_Buttons & CONTROLLER_X     ? 4 : 0) |
			       (Controller_2D_Buttons & CONTROLLER_Y     ? 2 : 0) |
			       (Controller_2D_Buttons & CONTROLLER_Z     ? 1 : 0)) + ebx;
			return eax;
		
		case TP_UNDEF:
		default:
			// Unknown state.
			eax = 0x0F;
			eax |= ebx;
			return eax;
	}
}


static inline int calc_IO_offset(int tp_num)
{
	//return (!(tp_num & 0x01) | ((tp_num & 0x0E) << 3));
	int ecx, edx;
	
	ecx = tp_num;
	edx = tp_num;
	ecx &= 0x1;
	edx &= 0xE;
	ecx ^= 0x1;
	edx <<= 3;
	
	return ((edx + (ecx * 4)) / 4) + 0xC;
}


static void Make_IO_Table_Player(int port, int typeA, int typeB, int typeC, int typeD)
{
	int tp_num = 1;	// Teamplayer number. (Each direction, ABC, and XYZ group counts as one.)
	int offset;	// Offset in the I/O table. (Calculated by calc_IO_offset().)
	
	// Player A
	offset = calc_IO_offset(tp_num++);
	tbl_tp_io[port][offset] = TP_PA_DIR;
	
	offset = calc_IO_offset(tp_num++);
	tbl_tp_io[port][offset] = TP_PA_ABC;
	
	if (typeA & 0x01)
	{
		offset = calc_IO_offset(tp_num++);
		tbl_tp_io[port][offset] = TP_PA_XYZ;
	}
	
	// Player B
	offset = calc_IO_offset(tp_num++);
	tbl_tp_io[port][offset] = TP_PB_DIR;
	
	offset = calc_IO_offset(tp_num++);
	tbl_tp_io[port][offset] = TP_PB_ABC;
	
	if (typeB & 0x01)
	{
		offset = calc_IO_offset(tp_num++);
		tbl_tp_io[port][offset] = TP_PB_XYZ;
	}
	
	// Player C
	offset = calc_IO_offset(tp_num++);
	tbl_tp_io[port][offset] = TP_PC_DIR;
	
	offset = calc_IO_offset(tp_num++);
	tbl_tp_io[port][offset] = TP_PC_ABC;
	
	if (typeC & 0x01)
	{
		offset = calc_IO_offset(tp_num++);
		tbl_tp_io[port][offset] = TP_PC_XYZ;
	}
	
	// Player D
	offset = calc_IO_offset(tp_num++);
	tbl_tp_io[port][offset] = TP_PD_DIR;
	
	offset = calc_IO_offset(tp_num++);
	tbl_tp_io[port][offset] = TP_PD_ABC;
	
	if (typeD & 0x01)
	{
		offset = calc_IO_offset(tp_num++);
		tbl_tp_io[port][offset] = TP_PD_XYZ;
	}
}


/**
 * Make_IO_Table(): Make the Teamplayer I/O table.
 */
void Make_IO_Table(void)
{
	Make_IO_Table_Player(0, Controller_1_Type,
				Controller_1B_Type,
				Controller_1C_Type,
				Controller_1D_Type);
	
	Make_IO_Table_Player(1, Controller_2_Type,
				Controller_2B_Type,
				Controller_2C_Type,
				Controller_2D_Type);
}

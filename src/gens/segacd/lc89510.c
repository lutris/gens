/***************************************************************************
 * Gens: LC89510 emulator.                                                *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#include <stdio.h>
#include <string.h>

// Message logging.
#include "macros/log_msg.h"

#include "gens_core/misc/misc.h"
#include "lc89510.h"
#include "gens_core/cpu/68k/star_68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/sound/pcm.h"
#include "cd_sys.hpp"

#define CDC_DMA_SPEED 256

int CDC_Decode_Reg_Read;


/**
 * CDD_Reset(): Reset the CDD.
 */
static void CDD_Reset(void)
{
	int i;
	
	// Reseting CDD
	
	CDD.Fader = 0;
	CDD.Control = 0;
	CDD.Cur_Comm = 0;
	CDD.Status = 0;
	CDD.Minute = 0;
	CDD.Seconde = 0;
	CDD.Frame = 0;
	CDD.Ext = 0;
	
	for (i = 0; i < 10; i++)
	{
		CDD.Rcv_Status[i] = 0;
		CDD.Trans_Comm[i] = 0;
	}
	
	// Default checksum
	CDD.Rcv_Status[8] = 0xF;
	
	SCD.Cur_Track = 0;
	SCD.Cur_LBA = -150;
	SCD.Status_CDD = READY;
}


/**
 * CDC_Reset(): Reset the CDC.
 */
static void CDC_Reset(void)
{
	//int i;
	
	// Reseting CDC
	
	// NOTE: Clearing sizeof(CDC.Buffer) bytes ((32 * 1024 * 2) + 2352) breaks SegaCD!
	// This must only clear ((16 * 1024 * 2) + 2352) bytes.
	memset(CDC.Buffer, 0x00, ((16 * 1024 * 2) + 2352));
	CDC_Update_Header();
	
	CDC.COMIN = 0;
	CDC.IFSTAT = 0xFF;
	CDC.DAC.N = 0;
	CDC.DBC.N = 0;
	CDC.HEAD.N = 0x01000000;
	CDC.PT.N = 0;
	CDC.WA.N = 2352 * 2;
	CDC.STAT.N = 0x00000080;
	CDC.SBOUT = 0;
	CDC.IFCTRL = 0;
	CDC.CTRL.N = 0;
	
	CDC_Decode_Reg_Read = 0;
}


void LC89510_Reset(void)
{
	CDD_Reset();
	CDC_Reset();
	
	CDC.RS0 = 0;
	CDC.RS1 = 0;
	CDC.Host_Data = 0;
	CDC.DMA_Adr = 0;
	CDC.Stop_Watch = 0;
	
	SCD.Status_CDC = 0;
	CDD_Complete = 0;
}

void Update_CDC_TRansfert(void)
{
	unsigned int dep, length, add_dest;
	unsigned char *dest;
	
	if ((SCD.Status_CDC & 0x08) == 0)
		return;
	
	switch (CDC.RS0 & 0x0700)
	{
		case 0x0200:	// MAIN CPU
		case 0x0300:	// SUB CPU
			// Data ready in host port
			CDC.RS0 |= 0x4000;
			return;
			break;
		
		case 0x0400:		// PCM RAM
			dest = (unsigned char *) Ram_PCM;
			dep = ((CDC.DMA_Adr & 0x03FF) << 2) + PCM_Chip.Bank;
			add_dest = 2;
			break;
		
		case 0x0500:		// PRG RAM
			dest = (unsigned char *) Ram_Prg;
			dep = (CDC.DMA_Adr & 0xFFFF) << 3;
			add_dest = 2;
			
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"DMA transfert PRG RAM : adr = %.8X", dep);
			break;
		
		case 0x0700:		// WORD RAM
			if (Ram_Word_State >= 2)
			{
				dest = (unsigned char*)Ram_Word_1M;
				add_dest = 2;
				if (Ram_Word_State & 1)
					dep = ((CDC.DMA_Adr & 0x3FFF) << 3);
				else
					dep = ((CDC.DMA_Adr & 0x3FFF) << 3) + 0x20000;
			}
			else
			{
				dest = (unsigned char*)Ram_Word_2M;
				dep = ((CDC.DMA_Adr & 0x7FFF) << 3);
				add_dest = 2;
			}
			break;
		
		default:
			return;
	}
	
	if (CDC.DBC.N <= (CDC_DMA_SPEED * 2))
	{
		length = (CDC.DBC.N + 1) >> 1;
		SCD.Status_CDC &= ~0x08;	// Last transfert
		CDC.RS0 |= 0x8000;		// End data transfert
		CDC.RS0 &= ~0x4000;		// no more data ready
		CDC.IFSTAT |= 0x08;		// No more data transfert in progress
		
		// DTEIEN = Data Trasnfert End Interrupt Enable ?
		if (CDC.IFCTRL & 0x40)
		{
			CDC.IFSTAT &= ~0x40;
			
			if (Int_Mask_S68K & 0x20)
				sub68k_interrupt (5, -1);
			
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC - DTE interrupt");
		}
	}
	else
		length = CDC_DMA_SPEED;
	
	LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
		"DMA length = %.4X", length);
	
	if ((CDC.RS0 & 0x0700) == 0x0400)
	{
		// PCM DMA
		int len = length;
		unsigned char *src = (unsigned char*)&CDC.Buffer[CDC.DAC.N];
		unsigned char *dst = (unsigned char*)dest + dep;
		
		while (len--)
		{
			*(unsigned short*)dst = *(unsigned short*)src;
			src += 2;
			dst += add_dest;
		}
		length <<= 1;
		CDC.DMA_Adr += length >> 2;
	}
	else
	{
		// OTHER DMA
		int len = length;
		unsigned char *src = (unsigned char*)&CDC.Buffer[CDC.DAC.N];
		unsigned char *dst = (unsigned char*)dest + dep;
		
		while (len--)
		{
			unsigned short outrol = *(unsigned short *) src;
			outrol = (outrol << 8) | (outrol >> 8);
			
			*(unsigned short*)dst = outrol;
			src += 2;
			dst += add_dest;
		}
		length <<= 1;
		CDC.DMA_Adr += length >> 3;
	}
	
	CDC.DAC.N = (CDC.DAC.N + length) & 0xFFFF;
	if (SCD.Status_CDC & 0x08)
		CDC.DBC.N -= length;
	else
		CDC.DBC.N = 0;
}


unsigned short Read_CDC_Host_SUB(void)
{
	unsigned short val;
	
	if (SCD.Status_CDC & 0x08)	// Transfert data
	{
		if ((CDC.RS0 & 0x0700) == 0x0300)	// SUB CPU
		{
			CDC.DBC.N -= 2;
			
			if (CDC.DBC.N <= 0)
			{
				CDC.DBC.N = 0;
				SCD.Status_CDC &= ~0x08;	// Last transfert
				CDC.RS0 |= 0x8000;	// End data transfert
				CDC.RS0 &= ~0x4000;	// no more data ready
				CDC.IFSTAT |= 0x08;	// No more data transfert in progress
				
				if (CDC.IFCTRL & 0x40)	// DTEIEN = Data Trasnfert End Interrupt Enable ?
				{
					CDC.IFSTAT &= ~0x40;
					
					if (Int_Mask_S68K & 0x20)
					sub68k_interrupt (5, -1);
					
					LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
						"CDC - DTE interrupt");
				}
			}
			val = *(unsigned short *) &CDC.Buffer[CDC.DAC.N];
			CDC.DAC.N += 2;
			val = (val >> 8) | (val << 8);
			
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"Host READ on SUB CPU side : %.4X  DBA = %.4X  DBC = %.4X",
				val, CDC.DBC.N, CDC.DAC.N);
			return val;
		}
	}
	
	return 0;
}


unsigned short Read_CDC_Host_MAIN(void)
{
	unsigned short val;
	
	if (SCD.Status_CDC & 0x08)	// Transfert data
	{
		if ((CDC.RS0 & 0x0700) == 0x0200)	// MAIN CPU
		{
			CDC.DBC.N -= 2;
		
			if (CDC.DBC.N <= 0)
			{
				SCD.Status_CDC &= ~0x08;	// Last transfert
				CDC.RS0 |= 0x8000;	// End data transfert
				CDC.RS0 &= ~0x4000;	// no more data ready
				CDC.IFSTAT |= 0x08;	// No more data transfert in progress
				
				if (CDC.IFCTRL & 0x40)	// DTEIEN = Data Trasnfert End Interrupt Enable ?
				{
					CDC.IFSTAT &= ~0x40;
					
					if (Int_Mask_S68K & 0x20)
						sub68k_interrupt (5, -1);
					
					LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
						"CDC - DTE interrupt");
				}
			}
			val = *(unsigned short *) &CDC.Buffer[CDC.DAC.N];
			CDC.DAC.N += 2;
			val = (val >> 8) | (val << 8);
			
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"Host READ on SUB CPU side : %.4X  DBA = %.4X  DBC = %.4X",
				val, CDC.DBC.N, CDC.DAC.N);
			return val;
		}
	}
	
	return 0;
}


void CDC_Update_Header(void)
{
	if (CDC.CTRL.B.B1 & 0x01)	// Sub-Hearder wanted ?
	{
		CDC.HEAD.B.B0 = 0;
		CDC.HEAD.B.B1 = 0;
		CDC.HEAD.B.B2 = 0;
		CDC.HEAD.B.B3 = 0;
	}
	else
	{
		_msf MSF;
		
		LBA_to_MSF (SCD.Cur_LBA, &MSF);
		
		CDC.HEAD.B.B0 = INT_TO_BCDB (MSF.M);
		CDC.HEAD.B.B1 = INT_TO_BCDB (MSF.S);
		CDC.HEAD.B.B2 = INT_TO_BCDB (MSF.F);
		CDC.HEAD.B.B3 = 0x01;
	}
}


unsigned char CDC_Read_Reg(void)
{
	unsigned char ret;
	
	switch (CDC.RS0 & 0xF)
	{
		case 0x0:			// COMIN
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.COMIN);
			
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x1;
			return CDC.COMIN;
		
		case 0x1:			// IFSTAT
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.IFSTAT);
			
			CDC_Decode_Reg_Read |= (1 << 1);	// Reg 1 (decoding)
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x2;
			return CDC.IFSTAT;
		
		case 0x2:			// DBCL
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.DBC.B.L);
			
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x3;
			return CDC.DBC.B.L;
		
		case 0x3:			// DBCH
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.DBC.B.H);
			
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x4;
			return CDC.DBC.B.H;
		
		case 0x4:			// HEAD0
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.HEAD.B.B0);
			
			CDC_Decode_Reg_Read |= (1 << 4);	// Reg 4 (decoding)
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x5;
			return CDC.HEAD.B.B0;
		
		case 0x5:			// HEAD1
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.HEAD.B.B1);
			
			CDC_Decode_Reg_Read |= (1 << 5);	// Reg 5 (decoding)
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x6;
			return CDC.HEAD.B.B1;
		
		case 0x6:			// HEAD2
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.HEAD.B.B2);
			
			CDC_Decode_Reg_Read |= (1 << 6);	// Reg 6 (decoding)
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x7;
			return CDC.HEAD.B.B2;
		
		case 0x7:			// HEAD3
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.HEAD.B.B3);
			
			CDC_Decode_Reg_Read |= (1 << 7);	// Reg 7 (decoding)
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x8;
			return CDC.HEAD.B.B3;
		
		case 0x8:			// PTL
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.PT.B.L);
			
			CDC_Decode_Reg_Read |= (1 << 8);	// Reg 8 (decoding)
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x9;
			return CDC.PT.B.L;
		
		case 0x9:			// PTH
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.PT.B.H);
			
			CDC_Decode_Reg_Read |= (1 << 9);	// Reg 9 (decoding)
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xA;
			return CDC.PT.B.H;
		
		case 0xA:			// WAL
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.WA.B.L);
			
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xB;
			return CDC.WA.B.L;
		
		case 0xB:			// WAH
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.WA.B.H);
			
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xC;
			return CDC.WA.B.H;
		
		case 0xC:			// STAT0
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.STAT.B.B0);
			
			CDC_Decode_Reg_Read |= (1 << 12);	// Reg 12 (decoding)
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xD;
			return CDC.STAT.B.B0;
		
		case 0xD:			// STAT1
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.STAT.B.B1);
			
			CDC_Decode_Reg_Read |= (1 << 13);	// Reg 13 (decoding)
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xE;
			return CDC.STAT.B.B1;
		
		case 0xE:			// STAT2
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.STAT.B.B2);
			
			CDC_Decode_Reg_Read |= (1 << 14);	// Reg 14 (decoding)
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xF;
			return CDC.STAT.B.B2;
		
		case 0xF:			// STAT3
			LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
				"CDC read reg %.2d = %.2X",
				CDC.RS0 & 0xF, CDC.STAT.B.B3);
			
			ret = CDC.STAT.B.B3;
			CDC.IFSTAT |= 0x20;	// decoding interrupt flag cleared
			if ((CDC.CTRL.B.B0 & 0x80) && (CDC.IFCTRL & 0x20))
			{
				if ((CDC_Decode_Reg_Read & 0x73F2) == 0x73F2)
				CDC.STAT.B.B3 = 0x80;
			}
			return ret;
	}
	
	return 0;
}


void CDC_Write_Reg(unsigned char Data)
{
	LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
		"CDC write reg%d = %.2X",
		CDC.RS0 & 0xF, Data);
	
	switch (CDC.RS0 & 0xF)
	{
		case 0x0:			// SBOUT
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x1;
			CDC.SBOUT = Data;
			break;
		
		case 0x1:			// IFCTRL
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x2;
			CDC.IFCTRL = Data;
			
			if ((CDC.IFCTRL & 0x02) == 0)	// Stop data transfert
			{
				CDC.DBC.N = 0;
				SCD.Status_CDC &= ~0x08;
				CDC.IFSTAT |= 0x08;	// No more data transfert in progress
			
				/*
				if (CDD.Trans_Comm[1] != 0)
				{
					SCD.Status_CDD = READY;
					SCD.Status_CDC &= 0;
				}
				*/
			}
			break;
		
		case 0x2:			// DBCL
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x3;
			CDC.DBC.B.L = Data;
			break;
		
		case 0x3:			// DBCH
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x4;
			CDC.DBC.B.H = Data;
			break;
		
		case 0x4:			// DACL
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x5;
			CDC.DAC.B.L = Data;
			break;
		
		case 0x5:			// DACH
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x6;
			CDC.DAC.B.H = Data;
			break;
		
		case 0x6:			// DTTRG
			if (CDC.IFCTRL & 0x02)	// Data transfert enable ?
			{
				CDC.IFSTAT &= ~0x08;	// Data transfert in progress
				SCD.Status_CDC |= 0x08;	// Data transfert in progress
				CDC.RS0 &= 0x7FFF;	// A data transfert start
				
				LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
					"************** Starting Data Transfert ***********");
				LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
					"RS0 = %.4X  DAC = %.4X  DBC = %.4X  DMA adr = %.4X",
					CDC.RS0, CDC.DAC.N, CDC.DBC.N, CDC.DMA_Adr);
			}
			break;
		
		case 0x7:			// DTACK
			CDC.IFSTAT |= 0x40;	// end data transfert interrupt flag cleared
			break;
		
		case 0x8:			// WAL
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0x9;
			CDC.WA.B.L = Data;
			break;
		
		case 0x9:			// WAH
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xA;
			CDC.WA.B.H = Data;
			break;
		
		case 0xA:			// CTRL0
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xB;
			CDC.CTRL.B.B0 = Data;
			break;
		
		case 0xB:			// CTRL1
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xC;
			CDC.CTRL.B.B1 = Data;
			break;
		
		case 0xC:			// PTL
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xD;
			CDC.PT.B.L = Data;
			break;
		
		case 0xD:			// PTH
			CDC.RS0 = (CDC.RS0 & 0xFFF0) | 0xE;
			CDC.PT.B.H = Data;
			break;
		
		case 0xE:			// CTRL2
			CDC.CTRL.B.B2 = Data;
			break;
		
		case 0xF:			// RESET
			CDC_Reset();
			break;
	}
}


void CDD_Processing(void)
{
	LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
		"CDD exporting status");
	LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
		"Status=%.4X, Minute=%.4X, Seconde=%.4X, Frame=%.4X, Ext=%.4X",
		CDD.Status, CDD.Minute, CDD.Seconde, CDD.Frame, CDD.Ext);
	
	CDD_Export_Status();
	sub68k_interrupt(4, -1);
}


void CDD_Import_Command(void)
{
	LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
		"CDD importing command");
	LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
		"Commande=%.4X, Minute=%.4X, Seconde=%.4X, Frame=%.4X  Checksum=%.4X",
		(CDD.Trans_Comm[0] & 0xFF) + ((CDD.Trans_Comm[1] & 0xFF) << 8),
		(CDD.Trans_Comm[2] & 0xFF) + ((CDD.Trans_Comm[3] & 0xFF) << 8),
		(CDD.Trans_Comm[4] & 0xFF) + ((CDD.Trans_Comm[5] & 0xFF) << 8),
		(CDD.Trans_Comm[6] & 0xFF) + ((CDD.Trans_Comm[7] & 0xFF) << 8),
		(CDD.Trans_Comm[8] & 0xFF) + ((CDD.Trans_Comm[9] & 0xFF) << 8));
	
	switch (CDD.Trans_Comm[1])
	{
		case 0x0:			// STATUS (?)
			Get_Status_CDD_c0();
			break;
		
		case 0x1:			// STOP ALL (?)
			Stop_CDD_c1();
			break;
		
		case 0x2:			// GET TOC INFORMATIONS
			switch (CDD.Trans_Comm[2])
			{
				case 0x0:		// get current position (MSF format)
					CDD.Status = (CDD.Status & 0xFF00);
					Get_Pos_CDD_c20();
					break;
				
				case 0x1:		// get elapsed time of current track played/scanned (relative MSF format)
					CDD.Status = (CDD.Status & 0xFF00) | 1;
					Get_Track_Pos_CDD_c21();
					break;
				
				case 0x2:		// get current track in RS2-RS3
					CDD.Status = (CDD.Status & 0xFF00) | 2;
					Get_Current_Track_CDD_c22 ();
					break;
				
				case 0x3:		// get total length (MSF format)
					CDD.Status = (CDD.Status & 0xFF00) | 3;
					Get_Total_Length_CDD_c23();
					break;
				
				case 0x4:		// first & last track number
					CDD.Status = (CDD.Status & 0xFF00) | 4;
					Get_First_Last_Track_CDD_c24();
					break;
				
				case 0x5:		// get track addresse (MSF format)
					CDD.Status = (CDD.Status & 0xFF00) | 5;
					Get_Track_Adr_CDD_c25();
					break;
				
				default:		// invalid, then we return status
					CDD.Status = (CDD.Status & 0xFF00) | 0xF;
					Get_Status_CDD_c0();
					break;
			}
			break;
		
		case 0x3:			// READ
			Play_CDD_c3();
			break;
		
		case 0x4:			// SEEK
			Seek_CDD_c4();
			break;
		
		case 0x6:			// PAUSE/STOP
			Pause_CDD_c6();
			break;
		
		case 0x7:			// RESUME
			Resume_CDD_c7();
			break;
		
		case 0x8:			// FAST FOWARD
			Fast_Foward_CDD_c8();
			break;
		
		case 0x9:			// FAST REWIND
			Fast_Rewind_CDD_c9();
			break;
		
		case 0xA:			// RECOVER INITIAL STATE (?)
			CDD_cA();
			break;
		
		case 0xC:			// CLOSE TRAY
			Close_Tray_CDD_cC();
			break;
		
		case 0xD:			// OPEN TRAY
			Open_Tray_CDD_cD();
			break;
		
		default:
			// UNKNOW
			CDD_Def();
			break;
	}
}


unsigned char SCD_Read_Byte(unsigned int Adr)
{
	LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
		"SCD read (B), address = %.8X", Adr);
	return 0;
}


unsigned short SCD_Read_Word(unsigned int Adr)
{
	LOG_MSG(lc89510, LOG_MSG_LEVEL_DEBUG1,
		"SCD read (W), address = %.8X", Adr);
	return 0;
}

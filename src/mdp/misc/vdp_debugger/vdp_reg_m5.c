/***************************************************************************
 * MDP: VDP Debugger. (Mode 5 Register Descriptions)                       *
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

#include "vdp_reg_m5.h"
#include "vdpdbg.h"

// libgsft includes.
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"


/**
 * vdp_m5_reg_name[]: Mode 5 register names.
 */
const char *vdp_m5_reg_name[24+1] =
{
	"Mode Set 1",			// 0
	"Mode Set 2",			// 1
	"Scroll A Pattern Addr",	// 2
	"Window Pattern Addr",		// 3
	"Scroll B Pattern Addr",	// 4
	"Sprite Attr Table Addr",	// 5
	"unused",			// 6
	"Background Color",		// 7
	"unused",			// 8
	"unused",			// 9
	"H Interrupt",			// 10
	"Mode Set 3",			// 11
	"Mode Set 4",			// 12
	"H Scroll Addr",		// 13
	"unused",			// 14
	"Auto Increment",		// 15
	"Scroll Size",			// 16
	"Window H Pos",			// 17
	"Window V Pos",			// 18
	"DMA Length Low",		// 19
	"DMA Length High",		// 20
	"DMA Src Low",			// 21
	"DMA Src Mid",			// 22
	"DMA Src High",			// 23
	NULL
};


/**
 * vdpdbg_get_m5_reg_desc(): Get a Mode 5 register description.
 * @param reg_num	[in] Register number.
 * @param reg_value	[in] Register value.
 * @param reg_vdp	[in, opt] All register values. If not specified, the function will call vdp_host_srv->reg_get().
 * @param buf		[out] Buffer for the description.
 * @param len		[in] Size of the buffer.
 */
void MDP_FNCALL vdpdbg_get_m5_reg_desc(int reg_num, uint8_t reg_value, mdp_reg_vdp_t *reg_vdp, char *buf, size_t len)
{
	// Create the description.
	switch (reg_num)
	{
		case 0:
			// Mode Set 1.
			szprintf(buf, len, "H Int %s; HV counter %s; %s %s",
					(reg_value & 0x10 ? "ON" : "OFF"),
					(reg_value & 0x02 ? "ON" : "OFF"),
					((reg_vdp->regs.mode_set2 & 0x04) ? "PSEL" : "M4"),
					(reg_value & 0x04 ? "ON" : "OFF"));
			break;
		
		case 1:
			// Mode Set 2.
			szprintf(buf, len, "Disp %s; V Int %s; DMA %s; V%d",
					(reg_value & 0x40 ? "ON" : "OFF"),
					(reg_value & 0x20 ? "ON" : "OFF"),
					(reg_value & 0x10 ? "ON" : "OFF"),
					(reg_value & 0x08 ? 30 : 28));
			break;
		
		case 2:
			// Scroll A Pattern Address.
			szprintf(buf, len, "0x%01X000",
					(reg_value & 0x38) >> 2);
			break;
		
		case 3:
			// Window Pattern Address.
			// NOTE: In H40 mode, bit 1 should be 0.
			// Should this be enforced?
			szprintf(buf, len, "0x%03X0",
					(reg_value & 0x3E) << 6);
			break;
		
		case 4:
			// Scroll B Pattern Address.
			szprintf(buf, len, "0x%01X000",
					(reg_value & 0x07) << 1);
			break;
		
		case 5:
			// Sprite Attribute Table Address.
			// NOTE: In H40 mode, bit 0 should be 0.
			// Should this be enforced?
			szprintf(buf, len, "0x%02X00",
					(reg_value & 0x7F) << 1);
			break;
		
		case 7:
			// Background Color.
			szprintf(buf, len, "Palette %d, Color %d",
					((reg_value >> 4) & 0x03),
					(reg_value & 0xF));
			break;
		
		case 10:
		{
			// H Interrupt.
			// If this value is >= vertical resolution, then it has no effect.
			uint32_t reg_mode_set2;
			if (reg_vdp)
				reg_mode_set2 = reg_vdp->regs.mode_set2;
			else
				vdpdbg_host_srv->reg_get(MDP_REG_IC_VDP, MDP_REG_VDP_MODE_SET2, &reg_mode_set2);
			
			int Vres = ((reg_mode_set2 & 0x08) ? (240+1) : (224+1));
			if (reg_value >= Vres)
			{
				// No effect.
				szprintf(buf, len, "%d lines (disabled)", reg_value + 1);
			}
			else
			{
				// Has effect.
				szprintf(buf, len, "%d line%s",
						reg_value + 1,
						(reg_value == 0 ? "" : "s"));
			}
			break;
		}
		
		case 11:
		{
			// Mode Set 3.
			static const char HScroll_Desc[][8] = {"Full", "Invalid", "1Cell", "1Line"};
			szprintf(buf, len, "Ext Int %s; VScroll %s; HScroll %s",
					(reg_value & 0x04 ? "ON" : "OFF"),
					(reg_value & 0x04 ? "2Cell" : "Full"),
					HScroll_Desc[reg_value & 0x03]);
			break;
		}
		
		case 12:
		{
			// Mode Set 4.
			// NOTE: Bits 7 and 0 both control H32/H40 mode.
			// We're checking if either bit is set.
			// What's the correct method?
			static const char *Interlace_Desc[] = {"OFF", "ON (1x res)", "Invalid", "ON (2x res)"};
			szprintf(buf, len, "H%d; S/H %s; Interlace: %s",
					(reg_value & 0x81 ? 40 : 32),
					(reg_value & 0x08 ? "ON" : "OFF"),
					Interlace_Desc[(reg_value & 0x06) >> 1]);
			break;
		}
		
		case 13:
			// H Scroll Address.
			szprintf(buf, len, "0x%02X00",
					(reg_value & 0x3F) << 2);
			break;
		
		case 15:
			// Auto Increment.
			if (reg_value == 0)
				strlcpy(buf, "Disabled", len);
			else
				szprintf(buf, len, "+%d bytes", reg_value);
			break;
		
		case 16:
		{
			// Scroll Size.
			static const char *ScrlSize_Desc[] = {"32 cells", "64 cells", "Invalid", "128 cells"};
			szprintf(buf, len, "H: %s; V: %s",
					ScrlSize_Desc[reg_value & 0x03],
					ScrlSize_Desc[(reg_value >> 4) & 0x03]);
			break;
		}
		
		case 17:
		{
			// Window H Pos.
			uint32_t reg_mode_set4;
			if (reg_vdp)
				reg_mode_set4 = reg_vdp->regs.mode_set4;
			else
				vdpdbg_host_srv->reg_get(MDP_REG_IC_VDP, MDP_REG_VDP_MODE_SET4, &reg_mode_set4);
			
			const int cell_max = ((reg_mode_set4 & 0x81) ? 40-1 : 32-1);
			int cell_num = (reg_value & 0x1F) * 2;
			
			if (reg_value & 0x80)
			{
				// Right-aligned window.
				if (cell_num > cell_max)
					strlcpy(buf, "Right-aligned: Offscreen", len);
				else
					szprintf(buf, len, "Right-aligned: Cells %d - %d",
							cell_num, cell_max);
			}
			else
			{
				// Left-aligned window.
				if (cell_num == 0)
					strlcpy(buf, "Left-aligned: Offscreen", len);
				else
					szprintf(buf, len, "Left-aligned: Cells 0 - %d", cell_num);
			}
			break;
		}
		
		case 18:
		{
			// Window V Pos.
			uint32_t reg_mode_set2;
			if (reg_vdp)
				reg_mode_set2 = reg_vdp->regs.mode_set2;
			else
				vdpdbg_host_srv->reg_get(MDP_REG_IC_VDP, MDP_REG_VDP_MODE_SET2, &reg_mode_set2);
			
			const int cell_max = ((reg_mode_set2 & 0x08) ? 30-1 : 28-1);
			int cell_num = (reg_value & 0x1F);
			
			if (reg_value & 0x80)
			{
				// Bottom-aligned window.
				if (cell_num > cell_max)
					strlcpy(buf, "Bottom-aligned: Offscreen", len);
				else
					szprintf(buf, len, "Bottom-aligned: Cells %d - %d",
							cell_num, cell_max);
			}
			else
			{
				// Top-aligned window.
				if (cell_num == 0)
					strlcpy(buf, "Top-aligned: Offscreen", len);
				else
					szprintf(buf, len, "Top-aligned: Cells 0 - %d", cell_num);
			}
			break;
		}
		
		case 19:
		case 20:
			// DMA Length.
		case 21:
		case 22:
		case 23:
			// DMA Src Address.
			// These are updated manually.
			buf[0] = 0x00;
			break;
		
		default:
			// Unused register.
			strlcpy(buf, "unused", len);
			break;
	}
}


/**
 * vdpdbg_get_m5_reg_desc(): Get the Mode 5 DMA length description.
 * @param reg_vdp	[in, opt] All register values. If not specified, the function will call vdp_host_srv->reg_get().
 * @param buf		[out] Buffer for the description.
 * @param len		[in] Size of the buffer.
 */
void MDP_FNCALL vdpdbg_get_m5_dma_len_desc(mdp_reg_vdp_t *reg_vdp, char *buf, size_t len)
{
	uint32_t dma_len;
	if (reg_vdp)
	{
		dma_len = (reg_vdp->regs.dma_len_l |
			   (reg_vdp->regs.dma_len_h << 8));
	}
	else
	{
		uint32_t reg_dma_len_l, reg_dma_len_h;
		vdpdbg_host_srv->reg_get(MDP_REG_IC_VDP, MDP_REG_VDP_DMA_LEN_L, &reg_dma_len_l);
		vdpdbg_host_srv->reg_get(MDP_REG_IC_VDP, MDP_REG_VDP_DMA_LEN_H, &reg_dma_len_h);
		
		dma_len = ((reg_dma_len_l & 0xFF) |
			   ((reg_dma_len_h & 0xFF) << 8));
	}
	
	szprintf(buf, len, "Length: 0x%04X words", dma_len);
}


void MDP_FNCALL vdpdbg_get_m5_dma_src_desc(mdp_reg_vdp_t *reg_vdp, char *buf, size_t len)
{
	uint32_t dma_src;
	if (reg_vdp)
	{
		dma_src = (reg_vdp->regs.dma_src_l |
			   (reg_vdp->regs.dma_src_m << 8) |
			   (reg_vdp->regs.dma_src_h << 16));
	}
	else
	{
		uint32_t reg_dma_src_l, reg_dma_src_m, reg_dma_src_h;
		vdpdbg_host_srv->reg_get(MDP_REG_IC_VDP, MDP_REG_VDP_DMA_SRC_L, &reg_dma_src_l);
		vdpdbg_host_srv->reg_get(MDP_REG_IC_VDP, MDP_REG_VDP_DMA_SRC_M, &reg_dma_src_m);
		vdpdbg_host_srv->reg_get(MDP_REG_IC_VDP, MDP_REG_VDP_DMA_SRC_H, &reg_dma_src_h);
		
		dma_src = ((reg_dma_src_l & 0xFF) |
			   ((reg_dma_src_m & 0xFF) << 8) |
			   ((reg_dma_src_h & 0xFF) << 16));
	}
	
	if (!(dma_src & 0x800000))
	{
		// Memory to VRAM copy.
		szprintf(buf, len, "Mem to VRAM: 0x%06X",
			 ((dma_src & 0x7FFFFF) << 1));
	}
	else
	{
		szprintf(buf, len, "VRAM %s: 0x%04X",
			 ((dma_src & 0x400000) ? "Copy" : "Fill"),
			 (dma_src & 0x3FFFFF));
	}
}

/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services. (Memory Functions)        *
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

#include "mdp_host_gens_mem.h"
#include "mdp/mdp_error.h"

/* Byteswapping macros. */
#include "gens_core/misc/byteswap.h"

#if GENS_BYTEORDER == GENS_LIL_ENDIAN

/* Little-endian memory macros. */
#define MEM_RW_8_BE(ptr, address)	(((uint8_t*)(ptr))[(address) ^ 1])
#define MEM_RW_8_LE(ptr, address)	(((uint8_t*)(ptr))[(address)])

#else

/* Big-endian memory macros. */
#define MEM_RW_8_BE(ptr, address)	(((uint8_t*)(ptr))[(address)])
#define MEM_RW_8_LE(ptr, address)	(((uint8_t*)(ptr))[(address) ^ 1])

#endif

/* Endian-neutral memory macros. */
#define MEM_RW_16(ptr, address)		(((uint16_t*)(ptr))[(address) >> 1])

/* TODO: Optimize 32-bit reads/writes for their respective architectures. */

#define MEM_READ_32_BE(ptr, address)	\
	(((((uint16_t*)(ptr))[(address) >> 1]) << 16) | (((uint16_t*)(ptr))[((address) >> 1) + 1]))

#define MEM_READ_32_LE(ptr, address)	\
	(((((uint16_t*)(ptr))[((address) >> 1) + 1]) << 16) | (((uint16_t*)(ptr))[(address) >> 1]))

#define MEM_WRITE_32_BE(ptr, address, data)					\
	((uint16_t*)(ptr))[(address) >> 1]       = (((data) >> 16) & 0xFFFF);	\
	((uint16_t*)(ptr))[((address) >> 1) + 1] = ((data) & 0xFFFF);

#define MEM_WRITE_32_LE(ptr, address, data)					\
	((uint16_t*)(ptr))[((address) >> 1) + 1] = (((data) >> 16) & 0xFFFF);	\
	((uint16_t*)(ptr))[(address) >> 1]       = ((data) & 0xFFFF);

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_z80.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"


/** Memory Read Functions **/

uint8_t MDP_FNCALL mdp_host_mem_read_8(int memID, uint32_t address)
{
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFF;
			return MEM_RW_8_BE(Rom_Data, address);
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFF;
			return MEM_RW_8_BE(Ram_68k, address);
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFF;
			return MEM_RW_8_BE(VRam, address);
		default:
			/* Unknown memory ID. */
			return -1;
	}
}

uint16_t MDP_FNCALL mdp_host_mem_read_16(int memID, uint32_t address)
{
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFE;
			return MEM_RW_16(Rom_Data, address);
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFE;
			return MEM_RW_16(Ram_68k, address);
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFE;
			return MEM_RW_16(VRam, address);
		default:
			/* Unknown memory ID. */
			return -1;
	}
}

uint32_t MDP_FNCALL mdp_host_mem_read_32(int memID, uint32_t address)
{
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFE;
			return MEM_READ_32_BE(Rom_Data, address);
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFE;
			return MEM_READ_32_BE(Ram_68k, address);
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFE;
			return MEM_READ_32_BE(VRam, address);
		default:
			/* Unknown memory ID. */
			return -1;
	}
}


/** Memory Write Functions **/

int MDP_FNCALL mdp_host_mem_write_8(int memID, uint32_t address, uint8_t data)
{
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFF;
			MEM_RW_8_BE(Rom_Data, address) = data;
			MEM_RW_8_LE(_32X_Rom, address) = data;
			break;
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFF;
			MEM_RW_8_BE(Ram_68k, address) = data;
			break;
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFF;
			MEM_RW_8_BE(VRam, address) = data;
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	return MDP_ERR_OK;
}

int MDP_FNCALL mdp_host_mem_write_16(int memID, uint32_t address, uint16_t data)
{
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFF;
			MEM_RW_16(Rom_Data, address) = data;
			MEM_RW_16(_32X_Rom, address) = data;
			break;
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFF;
			MEM_RW_16(Ram_68k, address) = data;
			break;
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFF;
			MEM_RW_16(VRam, address) = data;
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	return MDP_ERR_OK;
}

int MDP_FNCALL mdp_host_mem_write_32(int memID, uint32_t address, uint32_t data)
{
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFF;
			MEM_WRITE_32_BE(Rom_Data, address, data)
			MEM_WRITE_32_LE(_32X_Rom, address, data);
			break;
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFF;
			MEM_WRITE_32_BE(Ram_68k, address, data)
			break;
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFF;
			MEM_WRITE_32_BE(VRam, address, data)
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	return MDP_ERR_OK;
}


/** Memory Block Read Functions **/

int MDP_FNCALL mdp_host_mem_read_block_8(int memID, uint32_t address, uint8_t *data, uint32_t length)
{
	MDP_UNUSED_PARAMETER(memID);
	MDP_UNUSED_PARAMETER(address);
	MDP_UNUSED_PARAMETER(data);
	MDP_UNUSED_PARAMETER(length);
	
	/* TODO */
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}

int MDP_FNCALL mdp_host_mem_read_block_16(int memID, uint32_t address, uint16_t *data, uint32_t length)
{
	MDP_UNUSED_PARAMETER(memID);
	MDP_UNUSED_PARAMETER(address);
	MDP_UNUSED_PARAMETER(data);
	MDP_UNUSED_PARAMETER(length);
	
	/* TODO */
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}

int MDP_FNCALL mdp_host_mem_read_block_32(int memID, uint32_t address, uint32_t *data, uint32_t length)
{
	MDP_UNUSED_PARAMETER(memID);
	MDP_UNUSED_PARAMETER(address);
	MDP_UNUSED_PARAMETER(data);
	MDP_UNUSED_PARAMETER(length);
	
	/* TODO */
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}


/** Memory Block Write Functions **/

int MDP_FNCALL mdp_host_mem_write_block_8 (int memID, uint32_t address, uint8_t  *data, uint32_t length)
{
	/* TODO: Test this function. */
	
	uint8_t *ptr;
	uint32_t max_address;
	int big_endian;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			ptr = Rom_Data;
			max_address = 0x3FFFFF;
			big_endian = 1;
			break;
		case MDP_MEM_MD_RAM:
			ptr = Ram_68k;
			max_address = 0xFFFF;
			big_endian = 1;
			break;
		case MDP_MEM_MD_VRAM:
			ptr = VRam;
			max_address = 0xFFFF;
			big_endian = 1;
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	if (address + length > max_address)
	{
		/* Out of range. */
		return -MDP_ERR_MEM_OUT_OF_RANGE;
	}
	
	ptr = &ptr[address];
	
	/* Store data in 16-bit chunks. */
	if (big_endian)
	{
		for (; length > 1; length -= 2)
		{
			MEM_RW_8_BE(ptr, 0) = *data;
			MEM_RW_8_BE(ptr, 1) = *(data + 1);
			
			ptr += 2;
			data += 2;
		}
		
		if (length == 1)
		{
			// One byte left.
			MEM_RW_8_BE(ptr, 0) = *data;
		}
	}
	else
	{
		for (; length > 1; length -= 2)
		{
			MEM_RW_8_LE(ptr, 0) = *data;
			MEM_RW_8_LE(ptr, 1) = *(data + 1);
			
			ptr += 2;
			data += 2;
		}
		
		if (length == 1)
		{
			// One byte left.
			MEM_RW_8_LE(ptr, 0) = *data;
		}
	}
	
	/* Block written. */
	return -MDP_ERR_OK;
}

int MDP_FNCALL mdp_host_mem_write_block_16(int memID, uint32_t address, uint16_t *data, uint32_t length)
{
	MDP_UNUSED_PARAMETER(memID);
	MDP_UNUSED_PARAMETER(address);
	MDP_UNUSED_PARAMETER(data);
	MDP_UNUSED_PARAMETER(length);
	
	/* TODO */
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}

int MDP_FNCALL mdp_host_mem_write_block_32(int memID, uint32_t address, uint32_t *data, uint32_t length)
{
	MDP_UNUSED_PARAMETER(memID);
	MDP_UNUSED_PARAMETER(address);
	MDP_UNUSED_PARAMETER(data);
	MDP_UNUSED_PARAMETER(length);
	
	/* TODO */
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}

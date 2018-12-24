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
#include "mdp/mdp_mem.h"

/* C includes. */
#include <string.h>

/* Byteswapping macros. */
#include "libgsft/gsft_byteswap.h"

#if GSFT_BYTEORDER == GSFT_LIL_ENDIAN

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
do {										\
	((uint16_t*)(ptr))[(address) >> 1]       = (((data) >> 16) & 0xFFFF);	\
	((uint16_t*)(ptr))[((address) >> 1) + 1] = ((data) & 0xFFFF);		\
} while (0)

#define MEM_WRITE_32_LE(ptr, address, data)					\
do {										\
	((uint16_t*)(ptr))[((address) >> 1) + 1] = (((data) >> 16) & 0xFFFF);	\
	((uint16_t*)(ptr))[(address) >> 1]       = ((data) & 0xFFFF);		\
} while (0)

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_z80.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "util/file/rom.hpp"

// Unused Parameter macro.
#include "libgsft/gsft_unused.h"

extern int ice;


/** Memory Read Functions **/

int MDP_FNCALL mdp_host_mem_read_8(int memID, uint32_t address, uint8_t *ret_value)
{
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (!ret_value)
		return -MDP_ERR_INVALID_PARAMETERS;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFF;
			*ret_value = MEM_RW_8_BE(Rom_Data, address);
			break;
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFF;
			*ret_value = MEM_RW_8_BE(Ram_68k, address);
			break;
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFF;
			*ret_value = MEM_RW_8_BE(VRam, address);
			break;
		case MDP_MEM_MD_CRAM:
			address &= 0x7F;
			*ret_value = MEM_RW_8_BE(CRam, address);
			break;
		default:
			/* Unknown memory ID. */
			return -1;
	}
	
	return MDP_ERR_OK;
}

int MDP_FNCALL mdp_host_mem_read_16(int memID, uint32_t address, uint16_t *ret_value)
{
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (!ret_value)
		return -MDP_ERR_INVALID_PARAMETERS;
	if (address & 1)
		return -MDP_ERR_MEM_UNALIGNED;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFE;
			*ret_value = MEM_RW_16(Rom_Data, address);
			break;
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFE;
			*ret_value = MEM_RW_16(Ram_68k, address);
			break;
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFE;
			*ret_value = MEM_RW_16(VRam, address);
			break;
		case MDP_MEM_MD_CRAM:
			address &= 0x7E;
			*ret_value = MEM_RW_16(CRam, address);
			break;
		default:
			/* Unknown memory ID. */
			return -1;
	}
	
	return MDP_ERR_OK;
}

int MDP_FNCALL mdp_host_mem_read_32(int memID, uint32_t address, uint32_t *ret_value)
{
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (!ret_value)
		return -MDP_ERR_INVALID_PARAMETERS;
	if (address & 1)
		return -MDP_ERR_MEM_UNALIGNED;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFE;
			*ret_value = MEM_READ_32_BE(Rom_Data, address);
			break;
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFE;
			*ret_value = MEM_READ_32_BE(Ram_68k, address);
			break;
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFE;
			*ret_value = MEM_READ_32_BE(VRam, address);
			break;
		case MDP_MEM_MD_CRAM:
			address &= 0x7E;
			*ret_value = MEM_READ_32_BE(CRam, address);
			break;
		default:
			/* Unknown memory ID. */
			return -1;
	}
	
	return MDP_ERR_OK;
}


/** Memory Write Functions **/

int MDP_FNCALL mdp_host_mem_write_8(mdp_t *plugin, int memID, uint32_t address, uint8_t data)
{
	GSFT_UNUSED_PARAMETER(plugin);
	
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	ice = 0;
	
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
			VRam_Flag = 1;
			break;
		case MDP_MEM_MD_CRAM:
			address &= 0x7F;
			MEM_RW_8_BE(CRam, address) = data;
			CRam_Flag = 1;
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	return MDP_ERR_OK;
}

int MDP_FNCALL mdp_host_mem_write_16(mdp_t *plugin, int memID, uint32_t address, uint16_t data)
{
	GSFT_UNUSED_PARAMETER(plugin);
	
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (address & 1)
		return -MDP_ERR_MEM_UNALIGNED;
	ice = 0;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFE;
			MEM_RW_16(Rom_Data, address) = data;
			MEM_RW_16(_32X_Rom, address) = data;
			break;
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFE;
			MEM_RW_16(Ram_68k, address) = data;
			break;
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFE;
			MEM_RW_16(VRam, address) = data;
			VRam_Flag = 1;
			break;
		case MDP_MEM_MD_CRAM:
			address &= 0x7E;
			MEM_RW_16(CRam, address) = data;
			CRam_Flag = 1;
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	return MDP_ERR_OK;
}

int MDP_FNCALL mdp_host_mem_write_32(mdp_t *plugin, int memID, uint32_t address, uint32_t data)
{
	GSFT_UNUSED_PARAMETER(plugin);
	
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (address & 1)
		return -MDP_ERR_MEM_UNALIGNED;
	ice = 0;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			address &= 0x003FFFFE;
			MEM_WRITE_32_BE(Rom_Data, address, data);
			MEM_WRITE_32_LE(_32X_Rom, address, data);
			break;
		case MDP_MEM_MD_RAM:
			address &= 0x0000FFFE;
			MEM_WRITE_32_BE(Ram_68k, address, data);
			break;
		case MDP_MEM_MD_VRAM:
			address &= 0x0000FFFE;
			MEM_WRITE_32_BE(VRam, address, data);
			VRam_Flag = 1;
			break;
		case MDP_MEM_MD_CRAM:
			address &= 0x7E;
			MEM_WRITE_32_BE(CRam, address, data);
			CRam_Flag = 1;
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	return MDP_ERR_OK;
}


/** Memory Block Read Functions **/

static inline void mdp_host_mem_read_block_8_be(uint8_t *ptr, uint32_t address, uint8_t *data, uint32_t length)
{
	#if GSFT_BYTEORDER == GSFT_LIL_ENDIAN
		if (address & 1)
		{
			/* Starts on an odd byte. */
			*data++ = ptr[(address++) ^ 1];
			length--;
		}
		
		ptr = &ptr[address];
		for (; length > 1; length -= 2)
		{
			*data       = MEM_RW_8_BE(ptr, 0);
			*(data + 1) = MEM_RW_8_BE(ptr, 1);
			
			ptr += 2;
			data += 2;
		}
	#else
		ptr = &ptr[address];
		memcpy(data, ptr, (length & ~1));
		ptr += (length & ~1);
	#endif
	
	if (length & 1)
	{
		// One byte left.
		*data = MEM_RW_8_BE(ptr, 0);
	}
}

static inline void mdp_host_mem_read_block_8_le(uint8_t *ptr, uint32_t address, uint8_t *data, uint32_t length)
{
	#if GSFT_BYTEORDER == GSFT_BIG_ENDIAN
		if (address & 1)
		{
			/* Starts on an odd byte. */
			*data++ = ptr[(address++) ^ 1];
			length--;
		}
		
		ptr = &ptr[address];
		for (; length > 1; length -= 2)
		{
			*data       = MEM_RW_8_LE(ptr, 0);
			*(data + 1) = MEM_RW_8_LE(ptr, 1);
			
			ptr += 2;
			data += 2;
		}
	#else
		ptr = &ptr[address];
		memcpy(data, ptr, (length & ~1));
		ptr += (length & ~1);
	#endif
	
	if (length & 1)
	{
		// One byte left.
		*data = MEM_RW_8_LE(ptr, 0);
	}
}

int MDP_FNCALL mdp_host_mem_read_block_8(int memID, uint32_t address, uint8_t *data, uint32_t length)
{
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	
	uint8_t *ptr;
	uint32_t mem_size;
	int big_endian;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			ptr = Rom_Data;
			mem_size = 0x400000;
			big_endian = 1;
			break;
		case MDP_MEM_MD_RAM:
			ptr = Ram_68k;
			mem_size = 0x10000;
			big_endian = 1;
			break;
		case MDP_MEM_MD_VRAM:
			ptr = VRam;
			mem_size = 0x10000;
			big_endian = 1;
			break;
		case MDP_MEM_MD_CRAM:
			ptr = (uint8_t*)CRam;
			mem_size = 0x80;
			big_endian = 1;
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	if (address + length > mem_size)
	{
		/* Out of range. */
		return -MDP_ERR_MEM_OUT_OF_RANGE;
	}
	
	if (big_endian)
		mdp_host_mem_read_block_8_be(ptr, address, data, length);
	else
		mdp_host_mem_read_block_8_le(ptr, address, data, length);
	
	/* The block has been read. */
	return MDP_ERR_OK;
}

/* TODO: Test this function. */
int MDP_FNCALL mdp_host_mem_read_block_16(int memID, uint32_t address, uint16_t *data, uint32_t length)
{
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (address & 1)
		return -MDP_ERR_MEM_UNALIGNED;
	
	uint16_t *ptr;
	uint32_t mem_size;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			ptr = (uint16_t*)Rom_Data;
			mem_size = 0x400000;
			break;
		case MDP_MEM_MD_RAM:
			ptr = (uint16_t*)Ram_68k;
			mem_size = 0x10000;
			break;
		case MDP_MEM_MD_VRAM:
			ptr = (uint16_t*)VRam;
			mem_size = 0x10000;
			break;
		case MDP_MEM_MD_CRAM:
			ptr = (uint16_t*)CRam;
			mem_size = 0x80;
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	/* length is the number of 16-bit words. Convert to number of 8-bit bytes. */
	length <<= 1;
	
	if (address + length > mem_size)
	{
		/* Out of range. */
		return -MDP_ERR_MEM_OUT_OF_RANGE;
	}
	
	/* Copy the block. */
	memcpy(data, ptr, length);
	
	/* The block has been read. */
	return MDP_ERR_OK;
}

int MDP_FNCALL mdp_host_mem_read_block_32(int memID, uint32_t address, uint32_t *data, uint32_t length)
{
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (address & 1)
		return -MDP_ERR_MEM_UNALIGNED;
	
	MDP_UNUSED_PARAMETER(memID);
	MDP_UNUSED_PARAMETER(address);
	MDP_UNUSED_PARAMETER(data);
	MDP_UNUSED_PARAMETER(length);
	
	/* TODO */
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}


/** Memory Block Write Functions **/

static inline void mdp_host_mem_write_block_8_be(uint8_t *ptr, uint32_t address, uint8_t *data, uint32_t length)
{
	#if GSFT_BYTEORDER == GSFT_LIL_ENDIAN
		if (address & 1)
		{
			/* Starts on an odd byte. */
			ptr[(address++) ^ 1] = *data++;
			length--;
		}
		
		ptr = &ptr[address];
		for (; length > 1; length -= 2)
		{
			MEM_RW_8_BE(ptr, 0) = *data;
			MEM_RW_8_BE(ptr, 1) = *(data + 1);
			
			ptr += 2;
			data += 2;
		}
	#else
		ptr = &ptr[address];
		memcpy(ptr, data, (length & ~1));
		ptr += (length & ~1);
	#endif
	
	if (length & 1)
	{
		// One byte left.
		MEM_RW_8_BE(ptr, 0) = *data;
	}
}

static inline void mdp_host_mem_write_block_8_le(uint8_t *ptr, uint32_t address, uint8_t *data, uint32_t length)
{
	#if GSFT_BYTEORDER == GSFT_BIG_ENDIAN
		if (address & 1)
		{
			/* Starts on an odd byte. */
			ptr[(address++) ^ 1] = *data++;
			length--;
		}
		
		ptr = &ptr[address];
		for (; length > 1; length -= 2)
		{
			MEM_RW_8_LE(ptr, 0) = *data;
			MEM_RW_8_LE(ptr, 1) = *(data + 1);
			
			ptr += 2;
			data += 2;
		}
	#else
		ptr = &ptr[address];
		memcpy(ptr, data, (length & ~1));
		ptr += (length & ~1);
	#endif
	
	if (length & 1)
	{
		// One byte left.
		MEM_RW_8_LE(ptr, 0) = *data;
	}
}

int MDP_FNCALL mdp_host_mem_write_block_8(mdp_t *plugin, int memID, uint32_t address, uint8_t *data, uint32_t length)
{
	GSFT_UNUSED_PARAMETER(plugin);
	
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	ice = 0;
	
	uint8_t *ptr;
	uint32_t mem_size;
	int big_endian;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			ptr = Rom_Data;
			mem_size = 0x400000;
			big_endian = 1;
			break;
		case MDP_MEM_MD_RAM:
			ptr = Ram_68k;
			mem_size = 0x10000;
			big_endian = 1;
			break;
		case MDP_MEM_MD_VRAM:
			ptr = VRam;
			mem_size = 0x10000;
			big_endian = 1;
			break;
		case MDP_MEM_MD_CRAM:
			ptr = (uint8_t*)CRam;
			mem_size = 0x80;
			big_endian = 1;
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	if (address + length > mem_size)
	{
		/* Out of range. */
		return -MDP_ERR_MEM_OUT_OF_RANGE;
	}
	
	if (big_endian)
		mdp_host_mem_write_block_8_be(ptr, address, data, length);
	else
		mdp_host_mem_write_block_8_le(ptr, address, data, length);
	
	if (memID == MDP_MEM_MD_ROM && _32X_Started)
	{
		// MD ROM, and 32X is active.
		// Write to the 32X ROM, too.
		mdp_host_mem_write_block_8_le(_32X_Rom, address, data, length);
	}
	
	if (memID == MDP_MEM_MD_VRAM)
		VRam_Flag = 1;
	else if (memID == MDP_MEM_MD_CRAM)
		CRam_Flag = 1;
	
	/* The block has been written. */
	return MDP_ERR_OK;
}

/* TODO: Test this function. */
int MDP_FNCALL mdp_host_mem_write_block_16(mdp_t *plugin, int memID, uint32_t address, uint16_t *data, uint32_t length)
{
	GSFT_UNUSED_PARAMETER(plugin);
	
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (address & 1)
		return -MDP_ERR_MEM_UNALIGNED;
	ice = 0;
	
	uint16_t *ptr;
	uint32_t mem_size;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			ptr = (uint16_t*)Rom_Data;
			mem_size = 0x400000;
			break;
		case MDP_MEM_MD_RAM:
			ptr = (uint16_t*)Ram_68k;
			mem_size = 0x10000;
			break;
		case MDP_MEM_MD_VRAM:
			ptr = (uint16_t*)VRam;
			mem_size = 0x10000;
			break;
		case MDP_MEM_MD_CRAM:
			ptr = (uint16_t*)CRam;
			mem_size = 0x80;
			break;
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
	
	/* length is the number of 16-bit words. Convert to number of 8-bit bytes. */
	length <<= 1;
	
	if (address + length > mem_size)
	{
		/* Out of range. */
		return -MDP_ERR_MEM_OUT_OF_RANGE;
	}
	
	/* Copy the block. */
	memcpy(ptr, data, length);
	
	if (memID == MDP_MEM_MD_VRAM)
		VRam_Flag = 1;
	else if (memID == MDP_MEM_MD_VRAM)
		CRam_Flag = 1;
	
	/* The block has been written. */
	return MDP_ERR_OK;
}


int MDP_FNCALL mdp_host_mem_write_block_32(mdp_t *plugin, int memID, uint32_t address, uint32_t *data, uint32_t length)
{
	GSFT_UNUSED_PARAMETER(plugin);
	
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	if (address & 1)
		return -MDP_ERR_MEM_UNALIGNED;
	ice = 0;
	
	MDP_UNUSED_PARAMETER(memID);
	MDP_UNUSED_PARAMETER(address);
	MDP_UNUSED_PARAMETER(data);
	MDP_UNUSED_PARAMETER(length);
	
	/* TODO */
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
}


/** Memory Size Functions **/

int MDP_FNCALL mdp_host_mem_size_get(int memID)
{
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			return Rom_Size;
		case MDP_MEM_MD_RAM:
			return sizeof(Ram_68k);
		case MDP_MEM_MD_VRAM:
			return sizeof(VRam);
		case MDP_MEM_MD_CRAM:
			return sizeof(CRam);
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
}

static int mdp_host_mem_size_set_MD_ROM(unsigned int size)
{
	/* Attempt to resize the ROM. */
	/* TODO: SSF2 mapper support? */
	if (size <= 0 || size > 0x400000)
	{
		/* New size is out of range. */
		return -MDP_ERR_MEM_OUT_OF_RANGE;
	}
	
	if (size == Rom_Size)
	{
		/* New size is the same as the old size. */
		return MDP_ERR_OK;
	}
	else if (size > Rom_Size)
	{
		/* New size is larger. Zero the allocated memory. */
		int erase_size = size;
		if (Rom_Size & 1)
		{
			MEM_RW_8_BE(Rom_Data, Rom_Size) = 0x00;
			Rom_Size++;
			erase_size++;
			if (erase_size >= 0x400000)
				erase_size = 0x400000;
		}
		
		memset(&Rom_Data[Rom_Size], 0x00, (erase_size - Rom_Size));
	}
	else if (size < Rom_Size)
	{
		/* New size is smaller. Zero the blank area. */
		/* TODO */
	}
	
	/* Set the new ROM size. */
	Rom_Size = size;
	return MDP_ERR_OK;
}

int MDP_FNCALL mdp_host_mem_size_set(mdp_t *plugin, int memID, unsigned int size)
{
	GSFT_UNUSED_PARAMETER(plugin);
	
	if (!Game)
		return -MDP_ERR_ROM_NOT_LOADED;
	
	switch (memID)
	{
		case MDP_MEM_MD_ROM:
			return mdp_host_mem_size_set_MD_ROM(size);
		
		case MDP_MEM_MD_RAM:
		case MDP_MEM_MD_VRAM:
		case MDP_MEM_MD_CRAM:
			/* Non-resizable memory blocks. */
			return -MDP_ERR_MEM_NOT_RESIZABLE;
		
		default:
			/* Invalid memory ID. */
			return -MDP_ERR_MEM_INVALID_MEMID;
	}
}

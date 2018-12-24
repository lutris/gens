/***************************************************************************
 * MDP: IPS Patcher. (UPS Patch File Handler)                              *
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

/**
 * UPS patch file format: (see http://www.romhacking.net/forum/index.php/topic,6059.0.html)
 * (X == original file; Y = resulting file; Z == patch file)
 *
 * [4] signature ("UPS1")
 * [V] X file size
 * [V] Y file size
 * [?] {
 *   [V] relative difference offset
 *   [?] X ^ Y
 *   [1] 0x00 terminator
 * [4] X file CRC32
 * [4] Y file CRC32
 * [4] Z file CRC32
 */

#include "ups_file.hpp"
#include "ips.h"
#include "ips_plugin.h"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C++ includes.
#include <list>
using std::list;

// MDP includes.
#include "mdp/mdp_stdint.h"
#include "mdp/mdp_error.h"


typedef struct _ups_block_t
{
	uint32_t address;
	uint32_t length;
	uint8_t  *data_xor;
} ups_block_t;


static int ups_apply(uint32_t X_filesize, uint32_t Y_filesize,
		     uint32_t X_crc32,    uint32_t Y_crc32,
		     list<ups_block_t>& lstUPSBlocks);


/**
 * ups_varlen_decode(): Decode a variable length from a UPS patch.
 * @return Variable length, or 0 on error.
 */
static uint32_t ups_varlen_decode(uint8_t*& ups_ptr, uint8_t *const ups_ptr_end)
{
	size_t offset = 0, shift = 1;
	while (1)
	{
		if ((ups_ptr + 1) > ups_ptr_end)
			return ~0;
		
		uint8_t x = *ups_ptr++;
		offset += (x & 0x7F) * shift;
		if (x & 0x80)
			break;
		shift <<= 7;
		offset += shift;
	}
	
	if (offset > 0xFFFFFFFF)
		return ~0;
	
	return (uint32_t)offset;
}


#define UPS_LE32_TO_CPU(ptr)	\
	((ptr)[0] | ((ptr)[1] << 8) | ((ptr)[2] << 16) | ((ptr)[3] << 24))

/**
 * ups_file_load(): Load a UPS patch file.
 * @param filename Filename of the UPS patch file.
 * @return MDP error code.
 */
int MDP_FNCALL ups_file_load(const char* filename)
{
	int rval;
	size_t ups_size;
	mdp_z_t *zf_ups = NULL;
	uint8_t *ups_buf;
	
	if (ips_host_srv->z_open != NULL)
	{
		/* MDP decompression functions may be implemented. */
		rval = ips_host_srv->z_open(filename, &zf_ups);
		if (rval == -MDP_ERR_FUNCTION_NOT_IMPLEMENTED)
		{
			/* MDP decompression functions are not implemented. */
			zf_ups = NULL;
		}
		else if (rval != MDP_ERR_OK)
		{
			/* MDP decompression functions are implemented, but an error occurred. */
			return -1;
		}
	}
	
	/* Load the entire IPS into memory. */
	if (zf_ups)
	{
		/* MDP decompression functions are implemented. */
		ups_size = zf_ups->files->filesize;
		if (ups_size > 16*1024*1024)
			return -0xFF;
		
		ups_buf = (uint8_t*)malloc(ups_size);
		if (ips_host_srv->z_get_file(zf_ups, zf_ups->files, ups_buf, ups_size) <= 0)
		{
			/* Error loading the file. */
			free(ups_buf);
			ips_host_srv->z_close(zf_ups);
			return -2;
		}
		
		/* Close the file. */
		ips_host_srv->z_close(zf_ups);
	}
	else
	{
		/* MDP decompression functions are not implemented. */
		FILE *f = fopen(filename, "rb");
		if (!f)
		{
			/* Error opening the file. */
			return -3;
		}
		
		/* Get the length of the file. */
		fseek(f, 0, SEEK_END);
		ups_size = ftell(f);
		fseek(f, 0, SEEK_SET);
		
		if (ups_size > 16*1024*1024)
		{
			/* UPS file is too big. */
			fclose(f);
			return -0xFF;
		}
		
		/* Read the file into memory. */
		ups_buf = (uint8_t*)malloc(ups_size);
		size_t rsize = fread(ups_buf, 1, ups_size, f);
		
		/* Close the file. */
		fclose(f);
		
		/* Check that the filesize is correct. */
		if (rsize != ups_size)
		{
			/* Short read. Error! */
			free(ups_buf);
			return -4;
		}
	}
	
	// Check the "magic number".
	static const char ups_magic_number[] = {'U', 'P', 'S', '1'};
	
	if (memcmp(&ups_buf[0], ups_magic_number, sizeof(ups_magic_number)) != 0)
	{
		// Magic number doesn't match.
		return -5;
	}
	
	uint8_t *ups_ptr = &ups_buf[4];
	uint8_t *const ups_ptr_end = ups_buf + ups_size;
	
	// Pointer to the CRC32 area of the UPS patch.
	const uint8_t *const ups_crc32_ptr = &ups_buf[ups_size - 12];
	
	// Check the CRC32 of the UPS patch.
	bool crc32_is_available = false;
	
	const uint32_t X_crc32 = UPS_LE32_TO_CPU(&ups_crc32_ptr[0]);
	const uint32_t Y_crc32 = UPS_LE32_TO_CPU(&ups_crc32_ptr[4]);
	const uint32_t Z_crc32 = UPS_LE32_TO_CPU(&ups_crc32_ptr[8]);

	if (ips_host_srv->crc32 != NULL)
	{
		/* CRC32 might be available. */
		uint32_t calc_Z_crc32;
		rval = ips_host_srv->crc32(ups_buf, ups_size - 4, &calc_Z_crc32);
		
		if (rval == MDP_ERR_OK)
		{
			// CRC32 exists. Check if it matches.
			if (calc_Z_crc32 != Z_crc32)
			{
				// Incorrect CRC32.
				free(ups_buf);
				return -6;
			}
			
			// CRC32 matches.
			crc32_is_available = true;
		}
		else if (rval != -MDP_ERR_FUNCTION_NOT_IMPLEMENTED)
		{
			// Other error.
			free(ups_buf);
			return -7;
		}
	}
	
	// Get the filesizes.
	uint32_t X_filesize = ups_varlen_decode(ups_ptr, ups_ptr_end);
	uint32_t Y_filesize = ups_varlen_decode(ups_ptr, ups_ptr_end);
	if (X_filesize == ~0 || Y_filesize == ~0)
		return -6;
	
	// Read the data into a list.
	list<ups_block_t> lstUPSBlocks;
	
	bool ups_OK = true;
	ups_block_t block;
	uint32_t address = 0;
	uint32_t address_offset;
	uint8_t *ptr;
	
	while (true)
	{
		// Get the address offset for the next block.
		address_offset = ups_varlen_decode(ups_ptr, ups_ptr_end);
		if (address_offset == ~0)
		{
			// Short read. Invalid UPS patch file.
			ups_OK = false;
			break;
		}
		else if (address_offset >= Y_filesize)
		{
			// End of patch.
			break;
		}
		
		// Add the offset to the current address.
		address += address_offset;
		
		// Search for a NULL.
		for (ptr = ups_ptr; ptr < ups_ptr_end; ptr++)
		{
			if (*ptr == 0x00)
				break;
		}
		
		if (ptr == ups_ptr_end)
		{
			// Short read. Invalid UPS patch file.
			ups_OK = false;
			break;
		}
		
		// Found a NULL. Set the block information.
		block.address = address;
		block.length = (uint32_t)(ptr - ups_ptr);
		block.data_xor = ups_ptr;
		
		// Add the block to the list.
		lstUPSBlocks.push_back(block);
		
		// Go to the next block.
		ups_ptr = ptr + 1;
	}
	
	if (!ups_OK)
	{
		// Invalid UPS patch.
		lstUPSBlocks.clear();
		free(ups_buf);
		return -8;
	}
	
	// Apply the UPS patch.
	rval = ups_apply(X_filesize, Y_filesize, X_crc32, Y_crc32, lstUPSBlocks);
	
	/* Free the UPS buffer. */
	free(ups_buf);
	
	/* Return the MDP error code from ips_apply(). */
	return rval;
}


/**
 * ups_apply(): Apply a UPS patch.
 * @param X_filesize Original filesize.
 * @param Y_filesize New filesize.
 * @param X_crc32 CRC32 of the original file.
 * @param Y_crc32 CRC32 of the new file.
 * @param lstUPSBlocks List of UPS blocks to apply.
 * @return MDP error code.
 */
static int ups_apply(uint32_t X_filesize, uint32_t Y_filesize,
		     uint32_t X_crc32,    uint32_t Y_crc32,
		     list<ups_block_t>& lstUPSBlocks)
{
	// TODO
	return -MDP_ERR_FUNCTION_NOT_IMPLEMENTED;
#if 0
	// Check if the ROM memory area needs to be resized.
	int rom_size = ips_host_srv->mem_size_get(MDP_MEM_MD_ROM);
	if (rom_size < 0)
	{
		// Error obtaining the ROM size.
		return rom_size;
	}
	
	if (dest_length != rom_size)
	{
		// Resize the ROM.
		int rval = ips_host_srv->mem_size_set(MDP_MEM_MD_ROM, dest_length);
		if (rval != MDP_ERR_OK)
		{
			// Error resizing the ROM.
			return rval;
		}
	}
	
	// Patch the ROM.
	int rval;
	for (list<ips_block_t>::iterator iter = lstIPSBlocks.begin();
	     iter != lstIPSBlocks.end(); iter++)
	{
		ips_block_t *block = &(*iter);
		
		rval = ips_host_srv->mem_write_block_8(MDP_MEM_MD_ROM,
				block->address, block->data, block->length);
		
		// TODO: Better error handling.
		if (rval != MDP_ERR_OK)
			return rval;
	}
	
	// Patch applied successfully.
	// Reset the emulator for the patch to take effect.
	// TODO: Tell the emulator to recheck the ROM for changes to the header.
	ips_host_srv->emulator_control(&mdp, MDP_EMUCTRL_RESET, NULL);
	
	// TODO: Write a message to the OSD indicating that a patch has been loaded.
	
	return MDP_ERR_OK;
#endif
}

/***************************************************************************
 * MDP: IPS Patcher. (IPS Patch File Handler)                              *
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

#include "ips_file.hpp"
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
#include "mdp/mdp_mem.h"


typedef struct _ips_block_t
{
	uint32_t address;
	uint16_t length;
	uint8_t  *data;
} ips_block_t;


static int ips_apply(uint32_t dest_length, list<ips_block_t>& lstIPSBlocks);


/**
 * ips_file_load(): Load an IPS patch file.
 * @param filename Filename of the IPS patch file.
 * @return MDP error code.
 */
int MDP_FNCALL ips_file_load(const char* filename)
{
	int rval;
	size_t ips_size;
	mdp_z_t *zf_ips = NULL;
	uint8_t *ips_buf;
	
	if (ips_host_srv->z_open != NULL)
	{
		/* MDP decompression functions may be implemented. */
		rval = ips_host_srv->z_open(filename, &zf_ips);
		if (rval == -MDP_ERR_FUNCTION_NOT_IMPLEMENTED)
		{
			/* MDP decompression functions are not implemented. */
			zf_ips = NULL;
		}
		else if (rval != MDP_ERR_OK)
		{
			/* MDP decompression functions are implemented, but an error occurred. */
			return -1;
		}
	}
	
	/* Load the entire IPS into memory. */
	if (zf_ips)
	{
		/* MDP decompression functions are implemented. */
		ips_size = zf_ips->files->filesize;
		if (ips_size > 16*1024*1024)
			return -0xFF;
		
		ips_buf = (uint8_t*)malloc(ips_size);
		if (ips_host_srv->z_get_file(zf_ips, zf_ips->files, ips_buf, ips_size) <= 0)
		{
			/* Error loading the file. */
			free(ips_buf);
			ips_host_srv->z_close(zf_ips);
			return -2;
		}
		
		/* Close the file. */
		ips_host_srv->z_close(zf_ips);
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
		ips_size = ftell(f);
		fseek(f, 0, SEEK_SET);
		
		if (ips_size > 16*1024*1024)
		{
			/* IPS file is too big. */
			fclose(f);
			return -0xFF;
		}
		
		/* Read the file into memory. */
		ips_buf = (uint8_t*)malloc(ips_size);
		size_t rsize = fread(ips_buf, 1, ips_size, f);
		
		/* Close the file. */
		fclose(f);
		
		/* Check that the filesize is correct. */
		if (rsize != ips_size)
		{
			/* Short read. Error! */
			free(ips_buf);
			return -4;
		}
	}
	
	// Check the "magic number".
	static const char ips_magic_number[] = {'P', 'A', 'T', 'C', 'H'};
	
	if (memcmp(&ips_buf[0], ips_magic_number, sizeof(ips_magic_number)) != 0)
	{
		// Magic number doesn't match.
		return -3;
	}
	
	uint8_t *ips_ptr = &ips_buf[5];
	uint8_t *const ips_ptr_end = ips_buf + ips_size;
	
	// Read the data into a list.
	list<ips_block_t> lstIPSBlocks;
	
	bool ips_OK = true;
	ips_block_t block;
	uint32_t dest_length = 0;
	uint32_t cur_dest_length;
	uint16_t rle_length;
	
	while (true)
	{
		// Get the address for the next block.
		if ((ips_ptr + 3) > ips_ptr_end)
		{
			// Short read. Invalid IPS patch file.
			ips_OK = false;
			break;
		}
		
		// Check if this is an EOF.
		static const char ips_eof[] = {'E', 'O', 'F'};
		if (!memcmp(ips_ptr, ips_eof, sizeof(ips_eof)))
			break;
		
		// Address is stored as 24-bit, big-endian,
		block.address = (uint32_t)((ips_ptr[0] << 16) | (ips_ptr[1] << 8) | ips_ptr[2]);
		ips_ptr += 3;
		
		// Get the length of the next block.
		if ((ips_ptr + 2) > ips_ptr_end)
		{
			// Short read. Invalid IPS patch file.
			ips_OK = false;
			break;
		}
		
		// Length is stored as 16-bit, big-endian.
		block.length = (uint16_t)((ips_ptr[0] << 8) | ips_ptr[1]);
		ips_ptr += 2;
		
		// If the length is 0, this is an RLE-encoded block.
		if (block.length == 0)
		{
			// RLE-encoded block.
			
			// Get the length of the RLE data.
			if ((ips_ptr + 2) > ips_ptr_end)
			{
				// Short read. Invalid IPS patch file.
				ips_OK = false;
				break;
			}
			
			// RLE length is stored as 16-bit, big-endian.
			block.length = (uint16_t)((ips_ptr[0] << 8) | ips_ptr[1]);
			ips_ptr += 2;
			if (block.length == 0)
			{
				// Zero-length RLE block is invalid.
				ips_OK = false;
				break;
			}
			
			// Get the data byte for the RLE block.
			if ((ips_ptr + 1) > ips_ptr_end)
			{
				// Short read. Invalid IPS patch file.
				ips_OK = false;
				break;
			}
			
			// Create the RLE data block.
			block.data = (uint8_t*)malloc(block.length);
			memset(block.data, *ips_ptr, block.length);
			ips_ptr += 1;
		}
		else
		{
			// Regular IPS data block.
			
			// Get the data for the block.
			if ((ips_ptr + block.length) > ips_ptr_end)
			{
				// Short read. Invalid IPS patch file.
				ips_OK = false;
				break;
			}
			block.data = ips_ptr;
			ips_ptr += block.length;
		}
		
		// Check the destination length.
		cur_dest_length = block.address + block.length;
		if (cur_dest_length > dest_length)
			dest_length = cur_dest_length;
		
		// Add the block to the list.
		lstIPSBlocks.push_back(block);
	}
	
	if (!ips_OK)
	{
		// Invalid IPS patch.
		lstIPSBlocks.clear();
		free(ips_buf);
		return -4;
	}
	
	// Apply the IPS patch.
	rval = ips_apply(dest_length, lstIPSBlocks);
	
	/* Free the IPS buffer. */
	free(ips_buf);
	
	/* Return the MDP error code from ips_apply(). */
	return rval;
}


/**
 * ips_apply(): Apply an IPS patch.
 * @param dest_length Length of the resulting file.
 * @param lstIPSBlocks List of IPS blocks to apply.
 * @return MDP error code.
 */
static int ips_apply(uint32_t dest_length, list<ips_block_t>& lstIPSBlocks)
{
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
		int rval = ips_host_srv->mem_size_set(&mdp, MDP_MEM_MD_ROM, dest_length);
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
		
		rval = ips_host_srv->mem_write_block_8(&mdp, MDP_MEM_MD_ROM,
				block->address, block->data, block->length);
		
		// TODO: Better error handling.
		if (rval != MDP_ERR_OK)
			return rval;
	}
	
	// Patch applied successfully.
	// Reload ROM information and reset the emulator for the patch to take effect.
	ips_host_srv->emulator_control(&mdp, MDP_EMUCTRL_RELOAD_INFO, NULL);
	ips_host_srv->emulator_control(&mdp, MDP_EMUCTRL_RESET_HARD, NULL);
	
	// Write a message to the OSD.
	ips_host_srv->osd_printf("IPS patch applied.");
	
	return MDP_ERR_OK;
}

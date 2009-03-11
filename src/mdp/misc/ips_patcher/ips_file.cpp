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

// MDP includes.
#include "mdp/mdp_error.h"

// C includes.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C++ includes.
#include <list>
using std::list;


struct IPS_Block
{
	uint32_t address;
	uint16_t length;
	uint8_t  *data;
	
	IPS_Block() { data = NULL; }
	~IPS_Block() { free(data); data = NULL; }
};


static int ips_apply(uint32_t dest_length, list<IPS_Block>& lstIPSBlocks);


/**
 * ips_file_load(): Load an IPS patch file.
 * @param filename Filename of the IPS patch file.
 * @return MDP error code.
 */
int MDP_FNCALL ips_file_load(const char* filename)
{
	FILE* f_ips = fopen(filename, "rb");
	if (!f_ips)
		return -1;
	
	uint8_t buf[16];
	
	// Check the "magic number".
	static const char ips_magic_number[] = {'P', 'A', 'T', 'C', 'H'};
	
	fseek(f_ips, 0, SEEK_SET);
	fread(buf, 1, sizeof(ips_magic_number), f_ips);
	if (memcmp(buf, ips_magic_number, sizeof(ips_magic_number)) != 0)
	{
		// Magic number doesn't match.
		return -2;
	}
	
	// Read the data into a list.
	list<IPS_Block> lstIPSBlocks;
	
	bool ips_OK = true;
	IPS_Block block;
	uint32_t dest_length = 0;
	uint32_t cur_dest_length;
	uint16_t rle_length;
	
	while (true)
	{
		// Get the address for the next block.
		if (fread(buf, 1, 3, f_ips) != 3)
		{
			// Short read. Invalid IPS patch file.
			ips_OK = false;
			break;
		}
		
		// Check if this is an EOF.
		static const char ips_eof[] = {'E', 'O', 'F'};
		if (memcmp(buf, ips_eof, sizeof(ips_eof)) == 0)
			break;
		
		// Address is stored as 24-bit, big-endian,
		block.address = (uint32_t)((buf[0] << 16) | (buf[1] << 8) | buf[2]);
		
		// Get the length of the next block.
		if (fread(buf, 1, 2, f_ips) != 2)
		{
			// Short read. Invalid IPS patch file.
			ips_OK = false;
			break;
		}
		
		// Length is stored as 16-bit, big-endian.
		block.length = (uint16_t)((buf[0] << 8) | buf[1]);
		
		// If the length is 0, this is an RLE-encoded block.
		if (block.length == 0)
		{
			// RLE-encoded block.
			
			// Get the length of the RLE data.
			if (fread(buf, 1, 2, f_ips) != 2)
			{
				// Short read. Invalid IPS patch file.
				ips_OK = false;
				break;
			}
			
			// RLE length is stored as 16-bit, big-endian.
			block.length = (uint16_t)((buf[0] << 8) | buf[1]);
			if (block.length == 0)
			{
				// Zero-length RLE block is invalid.
				ips_OK = false;
				break;
			}
			
			// Get the data byte for the RLE block.
			if (fread(buf, 1, 1, f_ips) != 1)
			{
				// Short read. Invalid IPS patch file.
				ips_OK = false;
				break;
			}
			
			// Create the RLE data block.
			block.data = (uint8_t*)malloc(block.length);
			memset(block.data, buf[0], block.length);
		}
		else
		{
			// Regular IPS data block.
			
			// Get the data for the block.
			block.data = (uint8_t*)malloc(block.length);
			if (fread(block.data, 1, block.length, f_ips) != block.length)
			{
				// Short read. Invalid IPS patch file.
				free(block.data);
				ips_OK = false;
				break;
			}
		}
		
		// Check the destination length.
		cur_dest_length = block.address + block.length;
		if (cur_dest_length > dest_length)
			dest_length = cur_dest_length;
		
		// Add the block to the list.
		lstIPSBlocks.push_back(block);
	}
	
	// Make sure the temporary block's constructor doesn't free any memory.
	// TODO: Add a reference counter?
	block.data = NULL;
	
	if (!ips_OK)
	{
		// Invalid IPS patch.
		lstIPSBlocks.clear();
		return -3;
	}
	
	// Apply the IPS patch.
	return ips_apply(dest_length, lstIPSBlocks);
}


/**
 * ips_apply(): Apply an IPS patch.
 * @param dest_length Length of the resulting file.
 * @param lstIPSBlocks List of IPS blocks to apply.
 * @return MDP error code.
 */
static int ips_apply(uint32_t dest_length, list<IPS_Block>& lstIPSBlocks)
{
	// Check if the ROM memory area needs to be resized.
	int rom_size = ips_host_srv->val_get(MDP_VAL_ROM_SIZE);
	if (rom_size < 0)
	{
		// Error obtaining the ROM size.
		return rom_size;
	}
	
	if (dest_length > rom_size)
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
	for (list<IPS_Block>::iterator iter = lstIPSBlocks.begin();
	     iter != lstIPSBlocks.end(); iter++)
	{
		IPS_Block *block = &(*iter);
		
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
}

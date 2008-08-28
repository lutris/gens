/***************************************************************************
 * Gens: File Compression Function Definitions.                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#include "compress.h"

const struct COMPRESS_Handler_t CompressMethods[] =
{
	{
		// GZip format
		GZip_Detect_Format,
		GZip_Get_Num_Files,
		GZip_Get_File_Info,
		GZip_Get_File,
	},
	{
		// Zip format
		Zip_Detect_Format,
		Zip_Get_Num_Files,
		Zip_Get_File_Info,
		Zip_Get_File,
	},
	{
		// Everything else
		Dummy_Detect_Format,
		Dummy_Get_Num_Files,
		Dummy_Get_File_Info,
		Dummy_Get_File,
	},
	// End of list
	{NULL, NULL, NULL, NULL},
};


/**
 * COMPRESS_FileInfo_Free(): Convenience function to free a list of COMPRESS_FileInfo_t.
 * @param fileInfo First element in the COMPRESS_FileInfo_t list.
 */
void COMPRESS_FileInfo_Free(struct COMPRESS_FileInfo_t *fileInfo)
{
	struct COMPRESS_FileInfo_t *cur, *nxt;
	cur = fileInfo;
	while (cur)
	{
		nxt = cur->next;
		free(cur);
		cur = nxt;
	}
}

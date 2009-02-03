/***************************************************************************
 * Gens: File Decompression Function Definitions.                          *
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

#include "decompressor.h"

#include <stdlib.h>

/**
 * file_list_t_free(): Free a list of files.
 * @param file_list Pointer to the first file in the list.
 */
void file_list_t_free(file_list_t *file_list)
{
	if (!file_list)
		return;
	
	do
	{
		// Free the filename, if one's allocated.
		if (file_list->filename)
			free(file_list->filename);
		
		// Save the next pointer.
		file_list_t *next = file_list->next;
		
		// Free the file_list_t.
		free(file_list);
		
		// Go to the next file_list_t.
		file_list = next;
	} while (file_list);
}

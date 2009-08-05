/******************************************************************************
 * libgsft: Common functions.                                                 *
 * gsft_strtok_r.c: strtok_r() implementation for older systems.              *
 *                                                                            *
 * MDP implementation Copyright (c) 2008-2009 by David Korth                  *
 *                                                                            *
 * Original implementation derived from the GNU C Library.                    *
 * Copyright (C) 1991,1996-1999,2001,2004,2007 Free Software Foundation, Inc. *
 *                                                                            *
 * This program is free software; you can redistribute it and/or modify it    *
 * under the terms of the GNU General Public License as published by the      *
 * Free Software Foundation; either version 2 of the License, or (at your     *
 * option) any later version.                                                 *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    *
 * with this program; if not, write to the Free Software Foundation, Inc.,    *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.              *
 ******************************************************************************/

#include "gsft_strtok_r.h"

// C includes.
#include <string.h>

char* gsft_strtok_r(char *str, const char *delim, char **save_ptr)
{
	char *token;
	
	if (str == NULL)
		str = *save_ptr;
	
	/* Scan leading delimiters. */
	str += strspn(str, delim);
	if (*str == '\0')
	{
		*save_ptr = str;
		return NULL;
	}
	
	/* Find the end of the token. */
	token = str;
	str = strpbrk(token, delim);
	if (str == NULL)
	{
		/* This token finishes the string.  */
		*save_ptr = strchr(token, '\0');
	}
	else
	{
		/* Terminate the token and make *SAVE_PTR point past it.  */
		*str = '\0';
		*save_ptr = str + 1;
	}
	return token;
}

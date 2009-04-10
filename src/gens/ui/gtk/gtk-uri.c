/***************************************************************************
 * Gens: (GTK+) URI functions, copied from glib-2.18.4.                    *
 *                                                                         *
 * Copyright (C) 2006-2007 Red Hat, Inc.                                   *
 *                                                                         *
 * Gens/GS port Copyright (c) 2009 by David Korth                          *
 *                                                                         *
 * This library is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This library is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA           *
 * 02110-1301, USA.                                                        *
 *                                                                         *
 * Author: Alexander Larsson <alexl@redhat.com>                            *
 ***************************************************************************/

#include "gtk-uri.h"

#include <glib.h>
#include <string.h>


/**
 * SECTION:gurifuncs
 * @short_description: URI Functions
 * 
 * Functions for manipulating Universal Resource Identifiers (URIs) as 
 * defined by <ulink url="http://www.ietf.org/rfc/rfc3986.txt">
 * RFC 3986</ulink>. It is highly recommended that you have read and
 * understand RFC 3986 for understanding this API.
 */


static int unescape_character(const char *scanner)
{
	int first_digit;
	int second_digit;
	
	first_digit = g_ascii_xdigit_value(*scanner++);
	if (first_digit < 0)
		return -1;
	
	second_digit = g_ascii_xdigit_value(*scanner++);
	if (second_digit < 0)
		return -1;
	
	return (first_digit << 4) | second_digit;
}


/**
 * gens_g_uri_unescape_segment: [copied from glib-2.18.4]
 * @escaped_string: a string.
 * @escaped_string_end: a string.
 * @illegal_characters: an optional string of illegal characters not to be allowed.
 * 
 * Unescapes a segment of an escaped string.
 *
 * If any of the characters in @illegal_characters or the character zero appears
 * as an escaped character in @escaped_string then that is an error and %NULL
 * will be returned. This is useful it you want to avoid for instance having a
 * slash being expanded in an escaped path element, which might confuse pathname
 * handling.
 *
 * Returns: an unescaped version of @escaped_string or %NULL on error.
 * The returned string should be freed when no longer needed.
 *
 * Since: 2.16
 **/
static char* gens_g_uri_unescape_segment(const char *escaped_string,
					 const char *escaped_string_end,
					 const char *illegal_characters)
{
	const char *in;
	char *out, *result;
	gint character;
	
	if (escaped_string == NULL)
		return NULL;
		
	if (escaped_string_end == NULL)
		escaped_string_end = escaped_string + strlen (escaped_string);
	
	result = (char*)g_malloc(escaped_string_end - escaped_string + 1);
	
	out = result;
	for (in = escaped_string; in < escaped_string_end; in++)
	{
		character = *in;
		
		if (*in == '%')
		{
			in++;
			
			if (escaped_string_end - in < 2)
			{
				/* Invalid escaped char (to short) */
				g_free(result);
				return NULL;
			}
			
			character = unescape_character(in);
			
			/* Check for an illegal character. We consider '\0' illegal here. */
			if (character <= 0 || (illegal_characters != NULL &&
				strchr(illegal_characters, (char)character) != NULL))
			{
				g_free(result);
				return NULL;
			}
			
			in++; /* The other char will be eaten in the loop header */
		}
		*out++ = (char)character;
	}
	
	*out = '\0';
	
	return result;
}


/**
 * g_uri_unescape_string: [copied from glib-2.18.4]
 * @escaped_string: an escaped string to be unescaped.
 * @illegal_characters: an optional string of illegal characters not to be allowed.
 * 
 * Unescapes a whole escaped string.
 * 
 * If any of the characters in @illegal_characters or the character zero appears
 * as an escaped character in @escaped_string then that is an error and %NULL
 * will be returned. This is useful it you want to avoid for instance having a
 * slash being expanded in an escaped path element, which might confuse pathname
 * handling.
 *
 * Returns: an unescaped version of @escaped_string. The returned string 
 * should be freed when no longer needed.
 *
 * Since: 2.16
 **/
char* gens_g_uri_unescape_string(const char *escaped_string,
				 const char *illegal_characters)
{
	return gens_g_uri_unescape_segment(escaped_string, NULL, illegal_characters);
}

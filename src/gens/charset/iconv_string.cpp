/***************************************************************************
 * Gens: iconv wrapper functions.                                           *
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

#include "iconv_string.hpp"

// C includes.
#include <stdlib.h>

// C++ includes.
#include <string>
using std::string;

// iconv includes.
#include <iconv.h>
#include <errno.h>


/**
 * gens_iconv_to_utf8(): Convert a string from any character set to UTF-8.
 * @param src 		[in] Source string.
 * @param src_bytes_len [in] Source length, in bytes.
 * @param src_charset	[in] Source character set.
 * @param dest_charset	[in] Destination character set.
 * @return UTF-8 C++ string, or "" on error.
 */
string gens_iconv(const char *src, size_t src_bytes_len, const char *src_charset, const char *dest_charset)
{
	if (!src || src_bytes_len == 0)
		return "";
	
	if (!src_charset)
		src_charset = "";
	if (!dest_charset)
		dest_charset = "";
	
	// Based on examples from:
	// * http://www.delorie.com/gnu/docs/glibc/libc_101.html
	// * http://www.codase.com/search/call?name=iconv
	
	// Open an iconv descriptor.
	iconv_t cd;
	cd = iconv_open(dest_charset, src_charset);
	if (cd == (iconv_t)(-1))
	{
		// Error opening iconv.
		return "";
	}
	
	// Allocate the output buffer.
	// UTF-8 is variable length, and the largest UTF-8 character is 4 bytes long.
	const size_t out_bytes_len = src_bytes_len * 4;
	size_t out_bytes_remaining = out_bytes_len;
	char *outbuf = (char*)malloc(out_bytes_len);
	
	// Input and output pointers.
	char *inptr = const_cast<char*>(src);	// Input pointer.
	char *outptr = &outbuf[0];		// Output pointer.
	
	bool success = true;
	
	while (src_bytes_len > 0)
	{
		if (iconv(cd, &inptr, &src_bytes_len, &outptr, &out_bytes_remaining) == (size_t)(-1))
		{
			// An error occurred while converting the string.
			success = false;
			break;
		}
	}
	
	// Close the iconv descriptor.
	iconv_close(cd);
	
	if (success)
	{
		// The string was converted successfully.
		
		// Make sure the string is terminated.
		if (out_bytes_remaining >= out_bytes_len)
			*outptr = 0x00;
		
		string rstr = string(outbuf);
		free(outbuf);
		return rstr;
	}
	
	// The string was not converted successfully.
	free(outbuf);
	return "";
}

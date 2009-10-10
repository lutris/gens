/***************************************************************************
 * cbob: C Binary Object Builder.                                          *
 *                                                                         *
 * Copyright (c) 2009 by David Korth                                       *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define CBOB_VERSION_MAJOR 0
#define CBOB_VERSION_MINOR 0
#define CBOB_VERSION_REVISION 1

#ifdef _WIN32
#define DIR_SEP_CHR '\\'
#define DIR_SEP_STR "\\"
#else
#define DIR_SEP_CHR '/'
#define DIR_SEP_STR "/"
#endif

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

static void show_usage(char *filename)
{
	fprintf(stderr, "C Binary Object Builder v%d.%d.%d\n"
	       "Copyright 2009 by David Korth.\n\n"
	       "This program is licensed under the GNU General Public License v2.\n"
	       "See http://www.gnu.org/licenses/gpl-2.0.html for more information.\n\n"
	       "Usage: %s input.bin output.c\n",
	       CBOB_VERSION_MAJOR, CBOB_VERSION_MINOR,
	       CBOB_VERSION_REVISION, filename);
}

int main(int argc, char *argv[])
{
	// TODO: getopt()
	if (argc < 3)
	{
		show_usage(argv[0]);
		return EXIT_FAILURE;
	}
	
	// Open the input file.
	FILE *f_in = fopen(argv[1], "rb");
	if (!f_in)
	{
		fprintf(stderr, "Error: Could not open input file '%s'. Error %d: %s.\n",
			argv[1], errno, strerror(errno));
		return EXIT_FAILURE;
	}
	
	// Filename without full path.
	char in_filename[PATH_MAX];
	const char *slash = strrchr(argv[1], DIR_SEP_CHR);
	if (!slash)
	{
		// No path separator found. Copy the entire filename.
		strncpy(in_filename, argv[1], sizeof(in_filename));
	}
	else
	{
		// Path separator found. Copy the file part only.
		strncpy(in_filename, slash+1, sizeof(in_filename));
	}
	in_filename[sizeof(in_filename)-1] = 0x00;
	
	// C-symbol version of the filename.
	char in_filename_symbol[PATH_MAX];
	strncpy(in_filename_symbol, in_filename, sizeof(in_filename_symbol));
	in_filename_symbol[sizeof(in_filename_symbol)-1] = 0x00;
	
	// Convert non-alphanumeric characters to underscores.
	char *n;
	for (n = &in_filename_symbol[0]; *n != 0x00; n++)
	{
		if (!isalnum(*n))
			*n = '_';
	}
	
	// Get the output filenames.
	const char *out_filename_c = argv[2];
	
	// Open the C output file.
	FILE *f_out = fopen(out_filename_c, "wb");
	if (!f_out)
	{
		fprintf(stderr, "Error: Could not open output file '%s'. Error %d: %s.\n",
			out_filename_c, errno, strerror(errno));
		fclose(f_in);
		return EXIT_FAILURE;
	}
	
	// Get the size of the input file.
	fseek(f_in, 0, SEEK_END);
	long in_fsize = ftell(f_in);
	fseek(f_in, 0, SEEK_SET);
	
	// Write the CBOB header.
	// TODO: Get the filename portion of the input file.
	fprintf(f_out,
		"/**\n"
		" * This file was generated using the C Binary Object Builder v%d.%d.%d.\n"
		" *\n"
		" * Source file: %s\n"
		" */\n\n", CBOB_VERSION_MAJOR, CBOB_VERSION_MINOR, CBOB_VERSION_REVISION, in_filename);
	
	// Start the CBOB array.
	fprintf(f_out, "const unsigned char cbob_%s[%ld] =\n{\n", in_filename_symbol, in_fsize);
	
	// Write the CBOB data.
	unsigned char data[16];
	for (; in_fsize > 0; in_fsize -= 16)
	{
		size_t n = fread(data, sizeof(data[0]), sizeof(data), f_in);
		if (n == 0)
			break;
		
		fprintf(f_out, "\t");
		unsigned char *dataptr = &data[0];
		for (; n != 0; n--)
		{
			fprintf(f_out, "0x%02X", *dataptr++);
			if (n == 1)
			{
				if (in_fsize > 16)
					fprintf(f_out, ",");
				fprintf(f_out, "\n");
			}
			else
			{
				fprintf(f_out, ", ");
			}
		}
	}
	
	// End the CBOB array.
	fprintf(f_out, "};\n");
	
	// Success!
	return EXIT_SUCCESS;
}

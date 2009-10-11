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

/**
 * strzcpy(): strncpy() wrapper with automatic NULL termination.
 * TODO: Port BSD's strlcpy(), since it has optimizations.
 */
#define strzcpy(dest, src, n) \
do { \
	strncpy(dest, src, n); \
	dest[n-1] = 0x00; \
} while (0)

#define CBOB_VERSION_MAJOR 0
#define CBOB_VERSION_MINOR 1
#define CBOB_VERSION_REVISION 0
//#define CBOB_VERSION_DEVELOPMENT

#define _QUOTEME(x) #x
#define QUOTEME(x) _QUOTEME(x)

#ifndef CBOB_VERSION_DEVELOPMENT
#define CBOB_VERSION_STRING QUOTEME(CBOB_VERSION_MAJOR) "." QUOTEME(CBOB_VERSION_MINOR) "." QUOTEME(CBOB_VERSION_REVISION)
#else
#define CBOB_VERSION_STRING QUOTEME(CBOB_VERSION_MAJOR) "." QUOTEME(CBOB_VERSION_MINOR) "." QUOTEME(CBOB_VERSION_REVISION) "+"
#endif

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
	fprintf(stderr, "C Binary Object Builder v" CBOB_VERSION_STRING "\n"
		"Copyright 2009 by David Korth.\n\n"
		"This program is licensed under the GNU General Public License v2.\n"
		"See http://www.gnu.org/licenses/gpl-2.0.html for more information.\n\n"
		"Usage: %s input.bin output.c\n"
		"Note: output.h is automatically generated.\n", filename);
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
		strzcpy(in_filename, argv[1], sizeof(in_filename));
	}
	else
	{
		// Path separator found. Copy the file part only.
		strzcpy(in_filename, slash+1, sizeof(in_filename));
	}
	
	// C-symbol version of the filename.
	char in_filename_symbol[PATH_MAX];
	strzcpy(in_filename_symbol, in_filename, sizeof(in_filename_symbol));
	
	// Convert non-alphanumeric characters to underscores.
	char *n;
	for (n = &in_filename_symbol[0]; *n != 0x00; n++)
	{
		if (!isalnum(*n))
			*n = '_';
	}
	
	// Get the output filenames.
	const char *out_filename_c = argv[2];
	
	char out_filename_h[PATH_MAX + 16];
	strzcpy(out_filename_h, out_filename_c, sizeof(out_filename_h));
	
	// Change the H output filename's extension.
	char *h_extpos = strrchr(out_filename_h, '.');
	const char *h_slashpos = strrchr(out_filename_h, DIR_SEP_CHR);
	if (h_extpos && (!h_slashpos || (h_slashpos < h_extpos)))
	{
		// Extension found. Null it out.
		*h_extpos = 0x00;
	}
	
	// Concatenate ".h".
	strcat(out_filename_h, ".h");
	printf("Filename: %s\n", out_filename_h);
	
	// Open the C output file.
	FILE *f_out_c = fopen(out_filename_c, "wb");
	if (!f_out_c)
	{
		fprintf(stderr, "Error: Could not open C output file '%s'. Error %d: %s.\n",
			out_filename_c, errno, strerror(errno));
		fclose(f_in);
		return EXIT_FAILURE;
	}
	
	// Open the H output file.
	FILE *f_out_h = fopen(out_filename_h, "wb");
	if (!f_out_h)
	{
		fprintf(stderr, "Error: Could not open H output file '%s'. Error %d: %s.\n",
			out_filename_h, errno, strerror(errno));
		fclose(f_in);
		fclose(f_out_c);
		return EXIT_FAILURE;
	}
	
	// Get the size of the input file.
	fseek(f_in, 0, SEEK_END);
	long in_fsize = ftell(f_in);
	fseek(f_in, 0, SEEK_SET);
	
	/** Generate the C file. **/
	
	// Write the CBOB header.
	fprintf(f_out_c,
		"/**\n"
		" * This file was generated using the C Binary Object Builder v" CBOB_VERSION_STRING ".\n"
		" * CBOB Source Code File.\n"
		" *\n"
		" * Source file: %s\n"
		" */\n\n", in_filename);
	
	// Start the CBOB array.
	fprintf(f_out_c, "const unsigned char cbob_%s[%ld] =\n{\n", in_filename_symbol, in_fsize);
	
	// Write the CBOB data.
	unsigned char data[16];
	int write_fsize;
	for (write_fsize = in_fsize; write_fsize > 0; write_fsize -= 16)
	{
		size_t sz = fread(data, sizeof(data[0]), sizeof(data), f_in);
		if (sz == 0)
			break;
		
		fprintf(f_out_c, "\t");
		unsigned char *dataptr = &data[0];
		for (; sz != 0; sz--)
		{
			fprintf(f_out_c, "0x%02X", *dataptr++);
			if (sz == 1)
			{
				if (write_fsize > 16)
					fprintf(f_out_c, ",");
				fprintf(f_out_c, "\n");
			}
			else
			{
				fprintf(f_out_c, ", ");
			}
		}
	}
	
	// End the CBOB array.
	fprintf(f_out_c, "};\n");
	
	// Close the C and input files.
	fclose(f_out_c);
	fclose(f_in);
	
	/** Generate the H file. **/
	
	// Write the CBOB header.
	fprintf(f_out_h,
		"/**\n"
		" * This file was generated using the C Binary Object Builder v" CBOB_VERSION_STRING ".\n"
		" * CBOB Header File.\n"
		" *\n"
		" * Source file: %s\n"
		" */\n\n", in_filename);
	
	// Declare the CBOB array.
	fprintf(f_out_h,
		"#ifdef __cplusplus\n"
		"extern \"C\" {\n"
		"#endif\n\n"
		"extern const unsigned char cbob_%s[%ld];\n\n"
		"#ifdef __cplusplus\n"
		"}\n"
		"#endif\n", in_filename_symbol, in_fsize);
	
	// Close the H file.
	fclose(f_out_h);
	
	// Success!
	return EXIT_SUCCESS;
}

/***************************************************************************
 * Gens: ROM handler.                                                      *
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

#ifndef GENS_ROM_HPP
#define GENS_ROM_HPP

#include "emulator/gens.hpp"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _ROM_t
{
	char Console_Name[17];
	char Copyright[17];
	char ROM_Name[49];
	char ROM_Name_W[49];
	char Type[3];
	char Version[13];
	unsigned int Checksum;
	char IO_Support[17];
	unsigned int ROM_Start_Address;
	unsigned int ROM_End_Address;
	unsigned int R_Size;
	char RAM_Info[13];
	unsigned int RAM_Start_Address;
	unsigned int RAM_End_Address;
	char Modem_Info[13];
	char Description[41];
	char Countries[4];
} ROM_t;

extern char Rom_Dir[GENS_PATH_MAX];

extern ROM_t* Game;
extern char ROM_Name[512];

#ifdef __cplusplus
}
#endif

// ROM type values.
#define ROMTYPE_SYS_NONE	0x00000000
#define ROMTYPE_SYS_MD		0x00000001
#define ROMTYPE_SYS_32X		0x00000002
#define ROMTYPE_SYS_MCD		0x00000003
#define ROMTYPE_SYS_MCD32X	0x00000004
#define ROMTYPE_SYS_MASK	0x000000FF

// ROM type flags.
#define ROMTYPE_FLAG_INTERLEAVED	0x00000100
#define ROMTYPE_FLAG_CD_BIN_CUE		0x00000200
#define ROMTYPE_FLAG_MASK		0x0000FF00

#ifdef __cplusplus

#include <string>
#include <deque>

// New C++ ROM class.
class ROM
{
	public:
		static void updateCDROMName(const char *cdromName);
		
		static int getROM(void);
		static int openROM(const std::string& filename, std::string z_filename = "");
		static ROM_t* loadSegaCD_BIOS(const std::string& filename);
		
		static unsigned int detectFormat(const unsigned char buf[2048]);
		
		static unsigned int loadROM(const std::string& filename,
					    std::string& z_filename,
					    ROM_t*& out_ROM);
		static void fixChecksum(void);
		static void freeROM(ROM_t* ROM_MD);
		
		// Recent ROM struct.
		struct Recent_ROM_t
		{
			unsigned int type;	/* ROM type. */
			std::string filename;	/* ROM filename. */
			std::string z_filename;	/* Filename of the ROM inside of an archive. */
		};
		
		// Double-ended queue containing all Recent ROMs.
		static std::deque<Recent_ROM_t> Recent_ROMs;
	
	protected:
		static void updateRecentROMList(const std::string& filename,
						const std::string& z_filename,
						const unsigned int type);
		
		static void updateROMDir(const std::string& filename);
		static void updateROMName(const char* filename);
		static void deinterleaveSMD(void);
		static void fillROMInfo(void);
		static unsigned short calcChecksum(void);
};

#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

// Temporary C wrapper functions.
// TODO: Eliminate these.
unsigned int detectFormat(const unsigned char buf[2048]);

#ifdef __cplusplus
}
#endif

#endif /* GENS_ROM_HPP */

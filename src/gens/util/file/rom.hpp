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
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Make sure ROM_t is packed. */
#ifndef PACKED
#define PACKED __attribute__ ((packed))
#endif

/**
 * ROM_t: Contains a ROM header.
 * NOTE: Strings are NOT null-terminated!
 */
typedef struct PACKED _ROM_t
{
	char Console_Name[16];
	char Copyright[16];
	char ROM_Name_JP[48];	// Japanese ROM name.
	char ROM_Name_US[48];	// US/Europe ROM name.
	char Serial_Number[14];
	uint16_t CheckSum;
	char IO_Support[16];
	uint32_t ROM_Start_Address;
	uint32_t ROM_End_Address;
	uint32_t RAM_Start_Address;
	uint32_t RAM_End_Address;
	char SRAM_Info[4];
	uint32_t SRAM_Start_Address;
	uint32_t SRAM_End_Address;
	char Modem_Info[12];
	char Notes[40];
	char Country_Codes[16];
} ROM_t;

extern char Rom_Dir[GENS_PATH_MAX];

extern ROM_t* Game;
extern char ROM_Filename[512];

// Current byteswap state.
// A '1' in any bit indicates that the ROM is byteswapped.
// This does not necessarily mean it has actually been byteswapped;
// instead, it means that the byteswap macro has been executed.
// MD_ROM refers to Rom_Data; this is also used by SegaCD firmware.
#define ROM_BYTESWAPPED_MD_ROM		(1 << 0)
#define ROM_BYTESWAPPED_MD_TMSS		(1 << 1)
#define ROM_BYTESWAPPED_32X_ROM		(1 << 2)
#define ROM_BYTESWAPPED_32X_FW_68K	(1 << 3)
#define ROM_BYTESWAPPED_32X_FW_MSH2	(1 << 4)
#define ROM_BYTESWAPPED_32X_FW_SSH2	(1 << 5)
extern int ROM_ByteSwap_State;

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
		static void updateCDROMName(const unsigned char *cdromHeader, bool overseas);
		static void fillROMInfo(ROM_t *rom);
		
		static int getROM(void);
		static int openROM(const std::string& filename, std::string z_filename = "");
		static ROM_t* loadSegaCD_BIOS(const std::string& filename);
		
		static unsigned int detectFormat(const unsigned char buf[2048]);
		
		static unsigned int loadROM(const std::string& filename,
					    std::string& z_filename,
					    ROM_t*& out_ROM);
		static void fixChecksum(void);
		static void freeROM(ROM_t* ROM_MD);
		
		static std::string getRomName(ROM_t *rom, bool overseas);
		
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
		static unsigned short calcChecksum(void);
	
	private:
		// Don't allow instantiation of this class.
		ROM() { }
		~ROM() { }
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

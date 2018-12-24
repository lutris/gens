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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Message logging.
#include "macros/log_msg.h"

// C includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

// C++ includes
#include <string>
#include <list>
#include <deque>
using std::string;
using std::list;
using std::deque;

#include "rom.hpp"

#include "emulator/g_main.hpp"
#include "emulator/gens.hpp"
#include "emulator/g_md.hpp"
#include "emulator/g_mcd.hpp"
#include "emulator/g_32x.hpp"
#include "emulator/options.hpp"
#include "debugger/debugger.hpp"

#include "gens_core/cpu/68k/cpu_68k.h"
#include "segacd/cd_sys.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "util/file/save.hpp"

#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

// libgsft includes.
#include "libgsft/gsft_byteswap.h"
#include "libgsft/gsft_file.h"
#include "libgsft/gsft_szprintf.h"
#include "libgsft/gsft_strlcpy.h"

#include "ui/gens_ui.hpp"
#include "zip_select/zipsel_dialog.h"
#include "gens/gens_window.h"

// File Decompressors.
#include "util/file/decompressor/decompressor.h"
#include "util/file/decompressor/dummy.h"
#ifdef GENS_ZLIB
	#include "util/file/decompressor/md_gzip.h"
	#include "util/file/decompressor/md_zip.h"
#endif
#ifdef GENS_LZMA
	#include "util/file/decompressor/md_7z.h"
#endif
#include "util/file/decompressor/md_rar_t.h"

#include "mdp/mdp_constants.h"
#include "plugins/eventmgr.hpp"

// Audio Handler.
#include "audio/audio.h"

// File management functions.
#include "file.hpp"

// Debugger.
#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

// MDP includes.
#include "mdp/mdp_error.h"

#ifdef HAVE_ICONV
// String conversion.
#include "charset/iconv_string.hpp"
#endif


char Rom_Dir[GENS_PATH_MAX];

ROM_t* Game = NULL;

// ROM Filename.
// MD/32X: This is the ROM's filename without extensions.
// SegaCD: This is the CD-ROM's internal name (in the SegaCD header).
char ROM_Filename[512];

// Current byteswap state.
// A '1' in any bit indicates that the ROM is byteswapped.
// This does not necessarily mean it has actually been byteswapped;
// instead, it means that the byteswap macro has been executed.
int ROM_ByteSwap_State;

// Double-ended queue containing all Recent ROMs.
deque<ROM::Recent_ROM_t> ROM::Recent_ROMs;


/**
 * updateRecentROMList(): Update the Recent ROM list with the given ROM filename.
 * @param filename Full pathname to a ROM file.
 * @param z_filename ROM filename from inside of an archive.
 * @param type ROM type.
 */
void ROM::updateRecentROMList(const string& filename,
			      const string& z_filename,
			      const unsigned int type)
{
	for (deque<Recent_ROM_t>::iterator rom = Recent_ROMs.begin();
	     rom != Recent_ROMs.end(); rom++)
	{
		// Check if the ROM exists in the Recent ROM list.
		// If it does, update its ROM type and return.
		// TODO: If it does, move it up to position 1.
		
		if ((*rom).filename == filename &&
		    (*rom).z_filename == z_filename)
		{
			// ROM exists.
			(*rom).type = type;
			return;
		}
	}
	
	// ROM doesn't exist.
	Recent_ROM_t newROM;
	newROM.type = type;
	newROM.filename = filename;
	newROM.z_filename = z_filename;
	Recent_ROMs.push_front(newROM);
	
	// Make sure there's no more than 9 elements in the deque.
	if (Recent_ROMs.size() > 9)
		Recent_ROMs.resize(9);
}


/**
 * Update_Rom_Dir(): Update the Rom_Dir using the path of the specified ROM file.
 * @param filename Full pathname to a ROM file.
 */
void ROM::updateROMDir(const string& filename)
{
	string tmpROMDir = File::GetDirFromPath(filename);
	strlcpy(Rom_Dir, tmpROMDir.c_str(), sizeof(Rom_Dir));
}


// FIXME: This function is poorly written.
void ROM::updateROMName(const char* filename)
{
	int length = strlen(filename) - 1;
	
	while ((length >= 0) && (filename[length] != GSFT_DIR_SEP_CHR))
		length--;
	
	length++;
	
	int i = 0;
	while ((filename[length]) && (filename[length] != '.'))
		ROM_Filename[i++] = filename[length++];
	
	ROM_Filename[i] = 0;
}


/**
 * updateCDROMName(): Update the name of a SegaCD game.
 * @param cdromHeader SegaCD header.
 * @param overseas If true, use the US/Europe name; otherwise, use the Japan name.
 */
void ROM::updateCDROMName(const unsigned char *cdromHeader, bool overseas)
{
	// Copy the CD-ROM name to ROM_Filename.
	// TODO: Do this whenever getRomName() would be used.
	if (!overseas)
	{
		// Japan.
		memcpy(ROM_Filename, &cdromHeader[0x20], 48);
	}
	else
	{
		// US/Europe.
		memcpy(ROM_Filename, &cdromHeader[0x50], 48);
	}
	
	// Check for invalid characters.
	bool validName = false;
	for (int i = 47; i >= 0; i--)
	{
		if (!isspace(ROM_Filename[i]))
		{
			// Valid character.
			validName = true;
		}
		else
		{
			// Invalid character. Replace it with a space.
			ROM_Filename[i] = ' ';
		}
	}
	
	if (!validName)
	{
		// CD-ROM name is invalid. Assume that no disc is inserted.
		ROM_Filename[0] = 0x00;
		return;
	}
	
	// Make sure the name is null-terminated.
	ROM_Filename[48] = 0x00;
	
	// Trim the CD-ROM name.
	int i;
	for (i = 47; i >= 0; i--)
	{
		if (!isspace(ROM_Filename[i]))
			break;
	}
	ROM_Filename[i + 1] = 0;
	
#ifdef HAVE_ICONV
	// If overseas is false (Japan), convert from Shift-JIS to UTF-8, if necessary.
	if (!overseas)
	{
		// Attempt to convert the ROM name.
		string romNameJP = gens_iconv(ROM_Filename, sizeof(ROM_Filename), "SHIFT-JIS", "");
		if (!romNameJP.empty())
		{
			// The ROM name was converted successfully.
			strlcpy(ROM_Filename, romNameJP.c_str(), sizeof(ROM_Filename));
		}
	}
#endif
}

// Temporary C wrapper functions.
// TODO: Eliminate this.
unsigned int detectFormat(const unsigned char buf[2048])
{
	return ROM::detectFormat(buf);
}


/**
 * Detect_Format(): Detect the format of a given ROM header.
 * @param buf Buffer containing the first 2048 bytes of the ROM file.
 * @return ROM type.
 */
unsigned int ROM::detectFormat(const unsigned char buf[2048])
{
	bool interleaved = false;
	
	// SegaCD check
	if (!strncasecmp("SEGADISCSYSTEM", (char*)(&buf[0x00]), 14))
	{
		// SegaCD image, ISO9660 format.
		return ROMTYPE_SYS_MCD;
	}
	else if (!strncasecmp("SEGADISCSYSTEM", (char*)(&buf[0x10]), 14))
	{
		// SegaCD image, BIN/CUE format.
		// TODO: Proper BIN/CUE audio support, if it's not done already.
		return ROMTYPE_SYS_MCD | ROMTYPE_FLAG_CD_BIN_CUE;
	}
	
	// Check if this is an interleaved ROM.
	if (strncasecmp("SEGA", (char*)(&buf[0x100]), 4))
	{
		// No "SEGA" text in the header. This might be an interleaved ROM.
		if (((buf[0x08] == 0xAA) && (buf[0x09] == 0xBB) && (buf[0x0A] == 0x06)) ||
		    (!strncasecmp((char*)(&buf[0x280]), "EA", 2)))
		{
			// Interleaved.
			interleaved = true;
		}
	}
	
	// Check if this is a 32X ROM.
	if (interleaved)
	{
		// Interleaved 32X check.
		if (buf[0x0300] == 0xF9)
		{
			if ((!strncasecmp((char*)&buf[0x0282], "3X", 2)) ||
			    (!strncasecmp((char*)&buf[0x0407], "AS", 2)))
			{
				// Interleaved 32X ROM.
				return ROMTYPE_SYS_32X | ROMTYPE_FLAG_INTERLEAVED;
			}
		}
	}
	else
	{
		// Non-interleaved 32X check.
		if (buf[0x0200] == 0x4E)
		{
			if ((!strncasecmp((char*)(&buf[0x0105]), "32X", 3)) ||
			    (!strncasecmp((char*)(&buf[0x040E]), "MARS", 4)))
			{
				// Non-interleaved 32X ROM.
				return ROMTYPE_SYS_32X;
			}
		}
	}
	
	// Assuming this is a Genesis ROM.
	return (ROMTYPE_SYS_MD | (interleaved ? ROMTYPE_FLAG_INTERLEAVED : 0));
}


/**
 * deinterleaveSMD(): Deinterleaves an SMD-format ROM.
 */
void ROM::deinterleaveSMD(void)
{
	unsigned char buf[0x4000];
	unsigned char *Src;
	int i, j, Nb_Blocks, ptr;
	
	// SMD interleave format has a 512-byte header at the beginning of the ROM.
	// After the header, the ROM is broken into 16 KB chunks.
	// The first 8 KB of each 16 KB block are the odd bytes.
	// The second 8 KB of each 16 KB block are the even bytes.
	
	// Start at 0x200 bytes (after the SMD header).
	Src = &Rom_Data[0x200];
	
	// Subtract the SMD header length from the ROM size.
	Rom_Size -= 0x200;
	
	// Determine how many 16 KB blocks are in the ROM.
	Nb_Blocks = Rom_Size / 0x4000;
	
	// Deinterleave the blocks.
	for (ptr = 0, i = 0; i < Nb_Blocks; i++, ptr += 0x4000)
	{
		// Copy the current 16 KB block to a temporary buffer.
		memcpy(buf, &Src[ptr], 0x4000);
		
		// Go through both 8 KB sub-blocks at the same time.
		for (j = 0; j < 0x2000; j++)
		{
			// Odd byte; first 8 KB.
			Rom_Data[ptr + (j << 1) + 1] = buf[j];
			// Even byte; second 8 KB.
			Rom_Data[ptr + (j << 1)] = buf[j + 0x2000];
		}
	}
}


/**
 * fillROMInfo(): Fill in game information from the ROM header.
 * @param rom Pointer to ROM_t to fill in.
 */
void ROM::fillROMInfo(ROM_t *rom)
{
	if (!rom)
		return;
	
	// Load the ROM header.
	memcpy(rom, &Rom_Data[0x100], sizeof(*rom));
	
	// TODO: Make sure this is correct.
	if (ROM_ByteSwap_State & ROM_BYTESWAPPED_MD_ROM)
	{
		// ROM is byteswapped. Unswap the header.
		be16_to_cpu_array(rom, sizeof(*rom));
	}
	
	// Byteswap the values in the header.
	rom->CheckSum		= be16_to_cpu(rom->CheckSum);
	rom->ROM_Start_Address	= be32_to_cpu(rom->ROM_Start_Address);
	rom->ROM_End_Address	= be32_to_cpu(rom->ROM_End_Address);
	rom->RAM_Start_Address	= be32_to_cpu(rom->RAM_Start_Address);
	rom->RAM_End_Address	= be32_to_cpu(rom->RAM_End_Address);
	rom->SRAM_Start_Address	= be32_to_cpu(rom->SRAM_Start_Address);
	rom->SRAM_End_Address	= be32_to_cpu(rom->SRAM_End_Address);
	
	if (memcmp(rom->Serial_Number, "\107\115\040\060\060\060\060\061\060\065\061\055\060\060", sizeof(rom->Serial_Number)) == 0)
		ice = 1;
}


/**
 * getROM(): Show the OpenFile dialog and load the selected ROM file.
 * @return Error code from Open_Rom().
 */
int ROM::getROM(void)
{
	string filename;
	
	filename = GensUI::openFile("Open ROM", Rom_Dir, ROMFile);
	if (filename.length() == 0)
	{
		// No ROM selected.
		return 0;
	}
	
	// Open the ROM.
	return openROM(filename);
}


/**
 * openROM(): Open the specified ROM file.
 * @param filename Filename of the ROM file.
 * @param z_filename Filename of the compressed file inside of the ROM to open.
 * @return Unknown.
 */
int ROM::openROM(const string& filename, string z_filename)
{
	int romType;
	
	/*
	Free_Rom(Game);
	sys = Detect_Format(Name);
	
	if (sys < 1)
		return -1;
	*/
	
	// Close any loaded ROM first.
	freeROM(Game);
	congratulations = 0;
	
	romType = loadROM(filename, z_filename, Game);
	if (romType <= 0)
		return -1;
	
	updateRecentROMList(filename, z_filename, romType);
	updateROMDir(filename);
	
	int started, sysID;
	switch (romType & ROMTYPE_SYS_MASK)
	{
		default:
		case ROMTYPE_SYS_MD:
			if (Game)
				Genesis_Started = Init_Genesis(Game);
			
			started = Genesis_Started;
			sysID = MDP_SYSTEM_MD;
			break;
	
		case ROMTYPE_SYS_32X:
			if (Game)
				_32X_Started = Init_32X(Game);
			
			started = _32X_Started;
			sysID = MDP_SYSTEM_32X;
			break;
		
		case ROMTYPE_SYS_MCD:
			SegaCD_Started = Init_SegaCD(filename.c_str());
			
			started = SegaCD_Started;
			sysID = MDP_SYSTEM_MCD;
			break;
		
		case ROMTYPE_SYS_MCD32X:
			// Not supported right now.
			return -1;
			break;
	}
	
	// Update the window title.
	Options::setGameName();
	
	// Raise the MDP_EVENT_OPEN_ROM event.
	// TODO: Get rid of the file extension for event.rom_z_name.
	mdp_event_open_rom_t event;
	event.rom_name = ROM_Filename;
	event.rom_z_name = (z_filename.empty() ? NULL : z_filename.c_str());
	event.system_id = sysID;
	EventMgr::RaiseEvent(MDP_EVENT_OPEN_ROM, &event);
	return started;
}


/**
 * loadSegaCD_BIOS(): Load a SegaCD BIOS ROM image.
 * @param filename Filename of the SegaCD BIOS ROM image.
 * @return Pointer to Rom struct with the ROM information.
 */
ROM_t* ROM::loadSegaCD_BIOS(const string& filename)
{
	FILE *f;
	
	// This basically just checks if the BIOS ROM image can be opened.
	// TODO: Show an error message if it can't be opened.
	if ((f = fopen(filename.c_str(), "rb")) == 0)
		return 0;
	fclose(f);
	
	// Close any ROM that's currently running.
	freeROM(Game);
	
	// Load the SegaCD BIOS ROM image.
	string z_filename;
	loadROM(filename, z_filename, Game);
	return Game;
}


/**
 * loadROM(): Load a ROM file.
 * @param filename	[in]  Filename of the ROM file.
 * @param out_ROM	[out] ROM_t output.
 * @param z_filename	[in/out] Filename of the ROM selected from inside a compressed archive.
 * @return ROM type.
 */
unsigned int ROM::loadROM(const string& filename,
			  string& z_filename,
			  ROM_t*& out_ROM)
{
	// Array of decompressors.
	static const decompressor_t* const decompressors[] =
	{
		#ifdef GENS_ZLIB
			&decompressor_gzip,
			&decompressor_zip,
		#endif
		#ifdef GENS_LZMA
			&decompressor_7z,
		#endif
		&decompressor_rar,
		
		// Last decompressor is the Dummy decompressor.
		&decompressor_dummy,
		NULL
	};
	
	const decompressor_t *cmp = NULL;
	unsigned int romType;
	
	// Reset the ROM byteswap state.
	ROM_ByteSwap_State = 0;
	
	// Open the file.
	FILE *fROM = fopen(filename.c_str(), "rb");
	if (!fROM)
	{
		// Error opening the file.
		char msg[512];
		szprintf(msg, sizeof(msg), "The file '%s' could not be opened.",
			 File::GetNameFromPath(filename.c_str()).c_str());
		
#if defined(_WIN32) && (!defined(UNICODE) && !defined(_UNICODE))
		if (filename.find('?') != string::npos)
		{
			// A '?' was found in the filename.
			// On Win32, this usually means that the specified file
			// had symbols not representable in the local code page,
			// and this version of Gens/GS was compiled without Unicode support.
			strlcat(msg, "\n\nThe selected file has Unicode symbols in its filename;\n"
				"however, this version of Gens/GS does not have support\n"
				"for Unicode strings.", sizeof(msg));
		}
#endif
		
		GensUI::msgBox(msg, "Cannot Open File", GensUI::MSGBOX_ICON_ERROR);
		
		Game = NULL;
		out_ROM = NULL;
		return ROMTYPE_SYS_NONE;
	}
	
	// Attempt to find a usable decompressor.
	int zID = 0;
	while (decompressors[zID])
	{
		if (decompressors[zID]->detect_format(fROM))
		{
			// Found a usable decompressor.
			cmp = decompressors[zID];
			break;
		}
		
		// Next decompressor.
		zID++;
	}
	
	if (!cmp)
	{
		// No usable decompressors found.
		// This is a critical error, since the "dummy" decompressor should always be usable.
		LOG_MSG(gens, LOG_MSG_LEVEL_CRITICAL,
			"No usable decompressors found.");
		fclose(fROM);
		Game = NULL;
		out_ROM = NULL;
		return ROMTYPE_SYS_NONE;
	}
	
	// Selected file.
	mdp_z_entry_t *sel_file = NULL;
	
	// Get the file information.
	mdp_z_entry_t *z_list = NULL;
	int rval = cmp->get_file_info(fROM, filename.c_str(), &z_list);
	
	// Check how many files are available.
	if (rval != MDP_ERR_OK || !z_list)
	{
		// An error occurred while trying to open the file.
		switch (rval)
		{
			case -MDP_ERR_Z_ARCHIVE_NOT_FOUND:
				// Archive wasn't found.
				GensUI::msgBox("The selected file was not found.",
						"File Not Found", GensUI::MSGBOX_ICON_ERROR);
				break;
			
			case -MDP_ERR_Z_EXE_NOT_FOUND:
				// External binary wasn't found.
				GensUI::msgBox("The selected file requires an external decompressor program;\n"
						"however, the decompressor program was not found.\n\n"
						"Please verify the pathname to the decompressor program in\n"
						"Options, BIOS/Misc Files.",
						"External Program Required", GensUI::MSGBOX_ICON_ERROR);
				break;
			
			case -MDP_ERR_Z_ARCHIVE_NOT_SUPPORTED:
				// Archive not supported.
				GensUI::msgBox("The specified file is compressed using an unsupported format.",
						"Unsupported Compression Format", GensUI::MSGBOX_ICON_ERROR);
				break;
			
			case -MDP_ERR_Z_CANT_OPEN_ARCHIVE:
				// Can't open archive.
				GensUI::msgBox("The specified file could not be opened. It may be corrupted.",
						"Cannot Open File", GensUI::MSGBOX_ICON_ERROR);
				break;
			
			case -MDP_ERR_Z_NO_FILES_IN_ARCHIVE:
				// No files in archive.
				GensUI::msgBox("No files were detected in this compressed archive.",
						"No Files Detected", GensUI::MSGBOX_ICON_WARNING);
				break;
			
			default:
			{
				// Unknown error.
				char err_code[16];
				szprintf(err_code, sizeof(err_code), "0x%08X", rval);
				
				GensUI::msgBox("An unknown error occurred while attempting to open this file.\n"
						"Please report this as a bug to the Gens/GS developers.\n\n"
						"Error Code: " + string(err_code),
						"Unknown Error", GensUI::MSGBOX_ICON_ERROR);
				break;
			}
		}
		
		if (z_list)
			z_entry_t_free(z_list);
		
		fclose(fROM);
		Game = NULL;
		out_ROM = NULL;
		return ROMTYPE_SYS_NONE;
	}
	else if (!z_list->next)
	{
		// One file is in the archive. Load it.
		sel_file = z_list;
	}
	else
	{
		// More than one file is in the archive. Load it.
		
		if (!z_filename.empty())
		{
			// Compressed filename provided. Check if it exists inside of the archive.
			mdp_z_entry_t *cur = z_list;
			while (cur)
			{
#ifdef GENS_OS_WIN32
				if (!strcasecmp(z_filename.c_str(), cur->filename))
#else
				if (!strcmp(z_filename.c_str(), cur->filename))
#endif
				{
					// Found the compressed file.
					sel_file = cur;
					break;
				}
				
				// Check the next file.
				cur = cur->next;
			}
		}
		
		if (!sel_file)
		{
			// No file specified, or the specified file wasn't found.
			
			// Check if there's only a single ROM file in the archive.
			// This is done by checking file extensions, which isn't as reliable
			// as checking file contents. However, checking file contents would
			// take too much time for some types of archives, e.g. 7z.
			static const char* const ext[] = {".bin", ".smd", ".gen", ".32x", NULL};
			
			mdp_z_entry_t *cur = z_list;
			while (cur)
			{
				// Check if the current entry matches any of the file extensions.
				for (int i = 0; i < 4; i++)
				{
					size_t ext_len = strlen(ext[i]);
					size_t filename_len = strlen(cur->filename);
					if (ext_len > filename_len)
						continue;
					
					if (strcasecmp(&cur->filename[filename_len - ext_len], ext[i]) == 0)
					{
						// Extension matches.
						if (!sel_file)
						{
							// A ROM hasn't been found yet.
							sel_file = cur;
						}
						else
						{
							// A ROM has already been found.
							// Multiple ROMs exist in this archive.
							sel_file = NULL;
							cur = NULL;
						}
						break;
					}
				}
				
				// Next ROM.
				if (cur)
					cur = cur->next;
			}
			
			if (!sel_file)
			{
				// Either no ROMs matched the file extensions,
				// or more than one ROM did.
				
				// Show the Zip File Selection Dialog.
				// TODO: Improve this!
				sel_file = zipsel_dialog_get_file(z_list);
			}
		}
	}
	
	if (!sel_file)
	{
		// No file was selected and/or Cancel was clicked.
		z_entry_t_free(z_list);
		fclose(fROM);
		Game = NULL;
		out_ROM = NULL;
		return ROMTYPE_SYS_NONE;
	}
	
	// Change the mouse pointer to "busy".
	GensUI::setMousePointer(true);
	
	// Determine the ROM type.
	// TODO: Show an error if the ROM can't be opened.
	unsigned char detectBuf[2048];
	cmp->get_file(fROM, filename.c_str(), sel_file, detectBuf, sizeof(detectBuf));
	romType = detectFormat(detectBuf);
	unsigned int romSys = romType & ROMTYPE_SYS_MASK;
	if (romSys == ROMTYPE_SYS_NONE || romSys >= ROMTYPE_SYS_MCD)
	{
		// Unknown ROM type, or this is a SegaCD image.
		z_entry_t_free(z_list);
		fclose(fROM);
		Game = NULL;
		out_ROM = NULL;
		GensUI::setMousePointer(false);
		return romSys;
	}
	
	// If the ROM is larger than 6MB (+512 bytes for SMD interleaving), don't load it.
	if (sel_file->filesize > ((6 * 1024 * 1024) + 512))
	{
		GensUI::msgBox("ROM files larger than 6 MB are not supported.", "ROM File Error");
		z_entry_t_free(z_list);
		fclose(fROM);
		Game = NULL;
		out_ROM = NULL;
		GensUI::setMousePointer(false);
		return ROMTYPE_SYS_NONE;
	}
	
	ROM_t *myROM = (ROM_t*)malloc(sizeof(ROM_t));
	if (!myROM)
	{
		// Memory allocation error
		z_entry_t_free(z_list);
		fclose(fROM);
		Game = NULL;
		out_ROM = NULL;
		GensUI::setMousePointer(false);
		return ROMTYPE_SYS_NONE;
	}
	
	// Clear the ROM buffer and load the ROM.
	memset(Rom_Data, 0x00, sizeof(Rom_Data));
	size_t loaded_size = cmp->get_file(fROM, filename.c_str(), sel_file, Rom_Data, sel_file->filesize);
	if (loaded_size != sel_file->filesize)
	{
		// Incorrect filesize.
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"Incorrect filesize. Got %d; expected %d.",
			loaded_size, sel_file->filesize);
		
		GensUI::msgBox("Error loading the ROM file.", "ROM File Error");
		z_entry_t_free(z_list);
		fclose(fROM);
		free(myROM);
		myROM = NULL;
		Game = NULL;
		out_ROM = NULL;
		GensUI::setMousePointer(false);
		return ROMTYPE_SYS_NONE;
	}
	
	// Close the ROM file.
	fclose(fROM);
	
	// TODO: Should the compressed filename be included here?
	updateROMName(filename.c_str());
	Rom_Size = sel_file->filesize;
	if (Rom_Size < sizeof(Rom_Data) && (Rom_Size & 1))
	{
		// Odd ROM length. Increment the size and NULL the last byte.
		Rom_Data[Rom_Size++] = 0x00;
	}
	
	// Set the compressed ROM filename.
	if (z_list && z_list->next)
	{
		// Multi-file archive.
		z_filename = string(sel_file->filename);
	}
	else
	{
		// Single-file archive.
		z_filename.clear();
	}
	
	// Delete the mdp_z_entry_t.
	z_entry_t_free(z_list);
	
	// Deinterleave the ROM, if necessary.
	if (romType & ROMTYPE_FLAG_INTERLEAVED)
		deinterleaveSMD();
	
	fillROMInfo(myROM);
	
	GensUI::setMousePointer(false);
	out_ROM = myROM;
	return romType;
}


/**
 * calcChecksum(): Calculates the checksum of the loaded ROM.
 * @return Checksum of the loaded ROM.
 */
unsigned short ROM::calcChecksum(void)
{
	uint16_t checksum = 0;
	
	// A game needs to be loaded in order for this function to work...
	if (!Game)
		return 0;
	
	// Checksum starts at 0x200, past the vector table and ROM header.
	uint8_t *Rom_Ptr = &Rom_Data[0x200];
	if (ROM_ByteSwap_State & ROM_BYTESWAPPED_MD_ROM)
	{
		// ROM is byteswapped. (little-endian)
		for (unsigned int i = (Rom_Size - 0x200); i != 0; i--)
		{
			checksum += (uint16_t)(*Rom_Ptr) |
				    ((uint16_t)(*(Rom_Ptr + 1)) << 8);
			Rom_Ptr += 2;
		}
	}
	else
	{
		// ROM is not byteswapped. (big-endian)
		for (unsigned int i = (Rom_Size - 0x200); i != 0; i--)
		{
			checksum += ((uint16_t)(*Rom_Ptr) << 8) |
				    (uint16_t)(*(Rom_Ptr + 1));
			Rom_Ptr += 2;
		}
	}
	
	return checksum;
}


/**
 * fixChecksum(): Fixes the checksum of the loaded ROM.
 */
void ROM::fixChecksum(void)
{
	if (!Game || !Rom_Size)
		return;
	
	// Get the checksum.
	uint16_t checksum = calcChecksum();
	
	// MC68000 checksum.
	// MC68000 is big-endian.
	if (ROM_ByteSwap_State & ROM_BYTESWAPPED_MD_ROM)
	{
		// ROM is byteswapped. (little-endian)
		Rom_Data[0x18E] = (checksum & 0xFF);
		Rom_Data[0x18F] = (checksum >> 8);
	}
	else
	{
		// ROM is not byteswapped. (big-endian)
		Rom_Data[0x18E] = (checksum >> 8);
		Rom_Data[0x18F] = (checksum & 0xFF);
	}
	
	// SH2 checksum.
	// SH2 is little-endian.
	// TODO: Only do this if the 32X is active.
	if (ROM_ByteSwap_State & ROM_BYTESWAPPED_32X_ROM)
	{
		// ROM is byteswapped. (big-endian)
		_32X_Rom[0x18E] = (checksum >> 8);
		_32X_Rom[0x18F] = (checksum & 0xFF);
	}
	else
	{
		// ROM is not byteswapped. (little-endian)
		_32X_Rom[0x18E] = (checksum & 0xFF);
		_32X_Rom[0x18F] = (checksum >> 8);
	}
}


void ROM::freeROM(ROM_t* ROM_MD)
{
	if (!Game)
		return;
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	// Stop the debugger.
	STOP_DEBUGGING();
	
	if (SegaCD_Started)
		Savestate::SaveBRAM();
	
	Savestate::SaveSRAM();
	
	// Audio dumping.
	if (WAV_Dumping)
		wav_dump_stop();
	if (GYM_Dumping)
		gym_dump_stop();

	if (SegaCD_Started)
		Stop_CD();
	
#if 0	// TODO: Replace with MDP "exclusive mode" later.
	Net_Play = 0;
#endif
	Genesis_Started = 0;
	_32X_Started = 0;
	SegaCD_Started = 0;
	
	// Reset the ROM byteswap state.
	ROM_ByteSwap_State = 0;
	
	// Clear the border color palette entries.
	MD_Palette[0] = 0;
	MD_Palette32[0] = 0;
	
	Game = NULL;
	ice = 0;
	congratulations = 0;
	
	if (ROM_MD)
	{
		free(ROM_MD);
		ROM_MD = NULL;
	}
	
	if (Intro_Style == 3)
		Init_Genesis_Bios();
	
	// Raise the MDP_EVENT_CLOSE_ROM event.
	EventMgr::RaiseEvent(MDP_EVENT_CLOSE_ROM, NULL);
	
	GensUI::setWindowTitle_Idle();
}


/**
 * getRomName(): Get the ROM name from the specified ROM.
 * @param rom ROM_t to check.
 * @param overseas If true, use the US/Europe name; otherwise, use the Japan name.
 */
string ROM::getRomName(ROM_t *rom, bool overseas)
{
	if (!rom)
		return "";
	
	// Check which ROM name should be used.
	// Default: ROM_Name_US for US/Europe, ROM_Name_JP for Japan.
	// NOTE: These strings are *not* null-terminated!
	const char *defRomName;
	const char *altRomName;
	
	if (!overseas)
	{
		// Japan.
		defRomName = rom->ROM_Name_JP;
		altRomName = rom->ROM_Name_US;
	}
	else
	{
		// US/Europe.
		defRomName = rom->ROM_Name_US;
		altRomName = rom->ROM_Name_JP;
	}
	
	// If the default ROM name is blank, use the alternate ROM name.
	const char *romNameToUse = altRomName;
	for (unsigned int cpos = (sizeof(rom->ROM_Name_US) - 1); cpos != 0; cpos--)
	{
		if (!isspace(defRomName[cpos]))
		{
			// The default ROM name isn't blank. Use it.
			romNameToUse = defRomName;
			break;
		}
	}
	
	// Convert the ROM name to a null-terminated string.
	char RomName[49];
	memcpy(RomName, romNameToUse, sizeof(rom->ROM_Name_US));
	RomName[sizeof(RomName)-1] = 0x00;
	
#ifdef HAVE_ICONV
	// If this was ROM_Name_JP, convert from Shift-JIS to UTF-8, if necessary.
	if (romNameToUse == rom->ROM_Name_JP)
	{
		// Attempt to convert the ROM name.
		string romNameJP = gens_iconv(RomName, sizeof(RomName), "SHIFT-JIS", "");
		if (!romNameJP.empty())
		{
			// The ROM name was converted successfully.
			return romNameJP;
		}
	}
#endif
	
	return string(RomName);
}

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
#include "gens_core/cpu/68k/cpu_68k.h"
#include "segacd/cd_sys.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "util/file/save.hpp"

#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

#include "gens_core/misc/misc.h"
#include "gens_core/misc/byteswap.h"

#include "ui/gens_ui.hpp"
#include "zip_select/zip_select_dialog.hpp"
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

char Rom_Dir[GENS_PATH_MAX];

ROM_t* Game = NULL;
char ROM_Name[512];

ROM_t* myROM = NULL;

// Double-ended queue containing all Recent ROMs.
deque<ROM::Recent_ROM_t> ROM::Recent_ROMs;


/**
 * updateRecentROMList(): Update the Recent ROM list with the given ROM filename.
 * @param filename Full pathname to a ROM file.
 * @param type ROM type.
 */
void ROM::updateRecentROMList(const string& filename, const unsigned int type)
{
	for (deque<Recent_ROM_t>::iterator rom = Recent_ROMs.begin();
	     rom != Recent_ROMs.end(); rom++)
	{
		// Check if the ROM exists in the Recent ROM list.
		// If it does, update its ROM type and return.
		// TODO: If it does, move it up to position 1.
		if ((*rom).filename == filename)
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
	strncpy(Rom_Dir, tmpROMDir.c_str(), sizeof(Rom_Dir));
	Rom_Dir[sizeof(Rom_Dir) - 1] = 0x00;
}


// FIXME: This function is poorly written.
void ROM::updateROMName(const char* filename)
{
	int length = strlen(filename) - 1;
	
	while ((length >= 0) && (filename[length] != GENS_DIR_SEPARATOR_CHR))
		length--;
	
	length++;
	
	int i = 0;
	while ((filename[length]) && (filename[length] != '.'))
		ROM_Name[i++] = filename[length++];
	
	ROM_Name[i] = 0;
}


/**
 * updateCDROMName(): Update the name of a SegaCD game.
 * @param cdromName Name of the SegaCD game.
 */
void ROM::updateCDROMName(const char *cdromName)
{
	int i, j;
	bool validName = false;
	
	// Copy the CD-ROM name to ROM_Name.
	// NOTE: cdromName is only 32 bytes.
	// TODO: Increase it to 48 bytes.
	memcpy(ROM_Name, cdromName, 32);
	memset(&ROM_Name[32], ' ', 16);
	
	// Check for invalid characters.
	for (i = 0; i < 48; i++)
	{
		if (isalnum(ROM_Name[i]))
		{
			// Valid character.
			validName = true;
			continue;
		}
		
		// Invalid character. Replace it with a space.
		ROM_Name[i] = ' ';
	}
	
	if (!validName)
	{
		// CD-ROM name is invalid. Assume that no disc is inserted.
		ROM_Name[0] = 0x00;
		return;
	}
	
	// Make sure the name is null-terminated.
	ROM_Name[48] = 0x00;
	for (i = 47, j = 48; i >= 0; i--, j--)
	{
		if (ROM_Name[i] != ' ')
			i = -1;
	}
	ROM_Name[j + 1] = 0;
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
 */
void ROM::fillROMInfo(void)
{
	// Copy ROM text.
	// TODO: Use constants for the ROM addresses.
	memcpy(myROM->Console_Name,	&Rom_Data[0x100], 16);
	memcpy(myROM->Copyright,	&Rom_Data[0x110], 16);
	memcpy(myROM->ROM_Name,		&Rom_Data[0x120], 48);
	memcpy(myROM->ROM_Name_W,	&Rom_Data[0x150], 48);
	memcpy(myROM->Type,		&Rom_Data[0x180], 2);
	memcpy(myROM->Version,		&Rom_Data[0x182], 12);
	myROM->Checksum			= be16_to_cpu_from_ptr(&Rom_Data[0x18E]);
	memcpy(myROM->IO_Support,	&Rom_Data[0x190], 16);
	myROM->ROM_Start_Address	= be32_to_cpu_from_ptr(&Rom_Data[0x1A0]);
	myROM->ROM_End_Address		= be32_to_cpu_from_ptr(&Rom_Data[0x1A4]);
	memcpy(myROM->RAM_Info,		&Rom_Data[0x1A8], 12);
	myROM->RAM_Start_Address	= be32_to_cpu_from_ptr(&Rom_Data[0x1B4]);
	myROM->RAM_End_Address		= be32_to_cpu_from_ptr(&Rom_Data[0x1B8]);
	memcpy(myROM->Modem_Info,	&Rom_Data[0x1BC], 12);
	memcpy(myROM->Description,	&Rom_Data[0x1C8], 40);
	memcpy(myROM->Countries,	&Rom_Data[0x1F0], 4);
	
	char tmp[14];
	memcpy(&tmp[0], myROM->Type, 2);
	memcpy(&tmp[2], myROM->Version, 12);
	if (memcmp(tmp, "\107\115\040\060\060\060\060\061\060\065\061\055\060\060", 14) == 0)
		ice = 1;
	
	// Calculate internal ROM size using the ROM header's
	// starting address and ending address.
	myROM->R_Size = myROM->ROM_End_Address - myROM->ROM_Start_Address + 1;
	
	// Null-terminate the strings.
	myROM->Console_Name[16] = 0;
	myROM->Copyright[16] = 0;
	myROM->ROM_Name[48] = 0;
	myROM->ROM_Name_W[48] = 0;
	myROM->Type[2] = 0;
	myROM->Version[12] = 0;
	myROM->IO_Support[12] = 0;
	myROM->RAM_Info[12] = 0;
	myROM->Modem_Info[12] = 0;
	myROM->Description[40] = 0;
	myROM->Countries[3] = 0;
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
 * Open_Rom(): Open the specified ROM file.
 * @param filename Filename of the ROM file.
 * @return Unknown.
 */
int ROM::openROM(const string& filename)
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
	
	romType = loadROM(filename, &Game);
	if (romType <= 0)
		return -1;
	
	updateRecentROMList(filename, romType);
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
	
	// Raise the MDP_EVENT_OPEN_ROM event.
	mdp_event_open_rom_t event;
	event.rom_name = ROM_Name;
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
	loadROM(filename, &Game);
	return Game;
}


/**
 * loadROM(): Load a ROM file.
 * @param filename Filename of the ROM file.
 * @param interleaved If non-zero, the ROM is interleaved.
 * @return ROM type.
 */
unsigned int ROM::loadROM(const string& filename, ROM_t** retROM)
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
	
	// Open the file.
	FILE *fROM = fopen(filename.c_str(), "rb");
	if (!fROM)
	{
		// Error opening the file.
		Game = NULL;
		*retROM = NULL;
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
		// This is an error, since the "dummy" decompressor should always be usable.
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"No usable decompressors found.");
		fclose(fROM);
		Game = NULL;
		*retROM = NULL;
		return ROMTYPE_SYS_NONE;
	}
	
	file_list_t *file_list, *sel_file;
	
	// Get the file information.
	file_list = cmp->get_file_info(fROM, filename.c_str());
	
	// Check how many files are available.
	if (!file_list)
	{
		// No files in the archive.
		GensUI::msgBox("No files were detected in this archive.", "No Files Detected");
		
		if (file_list)
			file_list_t_free(file_list);
		
		fclose(fROM);
		Game = NULL;
		*retROM = NULL;
		return ROMTYPE_SYS_NONE;
	}
	else if (!file_list->next)
	{
		// One file is in the archive. Load it.
		sel_file = file_list;
	}
	else
	{
		// More than one file is in the archive. Load it.
		// TODO: Improve this!
		ZipSelectDialog *zip = new ZipSelectDialog();
		sel_file = zip->getFile(file_list);
		delete zip;
	}
	
	if (!sel_file)
	{
		// No file was selected and/or Cancel was clicked.
		file_list_t_free(file_list);
		fclose(fROM);
		Game = NULL;
		*retROM = NULL;
		return ROMTYPE_SYS_NONE;
	}
	
	// Determine the ROM type.
	// TODO: Show an error if the ROM can't be opened.
	unsigned char detectBuf[2048];
	cmp->get_file(fROM, filename.c_str(), sel_file, detectBuf, sizeof(detectBuf));
	romType = detectFormat(detectBuf);
	unsigned int romSys = romType & ROMTYPE_SYS_MASK;
	if (romSys == ROMTYPE_SYS_NONE || romSys >= ROMTYPE_SYS_MCD)
	{
		// Unknown ROM type, or this is a SegaCD image.
		file_list_t_free(file_list);
		fclose(fROM);
		Game = NULL;
		*retROM = NULL;
		return romSys;
	}
	
	// If the ROM is larger than 6MB (+512 bytes for SMD interleaving), don't load it.
	if (sel_file->filesize > ((6 * 1024 * 1024) + 512))
	{
		GensUI::msgBox("ROM files larger than 6 MB are not supported.", "ROM File Error");
		file_list_t_free(file_list);
		fclose(fROM);
		Game = NULL;
		*retROM = NULL;
		return ROMTYPE_SYS_NONE;
	}
	
	myROM = (ROM_t*)malloc(sizeof(ROM_t));
	if (!myROM)
	{
		// Memory allocation error
		file_list_t_free(file_list);
		fclose(fROM);
		Game = NULL;
		*retROM = NULL;
		return ROMTYPE_SYS_NONE;
	}
	
	// Clear the ROM buffer and load the ROM.
	memset(Rom_Data, 0, 6 * 1024 * 1024);
	size_t loaded_size = cmp->get_file(fROM, filename.c_str(), sel_file, Rom_Data, sel_file->filesize);
	if (loaded_size != sel_file->filesize)
	{
		// Incorrect filesize.
		LOG_MSG(gens, LOG_MSG_LEVEL_ERROR,
			"Incorrect filesize. Got %d; expected %d.",
			loaded_size, sel_file->filesize);
		
		GensUI::msgBox("Error loading the ROM file.", "ROM File Error");
		file_list_t_free(file_list);
		fclose(fROM);
		free(myROM);
		myROM = NULL;
		Game = NULL;
		*retROM = NULL;
		return ROMTYPE_SYS_NONE;
	}
	
	// Close the ROM file.
	fclose(fROM);
	
	updateROMName(filename.c_str());
	Rom_Size = sel_file->filesize;
	
	// Delete the file_list_t.
	file_list_t_free(file_list);
	
	// Deinterleave the ROM, if necessary.
	if (romType & ROMTYPE_FLAG_INTERLEAVED)
		deinterleaveSMD();
	
	fillROMInfo();
	
	*retROM = myROM;
	return romType;
}


/**
 * calcChecksum(): Calculates the checksum of the loaded ROM.
 * @return Checksum of the loaded ROM.
 */
unsigned short ROM::calcChecksum(void)
{
	unsigned short checksum = 0;
	unsigned int i;
	
	// A game needs to be loaded in order for this function to work...
	if (!Game)
		return 0;
	
	// Checksum starts at 0x200, past the vector table and ROM header.
	for (i = 0x200; i < Rom_Size; i += 2)
	{
		// Remember, since the MC68000 is little-endian, we can't
		// just cast Rom_Data[i] to an unsigned short directly.
		checksum += (unsigned short)(Rom_Data[i + 0]) +
			    (unsigned short)(Rom_Data[i + 1] << 8);
	}
	
	return checksum;
}


/**
 * fixChecksum(): Fixes the checksum of the loaded ROM.
 */
void ROM::fixChecksum(void)
{
	unsigned short checks;
	
	if (!Game)
		return;
	
	// Get the checksum.
	checks = calcChecksum();
	
	if (Rom_Size)
	{
		// MC68000 checksum.
		// MC68000 is big-endian.
		Rom_Data[0x18E] = checks & 0xFF;
		Rom_Data[0x18F] = checks >> 8;
		
		// SH2 checksum.
		// SH2 is little-endian.
		// TODO: Only do this if the 32X is active.
		_32X_Rom[0x18E] = checks >> 8;;
		_32X_Rom[0x18F] = checks & 0xFF;
	}
}


void ROM::freeROM(ROM_t* ROM_MD)
{
	if (!Game)
		return;
	
	// Clear the sound buffer.
	audio_clear_sound_buffer();
	
	if (SegaCD_Started)
		Savestate::SaveBRAM();
	
	Savestate::SaveSRAM();
	
	// TODO: Send "Close ROM" event to registered MDP event handlers.
	//Save_Patch_File();
	
	// Audio dumping.
	if (WAV_Dumping)
		wav_dump_stop();
	if (GYM_Dumping)
		Stop_GYM_Dump();

	if (SegaCD_Started)
		Stop_CD();
	
	Net_Play = 0;
	Genesis_Started = 0;
	_32X_Started = 0;
	SegaCD_Started = 0;
	
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

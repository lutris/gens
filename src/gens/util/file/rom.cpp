#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <fcntl.h>

#include <string>
#include <list>
using std::string;
using std::list;

#include "rom.hpp"

#include "emulator/g_main.hpp"
#include "emulator/gens.hpp"
#include "emulator/g_md.hpp"
#include "emulator/g_mcd.hpp"
#include "emulator/g_32x.hpp"
#include "util/file/ggenie.h"
#include "gens_core/cpu/68k/cpu_68k.h"
#include "segacd/cd_sys.hpp"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/vdp/vdp_io.h"
#include "util/file/save.hpp"
#include "util/sound/wave.h"
#include "util/sound/gym.hpp"

#include "gens_core/misc/misc.h"
#include "gens_core/misc/byteswap.h"

#include "ui/gens_ui.hpp"
#include "zip_select/zip_select_dialog_misc.hpp"

// New file compression handler.
#include "util/file/compress/compressor.hpp"

Rom *My_Rom = NULL;
struct Rom *Game = NULL;
char Rom_Name[512];
char Rom_Dir[GENS_PATH_MAX];
char IPS_Dir[GENS_PATH_MAX];
char Recent_Rom[9][GENS_PATH_MAX];


void
Get_Name_From_Path (char *Full_Path, char *Name)
{
  int i = 0;

  i = strlen (Full_Path) - 1;

  while ((i >= 0) && (Full_Path[i] != GENS_DIR_SEPARATOR_CHR))
    i--;

  if (i <= 0)
    {
      Name[0] = 0;
    }
  else
    {
      strcpy (Name, &Full_Path[++i]);
    }
}


/**
 * Get_Dir_From_Path(): Get the directory part of a pathname.
 * @param Full_Path Full pathname.
 * @param Dir Buffer to store the directory part.
 */
void Get_Dir_From_Path(const char *Full_Path, char *Dir)
{
	int i = strlen(Full_Path) - 1;
	
	while ((i >= 0) && (Full_Path[i] != GENS_DIR_SEPARATOR_CHR))
		i--;
	
	if (i <= 0)
	{
		// No directory found.
		Dir[0] = 0;
	}
	else
	{
		// Directory found. Copy it to the output buffer.
		strncpy(Dir, Full_Path, ++i);
		Dir[i] = 0;
	}
}


/**
 * Update_Recent_Rom(): Update the Recent ROM list with the given ROM filename.
 * @param Path Full pathname to a ROM file.
 */
void Update_Recent_Rom(const char *Path)
{
	int i;
	
	for (i = 0; i < 9; i++)
	{
		// Check if the ROM exists in the Recent ROM list.
		// If it does, don't do anything.
		// TODO: If it does, move it up to position 1.
		if (!(strcmp(Recent_Rom[i], Path)))
			return;
	}
	
	// Move all recent ROMs down by one index.
	for (i = 8; i > 0; i--)
		strcpy(Recent_Rom[i], Recent_Rom[i - 1]);
	
	// Add this ROM to the recent ROM list.
	strcpy(Recent_Rom[0], Path);
}


/**
 * Update_Rom_Dir(): Update the Rom_Dir using the path of the specified ROM file.
 * @param Path Full pathname to a ROM file.
 */
void Update_Rom_Dir(const char *Path)
{
	Get_Dir_From_Path(Path, Rom_Dir);
}


static void Update_Rom_Name(const char *Name)
{
	int i, leng;
	
	leng = strlen (Name) - 1;
	
	while ((leng >= 0) && (Name[leng] != GENS_DIR_SEPARATOR_CHR))
		leng--;
	
	leng++;
	i = 0;
	
	while ((Name[leng]) && (Name[leng] != '.'))
		Rom_Name[i++] = Name[leng++];
	
	Rom_Name[i] = 0;
}


void Update_CD_Rom_Name(char *Name)
{
	int i, j;
	
	memcpy(Rom_Name, Name, 48);
	
	for (i = 0; i < 48; i++)
	{
		if ((Rom_Name[i] >= '0') && (Rom_Name[i] <= '9'))
			continue;
		if (Rom_Name[i] == ' ')
			continue;
		if ((Rom_Name[i] >= 'A') && (Rom_Name[i] <= 'Z'))
			continue;
		if ((Rom_Name[i] >= 'a') && (Rom_Name[i] <= 'z'))
			continue;
		Rom_Name[i] = ' ';
	}
	
	for (i = 0; i < 48; i++)
	{
		if (Rom_Name[i] != ' ')
			i = 100;
	}
	
	if (i < 100)
		strcpy(Rom_Name, "no name");
	
	for (i = 47, j = 48; i >= 0; i--, j--)
	{
		if (Rom_Name[i] != ' ')
			i = -1;
	}
	
	Rom_Name[j + 1] = 0;
}


/**
 * Detect_Format(): Detect the format of a given ROM header.
 * @param buf Buffer containing the first 1024 bytes of the ROM file.
 * @return ROMType.
 */
ROMType detectFormat(const unsigned char *buf)
{
	int i = 0;
	
	// SegaCD check
	if (!strncasecmp("SEGADISCSYSTEM", (char*)(&buf[0x00]), 14))
	{
		// SegaCD image, ISO9660 format.
		return SegaCD_Image;
	}
	else if (!strncasecmp("SEGADISCSYSTEM", (char*)(&buf[0x10]), 14))
	{
		// SegaCD image, BIN/CUE format.
		// TODO: Proper BIN/CUE audio support, if it's not done already.
		return SegaCD_Image_BIN;
	}
	
	if (strncasecmp("SEGA", (char*)(&buf[0x100]), 4))
	{
		// No "SEGA" text in the header. This might be an interleaved ROM.
		// Maybe interleaved
		
		if (!strncasecmp("EA", (char*)(&buf[0x200 + (0x100 / 2)]), 2) ||
		    ((buf[0x08] == 0xAA) && (buf[0x09] == 0xBB) && (buf[0x0A] == 0x06)))
		{
			// Interleaved.
			i = 1;
		}
	}
	
	// Check if this is a 32X ROM.
	if (i)
	{
		// Interleaved 32X check.
		if ((buf[0x200 / 2] == 0x4E) &&
		    (!strncasecmp("3X", (char*)(&buf[0x200 + (0x105 / 2)]), 2)))
		{
			// Interleaved 32X ROM.
			return _32X_ROM_Interleaved;
		}
	}
	else
	{
		// Non-interleaved 32X check.
		if ((buf[0x200] == 0x4E) &&
		    (!strncasecmp("32X", (char*)(&buf[0x105]), 3)))
		{
			// Non-interleaved 32X ROM.
			return _32X_ROM;
		}
	}
	
	// Assuming this is a Genesis ROM.
	// If it is interleaved, i == 1.
	// Otherwise, i == 0.
	if (i)
		return MD_ROM_Interleaved;
	
	return MD_ROM;
}


/**
 * detectFormat_fopen(): Detect the format of a given ROM file.
 * @param filename Filename of the ROM file.
 * @return ROMType.
 */
ROMType detectFormat_fopen(const char* filename)
{
	Compressor *cmp;
	list<CompressedFile> *files;
	unsigned char buf[1024];
	ROMType rtype;
	
	// Open the ROM file using the compressor functions.
	cmp = new Compressor(filename);
	if (!cmp->isFileLoaded())
	{
		// Cannot load the file.
		delete cmp;
		return (ROMType)0;
	}
	
	// Get the file information.
	files = cmp->getFileInfo();
	
	// Check how many files are available.
	if (!files || files->empty())
	{
		// No files.
		delete files;
		delete cmp;
		return (ROMType)0;
	}
	
	// Get the first file in the archive.
	// TODO: Store the compressed filename in ROM history.
	cmp->getFile(&(*files->begin()), buf, 1024);
	rtype = detectFormat(buf);
	
	// Return the ROM type.
	delete files;
	delete cmp;
	return rtype;
}


/**
 * Deinterleave_SMD(): Deinterleaves an SMD-format ROM.
 */
static void Deinterleave_SMD(void)
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
			// Odd byte, first 8 KB
			Rom_Data[ptr + (j << 1) + 1] = buf[j];
			// Even byte, second 8 KB
			Rom_Data[ptr + (j << 1)] = buf[j + 0x2000];
		}
	}
}


/**
 * Fill_Infos(): Fill in game information from the ROM header.
 */
void Fill_Infos(void)
{
	// Finally we do the IPS patch here, we can have the translated game name
	IPS_Patching ();
	
	// Copy ROM text.
	// TODO: Use constants for the ROM addresses.
	memcpy(My_Rom->Console_Name,	&Rom_Data[0x100], 16);
	memcpy(My_Rom->Copyright,	&Rom_Data[0x110], 16);
	memcpy(My_Rom->Rom_Name,	&Rom_Data[0x120], 48);
	memcpy(My_Rom->Rom_Name_W,	&Rom_Data[0x150], 48);
	memcpy(My_Rom->Type,		&Rom_Data[0x180], 2);
	memcpy(My_Rom->Version,		&Rom_Data[0x182], 12);
	My_Rom->Checksum		= be16_to_cpu_from_ptr(&Rom_Data[0x18E]);
	memcpy(My_Rom->IO_Support,	&Rom_Data[0x190], 16);
	My_Rom->Rom_Start_Adress	= be32_to_cpu_from_ptr(&Rom_Data[0x1A0]);
	My_Rom->Rom_End_Adress		= be32_to_cpu_from_ptr(&Rom_Data[0x1A4]);
	memcpy(My_Rom->Ram_Infos,	&Rom_Data[0x1A8], 12);
	My_Rom->Ram_Start_Adress	= be32_to_cpu_from_ptr(&Rom_Data[0x1B4]);
	My_Rom->Ram_End_Adress		= be32_to_cpu_from_ptr(&Rom_Data[0x1B8]);
	memcpy(My_Rom->Modem_Infos,	&Rom_Data[0x1BC], 12);
	memcpy(My_Rom->Description,	&Rom_Data[0x1C8], 40);
	memcpy(My_Rom->Countries,	&Rom_Data[0x1F0], 4);
	
	char tmp[15];
	memcpy(&tmp[0], My_Rom->Type, 2);
	memcpy(&tmp[2], My_Rom->Version, 12);
	tmp[14] = 0;
	if (strcmp(tmp, "\107\115\040\060\060\060\060\061\060\065\061\055\060\060") == 0)
		ice = 1;
	
	// Calculate internal ROM size using the ROM header's
	// starting address and ending address.
	My_Rom->R_Size = My_Rom->Rom_End_Adress - My_Rom->Rom_Start_Adress + 1;
	
	// Null-terminate the strings.
	My_Rom->Console_Name[16] = 0;
	My_Rom->Copyright[16] = 0;
	My_Rom->Rom_Name[48] = 0;
	My_Rom->Rom_Name_W[48] = 0;
	My_Rom->Type[2] = 0;
	My_Rom->Version[12] = 0;
	My_Rom->IO_Support[12] = 0;
	My_Rom->Ram_Infos[12] = 0;
	My_Rom->Modem_Infos[12] = 0;
	My_Rom->Description[40] = 0;
	My_Rom->Countries[3] = 0;
}


/**
 * Get_Rom(): Show the OpenFile dialog and load the selected ROM file.
 * @return Error code from Open_Rom().
 */
int Get_Rom(void)
{
	string filename;
	
	filename = GensUI::openFile("Open ROM", Rom_Dir, ROMFile);
	if (filename.length() == 0)
	{
		// No ROM selected.
		return 0;
	}
	
	// Open the ROM.
	return Open_Rom(filename.c_str());
}


/**
 * Open_Rom(): Open the specified ROM file.
 * @param Name Filename of the ROM file.
 * @return Unknown.
 */
int Open_Rom(const char *Name)
{
	int sys;
	
	/*
	Free_Rom(Game);
	sys = Detect_Format(Name);
	
	if (sys < 1)
		return -1;
	*/
	
	// Close any loaded ROM first.
	Free_Rom(Game);
	
	sys = Load_ROM(Name, &Game);
	if (sys <= 0)
		return -1;
	
	Update_Recent_Rom(Name);
	Update_Rom_Dir(Name);
	
	switch (sys)
	{
		default:
		case MD_ROM:
		case MD_ROM_Interleaved:
			if (Game)
				Genesis_Started = Init_Genesis(Game);
			
			return Genesis_Started;
			break;
	
		case _32X_ROM:
		case _32X_ROM_Interleaved:
			if (Game)
				_32X_Started = Init_32X(Game);
			
			return _32X_Started;
			break;
		
		case SegaCD_Image:
		case SegaCD_Image_BIN:
			SegaCD_Started = Init_SegaCD(Name);
			
			return SegaCD_Started;
			break;
		
		case SegaCD_32X_Image:
		case SegaCD_32X_Image_BIN:
			break;
	}
	
	return -1;
}


/**
 * Load_SegaCD_BIOS(): Load a SegaCD BIOS ROM image.
 * @param filename Filename of the SegaCD BIOS ROM image.
 * @return Pointer to Rom struct with the ROM information.
 */
Rom *Load_SegaCD_BIOS(const char *filename)
{
	FILE *f;
	
	// This basically just checks if the BIOS ROM image can be opened.
	// TODO: Show an error message if it can't be opened.
	if ((f = fopen(filename, "rb")) == 0)
		return 0;
	fclose(f);
	
	// Close any ROM that's currently running.
	Free_Rom(Game);
	
	// Load the SegaCD BIOS ROM image.
	Load_ROM(filename, &Game);
	return Game;
}


/**
 * Load_ROM(): Load a ROM file.
 * @param filename Filename of the ROM file.
 * @param interleaved If non-zero, the ROM is interleaved.
 * @return Pointer to Rom struct with the ROM information.
 */
ROMType Load_ROM(const char *filename, struct Rom **retROM)
{
	Compressor *cmp;
	list<CompressedFile> *files;
	CompressedFile* selFile;
	unsigned char buf[1024];
	ROMType rtype;
	
	// Set up the compressor.
	cmp = new Compressor(filename, true);
	if (!cmp->isFileLoaded())
	{
		// Error loading the file.
		delete cmp;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	
	// Get the file information.
	files = cmp->getFileInfo();
	
	// Check how many files are available.
	if (!files || files->empty())
	{
		// No files in the archive.
		// TODO: For 7z, suggest setting the 7z binary filename.
		GensUI::msgBox("No files were detected in this archive.", "No Files Detected");
		
		if (files)
			delete files;
		if (cmp)
			delete cmp;
		
		files = NULL;
		cmp = NULL;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	else if (files->size() == 1)
	{
		// One file is in the archive. Load it.
		selFile = &(*files->begin());
	}
	else
	{
		// More than one file is in the archive. Load it.
		selFile = Open_Zip_Select_Dialog(files);
	}
	
	if (!selFile)
	{
		// No file was selected and/or Cancel was clicked.
		delete files;
		delete cmp;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	
	// Determine the ROM type.
	cmp->getFile(&(*selFile), buf, 1024);
	rtype = detectFormat(buf);
	if (rtype < MD_ROM ||
	    rtype >= SegaCD_Image)
	{
		// Unknown ROM type, or this is a SegaCD image.
		delete files;
		delete cmp;
		Game = NULL;
		*retROM = NULL;
		return rtype;
	}
	
	// If the ROM is larger than 6MB (+512 bytes for SMD interleaving), don't load it.
	if (selFile->filesize > ((6 * 1024 * 1024) + 512))
	{
		GensUI::msgBox("ROM files larger than 6 MB are not supported.", "ROM File Error");
		delete files;
		delete cmp;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	
	My_Rom = (Rom*)malloc(sizeof(Rom));
	if (!My_Rom)
	{
		// Memory allocation error
		delete files;
		delete cmp;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	//fseek(ROM_File, 0, SEEK_SET);
	
	// Clear the ROM buffer and load the ROM.
	memset(Rom_Data, 0, 6 * 1024 * 1024);
	int loadedSize = cmp->getFile(&(*selFile), Rom_Data, selFile->filesize);
	if (loadedSize != selFile->filesize)
	{
		// Incorrect filesize.
		GensUI::msgBox("Error loading the ROM file.", "ROM File Error");
		free(My_Rom);
		delete files;
		delete cmp;
		My_Rom = NULL;
		Game = NULL;
		*retROM = NULL;
		return (ROMType)0;
	}
	//fclose(ROM_File);
	
	Update_Rom_Name(filename);
	Rom_Size = selFile->filesize;
	
	// Delete the compression objects.
	delete files;
	delete cmp;
	
	// Deinterleave the ROM, if necessary.
	if (rtype == MD_ROM_Interleaved ||
	    rtype == _32X_ROM_Interleaved)
		Deinterleave_SMD();
	
	Fill_Infos();
	
	*retROM = My_Rom;
	return rtype;
}


/**
 * Calculate_Checksum(): Calculates the checksum of the loaded ROM.
 * @return Checksum of the loaded ROM.
 */
unsigned short Calculate_Checksum(void)
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
 * Fix_Checksum(): Fixes the checksum of the loaded ROM.
 */
void Fix_Checksum(void)
{
	unsigned short checks;
	
	if (!Game)
		return;
	
	// Get the checksum.
	checks = Calculate_Checksum ();
	
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


int
IPS_Patching (void)
{
  FILE *IPS_File;
  char Name[1024];
  unsigned char buf[16];
  unsigned int adr, len, i;

  strcpy (Name, IPS_Dir);
  strcat (Name, Rom_Name);
  strcat (Name, ".ips");

  IPS_File = fopen (Name, "rb");

  if (IPS_File == NULL)
    return 1;

  fseek (IPS_File, 0, SEEK_SET);

  fread (buf, 1, 5, IPS_File);
  buf[5] = 0;

  if (strcasecmp ((char *) buf, "patch"))
    {
      fclose (IPS_File);
      return 2;
    }

  fread (buf, 1, 3, IPS_File);
  buf[3] = 0;

  while (strcasecmp ((char *) buf, "eof"))
    {
      adr = (unsigned int) buf[2];
      adr += (unsigned int) (buf[1] << 8);
      adr += (unsigned int) (buf[0] << 16);

      if (fread (buf, 1, 2, IPS_File) == 0)
	{
	  fclose (IPS_File);
	  return 3;
	}

      len = (unsigned int) buf[1];
      len += (unsigned int) (buf[0] << 8);

      for (i = 0; i < len; i++)
	{
	  if (fread (buf, 1, 1, IPS_File) == 0)
	    {
	      fclose (IPS_File);
	      return 3;
	    }

	  if (adr < Rom_Size)
	    Rom_Data[adr++] = buf[0];
	}

      if (fread (buf, 1, 3, IPS_File) == 0)
	{
	  fclose (IPS_File);
	  return 3;
	}

      buf[3] = 0;
    }

  fclose (IPS_File);

  return 0;
}


void Free_Rom(Rom* Rom_MD)
{
	if (Game == NULL)
		return;
	
	if (SegaCD_Started)
		Save_BRAM();
	
	Save_SRAM();
	Save_Patch_File();
	
	// Audio dumping.
	if (audio->dumpingWAV())
		audio->stopWAVDump();
	if (GYM_Dumping)
		Stop_GYM_Dump();

	if (SegaCD_Started)
		Stop_CD();
	
	Net_Play = 0;
	Genesis_Started = 0;
	_32X_Started = 0;
	SegaCD_Started = 0;
	
	Game = NULL;
	ice = 0;
	
	if (Rom_MD)
	{
		free(Rom_MD);
		Rom_MD = NULL;
	}
	
	if (Intro_Style == 3)
		Init_Genesis_Bios();
	
	GensUI::setWindowTitle_Idle();
}

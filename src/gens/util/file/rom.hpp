#ifndef GENS_ROM_HPP
#define GENS_ROM_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "emulator/gens.hpp"
#include <stdlib.h>

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

extern char IPS_Dir[GENS_PATH_MAX];
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
		static std::string getNameFromPath(const std::string& fullPath);
		static std::string getDirFromPath(const std::string& fullPath);
		static void updateCDROMName(const char *cdromName);
		
		static int getROM(void);
		static int openROM(const char *Name);
		static ROM_t* loadSegaCD_BIOS(const char *filename);
		
		static unsigned int detectFormat(const unsigned char buf[2048]);
		static unsigned int detectFormat_fopen(const char* filename);
		
		static unsigned int loadROM(const char* filename, ROM_t** retROM);
		static void fixChecksum(void);
		static int applyIPSPatch(void);
		static void freeROM(ROM_t* ROM_MD);
		
		// Recent ROM struct.
		struct Recent_ROM_t
		{
			unsigned int type;
			std::string filename;
		};
		
		// Double-ended queue containing all Recent ROMs.
		static std::deque<Recent_ROM_t> Recent_ROMs;
	
	protected:
		static void updateRecentROMList(const std::string& filename, const unsigned int type);
		static void updateROMDir(const char *filename);
		static void updateROMName(const char *filename);
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

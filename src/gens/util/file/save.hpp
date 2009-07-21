#ifndef GENS_SAVE_HPP
#define GENS_SAVE_HPP

#include "emulator/gens.hpp"
#include <stdio.h>

#define GENESIS_LENGTH_EX1		0x2247C
#define GENESIS_LENGTH_EX2		0x11ED2
#define GENESIS_STATE_LENGTH		(GENESIS_LENGTH_EX1 + GENESIS_LENGTH_EX2)
#define GENESIS_V6_LENGTH_EX2		0x7EC
#define GENESIS_V6_STATE_LENGTH		(GENESIS_LENGTH_EX1 + GENESIS_V6_LENGTH_EX2)
#define SEGACD_LENGTH_EX1		0xE19A4
#define SEGACD_LENGTH_EX2		0x1238B
#define SEGACD_LENGTH_EX		(SEGACD_LENGTH_EX1 + SEGACD_LENGTH_EX2)
#define G32X_LENGTH_EX			0x849BF
#define MAX_STATE_FILE_LENGTH		(GENESIS_STATE_LENGTH + SEGACD_LENGTH_EX + G32X_LENGTH_EX)

/*
// Old GENS v5 savestate offsets.
#define GENESIS_STATE_FILE_LENGTH    0x22478
#define GENESIS_STATE_FILE_LENGTH_EX 0x25550
#define SEGACD_STATE_FILE_LENGTH     (0x22500 + 0xE0000)
#define G32X_STATE_FILE_LENGTH       (0x22500 + 0x82A00)
#define SCD32X_STATE_FILE_LENGTH     (0x22500 + 0xE0000 + 0x82A00)
#define MAX_STATE_FILE_LENGTH        SCD32X_STATE_FILE_LENGTH
*/

#ifdef __cplusplus
extern "C" {
#endif

extern char State_Dir[GENS_PATH_MAX];
extern char SRAM_Dir[GENS_PATH_MAX];
extern char BRAM_Dir[GENS_PATH_MAX];

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

// C++ includes
#include <string>

class Savestate
{
	public:
		static std::string SelectFile(const bool save = false, const std::string& dir = "");
		static std::string SelectCDImage(const std::string& dir);
		
		static FILE* GetStateFile(void);
		static std::string GetStateFilename(void);
		
		static int LoadState(const std::string& filename);
		static int SaveState(const std::string& filename);
		
		static int LoadSRAM(void);
		static int SaveSRAM(void);
		
		static void FormatSegaCD_BackupRAM(void);
		
		static int LoadBRAM(void);
		static int SaveBRAM(void);
	
	protected:
		// ImportData / ExportData functions from Gens Rerecording
		
		static void ImportData(void* into, const void* data, 
				       const unsigned int offset,
				       unsigned int numBytes);
		
		static void ExportData(const void* from, void* data,
				       const unsigned int offset,
				       unsigned int numBytes);
		
		static void ImportDataAuto(void* into, const void* data,
					   unsigned int& offset,
					   const unsigned int numBytes);
		
		static void ExportDataAuto(const void* from, void* data,
					   unsigned int& offset,
					   const unsigned int numBytes);
		
		static int GsxImportGenesis(const unsigned char* data);
		static void GsxExportGenesis(unsigned char* data);
		static void GsxImportSegaCD(const unsigned char* data);
		static void GsxExportSegaCD(unsigned char* data);
		static void GsxImport32X(const unsigned char* data);
		static void GsxExport32X(unsigned char* data);
		
		static std::string GetSRAMFilename(void);
		
		static void FormatSegaCD_BRAM(unsigned char *buf);
		
		static std::string GetBRAMFilename(void);
	
	private:
		// Don't allow instantiation of this class.
		Savestate() { }
		~Savestate() { }
};

//int Change_Dir(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext);

#endif /* __cplusplus */

#endif /* GENS_SAVE_H */

#ifndef GENS_ROM_H
#define GENS_ROM_H

#ifdef __cplusplus
extern "C" {
#endif


#include "emulator/gens.hpp"
#include "port/port.h"


typedef enum
{
	MD_ROM			= 2,
	MD_ROM_Interleaved	= 3,
	_32X_ROM		= 4,
	_32X_ROM_Interleaved	= 5,
	SegaCD_Image		= 6,
	SegaCD_Image_BIN	= 7,
	SegaCD_32X_Image	= 8,
	SegaCD_32X_Image_BIN	= 9,
} ROMType;


typedef struct Rom
{
	char Console_Name[17];
	char Copyright[17];
	char Rom_Name[49];
	char Rom_Name_W[49];
	char Type[3];
	char Version[13];
	unsigned int Checksum;
	char IO_Support[17];
	unsigned int Rom_Start_Adress;
	unsigned int Rom_End_Adress;
	unsigned int R_Size;
	char Ram_Infos[13];
	unsigned int Ram_Start_Adress;
	unsigned int Ram_End_Adress;
	char Modem_Infos[13];
	char Description[41];
	char Countries[4];
} Rom;


extern struct Rom *Game;
extern char Rom_Name[512];
extern char Recent_Rom[9][GENS_PATH_MAX];
extern char IPS_Dir[GENS_PATH_MAX];
extern char Rom_Dir[GENS_PATH_MAX];
extern char CDROM_DEV[64];

void Get_Name_From_Path(char *Full_Path, char *Name);
void Get_Dir_From_Path(const char *Full_Path, char *Dir);
void Update_CD_Rom_Name(char *Name);

int Get_Rom(void);
int Open_Rom(const char *Name);
struct Rom *Load_SegaCD_BIOS(const char *filename);
ROMType detectFormat(const unsigned char *buf);
ROMType detectFormat_fopen(const char* filename);
ROMType Load_ROM(const char *filename, struct Rom **retROM);
void Fix_Checksum(void);
int IPS_Patching();
void Free_Rom(struct Rom *Rom_Name);

#ifdef __cplusplus
}
#endif

#endif /* GENS_ROM_H */
